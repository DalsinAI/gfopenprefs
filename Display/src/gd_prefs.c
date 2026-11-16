/* gd_prefs: gfosdisplay.prefs's GDSP chunk (gd_prefs.h).
 * Plain C: tested on the host by Display/tests/run.sh.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "gd_prefs.h"

#include <stdlib.h>
#include <string.h>

/* The design's section 2.2 table as GFOS ships it today. The screen font and image set are what
 * GFOpenIntuition draws with no file (screen_core.c profiles_tab: GFOS Sans 13 on RTG, the system font on
 * the chipset, MEDRES everywhere); the other fields are the design's first guess, kept by Intuition until
 * their steps. */
static const struct {
    const char *screen; int ss;
    const char *sys; int sy;
    const char *icon; int is;
    const char *set;
    int titlebar, frame, bx, by, grey;
} shipped[GD_PROFILES] = {
    { "", 0, "topaz.font", 8, "", 0, "medres", 2, 1, 2, 2, 1 },
    { "", 0, "GFOSMono.font", 16, "", 0, "medres", 2, 1, 4, 2, 1 },
    { "", 0, "GFOSMono.font", 16, "", 0, "medres", 3, 1, 4, 2, 1 },
    { "GFOSSans.font", 13, "GFOSMono.font", 16, "GFOSSans.font", 11, "medres", 4, 1, 4, 2, 1 },
    { "GFOSSans.font", 13, "GFOSMono.font", 16, "GFOSSans.font", 11, "medres", 4, 1, 4, 2, 1 },
    { "GFOSSans.font", 13, "GFOSMono.font", 20, "GFOSSans.font", 11, "medres", 6, 2, 6, 3, 1 },
};

static const char *const sets[] = { "lores", "hires12", "medres", "large" };

static void copy_name(char *to, const char *from, size_t max)
{
    size_t n = strlen(from);
    if (n >= max) n = max - 1;
    memcpy(to, from, n);
    to[n] = 0;
}

int gd_profile_of(uint32_t id, unsigned rx, unsigned ry, int width, int height)
{
    if ((id & GD_MONITOR_MASK) >= GD_RTG_FIRST)
        return width <= 1024 ? GD_RTG_SMALL : width <= 1920 ? GD_RTG_1080 : GD_RTG_LARGE;
    if (!rx || !ry) rx = ry = 1;
    if (4 * ry >= 3 * rx && 4 * ry <= 5 * rx) return width <= 400 ? GD_LORES : GD_LACED;   /* 1:1, within 25% */
    if (2 * ry >= 3 * rx && 2 * ry <= 5 * rx) return GD_HIRES;                            /* 1:2, within 25% */
    if (height >= 384) return GD_LACED;
    return ry > rx ? GD_HIRES : GD_LORES;
}

int gd_set_known(const char *s)
{
    size_t k;
    for (k = 0; k < sizeof sets / sizeof sets[0]; k++)
        if (!strcmp(s, sets[k])) return 1;
    return 0;
}

void gd_reset(int n, gd_profile *p)
{
    int i = (n >= GD_LORES && n <= GD_RTG_LARGE ? n : GD_RTG_1080) - 1;
    memset(p, 0, sizeof *p);
    p->keep = GD_KEEP_ALL;
    copy_name(p->screen.name, shipped[i].screen, GD_NAME);
    p->screen.ysize = shipped[i].ss;
    copy_name(p->sys.name, shipped[i].sys, GD_NAME);
    p->sys.ysize = shipped[i].sy;
    copy_name(p->icon.name, shipped[i].icon, GD_NAME);
    p->icon.ysize = shipped[i].is;
    copy_name(p->set, shipped[i].set, GD_SET);
    p->titlebar = shipped[i].titlebar;
    p->frame = shipped[i].frame;
    p->border_x = shipped[i].bx;
    p->border_y = shipped[i].by;
    p->grey = shipped[i].grey;
}

void gd_shipped(int n, gd_profile *out)
{
    gd_reset(n, out);
}

/* a name field of max bytes into to: 1 when it ends within them and isn't empty */
static int get_name(char *to, const uint8_t *b, size_t max)
{
    size_t n;
    for (n = 0; n < max && b[n]; n++)
        ;
    if (!n || n == max) return 0;
    memcpy(to, b, n);
    to[n] = 0;
    return 1;
}

static void get_font(gd_profile *p, unsigned bit, gd_font *ft, const uint8_t *b)
{
    gd_font f;
    if (p->keep & bit) return;
    f.ysize = pf_get16(b + GD_NAME);
    if (get_name(f.name, b, GD_NAME) && f.ysize)
        *ft = f;
    else
        p->keep |= bit;               /* unusable: the shipped one, as Intuition reads it */
}

int gd_get(const pf_file *f, gd_profile out[GD_PROFILES])
{
    int i = pf_find(f, GD_GDSP, 0), n;
    const uint8_t *d;
    for (n = 0; n < GD_PROFILES; n++)
        gd_reset(n + 1, &out[n]);
    if (i < 0 || f->ch[i].len < GD_LEN) return 0;
    d = f->ch[i].data;
    for (n = 0; n < GD_PROFILES; n++)
        if (d[n * GD_RECORD] != n + 1) return 0;
    for (n = 0; n < GD_PROFILES; n++) {
        const uint8_t *r = d + n * GD_RECORD;
        gd_profile *p = &out[n];
        char set[GD_SET];
        p->keep = pf_get16(r + 2) & GD_KEEP_ALL;
        get_font(p, GD_KEEP_SCREENFONT, &p->screen, r + 4);
        get_font(p, GD_KEEP_SYSFONT, &p->sys, r + 38);
        get_font(p, GD_KEEP_ICONFONT, &p->icon, r + 72);
        if (!(p->keep & GD_KEEP_IMAGES)) {
            if (get_name(set, r + 106, GD_SET) && gd_set_known(set))
                copy_name(p->set, set, GD_SET);
            else
                p->keep |= GD_KEEP_IMAGES;
        }
        if (!(p->keep & GD_KEEP_TITLEBAR)) p->titlebar = r[122];
        if (!(p->keep & GD_KEEP_FRAME)) {
            p->frame = r[123];
            p->border_x = r[124];
            p->border_y = r[125];
        }
        if (!(p->keep & GD_KEEP_GREY)) p->grey = r[126] != 0;
    }
    return 1;
}

static void put_font(uint8_t *b, const gd_font *ft)
{
    memset(b, 0, GD_NAME);
    copy_name((char *)b, ft->name, GD_NAME);
    pf_put16(b + GD_NAME, (uint16_t)ft->ysize);
}

int gd_put(pf_file *f, const gd_profile in[GD_PROFILES])
{
    int i = pf_find(f, GD_GDSP, 0), n, ok;
    uint32_t have = i >= 0 ? f->ch[i].len : 0, len = have > GD_LEN ? have : GD_LEN;
    uint8_t *p = malloc(len);
    if (!p) return 0;
    memset(p, 0, len);
    /* a chunk whose records are out of order isn't carried: its bytes mean nothing to Intuition */
    if (have >= GD_LEN) {
        for (n = 0; n < GD_PROFILES && f->ch[i].data[n * GD_RECORD] == n + 1; n++)
            ;
        if (n == GD_PROFILES) memcpy(p, f->ch[i].data, have);
        else len = GD_LEN;
    }
    for (n = 0; n < GD_PROFILES; n++) {
        uint8_t *r = p + n * GD_RECORD;
        const gd_profile *g = &in[n];
        r[0] = (uint8_t)(n + 1);
        pf_put16(r + 2, (uint16_t)((pf_get16(r + 2) & ~GD_KEEP_ALL) | (g->keep & GD_KEEP_ALL)));
        if (!(g->keep & GD_KEEP_SCREENFONT)) put_font(r + 4, &g->screen);
        if (!(g->keep & GD_KEEP_SYSFONT)) put_font(r + 38, &g->sys);
        if (!(g->keep & GD_KEEP_ICONFONT)) put_font(r + 72, &g->icon);
        if (!(g->keep & GD_KEEP_IMAGES)) {
            memset(r + 106, 0, GD_SET);
            copy_name((char *)r + 106, g->set, GD_SET);
        }
        if (!(g->keep & GD_KEEP_TITLEBAR)) r[122] = (uint8_t)g->titlebar;
        if (!(g->keep & GD_KEEP_FRAME)) {
            r[123] = (uint8_t)g->frame;
            r[124] = (uint8_t)g->border_x;
            r[125] = (uint8_t)g->border_y;
        }
        if (!(g->keep & GD_KEEP_GREY)) r[126] = g->grey ? 1 : 0;
    }
    ok = i >= 0 ? pf_set(f, i, p, len) : pf_add(f, GD_GDSP, p, len);
    free(p);
    return ok;
}
