/* The GFOS mark, the Deep cradle (Dale, 14 November 2026: arc #0b5ed7, open
 * about 124 degrees at the top, holding a dot #ff7a1a), drawn at any height as
 * runs of pixels, so GFOpenBlanker draws it with RectFill in two pens that fade
 * with the rest of the card. The same shape as gfopenprefs
 * TitleBar/tools/cradle_mark.py: a band the stroke wide round a circle of
 * radius r (the stroke 0.13 of the height), round ends, the dot resting low in
 * the arc. The ends' tops touch row 0 and the band's bottom the last row. Each pixel is
 * in or out by its centre. Host-tested (tests/mark_test.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef BL_MARK_H
#define BL_MARK_H

enum { BLM_ARC = 1, BLM_DOT = 2 };

/* the arc's ends are 28 degrees above the circle's centre: sin 28 = cos 62 */
#define BLM_UP 0.46947

typedef struct { double r, s, cx, cy, dr, dy; int w, h; } bl_mark_geo;

static bl_mark_geo bl_mark_geometry(int h)
{
    bl_mark_geo g;
    g.h = h;
    g.s = h * 0.13;
    g.r = (h - g.s) / (1.0 + BLM_UP);           /* the band's middle; its ends' tops at 0, its bottom at h */
    g.w = (int)(2.0 * g.r + g.s + 0.999);
    g.cx = g.w / 2.0;
    g.cy = g.s / 2.0 + g.r * BLM_UP;
    g.dr = (g.r - g.s / 2.0) * 0.46;
    g.dy = g.cy + (g.r - g.s / 2.0) - g.dr - g.s * 0.7;
    return g;
}

/* what the pixel at x, y is: 0, BLM_ARC or BLM_DOT */
static int bl_mark_at(const bl_mark_geo *g, int x, int y)
{
    double px = x + 0.5 - g->cx, py = y + 0.5 - g->cy, d2 = px * px + py * py;
    double ro = g->r + g->s / 2.0, ri = g->r - g->s / 2.0, ex, ey, er = g->s / 2.0, mid = g->r;
    /* the dot */
    double qx = px, qy = y + 0.5 - g->dy;
    if (qx * qx + qy * qy <= g->dr * g->dr) return BLM_DOT;
    /* the band, less the gap at the top: within 62 degrees of straight up */
    if (d2 <= ro * ro && d2 >= ri * ri && !(py < 0 && py * py > BLM_UP * BLM_UP * d2)) return BLM_ARC;
    /* the round ends, at 62 degrees either side of up */
    ex = mid * 0.88295;            /* sin 62 */
    ey = -mid * BLM_UP;            /* cos 62, upwards */
    if ((px - ex) * (px - ex) + (py - ey) * (py - ey) <= er * er) return BLM_ARC;
    if ((px + ex) * (px + ex) + (py - ey) * (py - ey) <= er * er) return BLM_ARC;
    return 0;
}

/* calls span(ctx, x0, x1, y, what) for each run of arc or dot, row by row */
static void bl_mark_spans(int h, void (*span)(void *, int, int, int, int), void *ctx)
{
    bl_mark_geo g = bl_mark_geometry(h);
    int x, y;
    for (y = 0; y < g.h; y++) {
        int run = 0, start = 0;
        for (x = 0; x <= g.w; x++) {
            int k = x < g.w ? bl_mark_at(&g, x, y) : 0;
            if (k != run) {
                if (run) span(ctx, start, x - 1, y, run);
                run = k;
                start = x;
            }
        }
    }
}

#endif
