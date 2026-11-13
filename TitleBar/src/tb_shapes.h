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

#endif
