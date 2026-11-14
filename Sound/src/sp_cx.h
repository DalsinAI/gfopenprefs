/* GFOpenSpeaker 0.3 (Dale, 14 November 2026, 03:14: "the speaker icon in the systray is wrong, and it should now be a
 * commodity"): its commodity options and its pictures, plain C so the host tests (tests/test_sp_cx.c) run them.
 *   - The options: CX_PRIORITY (-128 to 127, default 10, before GFOpenWindows), CX_POPUP (YES opens the levels at
 *     the start, default NO) and MUTE (a commodity hotkey that mutes and unmutes, default none), from the icon's
 *     tool types or the Shell's KEY=VALUE words.
 *   - The pictures, 16 x 11 as the network icons beside it, in the bar's text colour (pixel 1) as theirs: muted (the
 *     speaker and a cross), off (the speaker alone), then one, two and three waves for the volume's thirds. Each wave
 *     is an unbroken arc round the cone's mouth, as the Wi-Fi's arcs are round its dot.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SP_CX_H
#define SP_CX_H

#define SPX_W       16
#define SPX_H       11
#define SPX_STATES  5
#define SPX_MUTED   0
#define SPX_OFF     1
#define SPX_WAVES1  2                          /* 1 to 33% */
#define SPX_WAVES2  3                          /* 34 to 66% */
#define SPX_WAVES3  4                          /* 67 to 100% */
#define SPX_PRI_DEFAULT 10
#define SPX_HOTKEYLEN 64

typedef struct spx_opts {
    int pri;                                   /* CX_PRIORITY */
    int popup;                                 /* CX_POPUP */
    char mute[SPX_HOTKEYLEN];                  /* MUTE: "" for none */
} spx_opts;

#include "cx_opts.h"                         /* CX_PRIORITY and CX_POPUP, as every GFOpenPrefs commodity */

#define spx_upper cxo_upper
#define spx_key   cxo_key

static inline void spx_defaults(spx_opts *o) { o->pri = SPX_PRI_DEFAULT; o->popup = 0; o->mute[0] = 0; }

/* one tool type or KEY=VALUE word; 1 when it was one of ours */
static inline int spx_word(spx_opts *o, const char *w)
{
    const char *v;
    cxo_opts c;
    if (!w) return 0;
    c.pri = o->pri; c.popup = o->popup;
    if (cxo_word(&c, w)) { o->pri = c.pri; o->popup = c.popup; return 1; }
    if (spx_key(w, "MUTE", &v)) {
        int i = 0;
        if (spx_upper((unsigned char)v[0]) == 'N' && spx_upper((unsigned char)v[1]) == 'O' && spx_upper((unsigned char)v[2]) == 'N'
            && spx_upper((unsigned char)v[3]) == 'E' && !v[4]) v += 4;   /* MUTE=NONE: none */
        for (; v[i] && i < SPX_HOTKEYLEN - 1; i++) o->mute[i] = v[i];
        o->mute[i] = 0;
        return 1;
    }
    return 0;
}

/* the picture for a level */
static inline int spx_state(int muted, int volume)
{
    if (muted) return SPX_MUTED;
    if (volume <= 0) return SPX_OFF;
    return volume <= 33 ? SPX_WAVES1 : volume <= 66 ? SPX_WAVES2 : SPX_WAVES3;
}

/* the wave of radius r round (cx, cy): for each row within reach, the arc's column, with the run between two rows'
 * columns filled, so the arc has no gap */
static inline void spx_arc(unsigned char *px, int cx, int cy, int r, int reach)
{
    int dy, last = -1;
    for (dy = -reach; dy <= reach; dy++) {
        int dx = 0, y = cy + dy, x, from, to;
        while ((dx + 1) * (dx + 1) + dy * dy <= r * r + r) dx++;   /* a radius of r + 1/2: round, not stepped */
        if (y < 0 || y >= SPX_H) { last = -1; continue; }
        from = to = cx + dx;
        if (last >= 0 && dy <= 0 && last < from) from = last + 1;            /* going out: fill to here */
        if (last >= 0 && dy > 0 && last > to + 1) to = last - 1;           /* coming back */
        for (x = from; x <= to; x++) if (x >= 0 && x < SPX_W) px[y * SPX_W + x] = 1;
        if (dy > 0 && last >= 0 && last > cx + dx + 1)                       /* the row above's run meets this one */
            for (x = cx + dx + 1; x < last; x++) if (x < SPX_W) px[(y - 1) * SPX_W + x] = 1;
        last = cx + dx;
    }
}

static inline void spx_pic(unsigned char *px, int state)
{
    const int cy = SPX_H / 2;
    int i, y;
    for (i = 0; i < SPX_W * SPX_H; i++) px[i] = 0;
#define SPX_P(a, b) (px[(b) * SPX_W + (a)] = 1)
    for (y = cy - 2; y <= cy + 2; y++) for (i = 0; i <= 2; i++) SPX_P(i, y);          /* the box */
    for (i = 0; i < 4; i++) for (y = cy - 2 - i; y <= cy + 2 + i; y++) SPX_P(3 + i, y);  /* the cone, 11 rows at its mouth */
    if (state == SPX_MUTED) {
        for (i = 0; i <= 6; i++) { SPX_P(9 + i, cy - 3 + i); SPX_P(9 + i, cy + 3 - i); }   /* 7 x 7, on the middle row */
    } else {
        if (state >= SPX_WAVES1) spx_arc(px, 6, cy, 3, 2);
        if (state >= SPX_WAVES2) spx_arc(px, 6, cy, 6, 4);
        if (state >= SPX_WAVES3) spx_arc(px, 6, cy, 9, 5);
    }
#undef SPX_P
}

#endif
