/* barstrip.h: where on the screen's bar a program reads the bar's own colour (barpaint.h's bp_sample).
 *
 * The strip has to be on bare bar. A bar window of ours (GFOpenTitle's row, its clock, GFOpenSpeaker's speaker) read
 * back paints the bar in its own colours: once the row was drawn in the pen's white, every later reading found white,
 * and the row stayed white with dark text (Dale, 16 November 2026, 08:37: "sort out the colour problems" the systray
 * has; the drag shot with no screen behind showed the same white row). So the strip starts where it wants to be and
 * steps left of every bar window that covers it. Plain C, host-tested (TitleBar/tests/test_tb.c).
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef BARSTRIP_H
#define BARSTRIP_H

typedef struct { int x0, x1; } bs_box;         /* a bar window's columns on the screen, x0 to x1 inclusive */

/* The left edge of a strip w wide, at want or left of it, that no box covers, inside 0..width-1; -1 when there is
 * none. Each step puts the strip left of a box, so a box is passed once: n + 1 passes are enough. */
static __attribute__((unused)) int bs_strip_x(int want, int w, int width, const bs_box *b, int n)
{
    int x = want, pass, i, moved;
    if (w <= 0 || w > width) return -1;
    if (x + w > width) x = width - w;
    for (pass = 0; pass <= n; pass++) {
        moved = 0;
        for (i = 0; i < n; i++)
            if (b[i].x0 <= x + w - 1 && b[i].x1 >= x) { x = b[i].x0 - w; moved = 1; }
        if (x < 0) return -1;
        if (!moved) return x;
    }
    return -1;
}

#endif
