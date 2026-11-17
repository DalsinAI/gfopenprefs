/* od_strip: the screen along the dock's whole row (or column), so the dock can
 * grow and shrink where it stands without closing to copy what is behind it.
 * Pure sums, tested on the host (tests/test_od_dock.c).
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OD_STRIP_H
#define OD_STRIP_H

/* The strip for a dock at x, y (w by h) on a screen sw by sh: the dock's row
 * across the whole screen, or for a standing dock its column down the whole
 * screen. */
static inline void od_strip_box(int standing, int sw, int sh, int x, int y, int w, int h,
                                int *bx, int *by, int *bw, int *bh)
{
    if (standing) { *bx = x; *by = 0; *bw = w; *bh = sh; }
    else { *bx = 0; *by = y; *bw = sw; *bh = h; }
}

/* 1 when a strip at sx, sy (sw by sh) has the picture behind a dock at x, y
 * (w by h): the same thickness in the same place across it, and the dock's
 * length inside it. A dock whose thickness changed (the icons' size, the
 * labels) or that moved to another edge needs a new copy. */
static inline int od_strip_holds(int standing, int sx, int sy, int sw, int sh, int x, int y, int w, int h)
{
    if (sw <= 0 || sh <= 0 || w <= 0 || h <= 0) return 0;
    if (standing) return x == sx && w == sw && y >= sy && y + h <= sy + sh;
    return y == sy && h == sh && x >= sx && x + w <= sx + sw;
}

#endif
