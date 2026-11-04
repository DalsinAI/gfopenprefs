/* GFOpenWindows' wheel, the pure part (wheel.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wheel.h"

static int has(const struct wheel_box *b, unsigned dir) { return (b->flags & dir) != 0; }

int wheel_pick(const struct wheel_box *b, int n, int x, int y, unsigned dir)
{
    int i, best = -1;
    long d, bestd = 0x7fffffffL;
    for (i = 0; i < n; i++)                     /* 1. on it */
        if (has(&b[i], dir) && x >= b[i].l && x < b[i].l + b[i].w && y >= b[i].t && y < b[i].t + b[i].h) return i;
    for (i = 0; i < n; i++) {                   /* 2. on its arrows, in line with it */
        const struct wheel_box *g = &b[i];
        if (!has(g, dir)) continue;
        if (dir == WS_VERT ? x >= g->l && x < g->l + g->w && y >= g->t - WHEEL_ARROWS && y < g->t + g->h + WHEEL_ARROWS
                           : y >= g->t && y < g->t + g->h && x >= g->l - WHEEL_ARROWS && x < g->l + g->w + WHEEL_ARROWS)
            return i;
    }
    for (i = 0; i < n; i++) {                   /* 3. beside the place */
        const struct wheel_box *g = &b[i];
        if (!has(g, dir)) continue;
        if (dir == WS_VERT) {
            if (y < g->t || y >= g->t + g->h || g->l + g->w <= x) continue;
            d = g->l - x;
        } else {
            if (x < g->l || x >= g->l + g->w || g->t + g->h <= y) continue;
            d = g->t - y;
        }
        if (d < bestd) { bestd = d; best = i; }
    }
    return best;
}

int wheel_main(const struct wheel_box *b, int n, unsigned dir)
{
    int i, best = -1, bestlen = -1;
    for (i = 0; i < n; i++) {
        int len;
        if (!has(&b[i], dir)) continue;
        len = (dir == WS_VERT ? b[i].h : b[i].w) + ((b[i].flags & WS_BORDER) ? 100000 : 0);
        if (len > bestlen) { bestlen = len; best = i; }
    }
    return best;
}

int wheel_step(unsigned code, unsigned qual, unsigned *dir)
{
    if (code & 0x80) return 0;                  /* let go */
    switch (code) {
    case 0x7A: *dir = (qual & WHEEL_SHIFT) ? WS_HORIZ : WS_VERT; return -1;
    case 0x7B: *dir = (qual & WHEEL_SHIFT) ? WS_HORIZ : WS_VERT; return 1;
    case 0x7C: *dir = WS_HORIZ; return -1;
    case 0x7D: *dir = WS_HORIZ; return 1;
    }
    return 0;
}

unsigned wheel_pot(unsigned pot, unsigned body, int view_px, int line_px, int lines)
{
    unsigned long long num, den, step;
    long v, np;
    if (body >= 0xFFFF || !lines) return pot;
    if (line_px < 1) line_px = 1;
    v = view_px / line_px;
    if (v < 1) v = 1;
    /* a line: MAXPOT / (T - V), T - V = V * (MAXBODY - body) / body */
    num = 0xFFFFULL * body * (unsigned long long)(lines < 0 ? -lines : lines);
    den = (unsigned long long)v * (0xFFFF - body);
    step = (num + den - 1) / den;
    if (step < 1) step = 1;
    if (step > 0xFFFF) step = 0xFFFF;
    np = (long)pot + (lines < 0 ? -(long)step : (long)step);
    if (np < 0) np = 0;
    if (np > 0xFFFF) np = 0xFFFF;
    return (unsigned)np;
}

long wheel_top(long top, long total, long visible, long lines)
{
    long most = total - visible, t = top + lines;
    if (most < 0) most = 0;
    if (t > most) t = most;
    if (t < 0) t = 0;
    return t;
}

long wheel_units(long visible, int view_px, int line_px)
{
    long lines, u;
    if (line_px < 1) line_px = 1;
    lines = view_px / line_px;
    if (lines < 1) return 1;
    u = (visible + lines / 2) / lines;
    return u < 1 ? 1 : u;
}

int wheel_probe(int moved, int could)
{
    if (moved) return WC_SELF;
    return could ? WC_EMULATE : WC_UNKNOWN;
}
