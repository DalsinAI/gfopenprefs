/* Common/idlepace.h: GFOpenMenus' and GFOpenSpeaker's tick, slowed after 2 s with no input and back to full speed on
 * the first input. A model of the tasks' loops (a timer, the input handler's wake signal, AbortIO of a slow request)
 * run on simulated time. Prints "ok"/"FAIL" lines, then PASS or FAIL.
 * Copyright (c) 2026 AmigaChrome Team. MIT. */
#include <stdio.h>
#include <string.h>
#include "idlepace.h"

static int fails;

static void check(const char *what, int ok)
{
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) fails++;
}

/* The task's loop over [0, end_us), with input events at inputs[] (sorted, in us). Returns the number of times the
 * task woke for its timer; *late_us the longest wait from an input to the next tick; *slow_at when it first went slow. */
struct run { long wakes; unsigned long late_us, slow_at; unsigned long first_after[8]; };

static struct run simulate(unsigned long busy, unsigned long idle, unsigned long end_us, const unsigned long *inputs, int n)
{
    struct idle_pace p;
    struct run r;
    unsigned long now = 0, asked, due;
    int i = 0;
    memset(&r, 0, sizeof r);
    idle_pace_init(&p, busy, idle);
    asked = busy;
    due = now + asked;
    while (due < end_us) {
        /* the next thing to happen: the timer, or an input event */
        if (i < n && inputs[i] < due) {
            now = inputs[i];
            if (idle_pace_input(&p)) {
                /* the handler's Signal: the task AbortIO()s the slow request and ticks at once */
                unsigned long next = idle_pace_tick(&p, 0);
                r.wakes++;
                if (i < 8) r.first_after[i] = 0;
                asked = next;
                due = now + asked;
            } else if (i < 8) {
                r.first_after[i] = due - now;
            }
            if (due - now > r.late_us) r.late_us = due - now;
            i++;
            continue;
        }
        now = due;
        r.wakes++;
        asked = idle_pace_tick(&p, asked);
        if (p.slow && !r.slow_at) r.slow_at = now;
        due = now + asked;
    }
    return r;
}

int main(void)
{
    struct idle_pace p;
    struct run before, after;
    unsigned long t;
    int k;
    static const unsigned long one[] = { 30000000UL };
    unsigned long busy_minute[600];

    /* a minute with nobody there: before (one interval) and after (the idle one after 2 s) */
    before = simulate(100000, 100000, 60000000UL, NULL, 0);
    after = simulate(100000, 500000, 60000000UL, NULL, 0);
    printf("     GFOpenMenus, idle minute: %ld wakes before, %ld after\n", before.wakes, after.wakes);
    check("GFOpenMenus: an idle minute wakes 600 times at 100 ms (before)", before.wakes == 599);
    check("GFOpenMenus: idle after 2 s, then every 500 ms: about 135 wakes", after.wakes >= 130 && after.wakes <= 140);
    check("GFOpenMenus: slow from 2 s with no input", after.slow_at == 2000000UL);
    before = simulate(200000, 200000, 60000000UL, NULL, 0);
    after = simulate(200000, 1000000, 60000000UL, NULL, 0);
    printf("     GFOpenSpeaker, idle minute: %ld wakes before, %ld after\n", before.wakes, after.wakes);
    check("GFOpenSpeaker: an idle minute wakes 300 times at 200 ms (before)", before.wakes == 299);
    check("GFOpenSpeaker: idle after 2 s, then every 1 s: about 68 wakes", after.wakes >= 65 && after.wakes <= 70);

    /* input while slow: the next tick comes at once, and the one after it at full speed */
    after = simulate(200000, 1000000, 40000000UL, one, 1);
    check("first input while slow: ticks at once (no wait for the 1 s request)", after.first_after[0] == 0);
    check("no input waits longer than the busy interval for its tick", after.late_us <= 200000UL);
    idle_pace_init(&p, 200000, 1000000);
    for (t = 0, k = 0; k < 10; k++) t = idle_pace_tick(&p, 200000);
    check("idle: asks for the idle interval", t == 1000000UL && p.slow);
    check("input while slow: the handler is told to wake the task", idle_pace_input(&p) == 1);
    check("the tick after input: busy interval, not slow", idle_pace_tick(&p, 0) == 200000UL && !p.slow);
    check("input while busy: no wake", idle_pace_input(&p) == 0);
    /* that input is taken by the next tick: quiet from there */
    check("busy until 2 s after the last input", idle_pace_tick(&p, 200000) == 200000UL &&
          idle_pace_tick(&p, 1800000) == 200000UL && idle_pace_tick(&p, 200000) == 1000000UL);

    /* someone working the whole minute (an event every 100 ms): never slow, every tick at full speed */
    for (k = 0; k < 600; k++) busy_minute[k] = (unsigned long)k * 100000UL + 50000UL;
    after = simulate(100000, 500000, 60000000UL, busy_minute, 600);
    check("in use: never slow, so no tick later than before", after.slow_at == 0 && after.late_us <= 100000UL);

    /* time counted without wrapping (32-bit unsigned long on the 68k) */
    idle_pace_init(&p, 100000, 500000);
    p.quiet_us = IDLE_PACE_AFTER_US - 1;
    idle_pace_tick(&p, 0xFFFFFFFFUL);
    check("a huge elapsed time stops at the limit", p.quiet_us == IDLE_PACE_AFTER_US && p.slow);
    idle_pace_tick(&p, 0xFFFFFFFFUL);
    check("and stays there", p.quiet_us == IDLE_PACE_AFTER_US && p.slow);
    idle_pace_init(&p, 100000, 500000);
    idle_pace_tick(&p, 1999999UL);
    check("1 us short of 2 s: still busy", !p.slow);
    idle_pace_tick(&p, 1UL);
    check("at 2 s: slow", p.slow);

    /* which events count */
    check("keys, buttons, moves, wheels and tablets are input",
          idle_pace_is_input(0x01) && idle_pace_is_input(0x02) && idle_pace_is_input(0x04) && idle_pace_is_input(0x13) &&
          idle_pace_is_input(0x16));
    check("input.device's timer and Intuition's own events are not",
          !idle_pace_is_input(0x00) && !idle_pace_is_input(0x06) && !idle_pace_is_input(0x03) && !idle_pace_is_input(0x0E) &&
          !idle_pace_is_input(0x10) && !idle_pace_is_input(0x11));

    printf("%s\n", fails ? "FAIL" : "PASS");
    return fails != 0;
}
