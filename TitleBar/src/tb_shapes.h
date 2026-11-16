/* tb_shapes.h: the shapes 0.9's cog, power and systray cells draw (a glyph's runs, Active's tick, a commodity's
 * rounded box), as boxes for GFOpenLayout's oldraw_fill (gfopenlayout_draw.h): GFOpenTitle draws them through
 * GFOpenLayout, not with graphics.library's Move/Draw/RectFill (the Team, 13 November 2026). Plain C, so the host
 * tests see the same pixels the bar gets.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef TB_SHAPES_H
#define TB_SHAPES_H

struct tb_box { short x, y, w, h; };

static __attribute__((unused)) int tb_box_add(struct tb_box *out, int n, int max, int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0 || n >= max) return n;
    out[n].x = (short)x; out[n].y = (short)y; out[n].w = (short)w; out[n].h = (short)h;
    return n + 1;
}

/* A glyph's '#' rows (size by size) at ox, oy, rows from h on cut off: one box per run of '#' in a row. How many. */
static __attribute__((unused)) int tb_glyph_boxes(const char *const *rows, int size, int ox, int oy, int h,
                                                  struct tb_box *out, int max)
{
    int gy, gx, run, n = 0;
    for (gy = 0; gy < size && oy + gy < h; gy++)
        for (gx = 0; gx < size; ) {
            if (rows[gy][gx] != '#') { gx++; continue; }
            for (run = gx; run < size && rows[gy][run] == '#'; run++) ;
            n = tb_box_add(out, n, max, ox + gx, oy + gy, run - gx, 1);
            gx = run;
        }
    return n;
}

/* Active's tick, its corner at x 4 and cy0 + 2: down from (2, cy0) and up to (8, cy0 - 3), two pixels thick (the
 * second stroke a row lower). One box per pixel row of each stroke: 18 at most. How many. */
static __attribute__((unused)) int tb_tick_boxes(int cy0, struct tb_box *out, int max)
{
    /* the up stroke's x for each row from cy0 + 2 up to cy0 - 3 (4 across in 5 rows, rounded) */
    static const signed char up[6] = { 4, 5, 6, 6, 7, 8 };
    int s, i, n = 0;
    for (s = 0; s < 2; s++) {
        for (i = 0; i < 2; i++) n = tb_box_add(out, n, max, 2 + i, cy0 + s + i, 1, 1);   /* (2,cy0), (3,cy0+1) */
        for (i = 0; i < 6; i++) n = tb_box_add(out, n, max, up[i], cy0 + s + 2 - i, 1, 1);
    }
    return n;
}

/* A commodity's rounded box, size by size at ox, oy: four sides two pixels short of each corner, and one pixel in
 * each corner. 8 boxes (fewer for a box under 5). How many. */
static __attribute__((unused)) int tb_rbox_boxes(int ox, int oy, int size, struct tb_box *out, int max)
{
    int n = 0, side = size - 4;
    n = tb_box_add(out, n, max, ox + 2, oy, side, 1);                   /* top */
    n = tb_box_add(out, n, max, ox + size - 1, oy + 2, 1, side);        /* right */
    n = tb_box_add(out, n, max, ox + 2, oy + size - 1, side, 1);        /* bottom */
    n = tb_box_add(out, n, max, ox, oy + 2, 1, side);                   /* left */
    if (size >= 3) {
        n = tb_box_add(out, n, max, ox + 1, oy + 1, 1, 1);
        n = tb_box_add(out, n, max, ox + size - 2, oy + 1, 1, 1);
        n = tb_box_add(out, n, max, ox + 1, oy + size - 2, 1, 1);
        n = tb_box_add(out, n, max, ox + size - 2, oy + size - 2, 1, 1);
    }
    return n;
}

/* ---- GFOpenTitle 0.12.2: the rest of the bar's shapes, as boxes (the Team's drawing rule, 13 November 2026) ----
 * These append to out from n and return the new count, so one list holds a whole picture. */

/* A pixel at x,y, joined to the last box when it carries on its row (or its column) by one pixel. */
static __attribute__((unused)) int tb_pixel_add(struct tb_box *out, int n, int max, int x, int y)
{
    if (n > 0) {
        struct tb_box *l = &out[n - 1];
        if (l->h == 1 && l->y == y && l->x + l->w == x) { l->w++; return n; }
        if (l->w == 1 && l->x == x && l->y + l->h == y && l->h > 1) { l->h++; return n; }
        if (l->w == 1 && l->h == 1 && l->x == x && l->y + 1 == y) { l->h++; return n; }
    }
    return tb_box_add(out, n, max, x, y, 1, 1);
}

/* A line from x0,y0 to x1,y1 with both ends lit, as graphics.library's Draw() lights it: Bresenham's pixels, the
 * nearest row (or column) at each step of the longer way (no line the bar draws has a step half way between two).
 * A straight line is one box. */
static __attribute__((unused)) int tb_line_add(struct tb_box *out, int n, int max, int x0, int y0, int x1, int y1)
{
    int dx = x1 - x0, dy = y1 - y0, ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy, sx = dx < 0 ? -1 : 1,
        sy = dy < 0 ? -1 : 1, i;
    if (!dy) return tb_box_add(out, n, max, dx < 0 ? x1 : x0, y0, ax + 1, 1);
    if (!dx) return tb_box_add(out, n, max, x0, dy < 0 ? y1 : y0, 1, ay + 1);
    for (i = 0; i <= (ax >= ay ? ax : ay); i++) {
        int x, y;
        if (ax >= ay) { x = x0 + sx * i; y = y0 + sy * ((2 * i * ay + ax) / (2 * ax)); }
        else { y = y0 + sy * i; x = x0 + sx * ((2 * i * ax + ay) / (2 * ay)); }
        /* a run going left (or up) is joined from its other end: kept as pixels, in the order drawn */
        if ((ax >= ay && sx > 0) || (ax < ay && sy > 0)) n = tb_pixel_add(out, n, max, x, y);
        else n = tb_box_add(out, n, max, x, y, 1, 1);
    }
    return n;
}

/* The outline of the box x0,y0 to x1,y1 (Move to a corner and Draw round it): four sides, no pixel twice. */
static __attribute__((unused)) int tb_frame_add(struct tb_box *out, int n, int max, int x0, int y0, int x1, int y1)
{
    n = tb_box_add(out, n, max, x0, y0, x1 - x0 + 1, 1);                 /* top */
    n = tb_box_add(out, n, max, x1, y0 + 1, 1, y1 - y0);                 /* right */
    n = tb_box_add(out, n, max, x0, y1, x1 - x0, y1 > y0 ? 1 : 0);       /* bottom */
    n = tb_box_add(out, n, max, x0, y0 + 1, 1, y1 - y0 - 1);             /* left */
    return n;
}

/* The LAN icon at x, its middle row cy: a box above two, joined; filled when connected (state 2), hollow when not
 * (1), and crossed with no network (0). 64 boxes are plenty. */
static __attribute__((unused)) int tb_lan_boxes(int x, int cy, int state, struct tb_box *out, int max)
{
    int t = cy - 5, n = 0;
    if (state == 2) {
        n = tb_box_add(out, n, max, x + 5, t, 6, 4);
        n = tb_box_add(out, n, max, x, t + 7, 5, 4);
        n = tb_box_add(out, n, max, x + 11, t + 7, 5, 4);
    } else {
        n = tb_frame_add(out, n, max, x + 5, t, x + 10, t + 3);
        n = tb_frame_add(out, n, max, x, t + 7, x + 4, t + 10);
        n = tb_frame_add(out, n, max, x + 11, t + 7, x + 15, t + 10);
    }
    n = tb_line_add(out, n, max, x + 7, t + 4, x + 7, t + 5);
    n = tb_line_add(out, n, max, x + 2, t + 5, x + 13, t + 5);
    n = tb_line_add(out, n, max, x + 2, t + 5, x + 2, t + 6);
    n = tb_line_add(out, n, max, x + 13, t + 5, x + 13, t + 6);
    if (state == 0) {
        n = tb_line_add(out, n, max, x, t, x + 15, t + 10);
        n = tb_line_add(out, n, max, x + 15, t, x, t + 10);
    }
    return n;
}

/* The Wi-Fi icon at x, its middle row cy: a dot and up to three arcs by the signal (0 to 100); only the dot and a
 * cross when not connected (state 0). 96 boxes are plenty. */
static __attribute__((unused)) int tb_wifi_boxes(int x, int cy, int state, int signal, struct tb_box *out, int max)
{
    int ox = x + 8, oy = cy + 4, arcs = !state ? 0 : signal > 66 ? 3 : signal > 33 ? 2 : 1, k, n = 0;
    n = tb_box_add(out, n, max, ox - 1, oy - 1, 2, 2);
    for (k = 1; k <= arcs; k++) {
        int r = 3 * k + 1, dx;
        for (dx = -r; dx <= r; dx++) {
            int dy = 0;
            while ((dy + 1) * (dy + 1) + dx * dx <= r * r) dy++;
            if (dx * dx <= dy * dy) n = tb_pixel_add(out, n, max, ox + dx, oy - dy);
        }
    }
    if (!state) {
        n = tb_line_add(out, n, max, ox + 2, oy - 8, ox + 7, oy - 3);
        n = tb_line_add(out, n, max, ox + 7, oy - 8, ox + 2, oy - 3);
    }
    return n;
}

/* The rows y..y+h-1 of a box x..x+w-1 in the colours rgb[0..h-1] (0xRRGGBB, the top 8 bits not looked at): one box
 * for each run of rows of the same colour, top to bottom; out[i]'s colour is rgb[out[i].y - y]. How many (max h is
 * always enough; past max the rest are left out). */
static __attribute__((unused)) int tb_row_runs(const unsigned long *rgb, int x, int y, int w, int h, struct tb_box *out,
                                               int max)
{
    int j = 0, k, n = 0;
    if (w <= 0) return 0;
    while (j < h && n < max) {
        for (k = j + 1; k < h && ((rgb[k] ^ rgb[j]) & 0xffffffUL) == 0; k++) ;
        n = tb_box_add(out, n, max, x, y + j, w, k - j);
        j = k;
    }
    return n;
}

#endif
