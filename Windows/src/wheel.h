/* GFOpenWindows' wheel (0.11, 4 November 2026). Dale: "when the scrollable
 * window or area is selected by a mouse click or the scroll bars have been
 * pushed once, the middle mouse wheel ... should work to scroll up and down".
 *
 * input.device sends each wheel step as the NewMouse standard has it
 * (IECLASS_NEWMOUSE and IECLASS_RAWKEY NM_WHEEL_UP, DOWN, LEFT, RIGHT, $7A-$7D,
 * pressed and let go). A program that knows the wheel scrolls itself; for one
 * that doesn't, GFOpenWindows moves its scroller as a drag of the knob would,
 * and the program hears what a drag sends it:
 *
 *   - the scroll focus: in the active window, the scroller the user last
 *     pushed, or the one beside the place the user last clicked in; until
 *     then the window's main one (its right border's, its bottom border's
 *     across). With "wheel.target pointer", the one beside the pointer in the
 *     window under it.
 *   - Shift with the wheel, or the horizontal wheel, moves the scroller across.
 *   - a window's program is asked once, at its first wheel step: the step goes
 *     to it as it came, and if its scroller moved by itself the program knows
 *     the wheel (MUI's, the Team's own) and GFOpenWindows leaves it alone from
 *     then on; if not, GFOpenWindows moves the scroller, then and after. A
 *     program named on a "wheel.self" line is never moved.
 *
 * This file is the pure part (no Amiga calls): Windows/tests/test_wheel.c
 * runs it on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OPENWINDOWS_WHEEL_H
#define OPENWINDOWS_WHEEL_H

#define WS_VERT     1                       /* a box that scrolls down (a prop's FREEVERT) */
#define WS_HORIZ    2                       /* ... across (FREEHORIZ) */
#define WS_BORDER   4                       /* in the window's right border (down) or bottom border (across) */

struct wheel_box { int l, t, w, h; unsigned flags; };

/* The scroller a click at (x, y) (the window's own pixels) picks for dir
 * (WS_VERT or WS_HORIZ), or -1:
 *   1. a scroller the click is on (its knob or its container: the bar pushed);
 *   2. one whose arrows it is on: within WHEEL_ARROWS pixels past either end,
 *      in line with it;
 *   3. the nearest one beside the place clicked: down, the nearest to its
 *      right whose height takes in the click; across, the nearest below it
 *      whose width does. */
#define WHEEL_ARROWS 28
int wheel_pick(const struct wheel_box *b, int n, int x, int y, unsigned dir);

/* The window's main scroller for dir: one in its border, else the longest; -1 none */
int wheel_main(const struct wheel_box *b, int n, unsigned dir);

/* A wheel event's step: its sign (+1 down or right, toward the end; -1 up or
 * left) for a raw key pressed (not let go), and *dir: WS_HORIZ for the
 * horizontal wheel or Shift with the vertical one. 0 for any other code. */
#define WHEEL_SHIFT 0x0003                  /* IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT */
int wheel_step(unsigned code, unsigned qual, unsigned *dir);

/* A plain prop's new pot for lines steps (signed: + toward the end). The
 * program's line isn't known, so a line is taken as line_px of the view_px
 * shown: the visible lines V = view_px / line_px, the whole T = V * MAXBODY /
 * body, and a line MAXPOT / (T - V) of the pot (at least 1). Clamped to the
 * pot's range. body 0xFFFF (all shown): the pot as it is. */
unsigned wheel_pot(unsigned pot, unsigned body, int view_px, int line_px, int lines);

/* A scroller with a top, a total and a visible (propgclass's PGA_Top,
 * PGA_Total, PGA_Visible; GadTools' GTSC_*): the new top, in 0 .. total - visible. */
long wheel_top(long top, long total, long visible, long lines);

/* A top's units a line: a scroller may count lines (a list's items) or pixels (OpenFiles' lists, a picture): its
 * visible over the lines its view_px shows (view_px / line_px), rounded, at least 1 */
long wheel_units(long visible, int view_px, int line_px);

/* A window's program, as its first step tells (WC_*): moved, its scroller
 * moved by itself after the step went to it; could, it was not at its end
 * that way, so a program that knows the wheel would have moved it. */
enum { WC_UNKNOWN = 0, WC_SELF, WC_EMULATE, WC_NONE };
int wheel_probe(int moved, int could);

#endif
