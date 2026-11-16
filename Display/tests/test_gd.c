/* Host tests for gfosdisplay.prefs's GDSP chunk (gd_prefs.c), on pf_iff's list.
 *   Display/tests/run.sh
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "pf_iff.h"
#include "gd_prefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fprintf(stderr, "FAIL %s:%d: ", __FILE__, __LINE__); fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); fails++; } } while (0)

static void round_trip(const pf_file *f, pf_file *back)
{
    long n = pf_size(f);
    uint8_t *buf = malloc((size_t)n);
    CHECK(pf_write(f, buf, n) == n, "write");
    pf_init(back);
    CHECK(pf_parse(back, buf, n), "parse");
    free(buf);
}

static int same_row(const gd_profile *a, const gd_profile *b)
{
    return a->keep == b->keep && !strcmp(a->screen.name, b->screen.name) && a->screen.ysize == b->screen.ysize
        && !strcmp(a->sys.name, b->sys.name) && a->sys.ysize == b->sys.ysize
        && !strcmp(a->icon.name, b->icon.name) && a->icon.ysize == b->icon.ysize && !strcmp(a->set, b->set)
        && a->titlebar == b->titlebar && a->frame == b->frame && a->border_x == b->border_x
        && a->border_y == b->border_y && a->grey == b->grey;
}

static void test_shipped(void)
{
    pf_file f;
    gd_profile p[GD_PROFILES], s;
    int n;
    pf_init(&f);
    CHECK(pf_new(&f), "new");
    CHECK(!gd_get(&f, p), "no GDSP yet");
    for (n = 0; n < GD_PROFILES; n++) {
        gd_shipped(n + 1, &s);
        CHECK(same_row(&p[n], &s) && p[n].keep == GD_KEEP_ALL, "profile %d: shipped, every field kept", n + 1);
    }
    gd_shipped(GD_LORES, &s);
    CHECK(!s.screen.name[0] && !strcmp(s.sys.name, "topaz.font") && s.sys.ysize == 8 && !s.grey,
          "lores: the system font, topaz 8, no grey edges");
    gd_shipped(GD_RTG_1080, &s);
    CHECK(!strcmp(s.screen.name, "GFOSSans.font") && s.screen.ysize == 13 && !strcmp(s.set, "medres") && s.grey,
          "1080p: GFOS Sans 13, medres, grey edges");
    gd_shipped(GD_RTG_LARGE, &s);
    CHECK(s.screen.ysize == 13 && s.sys.ysize == 20 && s.titlebar == 6 && s.frame == 2,
          "large: GFOS Sans 13 (Dale 14 Nov), Mono 20, bar +6, frame 2");
    gd_shipped(0, &p[0]);
    gd_shipped(9, &p[1]);
    CHECK(same_row(&p[0], &s) == 0 && same_row(&p[0], &p[1]), "a number outside 1-6 gives 1080p's");
    /* shipped rows put: one chunk, records numbered, every keep bit, kept fields zero */
    for (n = 0; n < GD_PROFILES; n++) gd_shipped(n + 1, &p[n]);
    CHECK(gd_put(&f, p), "put shipped");
    CHECK(f.n == 2 && f.ch[1].id == GD_GDSP && f.ch[1].len == GD_LEN, "one GDSP of 768 bytes after PRHD");
    {
        int bad = 0, i;
        for (n = 0; n < GD_PROFILES; n++) {
            const uint8_t *r = f.ch[1].data + n * GD_RECORD;
            if (r[0] != n + 1 || pf_get16(r + 2) != GD_KEEP_ALL) bad++;
            for (i = 4; i < GD_RECORD; i++) if (r[i]) bad++;
        }
        CHECK(!bad, "records 1-6, keep all, the rest zero");
    }
    pf_free(&f);
}

static void test_changes(void)
{
    pf_file f, g;
    gd_profile p[GD_PROFILES], q[GD_PROFILES];
    int n;
    pf_init(&f);
    CHECK(pf_new(&f), "new");
    for (n = 0; n < GD_PROFILES; n++) gd_shipped(n + 1, &p[n]);
    /* lores: GFOS Sans 11 on the lores set; 1080p: Mono 13, no grey; large: frame 3, borders 8/4 */
    p[0].keep &= ~(GD_KEEP_SCREENFONT | GD_KEEP_IMAGES);
    strcpy(p[0].screen.name, "GFOSSans.font"); p[0].screen.ysize = 11; strcpy(p[0].set, "lores");
    p[4].keep &= ~(GD_KEEP_SCREENFONT | GD_KEEP_GREY);
    strcpy(p[4].screen.name, "GFOSMono.font"); p[4].screen.ysize = 13; p[4].grey = 0;
    p[5].keep &= ~(GD_KEEP_FRAME | GD_KEEP_TITLEBAR);
    p[5].frame = 3; p[5].border_x = 8; p[5].border_y = 4; p[5].titlebar = 7;
    CHECK(gd_put(&f, p), "put");
    {
        const uint8_t *r = f.ch[1].data;
        CHECK(pf_get16(r + 2) == (GD_KEEP_ALL & ~(GD_KEEP_SCREENFONT | GD_KEEP_IMAGES)) && !strcmp((const char *)r + 4, "GFOSSans.font")
              && pf_get16(r + 36) == 11 && !strcmp((const char *)r + 106, "lores") && !r[38],
              "lores record: its font and set at 4, 36 and 106; the kept system font left zero");
        r += 5 * GD_RECORD;
        CHECK(r[122] == 7 && r[123] == 3 && r[124] == 8 && r[125] == 4 && !r[126], "large record: bar, frame, borders");
    }
    round_trip(&f, &g);
    CHECK(gd_get(&g, q) == 1, "read back");
    for (n = 0; n < GD_PROFILES; n++) CHECK(same_row(&p[n], &q[n]), "profile %d round trips", n + 1);
    /* the same rows put again give the same bytes */
    {
        pf_file h;
        pf_init(&h);
        CHECK(pf_copy(&h, &g), "copy");
        CHECK(gd_put(&h, q) && pf_same(&h, &g), "settings put back unchanged give the file's bytes");
        pf_free(&h);
    }
    /* reset a row: its keep bits come back, the file keeps the old bytes but Intuition reads none */
    gd_reset(1, &q[0]);
    CHECK(gd_put(&g, q), "put reset");
    CHECK(pf_get16(g.ch[1].data + 2) == GD_KEEP_ALL, "profile 1: keep all again");
    CHECK(gd_get(&g, p) && !p[0].screen.name[0] && !strcmp(p[0].set, "medres"), "profile 1 reads shipped");
    pf_free(&f);
    pf_free(&g);
}

static void test_foreign(void)
{
    pf_file f, g;
    gd_profile p[GD_PROFILES];
    uint8_t c[GD_LEN + 8];
    int n;
    memset(c, 0, sizeof c);
    for (n = 0; n < GD_PROFILES; n++) {
        c[n * GD_RECORD] = (uint8_t)(n + 1);
        pf_put16(c + n * GD_RECORD + 2, 0x8000 | GD_KEEP_ALL);  /* a bit from a later release */
        c[n * GD_RECORD + 1] = 0x5a;                            /* reserved bytes someone used */
        c[n * GD_RECORD + 127] = 0xa5;
    }
    strcpy((char *)c + 2 * GD_RECORD + 38, "Later.font");        /* profile 3: a kept system font's bytes */
    memcpy(c + GD_LEN, "TRAILING", 8);
    pf_init(&f);
    CHECK(pf_new(&f) && pf_add(&f, GD_GDSP, c, sizeof c), "a file from a later release");
    CHECK(gd_get(&f, p) == 1, "read");
    p[1].keep &= ~GD_KEEP_GREY;
    p[1].grey = 1;
    CHECK(gd_put(&f, p), "put");
    {
        const uint8_t *d = f.ch[1].data;
        int kept = f.ch[1].len == sizeof c && !memcmp(d + GD_LEN, "TRAILING", 8) && d[1] == 0x5a && d[127] == 0xa5
                && !strcmp((const char *)d + 2 * GD_RECORD + 38, "Later.font")
                && pf_get16(d + GD_RECORD + 2) == (0x8000 | (GD_KEEP_ALL & ~GD_KEEP_GREY)) && d[GD_RECORD + 126] == 1;
        CHECK(kept, "unknown bits, reserved bytes, kept fields and the bytes after the records stay");
    }
    /* unusable fields read as kept, as Intuition takes them */
    {
        uint8_t *r = f.ch[1].data;
        pf_put16(r + 2, GD_KEEP_ALL & ~(GD_KEEP_SCREENFONT | GD_KEEP_SYSFONT | GD_KEEP_ICONFONT | GD_KEEP_IMAGES));
        memset(r + 4, 'x', GD_NAME);                               /* no end */
        strcpy((char *)r + 38, "topaz.font"); pf_put16(r + 70, 0); /* size 0 */
        r[72] = 0;                                                 /* empty */
        strcpy((char *)r + 106, "huge");                           /* not a set */
        CHECK(gd_get(&f, p) == 1 && p[0].keep == GD_KEEP_ALL && !p[0].screen.name[0] && !strcmp(p[0].set, "medres"),
              "a name with no end, size 0, an empty name and an unknown set: kept");
    }
    /* out of order or short: not read, and a put writes six fresh records */
    pf_init(&g);
    c[2 * GD_RECORD] = 4;
    CHECK(pf_new(&g) && pf_add(&g, GD_GDSP, c, sizeof c), "records out of order");
    CHECK(gd_get(&g, p) == 0 && p[2].keep == GD_KEEP_ALL, "not read: shipped");
    CHECK(gd_put(&g, p) && g.ch[1].len == GD_LEN && g.ch[1].data[2 * GD_RECORD] == 3 && !g.ch[1].data[1],
          "put: six fresh records");
    pf_set(&g, 1, c, GD_LEN - 1);
    CHECK(gd_get(&g, p) == 0, "a short chunk: not read");
    CHECK(gd_set_known("hires12") && gd_set_known("large") && !gd_set_known("Large") && !gd_set_known(""), "the four sets");
    pf_free(&f);
    pf_free(&g);
}

/* the same modes as amicradle-guest tests/m68k/openintuition_screen_test.c's profile checks (Intuition's ticks) */
static void test_profile_of(void)
{
    static const struct { uint32_t id; unsigned rx, ry; int w, h, want; } m[] = {
        { 0x00000000, 44, 44, 320, 256, GD_LORES }, { 0x00008000, 22, 44, 640, 256, GD_HIRES },
        { 0x00008004, 22, 22, 640, 512, GD_LACED }, { 0x00008020, 11, 44, 1280, 256, GD_HIRES },
        { 0x00000004, 44, 22, 320, 512, GD_LACED }, { 0x00000004, 44, 22, 320, 256, GD_LORES },
        { 0x000A1000, 22, 44, 640, 256, GD_HIRES },             /* DBLPAL stays chipset */
        { 0x610A1200, 1, 1, 800, 600, GD_RTG_SMALL }, { 0x610A1200, 1, 1, 1024, 768, GD_RTG_SMALL },
        { 0x610A1600, 1, 1, 1920, 1080, GD_RTG_1080 }, { 0x610A1600, 1, 1, 1280, 720, GD_RTG_1080 },
        { 0x610A1900, 1, 1, 3840, 2160, GD_RTG_LARGE }, { 0x00008000, 0, 0, 640, 256, GD_LACED },
    };
    size_t i;
    for (i = 0; i < sizeof m / sizeof m[0]; i++)
        CHECK(gd_profile_of(m[i].id, m[i].rx, m[i].ry, m[i].w, m[i].h) == m[i].want, "mode %08lx %dx%d: profile %d",
              (unsigned long)m[i].id, m[i].w, m[i].h, m[i].want);
}

int main(void)
{
    test_profile_of();
    test_shipped();
    test_changes();
    test_foreign();
    printf("gd_prefs: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
