/* GFOpenWindows' wheel (src/wheel.c) on the host: which scroller a click
 * makes the scroll focus, the window's main one, a step's direction, and the
 * scroller's new place.
 *   Windows/tests/run.sh
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "../src/wheel.h"

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

int main(void)
{
    /* an ASL-like window, 400 x 300: a file list (0..360 across, 20..220 down) with its scroller at 362..378
     * (its arrows below it, 220..252), a second list lower down with its own scroller, the right border's
     * scroller of a drawer-like window and a bottom border one across */
    struct wheel_box b[] = {
        { 362, 20, 16, 200, WS_VERT },                 /* 0 the file list's scroller */
        { 362, 260, 16, 30, WS_VERT },                 /* 1 a second, short list's */
        { 384, 12, 14, 270, WS_VERT | WS_BORDER },     /* 2 the right border's */
        { 4, 288, 360, 10, WS_HORIZ | WS_BORDER },     /* 3 the bottom border's, across */
        { 10, 240, 200, 12, WS_HORIZ },                /* 4 a slider across, inside */
    };
    int n = sizeof b / sizeof b[0];
    unsigned dir = 0;

    CHECK(wheel_pick(b, n, 370, 100, WS_VERT) == 0, "a click on the list's scroller: that one");
    CHECK(wheel_pick(b, n, 370, 230, WS_VERT) == 0, "on its arrows below it: that one");
    CHECK(wheel_pick(b, n, 390, 100, WS_VERT) == 2, "on the right border's: that one");
    CHECK(wheel_pick(b, n, 100, 100, WS_VERT) == 0, "in the list: the scroller to its right, not the border's further on");
    CHECK(wheel_pick(b, n, 100, 270, WS_VERT) == 1, "in the second list: its own scroller");
    CHECK(wheel_pick(b, n, 100, 5, WS_VERT) == -1, "above every scroller: none (the focus stays as it was)");
    CHECK(wheel_pick(b, n, 100, 100, WS_HORIZ) == 4, "across from the list: the slider below it");
    CHECK(wheel_pick(b, n, 100, 292, WS_HORIZ) == 3, "on the bottom border's: that one");
    CHECK(wheel_pick(b, n, 225, 245, WS_HORIZ) == 4, "just past the slider's right end, in line: its arrows, so that one");
    CHECK(wheel_pick(b, 0, 10, 10, WS_VERT) == -1, "no scrollers: none");
    CHECK(wheel_main(b, n, WS_VERT) == 2, "the window's main scroller down: its right border's, though the list's is in front");
    CHECK(wheel_main(b, 2, WS_VERT) == 0, "with no border scroller: the tallest");
    CHECK(wheel_main(b, n, WS_HORIZ) == 3, "across: the bottom border's");
    CHECK(wheel_main(b, 2, WS_HORIZ) == -1, "no scroller across: none");

    CHECK(wheel_step(0x7A, 0, &dir) == -1 && dir == WS_VERT, "NM_WHEEL_UP: one up");
    CHECK(wheel_step(0x7B, 0, &dir) == 1 && dir == WS_VERT, "NM_WHEEL_DOWN: one down");
    CHECK(wheel_step(0x7B, 0x0001, &dir) == 1 && dir == WS_HORIZ, "Shift with the wheel down: one right");
    CHECK(wheel_step(0x7A, 0x0002, &dir) == -1 && dir == WS_HORIZ, "right Shift with the wheel up: one left");
    CHECK(wheel_step(0x7C, 0, &dir) == -1 && dir == WS_HORIZ, "NM_WHEEL_LEFT: one left");
    CHECK(wheel_step(0x7D, 0, &dir) == 1 && dir == WS_HORIZ, "NM_WHEEL_RIGHT: one right");
    CHECK(wheel_step(0xFB, 0, &dir) == 0, "the let-go: no step");
    CHECK(wheel_step(0x45, 0, &dir) == 0, "Esc: no step");

    /* a list of 100 lines with 10 shown, in a 80-pixel view of 8-pixel lines: body 1/10, a line 1/90 of the pot */
    {
        unsigned body = 0xFFFF / 10, p = wheel_pot(0, body, 80, 8, 3);
        CHECK(p >= 3 * 0xFFFF / 90 && p <= 3 * 0xFFFF / 90 + 3, "3 lines down from the top: 3/90 of the pot (%u)", p);
        CHECK(wheel_pot(p, body, 80, 8, -3) == 0, "and 3 back up: the top again");
        CHECK(wheel_pot(0xFFF0, body, 80, 8, 3) == 0xFFFF, "near the end: to the end, no further");
        CHECK(wheel_pot(10, body, 80, 8, -3) == 0, "near the top: to the top");
        CHECK(wheel_pot(1234, 0xFFFF, 80, 8, 3) == 1234, "all of it shown: nothing to scroll");
        CHECK(wheel_pot(0, 0xFFFF - 1, 80, 8, 1) >= 1, "almost all shown: at least one step");
        CHECK(wheel_pot(500, body, 0, 0, 1) > 500, "a view of no height and no font: still a step");
    }
    CHECK(wheel_top(0, 100, 10, 3) == 3, "top 0 of 100, 10 shown, 3 down: 3");
    CHECK(wheel_top(88, 100, 10, 3) == 90, "near the end: 90, the last top");
    CHECK(wheel_top(2, 100, 10, -3) == 0, "near the top, up: 0");
    CHECK(wheel_top(0, 5, 10, 3) == 0, "all shown: 0");

    CHECK(wheel_units(20, 180, 8) == 1, "a list of items, 20 shown in 22 lines' height: a line is one");
    CHECK(wheel_units(342, 342, 13) == 13, "a list counted in pixels (OpenFiles): a line is the font's height, 13");
    CHECK(wheel_units(10, 0, 8) == 1 && wheel_units(0, 100, 8) == 1, "no height, or nothing shown: one");
    CHECK(wheel_probe(1, 1) == WC_SELF, "it moved by itself: the program knows the wheel");
    CHECK(wheel_probe(0, 1) == WC_EMULATE, "it could have moved and didn't: GFOpenWindows moves it");
    CHECK(wheel_probe(0, 0) == WC_UNKNOWN, "at its end that way: not known yet, ask again next time");

    printf("test_wheel: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
