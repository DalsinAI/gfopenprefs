/* Host tests for Blanker/src/bl_mark.h: the Deep cradle at the blanker's sizes
 * is as wide as its geometry says, symmetric, open at the top, closed at the
 * bottom, with the dot inside and clear of the arc; the spans cover exactly
 * the pixels bl_mark_at says.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "bl_mark.h"

static int fails, oks;
#define CHECK(c, what) do { if (c) { oks++; printf("ok   %s\n", what); } else { fails++; printf("FAIL %s\n", what); } } while (0)

static unsigned char grid[200][300];
static int bad_span;
static void put(void *ctx, int x0, int x1, int y, int what)
{
    int x;
    (void)ctx;
    if (x0 > x1 || x0 < 0 || x1 >= 300 || y < 0 || y >= 200) { bad_span = 1; return; }
    for (x = x0; x <= x1; x++) grid[y][x] = (unsigned char)what;
}

int main(void)
{
    static const int sizes[] = { 16, 24, 40, 64, 83, 120 };
    unsigned i;
    char what[96];
    for (i = 0; i < sizeof sizes / sizeof sizes[0]; i++) {
        int h = sizes[i], x, y, sym = 1, same = 1, top_open = 1, arc = 0, dot = 0, dot_clear = 1, minx = 999, maxx = -1;
        bl_mark_geo g = bl_mark_geometry(h);
        memset(grid, 0, sizeof grid);
        bad_span = 0;
        bl_mark_spans(h, put, NULL);
        for (y = 0; y < h; y++)
            for (x = 0; x < g.w; x++) {
                int k = grid[y][x];
                if (k != bl_mark_at(&g, x, y)) same = 0;
                if (k != grid[y][g.w - 1 - x]) sym = 0;
                if (k == BLM_ARC) arc++;
                if (k == BLM_DOT) { dot++; if (grid[y][x - 1] == BLM_ARC || grid[y][x + 1] == BLM_ARC || grid[y - 1][x] == BLM_ARC || grid[y + 1][x] == BLM_ARC) dot_clear = 0; }
                if (k) { if (x < minx) minx = x; if (x > maxx) maxx = x; }
            }
        for (y = 0; y < (int)(g.cy - g.r * 0.6); y++)          /* above the arc's ends, nothing in the middle */
            for (x = g.w / 3; x < g.w - g.w / 3; x++) if (grid[y][x]) top_open = 0;
        snprintf(what, sizeof what, "%d high: %d wide, the spans are bl_mark_at's, in bounds", h, g.w);
        CHECK(same && !bad_span && g.w > h && g.w < h * 3 / 2, what);
        snprintf(what, sizeof what, "%d high: symmetric left to right", h);
        CHECK(sym, what);
        snprintf(what, sizeof what, "%d high: open at the top, closed at the bottom row", h);
        CHECK(top_open && grid[h - 1][g.w / 2] == BLM_ARC, what);
        snprintf(what, sizeof what, "%d high: a dot inside and clear of the arc (%d arc, %d dot pixels)", h, arc, dot);
        CHECK(dot > 0 && arc > dot && dot_clear, what);
        snprintf(what, sizeof what, "%d high: fills its width (%d..%d)", h, minx, maxx);
        CHECK(minx <= 1 && maxx >= g.w - 2, what);
    }
    printf("%d checks\n%s\n", oks + fails, fails ? "FAIL" : "PASS");
    return fails != 0;
}
