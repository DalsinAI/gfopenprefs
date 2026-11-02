/* idlepace.h: a desktop commodity's tick, slowed while nobody uses the machine (Repo review 1 Nov 2026, gfopenprefs
 * OLD 2; Dale, 1 Nov 2026: "yes"). GFOpenMenus' bar polled every 100 ms and GFOpenSpeaker every 200 ms whether anyone was
 * there or not, so an idle desktop woke 10-20 times a second. With this, after a quiet spell (2 s) each runs at its
 * idle interval (GFOpenMenus 500 ms, GFOpenSpeaker 1 s), and goes back to its full speed on the first input event.
 *
 * Who calls what:
 *   - the commodity's input handler (CxCustom, input.device's task): idle_pace_input() for each key, button, move or
 *     wheel event it sees; when it returns 1 the task is slow, and the handler Signal()s it to come back at once;
 *   - the task, when its timer request comes back (or it AbortIO()s a slow one on that signal): idle_pace_tick() with
 *     the time that went by, which says how long to ask for next. Under Forbid(): the input handler is a task too,
 *     so it can't come between reading the input mark and setting slow.
 *
 * Plain C with no Amiga headers, so Common/tests builds it on the host as it is.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef IDLEPACE_H
#define IDLEPACE_H

#define IDLE_PACE_AFTER_US 2000000UL        /* this long with no input: the idle interval */

struct idle_pace {
    unsigned long busy_us, idle_us;         /* the two intervals the task asks its timer for */
    unsigned long quiet_us;                 /* time since the last input, as the ticks counted it (stops at the limit) */
    volatile unsigned char input;           /* set by the input handler, taken by the next tick */
    volatile unsigned char slow;            /* the timer is out for idle_us */
};

static inline void idle_pace_init(struct idle_pace *p, unsigned long busy_us, unsigned long idle_us)
{
    p->busy_us = busy_us;
    p->idle_us = idle_us;
    p->quiet_us = 0;
    p->input = 0;
    p->slow = 0;
}

/* An input event a person made: a key, a button or a move (IECLASS_RAWKEY 0x01, RAWMOUSE 0x02, POINTERPOS 0x04,
 * NEWPOINTERPOS 0x13, a tablet's too) or a wheel (NewMouse's IECLASS_NEWMOUSE 0x16). Not input.device's own
 * IECLASS_TIMER (0x06, ten a second), nor what Intuition says back (windows, disks, prefs). */
static inline int idle_pace_is_input(unsigned char ie_class)
{
    return ie_class == 0x01 || ie_class == 0x02 || ie_class == 0x04 || ie_class == 0x13 || ie_class == 0x16;
}

/* The input handler saw input. 1: the task is on the idle interval; wake it so it can go back to full speed now. */
static inline int idle_pace_input(struct idle_pace *p)
{
    p->input = 1;
    return p->slow;
}

/* A tick: elapsed_us went by since the last one (0 for a slow request cut short). The interval to ask for next. */
static inline unsigned long idle_pace_tick(struct idle_pace *p, unsigned long elapsed_us)
{
    if (p->input) {
        p->input = 0;
        p->quiet_us = 0;
    } else if (p->quiet_us < IDLE_PACE_AFTER_US) {
        /* no wrap: stops at the limit */
        p->quiet_us = elapsed_us >= IDLE_PACE_AFTER_US - p->quiet_us ? IDLE_PACE_AFTER_US : p->quiet_us + elapsed_us;
    }
    p->slow = p->quiet_us >= IDLE_PACE_AFTER_US;
    return p->slow ? p->idle_us : p->busy_us;
}

#endif
