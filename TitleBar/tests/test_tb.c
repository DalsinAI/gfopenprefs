/* Host tests for tb_prefs.h (ENV:GFOpenPrefs/TitleBar) and tb_fade.h (the see-through bar's fade, and the pointer's
 * part in it).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "tb_prefs.h"
#include "tb_fade.h"
#include "tb_format.h"
#include "tb_shapes.h"
#include "tb_sbsync.h"
#include "tb_owner.h"
#include <stdio.h>
#include <string.h>

static int checks, failed;
#define CHECK(x) do { checks++; if (!(x)) { failed++; printf("FAILED line %d: %s\n", __LINE__, #x); } } while (0)

static void parse(tb_prefs *p, const char *text)
{
    static char buf[1024];
    strcpy(buf, text);
    tb_parse(p, buf);
}

static void prefs_tests(void)
{
    tb_prefs p, q;
    char out[256];
    int n;
    /* no file: the defaults; the see-through bar is on at 40% */
    tb_parse(&p, NULL);
    CHECK(p.seethrough == 1 && p.see_opacity == 40);
    CHECK(p.logo && p.memory && p.clock && p.date && p.network && p.cog && !p.border_black);
    /* a file written before 0.7.5 (no seethrough lines): on, at 40% */
    parse(&p, "; GFOpenPrefs Title bar 0.2: what GFOpenTitle shows\nlogo off\nmemory on\nclock on\nclock.date off\n"
              "network on\ncog on\nborder.black on\n");
    CHECK(!p.logo && p.memory && !p.date && p.border_black);
    CHECK(p.seethrough == 1 && p.see_opacity == 40);
    /* switched off; another level */
    parse(&p, "seethrough off\nseethrough.opacity 60\n");
    CHECK(p.seethrough == 0 && p.see_opacity == 60);
    /* the level kept to 10-100; not a number keeps the default */
    parse(&p, "seethrough.opacity 3\n");
    CHECK(p.see_opacity == 10);
    parse(&p, "seethrough.opacity 250\n");
    CHECK(p.see_opacity == 100);
    parse(&p, "seethrough.opacity lots\n");
    CHECK(p.see_opacity == 40);
    /* "seethrough" is not taken for "seethrough.opacity", nor the other way */
    parse(&p, "seethrough.opacity 70\n");
    CHECK(p.seethrough == 1 && p.see_opacity == 70);
    /* what the editor writes reads back the same, and fits its 256 bytes */
    tb_defaults(&p);
    p.logo = 0; p.border_black = 1; p.seethrough = 0; p.see_opacity = 55;
    n = tb_text(&p, out, sizeof out);
    CHECK(n > 0 && n < (int)sizeof out);
    CHECK(strstr(out, "\nseethrough off\n") && strstr(out, "\nseethrough.opacity 55\n"));
    parse(&q, out);
    CHECK(!memcmp(&p, &q, sizeof p));
    /* an older reader skips the new lines: each is "name value" on its own line, after the old ones */
    CHECK(strstr(out, "border.black on\nseethrough off\n") != NULL);
}

/* steps of dt ms with hot until the level is to, or max ms pass; answers the ms it took (-1: never) */
static int run(tb_fade *f, int dt, int hot, int to, int max)
{
    int t = 0;
    while (t <= max) {
        if (f->level == to) return t;
        tbf_step(f, dt, hot);
        t += dt;
    }
    return -1;
}

static void fade_tests(void)
{
    tb_fade f;
    int t, lvl, prev, i, wakes, ms;
    const int rest = 40 * 255 / 100;                       /* 102: tb_prefs' default */

    /* at rest: the rest level, and nothing to wake for */
    tbf_init(&f, rest, 0);
    CHECK(f.level == rest && tbf_next(&f) == 0);
    tbf_step(&f, 500, 0);
    CHECK(f.level == rest && tbf_next(&f) == 0);
    /* started with the pointer on the bar: solid at once */
    tbf_init(&f, rest, 1);
    CHECK(f.level == 255 && tbf_next(&f) == 0);

    /* the pointer comes: solid in about 150 ms, in steps of 40 ms, each one up */
    tbf_init(&f, rest, 0);
    CHECK(tbf_next(&f) == 0);
    tbf_step(&f, 0, 1);                                    /* the tick that sees it */
    CHECK(tbf_next(&f) == TBF_STEP_MS);
    prev = f.level;
    for (t = 0, i = 0; f.level < 255 && i < 20; i++) {
        lvl = tbf_step(&f, TBF_STEP_MS, 1);
        CHECK(lvl > prev);
        prev = lvl;
        t += TBF_STEP_MS;
    }
    CHECK(f.level == 255 && t >= 120 && t <= 160);
    CHECK(tbf_next(&f) == 0);                              /* solid while hot: the caller's own tick sees it leave */

    /* it leaves: solid for the 300 ms grace from the step that sees it gone, then back to rest in about 300 ms */
    tbf_step(&f, 100, 0);
    CHECK(f.level == 255 && tbf_next(&f) == TBF_GRACE_MS);
    tbf_step(&f, 100, 0);
    CHECK(f.level == 255 && tbf_next(&f) == TBF_GRACE_MS - 100);
    tbf_step(&f, 200, 0);
    CHECK(f.level == 255);                                 /* exactly the grace: not yet */
    CHECK(tbf_next(&f) == TBF_STEP_MS);
    prev = f.level;
    for (t = 0, i = 0; f.level > rest && i < 30; i++) {
        lvl = tbf_step(&f, TBF_STEP_MS, 0);
        CHECK(lvl < prev);
        prev = lvl;
        t += TBF_STEP_MS;
    }
    CHECK(f.level == rest && t >= 280 && t <= 320);
    CHECK(tbf_next(&f) == 0);

    /* a gap that crosses the grace counts only its part after the grace towards the fade out */
    tbf_init(&f, rest, 1);
    tbf_step(&f, 0, 0);
    tbf_step(&f, 280, 0);
    CHECK(f.level == 255);
    tbf_step(&f, 40, 0);                                   /* 20 ms of grace, 20 of fading */
    CHECK(f.level < 255 && f.level >= 255 - (255 - rest) * 30 / TBF_OUT_MS);

    /* back while fading out: turns round from where it is, no jump */
    tbf_init(&f, rest, 1);
    tbf_step(&f, 0, 0);
    tbf_step(&f, TBF_GRACE_MS, 0);
    tbf_step(&f, 120, 0);
    lvl = f.level;
    CHECK(lvl < 255 && lvl > rest);
    tbf_step(&f, 0, 1);
    CHECK(f.level == lvl);
    tbf_step(&f, TBF_STEP_MS, 1);
    CHECK(f.level > lvl && f.level - lvl <= (255 - rest) * TBF_STEP_MS / TBF_IN_MS + 1);
    CHECK(run(&f, TBF_STEP_MS, 1, 255, 200) >= 0);

    /* gone while fading in: it finishes going solid within the grace, then fades out */
    tbf_init(&f, rest, 0);
    tbf_step(&f, 500, 1);                                  /* the wait before the step that sees it isn't hot time */
    CHECK(f.level == rest);
    tbf_step(&f, TBF_STEP_MS, 1);
    CHECK(f.level > rest && f.level < 255);
    CHECK(run(&f, TBF_STEP_MS, 0, 255, TBF_GRACE_MS) >= 0);
    CHECK(run(&f, TBF_STEP_MS, 0, rest, TBF_GRACE_MS + TBF_OUT_MS + 2 * TBF_STEP_MS) >= 0);

    /* a menu (hot, the pointer elsewhere) keeps it solid however long */
    tbf_init(&f, rest, 1);
    for (i = 0; i < 100; i++) tbf_step(&f, 100, 1);
    CHECK(f.level == 255);

    /* a long gap (the task held up) counts as at most TBF_DT_MAX; a negative one as none */
    tbf_init(&f, rest, 1);
    tbf_step(&f, 0, 0);
    tbf_step(&f, 100000, 0);
    CHECK(f.level == rest);
    tbf_init(&f, rest, 0);
    tbf_step(&f, -50, 1);
    CHECK(f.level == rest);

    /* never outside rest..255, whatever comes */
    tbf_init(&f, rest, 0);
    {
        unsigned r = 12345;
        int ok = 1;
        for (i = 0; i < 20000; i++) {
            r = r * 1103515245u + 12345u;
            tbf_step(&f, (int)(r >> 8) % 700 - 50, (r >> 20) & 1);
            if (f.level < rest || f.level > 255 || f.cool < 0 || f.cool > TBF_GRACE_MS) ok = 0;
        }
        CHECK(ok);
    }

    /* opacity 100% (rest 255): always solid, nothing to wake for; 10% (the least): fades as well */
    tbf_init(&f, 255, 0);
    tbf_step(&f, 40, 1); tbf_step(&f, 1000, 0);
    CHECK(f.level == 255 && tbf_next(&f) == 0);
    tbf_init(&f, 10 * 255 / 100, 0);
    CHECK(run(&f, TBF_STEP_MS, 1, 255, 200) >= 0);

    /* GFOpenTitle's loop over a hover: how many wakes the bar adds. The loop wakes every 100 ms while the pointer is
     * on the bar and every 500 ms otherwise (as before 0.7.5), sooner only when tbf_next asks. The pointer is on
     * the bar from 1000 ms to 2000 ms. */
    tbf_init(&f, rest, 0);
    for (t = 0, ms = 0, wakes = 0; t < 5000; t += ms) {
        int hot = t >= 1000 && t < 2000, n, plain;
        tbf_step(&f, ms, hot);                             /* each wake: the time since the last, and the pointer now */
        plain = hot ? 100 : 500;
        n = tbf_next(&f);
        ms = n > 0 && n < plain ? (n < 20 ? 20 : n) : plain;
        if (ms != plain) wakes++;
        if (t < 1000 || t >= 4000) CHECK(ms == plain);    /* at rest: the loop's own pace */
    }
    printf("     a hover of 1 s adds %d wakes to the loop's own (the fade's steps, the grace's end); none at rest\n", wakes);
    CHECK(wakes > 0 && wakes <= 16);
    CHECK(f.level == rest && tbf_next(&f) == 0);
}

/* tbp_on_bar (GFOpenTitle 0.7.7): the pointer where the boot left it, on the bar, doesn't make the bar solid until it
 * has moved; then it counts as before */
static void pointer_tests(void)
{
    tb_ptr p = { 0, 0, 0, 0 };
    tb_fade f;
    const int rest = 40 * 255 / 100;
    int i;
    /* Instance-32 after a boot: the pointer at 0,0, on the 24-row bar, never moved */
    CHECK(!tbp_on_bar(&p, 0, 0, 1));                      /* the first look */
    for (i = 0; i < 50; i++) CHECK(!tbp_on_bar(&p, 0, 0, 1));
    /* the fade with it: the bar stays at rest */
    tbf_init(&f, rest, tbp_on_bar(&p, 0, 0, 1));
    for (i = 0; i < 20; i++) tbf_step(&f, 500, tbp_on_bar(&p, 0, 0, 1));
    CHECK(f.level == rest && tbf_next(&f) == 0);
    /* moved off the bar: still not on it */
    CHECK(!tbp_on_bar(&p, 900, 700, 0));
    /* back onto the bar: now it counts, and the bar goes solid */
    CHECK(tbp_on_bar(&p, 600, 10, 1));
    tbf_step(&f, 0, 1);
    for (i = 0; i < 10; i++) tbf_step(&f, TBF_STEP_MS, tbp_on_bar(&p, 600, 10, 1));
    CHECK(f.level == 255);
    /* even back at 0,0, now that it has moved */
    CHECK(tbp_on_bar(&p, 0, 0, 1));
    /* and off again: no longer, and the bar goes back to rest */
    CHECK(!tbp_on_bar(&p, 900, 700, 0));
    for (i = 0; i < 40; i++) tbf_step(&f, TBF_STEP_MS, tbp_on_bar(&p, 900, 700, 0));
    CHECK(f.level == rest);

    /* a move along the bar alone (x only) counts as a move */
    p = (tb_ptr){ 0, 0, 0, 0 };
    CHECK(!tbp_on_bar(&p, 0, 0, 1));
    CHECK(tbp_on_bar(&p, 1, 0, 1));
    /* so does one in y only */
    p = (tb_ptr){ 0, 0, 0, 0 };
    CHECK(!tbp_on_bar(&p, 5, 5, 1));
    CHECK(tbp_on_bar(&p, 5, 6, 1));
    /* a pointer first seen off the bar that is moved onto it counts at once */
    p = (tb_ptr){ 0, 0, 0, 0 };
    CHECK(!tbp_on_bar(&p, 960, 600, 0));
    CHECK(tbp_on_bar(&p, 960, 10, 1));
}

/* 0.9: the bar's memory and time (CM4 item 17) */
static void rect_tests(void)
{
    struct tb_rect r = { 0, 0, 0, 0 };
    CHECK(!tb_rect_hit(&r, 0, 0));                     /* nothing published: nothing taken */
    tb_rect_set(&r, 1800, 0, 60, 24);
    CHECK(tb_rect_hit(&r, 1800, 0) && tb_rect_hit(&r, 1859, 23) && tb_rect_hit(&r, 1830, 11));
    CHECK(!tb_rect_hit(&r, 1799, 11) && !tb_rect_hit(&r, 1860, 11) && !tb_rect_hit(&r, 1830, 24) && !tb_rect_hit(&r, 1830, -1));
    tb_rect_set(&r, 0, 0, 0, 0);                       /* cleared before the slot closes */
    CHECK(!tb_rect_hit(&r, 1830, 11) && !tb_rect_hit(&r, 0, 0));
    tb_rect_set(&r, 10, 0, 40, 0);                     /* no height: off */
    CHECK(!tb_rect_hit(&r, 20, 0));
    tb_rect_set(&r, 10, 0, -5, 20);
    CHECK(!tb_rect_hit(&r, 10, 5));
}

static void format_tests(void)
{
    char t[64];
    tb_size(3ULL << 30, t, sizeof t); CHECK(!strcmp(t, "3 GB"));
    tb_size(3113851290ULL, t, sizeof t); CHECK(!strcmp(t, "2.9 GB"));
    tb_size(314573, t, sizeof t); CHECK(!strcmp(t, "0.3 MB"));
    tb_size(150ULL << 20, t, sizeof t); CHECK(!strcmp(t, "0.15 GB"));
    tb_size(5ULL << 20, t, sizeof t); CHECK(!strcmp(t, "5 MB"));
    tb_size(57ULL << 20, t, sizeof t); CHECK(!strcmp(t, "57 MB"));
    tb_size(2621440, t, sizeof t); CHECK(!strcmp(t, "2.5 MB"));
    tb_size(512ULL << 10, t, sizeof t); CHECK(!strcmp(t, "0.5 MB"));
    tb_size(20ULL << 10, t, sizeof t); CHECK(!strcmp(t, "20 KB"));
    tb_size(0, t, sizeof t); CHECK(!strcmp(t, "0 KB"));
    tb_size(1000ULL << 20, t, sizeof t); CHECK(!strcmp(t, "0.98 GB"));
    tb_memory(3ULL << 30, 314573, t, sizeof t); CHECK(!strcmp(t, "3 GB RAM (0.3 MB in use)"));
    tb_clock(11, 12, 10, 59, 1, t, sizeof t); CHECK(!strcmp(t, "Nov 12 1059"));
    tb_clock(1, 3, 0, 5, 1, t, sizeof t); CHECK(!strcmp(t, "Jan 3 0005"));
    tb_clock(11, 12, 10, 59, 0, t, sizeof t); CHECK(!strcmp(t, "1059"));
    tb_zeros("Nov 12 1059", t, sizeof t); CHECK(!strcmp(t, "Nov 00 0000"));
}

/* the boxes' pixels in a grid, each counted */
#define GW 32
static int grid[GW][GW];
static int paint(const struct tb_box *b, int n)
{
    int i, x, y, overlap = 0;
    memset(grid, 0, sizeof grid);
    for (i = 0; i < n; i++)
        for (y = b[i].y; y < b[i].y + b[i].h; y++)
            for (x = b[i].x; x < b[i].x + b[i].w; x++)
                if (x >= 0 && x < GW && y >= 0 && y < GW && grid[y][x]++) overlap = 1;
    return overlap;
}
static int lit(int x, int y) { return x >= 0 && x < GW && y >= 0 && y < GW && grid[y][x]; }
static int count(void)
{
    int x, y, c = 0;
    for (y = 0; y < GW; y++) for (x = 0; x < GW; x++) c += grid[y][x] != 0;
    return c;
}

static void shapes_tests(void)
{
    static const char *const g[4] = { ".##.", "####", "#..#", "..##" };
    struct tb_box b[32];
    int n, x, y, ok;
    /* a glyph: one box per run, each one row high, where the '#'s are */
    n = tb_glyph_boxes(g, 4, 3, 2, 99, b, 32);
    CHECK(n == 5 && !paint(b, n) && count() == 10);
    for (ok = 1, y = 0; y < 4; y++) for (x = 0; x < 4; x++) ok &= lit(3 + x, 2 + y) == (g[y][x] == '#');
    CHECK(ok);
    for (ok = 1, x = 0; x < n; x++) ok &= b[x].h == 1;
    CHECK(ok);
    /* rows from h on are cut off; no room past max */
    n = tb_glyph_boxes(g, 4, 0, 1, 3, b, 32);
    CHECK(n == 2 && !paint(b, n) && count() == 6 && !lit(0, 3));
    CHECK(tb_glyph_boxes(g, 4, 0, 0, 99, b, 2) == 2);
    /* the rounded box: as 0.9.1's Move/Draw/WritePixel drew it, size 9 at 4,5 */
    n = tb_rbox_boxes(4, 5, 9, b, 32);
    CHECK(n == 8 && !paint(b, n));
    for (ok = 1, y = 0; y < 9; y++)
        for (x = 0; x < 9; x++) {
            int edge = (y == 0 || y == 8) ? (x >= 2 && x <= 6) : (x == 0 || x == 8) ? (y >= 2 && y <= 6) : 0;
            int corner = (x == 1 || x == 7) && (y == 1 || y == 7);
            ok &= lit(4 + x, 5 + y) == (edge || corner);
        }
    CHECK(ok && count() == 24);
    CHECK(tb_rbox_boxes(0, 0, 4, b, 32) == 4);              /* too small for sides: only the corners */
    /* the tick: two strokes a row apart, its corner at (4, cy0 + 2), from (2, cy0) to (8, cy0 - 3) */
    n = tb_tick_boxes(10, b, 32);
    paint(b, n);
    CHECK(n == 16 && count() == 15);              /* the strokes meet at (6, cy0) */
    CHECK(lit(2, 10) && lit(3, 11) && lit(4, 12) && lit(8, 7) && lit(2, 11) && lit(4, 13) && lit(8, 8));
    CHECK(!lit(1, 10) && !lit(9, 7) && !lit(4, 14) && !lit(8, 6));
    /* each stroke joined: every pixel but the ends has a neighbour row above and below */
    for (ok = 1, y = 8; y <= 11; y++) {
        int any = 0;
        for (x = 0; x < GW; x++) any |= lit(x, y);
        ok &= any;
    }
    CHECK(ok);
}

/* 0.11: whose menu, which broker, which letters (tb_owner.h) */
static void owner_tests(void)
{
    int a, b, c;
    const void *tasks[3] = { &a, &b, NULL };
    /* tbo_owner(cog, cx, ts): the cog first, then a letter, then a picture item */
    CHECK(tbo_owner(1, 2, 3) == 1 && tbo_owner(2, -1, -1) == 2);
    CHECK(tbo_owner(0, 2, 3) == TBO_CX + 2);
    CHECK(tbo_owner(0, -1, 3) == TBO_TS + 3 && tbo_owner(0, -1, 0) == TBO_TS);
    CHECK(tbo_owner(0, -1, -1) == 0);
    CHECK(TBO_TS + 8 <= TBO_CX);                                /* the two ranges never meet (TS_ICONS is 8) */
    /* tbo_broker_of: the program's broker, none for a program without one or no task */
    CHECK(tbo_broker_of(tasks, 3, &b) == 1 && tbo_broker_of(tasks, 3, &a) == 0);
    CHECK(tbo_broker_of(tasks, 3, &c) == -1 && tbo_broker_of(tasks, 3, NULL) == -1 && tbo_broker_of(tasks, 0, &a) == -1);
    /* tbo_in_strip(ours, active, has_icons) */
    CHECK(tbo_in_strip(0, 1, 0));                               /* ClickToFront, active: a letter */
    CHECK(!tbo_in_strip(0, 0, 0));                              /* disabled: off the bar, in All commodities... */
    CHECK(!tbo_in_strip(0, 1, 1));                              /* GFOpenSpeaker: its own picture instead */
    CHECK(!tbo_in_strip(1, 1, 0));                              /* GFOpenTitle's own broker */
}

/* 0.10: gfopensysbar.library's items in the row (tb_sbsync.h) */
static void sbsync_tests(void)
{
    /* tbs_plan(slot, slot_is_lib, free_slot, has_picture, same_place, same_picture) */
    CHECK(tbs_plan(-1, 0, 1, 1, 0, 0) == TBS_NEW);              /* a new item, a slot free */
    CHECK(tbs_plan(-1, 0, 0, 1, 0, 0) == TBS_FULL);             /* every slot taken */
    CHECK(tbs_plan(-1, 0, 1, 0, 0, 0) == TBS_SKIP);             /* a text item (no picture) isn't shown yet */
    CHECK(tbs_plan(3, 0, 1, 1, 1, 1) == TBS_SKIP);              /* the taskspace's icon of that name stays */
    CHECK(tbs_plan(3, 1, 0, 1, 1, 1) == TBS_KEEP);              /* unchanged, even with no slot free */
    CHECK(tbs_plan(3, 1, 0, 1, 1, 0) == TBS_REDRAW);            /* new pixels in the same place */
    CHECK(tbs_plan(3, 1, 0, 1, 0, 1) == TBS_MOVE);              /* a new width or order */
    CHECK(tbs_plan(3, 1, 0, 1, 0, 0) == TBS_MOVE);              /* both: moved, which draws the new picture too */
    CHECK(tbs_plan(3, 1, 1, 0, 1, 1) == TBS_SKIP);              /* pixels taken away: not shown, and closed */
    /* taskspace events as the library's: TSE_CLICK 1 -> SBE_CLICK 1, TSE_PLACE 3 -> SBE_PLACE 3, TSE_GONE 2 -> none */
    CHECK(tbs_event(1) == 1 && tbs_event(3) == 3 && tbs_event(2) == 0 && tbs_event(0) == 0);
    /* tbs_after(is_lib, seen, has_win): the windows after the lock */
    CHECK(tbs_after(0, 0, 1) == TBA_NONE);                      /* a taskspace icon isn't the library's to drop */
    CHECK(tbs_after(1, 0, 1) == TBA_DROP);                      /* gone from the list */
    CHECK(tbs_after(1, 1, 0) == TBA_OPEN && tbs_after(1, 2, 0) == TBA_OPEN);   /* new: opened */
    CHECK(tbs_after(1, 2, 1) == TBA_REOPEN);                    /* moved: closed and opened where it now goes */
    CHECK(tbs_after(1, 1, 1) == TBA_DRAW);                      /* kept: drawn (ts_draw skips an unchanged key) */
    /* removed and added again under the same name between two reads: the plan is KEEP, and the icon takes the new
     * item and task (Marcus's review: the old pointer was kept, clicks went to a freed item) */
    {
        int old_item, new_item, old_task, new_task;
        void *sbi = &old_item, *task = &old_task;
        CHECK(tbs_plan(3, 1, 0, 1, 1, 1) == TBS_KEEP);
        CHECK(tbs_rebind(&sbi, &task, &new_item, &new_task) == 1 && sbi == &new_item && task == &new_task);
        CHECK(tbs_rebind(&sbi, &task, &old_item, &new_task) == 0 && sbi == &old_item && task == &new_task);
    }
}

int main(void)
{
    prefs_tests();
    fade_tests();
    pointer_tests();
    format_tests();
    rect_tests();
    shapes_tests();
    sbsync_tests();
    owner_tests();
    printf("%s   %d checks%s\n", failed ? "FAIL" : "ok", checks, failed ? ", some failed" : " (settings, fade, pointer, words, shapes, system bar items)");
    return failed != 0;
}
