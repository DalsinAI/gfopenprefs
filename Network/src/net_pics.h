/* GFOpenNetwork's pictures for GFOpenTitle's row (TitleBar/include/taskspace.h): the network icons GFOpenTitle 0.7.10
 * drew itself, as one byte a pixel (0 see-through, 1 the bar's text colour), NP_W x NP_H each.
 *
 *   LAN:   a box above two, joined; filled when connected, hollow when not, crossed with no network.
 *          States 0 no network, 1 not connected, 2 connected.
 *   Wi-Fi: a dot and up to three arcs by the signal; the dot and a cross when not connected.
 *          States 0 not connected, 1 to 3 arcs.
 *
 * Plain C, no Amiga headers, so the host test (Network/tests) draws them too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef NET_PICS_H
#define NET_PICS_H

#include <string.h>

#define NP_W 16
#define NP_H 11

static void np_pix(unsigned char *p, int x, int y)
{
    if (x >= 0 && x < NP_W && y >= 0 && y < NP_H) p[y * NP_W + x] = 1;
}

/* every pixel from (x0, y0) to (x1, y1), both ends, as graphics.library's Draw() */
static void np_line(unsigned char *p, int x0, int y0, int x1, int y1)
{
    int dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y1 - y0 : y0 - y1;
    int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1, err = dx - dy;
    for (;;) {
        int e2;
        np_pix(p, x0, y0);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

static void np_fill(unsigned char *p, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++) for (x = x0; x <= x1; x++) np_pix(p, x, y);
}

static void np_box(unsigned char *p, int x0, int y0, int x1, int y1)
{
    np_line(p, x0, y0, x1, y0); np_line(p, x1, y0, x1, y1); np_line(p, x1, y1, x0, y1); np_line(p, x0, y1, x0, y0);
}

/* GFOpenTitle 0.7.10's draw_lan() with t = 0 */
static void np_lan(unsigned char *p, int state)
{
    memset(p, 0, NP_W * NP_H);
    if (state == 2) { np_fill(p, 5, 0, 10, 3); np_fill(p, 0, 7, 4, 10); np_fill(p, 11, 7, 15, 10); }
    else { np_box(p, 5, 0, 10, 3); np_box(p, 0, 7, 4, 10); np_box(p, 11, 7, 15, 10); }
    np_line(p, 7, 4, 7, 5); np_line(p, 2, 5, 13, 5);
    np_line(p, 2, 5, 2, 6); np_line(p, 13, 5, 13, 6);
    if (state == 0) { np_line(p, 0, 0, 15, 10); np_line(p, 15, 0, 0, 10); }
}

/* GFOpenTitle 0.7.10's draw_wifi() with the dot at (8, 10): arcs 0 is "not connected" */
static void np_wifi(unsigned char *p, int arcs)
{
    int ox = 8, oy = 10, k;
    memset(p, 0, NP_W * NP_H);
    np_fill(p, ox - 1, oy - 1, ox, oy);
    for (k = 1; k <= arcs && k <= 3; k++) {
        int r = 3 * k + 1, dx;
        for (dx = -r; dx <= r; dx++) {
            int dy = 0;
            while ((dy + 1) * (dy + 1) + dx * dx <= r * r) dy++;
            if (dx * dx <= dy * dy) np_pix(p, ox + dx, oy - dy);
        }
    }
    if (!arcs) { np_line(p, ox + 2, oy - 8, ox + 7, oy - 3); np_line(p, ox + 7, oy - 8, ox + 2, oy - 3); }
}

/* the Wi-Fi picture for a signal in percent (as GFOpenTitle: over 66 three arcs, over 33 two, else one) */
static int np_arcs(int connected, int signal)
{
    return !connected ? 0 : signal > 66 ? 3 : signal > 33 ? 2 : 1;
}

#endif
