/* GFOpenWindows' places (src/places.c) on the host: the keys, the placing rule,
 * the table, and the tags a window with a kept place opens with, read as
 * GFOpenIntuition reads them (a tag's last wins) and as an Intuition that reads
 * a tag's first would. Each kind of opener the lab tried on 24 October 2026
 * is here: tags (most programs, GFOpenBench's drawers), a NewWindow, a NewWindow
 * with tags, an ExtNewWindow, by inner size (the Shell's CON: window), with
 * no size gadget, and tag lists chained with TAG_MORE and TAG_SKIP.
 *   Windows/tests/run.sh
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "../src/places.h"

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* ---- two Intuitions ---- */

struct box { short l, t, w, h; };
#define BL 4                                /* the borders an inner size is given inside */
#define BR 18
#define BT 24
#define BB 4

static const struct TagItem *walk(const struct TagItem **t)
{
    while (*t) {
        const struct TagItem *x = *t;
        switch (x->ti_Tag) {
        case TAG_DONE: *t = NULL; return NULL;
        case TAG_IGNORE: *t = x + 1; break;
        case TAG_MORE: *t = (const struct TagItem *)x->ti_Data; break;
        case TAG_SKIP: *t = x + 1 + x->ti_Data; break;
        default: *t = x + 1; return x;
        }
    }
    return NULL;
}

/* GFOpenIntuition (window_core.c, read_tags): the NewWindow, then its Extension, then the tags, each tag as it
 * comes, so a tag's last wins; WA_Width after WA_InnerWidth wins, and the other way round */
static void open_last(const struct box *nw, const struct TagItem *ext, const struct TagItem *tags, struct box *out)
{
    const struct TagItem *lists[2], *t, *x;
    long iw = -1, ih = -1;
    int k;
    struct box b = { 0, 0, -1, -1 };
    if (nw) b = *nw;
    lists[0] = ext; lists[1] = tags;
    for (k = 0; k < 2; k++)
        for (t = lists[k]; (x = walk(&t)); ) {
            if (x->ti_Tag == WA_Left) b.l = (short)x->ti_Data;
            else if (x->ti_Tag == WA_Top) b.t = (short)x->ti_Data;
            else if (x->ti_Tag == WA_Width) { b.w = (short)x->ti_Data; iw = -1; }
            else if (x->ti_Tag == WA_Height) { b.h = (short)x->ti_Data; ih = -1; }
            else if (x->ti_Tag == WA_InnerWidth) iw = (long)x->ti_Data;
            else if (x->ti_Tag == WA_InnerHeight) ih = (long)x->ti_Data;
        }
    if (iw >= 0) b.w = (short)(iw + BL + BR);
    if (ih >= 0) b.h = (short)(ih + BT + BB);
    *out = b;
}

static const struct TagItem *first(const struct TagItem *list, unsigned long tag)
{
    const struct TagItem *t = list, *x;
    while ((x = walk(&t))) if (x->ti_Tag == tag) return x;
    return NULL;
}

/* An Intuition that reads a tag's first (FindTagItem), the tags before the Extension, either before the
 * NewWindow; an inner size, when given, wins */
static void open_first(const struct box *nw, const struct TagItem *ext, const struct TagItem *tags, struct box *out)
{
    static const unsigned long tag[6] = { WA_Left, WA_Top, WA_Width, WA_Height, WA_InnerWidth, WA_InnerHeight };
    long v[6];
    int k;
    struct box b = { 0, 0, -1, -1 };
    if (nw) b = *nw;
    v[0] = b.l; v[1] = b.t; v[2] = b.w; v[3] = b.h; v[4] = v[5] = -1;
    for (k = 0; k < 6; k++) {
        const struct TagItem *x = first(tags, tag[k]);
        if (!x) x = first(ext, tag[k]);
        if (x) v[k] = (long)x->ti_Data;
    }
    b.l = (short)v[0]; b.t = (short)v[1]; b.w = (short)v[2]; b.h = (short)v[3];
    if (v[4] >= 0) b.w = (short)(v[4] + BL + BR);
    if (v[5] >= 0) b.h = (short)(v[5] + BT + BB);
    *out = b;
}

/* ---- the patch, as ow_open_window does it ---- */

struct opening {
    const char *what;
    struct box nw; int has_nw;              /* a NewWindow, and its box */
    const struct TagItem *ext;              /* its ExtNewWindow's Extension */
    const struct TagItem *tags;
    struct place_ask ask;
};

/* What each Intuition makes of the opening with the plan applied: copies of the tags and the Extension, the
 * NewWindow's box changed in a copy */
static void open_planned(const struct opening *o, const struct place_plan *p, struct box *last, struct box *firstb)
{
    struct TagItem tags[64], ext[64];
    struct box nw = o->nw;
    int n = place_tags(tags, 64, p, o->tags, 1), m = o->ext ? place_tags(ext, 64, p, o->ext, 0) : 0;
    CHECK(n >= 2 && m >= 0, "%s: the tags made (%d %d)", o->what, n, m);
    if (o->has_nw) { nw.l = p->l; nw.t = p->t; if (p->size) { nw.w = p->w; nw.h = p->h; } }
    open_last(o->has_nw ? &nw : NULL, o->ext ? ext : NULL, tags, last);
    open_first(o->has_nw ? &nw : NULL, o->ext ? ext : NULL, tags, firstb);
}

static void check_opening(const struct opening *o, const struct place *pl, short sw, short sh, struct box want)
{
    struct place_plan p;
    struct box a, b;
    place_plan(pl, &o->ask, sw, sh, &p);
    open_planned(o, &p, &a, &b);
    CHECK(a.l == want.l && a.t == want.t && a.w == want.w && a.h == want.h,
          "%s, a tag's last (GFOpenIntuition): %d %d %d %d, wanted %d %d %d %d", o->what, a.l, a.t, a.w, a.h, want.l, want.t, want.w, want.h);
    CHECK(b.l == want.l && b.t == want.t && b.w == want.w && b.h == want.h,
          "%s, a tag's first: %d %d %d %d, wanted %d %d %d %d", o->what, b.l, b.t, b.w, b.h, want.l, want.t, want.w, want.h);
}

#define T(tag, v) { (tag), (unsigned long)(v) }

int main(void)
{
    char key[64];
    struct place pl, table[PLACES_MAX];
    int n, i, order[PLACES_MAX];

    /* ---- keys ---- */
    place_key("AmigaShell", "CON", key, sizeof key);
    CHECK(!strcmp(key, "CON:AmigaShell"), "the Shell's window: %s", key);
    place_key("AmigaShell [3]", "CON", key, sizeof key);
    CHECK(!strcmp(key, "CON:AmigaShell"), "cut at a bracket: %s", key);
    place_key("Work: - 17 items", "GFOpenBench", key, sizeof key);
    CHECK(!strcmp(key, "GFOpenBench:Work: -"), "a drawer's count cut off: %s", key);
    place_key("Untitled 2 - GFOpenWrite", "GFOpenWrite", key, sizeof key);
    CHECK(!strcmp(key, "GFOpenWrite:Untitled"), "a digit cuts, the spaces before it go: %s", key);
    place_key("\"quoted\"", "P", key, sizeof key);
    CHECK(!strcmp(key, "P:"), "a quote cuts: %s", key);
    place_key(NULL, "window", key, sizeof key);
    CHECK(!strcmp(key, "window:"), "no title: %s", key);
    place_key("An exceedingly long window title that goes on", "Prog", key, sizeof key);
    CHECK(!strcmp(key, "Prog:An exceedingly long window tit"), "30 characters of a title: %s", key);
    place_key("GFOpenPlay \xb7 Boing.mod", "GFOpenPlay", key, sizeof key);
    CHECK(!strcmp(key, "GFOpenPlay:GFOpenPlay \xb7 Boing.mod"), "Latin-1 kept: %s", key);
    place_key("Title", "AVeryLongProgramNameThatFillsTheKeyAlmostEntirelyOnItsOwnXYZ", key, 16);
    CHECK(strlen(key) == 15, "the key is cut at its size: %s", key);

    CHECK(place_candidate(0, 1, NULL), "a drag bar and no title: kept");
    CHECK(place_candidate(0, 0, "Title"), "a title and no drag bar: kept");
    CHECK(!place_candidate(0, 0, ""), "no drag bar and an empty title: a popup, never kept (it was \"window:\")");
    CHECK(!place_candidate(0, 0, NULL), "no drag bar, no title: never");
    CHECK(!place_candidate(1, 1, "Backdrop"), "a backdrop or GimmeZeroZero window: never");

    CHECK(place_workbench("Workbench", 0) && place_workbench("GFOpenBench", 0) && place_workbench("gfopenbench", 0),
          "Workbench's and GFOpenBench's windows are the Workbench's");
    CHECK(place_workbench("Anything", 1), "WFLG_WBENCHWINDOW is the Workbench's");
    CHECK(!place_workbench("CON", 0) && !place_workbench("GFOpenBenchmark", 0), "others are not");

    /* ---- the placing rule ---- */
    memset(&pl, 0, sizeof pl);
    strcpy(pl.key, "CON:AmigaShell"); pl.l = 145; pl.t = 523; pl.w = 853; pl.h = 421;
    {
        short l, t, w = 640, h = 200;
        fit_place(&pl, 1, 64, 24, (short)0xffff, (short)0xffff, 1920, 1200, &l, &t, &w, &h);
        CHECK(l == 145 && t == 523 && w == 853 && h == 421, "sizeable: the kept corner and size (%d %d %d %d)", l, t, w, h);
        w = 640; h = 200;
        fit_place(&pl, 0, 64, 24, 0, 0, 1920, 1200, &l, &t, &w, &h);
        CHECK(l == 145 && t == 523 && w == 640 && h == 200, "no size gadget: the corner only (%d %d %d %d)", l, t, w, h);
        w = 640; h = 200;
        fit_place(&pl, 1, 900, 24, 0, 0, 1920, 1200, &l, &t, &w, &h);
        CHECK(w == 640 && h == 200, "a kept size under its minimum: not given (%d %d)", w, h);
        w = 640; h = 200;
        fit_place(&pl, 1, 64, 24, 800, 0, 1920, 1200, &l, &t, &w, &h);
        CHECK(w == 640, "over its maximum: not given (%d)", w);
        w = 640; h = 200;
        fit_place(&pl, 1, 64, 24, 0, 0, 800, 600, &l, &t, &w, &h);
        CHECK(l == 0 && t == 179 && w == 800 && h == 421, "a smaller screen: kept on it (%d %d %d %d)", l, t, w, h);
    }

    /* ---- the openers ---- */
    {
        static const struct TagItem prog_tags[] = {          /* most programs; GFOpenBench's drawers; the CON: handler */
            T(WA_Left, 0), T(WA_Top, 50), T(WA_Width, 640), T(WA_Height, 130), T(WA_Title, 0),
            T(WA_MinWidth, 64), T(WA_MinHeight, 24), T(WA_MaxWidth, ~0UL), T(WA_MaxHeight, ~0UL), T(TAG_DONE, 0) };
        static const struct TagItem inner_tags[] = {         /* the CON: handler from guest #472: no size asked */
            T(WA_Left, 0), T(WA_Top, 0), T(WA_InnerWidth, 640), T(WA_InnerHeight, 312),
            T(WA_MinWidth, 64), T(WA_MinHeight, 24), T(WA_MaxWidth, ~0UL), T(WA_MaxHeight, ~0UL), T(TAG_DONE, 0) };
        static const struct TagItem corner_tags[] = { T(WA_Left, 100), T(WA_Top, 100), T(TAG_DONE, 0) };
        static const struct TagItem tail[] = { T(WA_Top, 77), T(TAG_IGNORE, 5), T(WA_Height, 99), T(TAG_DONE, 0) };
        static struct TagItem chained[] = {                  /* a TAG_SKIP over a WA_Left, then TAG_MORE on */
            T(WA_Left, 10), T(TAG_SKIP, 1), T(WA_Left, 999), T(WA_Width, 300), T(TAG_MORE, 0) };
        struct opening o;
        chained[4].ti_Data = (unsigned long)tail;

        memset(&o, 0, sizeof o);
        o.what = "tags (a program, a drawer, the Shell)"; o.tags = prog_tags;
        o.ask.w = 640; o.ask.h = 130; o.ask.sizeable = 1; o.ask.minw = 64; o.ask.minh = 24; o.ask.maxw = o.ask.maxh = (short)0xffff;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });
        {   /* 0.9.1's way, ours in front of the program's: right only where a tag's first wins */
            struct TagItem old[16];
            struct box a, b;
            old[0] = (struct TagItem)T(WA_Left, 145); old[1] = (struct TagItem)T(WA_Top, 523);
            old[2] = (struct TagItem)T(WA_Width, 853); old[3] = (struct TagItem)T(WA_Height, 421);
            old[4] = (struct TagItem)T(TAG_MORE, (unsigned long)prog_tags);
            open_last(NULL, NULL, old, &a);
            open_first(NULL, NULL, old, &b);
            CHECK(a.l == 0 && a.t == 50 && a.w == 640, "0.9.1 on GFOpenIntuition: the program's corner and size won (the jump)");
            CHECK(b.l == 145 && b.t == 523, "0.9.1 where a tag's first wins: placed");
        }

        o.what = "tags, no size gadget"; o.ask.sizeable = 0;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 640, 130 });

        memset(&o, 0, sizeof o);
        o.what = "by inner size (CON:, ReAction)"; o.tags = inner_tags;
        o.ask.inner = 1; o.ask.sizeable = 1; o.ask.minw = 64; o.ask.minh = 24; o.ask.maxw = o.ask.maxh = (short)0xffff;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });
        o.what = "by inner size, no size gadget: moved, its own size"; o.ask.sizeable = 0;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 640 + BL + BR, 312 + BT + BB });

        memset(&o, 0, sizeof o);
        o.what = "a NewWindow (OpenWindow)"; o.has_nw = 1; o.nw = (struct box){ 100, 100, 400, 300 };
        o.ask.w = 400; o.ask.h = 300; o.ask.sizeable = 1; o.ask.minw = 80; o.ask.minh = 50; o.ask.maxw = o.ask.maxh = (short)0xffff;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });
        o.what = "a NewWindow and tags"; o.tags = corner_tags;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });
        o.what = "an ExtNewWindow"; o.tags = NULL; o.ext = prog_tags;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });
        o.what = "an ExtNewWindow and tags"; o.tags = corner_tags;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });

        memset(&o, 0, sizeof o);
        o.what = "tags chained with TAG_SKIP and TAG_MORE"; o.tags = chained;
        o.ask.w = 300; o.ask.h = 99; o.ask.sizeable = 1; o.ask.maxw = o.ask.maxh = 0;
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 853, 421 });
        o.ask.sizeable = 0; o.what = "chained, no size gadget";
        check_opening(&o, &pl, 1920, 1200, (struct box){ 145, 523, 300, 99 });

        {   /* the copy: every tag of the program's but the placing ones, flattened, in order */
            struct TagItem out[16];
            struct place_plan p = { 1, 2, 3, 4, 0 };
            n = place_tags(out, 16, &p, chained, 1);
            CHECK(n == 4 && out[0].ti_Tag == WA_Left && out[1].ti_Tag == WA_Top && out[2].ti_Tag == WA_Width &&
                  out[2].ti_Data == 300 && out[3].ti_Tag == WA_Height && out[4].ti_Tag == TAG_DONE,
                  "plan, then the program's own without its corner (%d)", n);
            CHECK(place_tags(out, 4, &p, chained, 1) < 0, "too small a buffer: refused");
            CHECK(place_tags_count(chained) == 4 && place_tags_count(NULL) == 0, "counted through TAG_SKIP and TAG_MORE");
        }
        {   /* a TAG_MORE loop: refused, not followed for ever */
            static struct TagItem loop[2];
            loop[0] = (struct TagItem)T(WA_Left, 1);
            loop[1] = (struct TagItem)T(TAG_MORE, (unsigned long)loop);
            CHECK(place_tags_count(loop) < 0, "a list that loops: too long");
        }

        /* the kept corner off the screen's right: kept on it, with the size it will have */
        memset(&o, 0, sizeof o);
        o.what = "off the screen"; o.tags = inner_tags;
        o.ask.inner = 1; o.ask.sizeable = 0;
        {
            struct place far = pl;
            far.l = 1800; far.t = 1100;
            check_opening(&o, &far, 1920, 1200, (struct box){ 1920 - 853, 1200 - 421, 640 + BL + BR, 312 + BT + BB });
        }
    }

    /* ---- the table ---- */
    n = 0;
    for (i = 0; i < PLACES_MAX; i++) {
        char k[16];
        int x;
        snprintf(k, sizeof k, "p:%d", i);
        x = place_index(table, &n, k, 1);
        table[x].used = (unsigned long)(i + 10);
    }
    CHECK(n == PLACES_MAX, "full: %d", n);
    table[5].used = 1;                                      /* used longest ago */
    i = place_index(table, &n, "new:one", 1);
    CHECK(i == 5 && n == PLACES_MAX && !strcmp(table[5].key, "new:one"), "full: the one used longest ago makes way (%d)", i);
    CHECK(place_index(table, &n, "new:one", 0) == 5 && place_index(table, &n, "no:such", 0) == -1, "found, and not");

    CHECK(place_parse("\"CON:AmigaShell\" 145 523 853 421", &pl) && !strcmp(pl.key, "CON:AmigaShell") &&
          pl.l == 145 && pl.t == 523 && pl.w == 853 && pl.h == 421, "a line read");
    CHECK(place_parse("\"window:\" 0 638 1920 100", &pl) && !strcmp(pl.key, "window:"), "an empty title read");
    CHECK(!place_parse("; a comment", &pl) && !place_parse("\"bad\" 1 2 0 4", &pl) && !place_parse("\"open 1 2 3 4", &pl),
          "a comment, no size, no closing quote: not places");

    /* the file's order: used longest ago first, so reading it back (used = the line's number) keeps which came last */
    {
        struct place t3[4];
        memset(t3, 0, sizeof t3);
        t3[0].used = 40; t3[1].used = 7; t3[2].used = 40; t3[3].used = 1;
        place_order(t3, 4, order);
        CHECK(order[0] == 3 && order[1] == 1 && order[2] == 0 && order[3] == 2, "oldest first, ties as they were (%d %d %d %d)",
              order[0], order[1], order[2], order[3]);
    }

    /* ---- 0.10: snapped places ---- */
    CHECK(place_parse("\"wintest:Snap me\" 300 200 500 300 snap", &pl) && pl.snap && pl.l == 300 && pl.h == 300 &&
          !strcmp(pl.key, "wintest:Snap me"), "a snapped line read");
    CHECK(place_parse("\"CON:AmigaShell\" 145 523 853 421", &pl) && !pl.snap, "a line with no word: kept by the look, not snapped");
    /* the table full: a place the look kept makes way before a snapped one, even one used longer ago */
    for (i = 0; i < n; i++) table[i].snap = 0;
    table[5].used = 1; table[5].snap = 1;
    table[9].used = 2;
    i = place_index(table, &n, "new:two", 1);
    CHECK(i == 9 && strcmp(table[5].key, "new:two"), "full: the look's place used longest ago goes, the snapped one stays (%d)", i);
    for (i = 0; i < n; i++) table[i].snap = 1;
    table[7].used = 0;
    i = place_index(table, &n, "new:three", 1);
    CHECK(i == 7, "full of snapped places: the one used longest ago goes (%d)", i);
    {
        int m = n;
        int k = place_index(table, &m, "new:three", 0);
        place_remove(table, &m, k);
        CHECK(m == n - 1 && place_index(table, &m, "new:three", 0) == -1 && place_index(table, &m, "new:two", 0) >= 0,
              "forgotten (Shift-click): out of the table, the others kept");
        place_remove(table, &m, -1);
        place_remove(table, &m, m);
        CHECK(m == n - 1, "nothing out for an index that isn't one");
    }

    /* ---- 0.10: centring ---- */
    {
        struct place_area a;
        short l, t;
        place_area_init(&a, 1920, 1080, 23);
        CHECK(a.l == 0 && a.t == 24 && a.r == 1920 && a.b == 1080, "the area: the screen less its bar (%d %d %d %d)", a.l, a.t, a.r, a.b);
        place_centre(&a, 1920, 1080, 500, 300, 0, 0, 0, 0, &l, &t);
        CHECK(l == 710 && t == 24 + (1056 - 300) / 2, "centred in the area (%d %d)", l, t);
        /* GFOpenDock's shelf along the bottom (the lab: 460,991 1000 x 89) */
        place_area_reserve(&a, 1920, 1080, 460, 991, 1000, 89);
        CHECK(a.b == 991 && a.t == 24, "the dock's depth off the bottom (%d)", a.b);
        place_centre(&a, 1920, 1080, 860, 600, 0, 0, 0, 0, &l, &t);
        CHECK(l == 530 && t == 24 + (967 - 600) / 2, "a drawer centred above the dock (%d %d)", l, t);
        /* a popup near the dock and a window in the middle are not docks */
        {
            struct place_area b = a;
            place_area_reserve(&b, 1920, 1080, 900, 940, 120, 30);
            place_area_reserve(&b, 1920, 1080, 600, 400, 700, 300);
            CHECK(b.b == a.b && b.t == a.t && b.l == a.l && b.r == a.r, "a bubble and a window in the middle: no change");
        }
        /* docks on the left and the right, and one at the top */
        {
            struct place_area b;
            place_area_init(&b, 1920, 1080, 23);
            place_area_reserve(&b, 1920, 1080, 0, 200, 90, 600);
            place_area_reserve(&b, 1920, 1080, 1830, 200, 90, 600);
            CHECK(b.l == 90 && b.r == 1830, "side docks (%d %d)", b.l, b.r);
            place_area_init(&b, 1920, 1080, 23);
            place_area_reserve(&b, 1920, 1080, 400, 24, 1100, 80);
            CHECK(b.t == 104, "a dock under the bar (%d)", b.t);
            /* a full-width backdrop strip half the screen tall is not a dock */
            place_area_init(&b, 1920, 1080, 23);
            place_area_reserve(&b, 1920, 1080, 0, 500, 1920, 580);
            CHECK(b.b == 1080, "half the screen: not a dock (%d)", b.b);
        }
        /* bigger than the area: its top left, and never off the screen */
        place_area_init(&a, 640, 256, 10);
        place_centre(&a, 640, 256, 600, 250, 0, 0, 0, 0, &l, &t);
        CHECK(l == 20 && t == 6, "taller than the area: kept on the screen (%d %d)", l, t);
        place_centre(&a, 640, 256, 300, 100, 0, 0, 0, 0, &l, &t);
        CHECK(l == 170 && t == 11 + (245 - 100) / 2, "PAL: centred under the bar (%d %d)", l, t);
        /* over a parent window, kept in the area */
        place_area_init(&a, 1920, 1080, 23);
        place_centre(&a, 1920, 1080, 300, 120, 100, 120, 700, 500, &l, &t);
        CHECK(l == 300 && t == 310, "a dialog over its window (the lab's 300,310) (%d %d)", l, t);
        place_centre(&a, 1920, 1080, 300, 120, 1800, 1000, 100, 60, &l, &t);
        CHECK(l == 1620 && t == 960, "over a window at the corner: kept on the screen (%d %d)", l, t);
        place_centre(&a, 1920, 1080, 300, 120, 0, 0, 100, 40, &l, &t);
        CHECK(l == 0 && t == 24, "over a window at the top: kept under the bar (%d %d)", l, t);
        /* the rules */
        CHECK(place_too_big(&a, 1920, 1080, 1920, 1080) && place_too_big(&a, 1920, 1080, 800, 1080) &&
              place_too_big(&a, 1920, 1080, 0, 300) && !place_too_big(&a, 1920, 1080, 1919, 1000),
              "as big as the screen either way, or no size: left alone");
        CHECK(!place_asked(0, 0, 0, 23) && !place_asked(1, 0, 0, 23) && !place_asked(1, 0, 24, 23) && !place_asked(1, 0, 12, 10),
              "no corner, 0,0 and just under the bar: Intuition's default, centred");
        CHECK(!place_asked(1, 40, 33, 23) && !place_asked(1, 64, 88, 23) && !place_asked(1, 5, 0, 23),
              "a step in from the top left (GFOpenPrefs' 40,33): a default, centred");
        CHECK(place_asked(1, 300, 200, 23) && place_asked(1, 0, 120, 23) && place_asked(1, 65, 30, 23) && place_asked(1, 30, 89, 23) &&
              place_asked(1, 480, 180, 23) && place_asked(1, -4, 30, 23),
              "a corner worked out (MultiView's and ASL's centred 480,180, anywhere further in): the program's own, left there");
    }

    printf("test_places: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
