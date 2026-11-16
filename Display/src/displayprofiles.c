/* GFOpenDisplay: the display profiles (amicradle-teams designs/DISPLAY-PROFILES-20261113.md, section 4), one
 * section of GFOpenPrefs. GFOS chooses one of six profiles for each screen when it opens (by the mode's pixel
 * shape and width), and the profile gives the screen's fonts, gadget images and sizes. This page changes them.
 *
 *   GFOpenDisplay [FROM file] [USE] [SAVE]
 *
 *   - Profile: the six, the one of the screen this page is on first (marked "this screen")
 *   - for that profile: the screen font, the system default font and the icon font (a name and a size), the
 *     gadget image set, the title bar (pixels over the font), the frame and borders, and grey-level text
 *     edges; each with Default, the value GFOS ships
 *   - a preview of a window frame, a menu bar and a line of text in that profile, drawn through GFOpenLayout
 *
 * Save writes ENV:Sys/gfosdisplay.prefs and ENVARC:; Use writes ENV: only (GFOpenIntuition's prefs task reads
 * it at once: screens take a changed font or image set at their next open, and the Workbench is reset for it);
 * Cancel changes nothing. Every chunk and byte this page doesn't edit is kept as the file had it (gd_prefs.c).
 * GFOpenIntuition reads the screen font and the image set today; the other fields are saved for their steps.
 * The classic profile is per program only (Dale, 14 November 2026), so it isn't offered here.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/gadtools.h>
#include <graphics/displayinfo.h>
#include <graphics/modeid.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef LT_ONE
#include "looktabs.h"                       /* a page of GFOpenPrefs (LT_OP): its frame and sections */
#define LT_REFRESH(w) lt_draw((w), -1)
#else
#define LT_REFRESH(w) ((void)0)
#define LT_DIRTY 1
#define LT_DX 0
#define lt_note_to(f) ((void)0)
#endif

#include "gfopenlayout_draw.h"
#include "pf_iff.h"
#include "gd_prefs.h"

static const char version[] __attribute__((used)) = "$VER: GFOpenDisplay 0.1 (16.11.2026) GFOpenPrefs, MIT, Copyright (c) 2026 AmiCradle Team";

#define GD_ENV "ENV:Sys/gfosdisplay.prefs"
#define GD_ENVARC "ENVARC:Sys/gfosdisplay.prefs"

static pf_file file;
static gd_profile rows[GD_PROFILES];
static int cur;                             /* the profile shown, 0-5 */
static int here = GD_RTG_1080;              /* the profile of the screen the page is on, 1-6 */

static void load_file(const char *from)
{
    int r = pf_load(&file, from ? from : GD_ENV);
    if (r == 0 && !from) r = pf_load(&file, GD_ENVARC);
    if (r != 1) pf_new(&file);
    gd_get(&file, rows);
}

/* gfosdisplay.prefs into ENV:, and ENVARC: when save. 1 done. */
static int write_file(int save, void (*say)(const char *))
{
    if (!file.n && !pf_new(&file)) { say("Out of memory."); return 0; }
    if (!gd_put(&file, rows)) { say("Out of memory."); return 0; }
    if (save && !pf_save(&file, GD_ENVARC)) { say("Couldn't write ENVARC:Sys/gfosdisplay.prefs."); return 0; }
    if (!pf_save(&file, GD_ENV)) { say("Couldn't write ENV:Sys/gfosdisplay.prefs."); return 0; }
    return 1;
}

static void say_shell(const char *s) { Printf((STRPTR)"GFOpenDisplay: %s\n", (ULONG)s); }

/* ---- the window ------------------------------------------------------------------------------------ */

enum { G_PROFILE = 1, G_SNAME, G_SSIZE, G_SDEF, G_YNAME, G_YSIZE, G_YDEF, G_INAME, G_ISIZE, G_IDEF,
       G_SET, G_SETDEF, G_BAR, G_BARDEF, G_FRAME, G_BX, G_BY, G_FRAMEDEF, G_GREY, G_GREYDEF,
       G_STATUS, G_SAVE, G_USE, G_CANCEL, G_COUNT };

static struct Window *win;
static struct Gadget *gad[G_COUNT];
static char status_text[80];
static char profile_names[GD_PROFILES][32];
static STRPTR profile_labels[GD_PROFILES + 1];
static const char *const set_names[] = { "lores", "hires12", "medres", "large" };
static STRPTR set_labels[] = { (STRPTR)"Lores", (STRPTR)"Hires 1:2", (STRPTR)"Medres", (STRPTR)"Large", NULL };
static int px, py, pw, ph;                  /* the preview's box in the window (before LT_DX) */

static void status(const char *s)
{
    strncpy(status_text, s, sizeof status_text - 1);
    if (win && gad[G_STATUS]) GT_SetGadgetAttrs(gad[G_STATUS], win, NULL, GTTX_Text, (ULONG)status_text, TAG_DONE);
}

static void set2(int id, ULONG tag, ULONG v, ULONG tag2, ULONG v2)
{
    if (win && gad[id]) GT_SetGadgetAttrs(gad[id], win, NULL, tag, v, tag2, v2, TAG_DONE);
}

static LONG number(int id)
{
    LONG v = 0;
    if (gad[id]) GT_GetGadgetAttrs(gad[id], win, NULL, GTIN_Number, (ULONG)&v, TAG_DONE);
    return v;
}

static void text_of(int id, char *to, size_t max)
{
    STRPTR s = NULL;
    if (gad[id]) GT_GetGadgetAttrs(gad[id], win, NULL, GTST_String, (ULONG)&s, TAG_DONE);
    if (!s) s = (STRPTR)"";
    strncpy(to, (const char *)s, max - 1);
    to[max - 1] = 0;
}

static int set_index(const char *s)
{
    int k;
    for (k = 0; k < 4; k++) if (!strcmp(s, set_names[k])) return k;
    return 2;
}

static int clamp(LONG v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : (int)v; }

/* The profile's names in the cycle: the screen this page is on marked */
static void name_profiles(void)
{
    static const char *const names[GD_PROFILES] = { "1 Lores", "2 Hires 1:2", "3 Laced", "4 Graphics card, small",
                                                    "5 Graphics card, 1080p", "6 Graphics card, large" };
    int i;
    for (i = 0; i < GD_PROFILES; i++) {
        snprintf(profile_names[i], sizeof profile_names[i], "%s%s", names[i], i + 1 == here ? " (this screen)" : "");
        profile_labels[i] = (STRPTR)profile_names[i];
    }
    profile_labels[GD_PROFILES] = NULL;
}

/* ---- the preview: a window of this profile, through GFOpenLayout's oldraw --------------------------- */

static void preview(void)
{
    const gd_profile *p = &rows[cur];
    oldraw *d;
    oldraw_font sf, yf;
    oldraw_text_size ts = { 0, 8, 0 };
    oldraw_rect r, bar, in;
    int fh, bh, fr, bx, by, x = px + LT_DX;
    if (!win) return;
    if (!(d = oldraw_new_rastport(win->RPort, win->WScreen, win->Width, win->Height))) return;
    sf.family = p->screen.name[0] ? p->screen.name : NULL;
    sf.px = p->screen.name[0] ? p->screen.ysize : 0;
    sf.style = 0;
    yf.family = p->sys.name[0] ? p->sys.name : NULL;
    yf.px = p->sys.ysize;
    yf.style = 0;
    fh = oldraw_text_measure(d, "Ag", 2, &sf, 0, &ts) ? ts.ascent + ts.descent : 8;
    fr = p->frame > 0 ? p->frame : 1;
    bx = p->border_x;
    by = p->border_y;
    bh = fh + p->titlebar;
    r.x = x; r.y = py; r.w = pw; r.h = ph;
    oldraw_fill(d, &r, OLDRAW_PEN(OLDRAW_PEN_BACKGROUND));
    oldraw_frame(d, &r, 1);
    /* the screen's bar, a menu strip in it */
    bar.x = x + 1; bar.y = py + 1; bar.w = pw - 2; bar.h = bh;
    oldraw_fill(d, &bar, OLDRAW_PEN(OLDRAW_PEN_BARBLOCK));
    oldraw_text(d, bar.x + 6, bar.y + (bh - fh) / 2 + (fh - ts.descent), "Project   Edit   Settings", (size_t)-1, &sf,
                OLDRAW_PEN(OLDRAW_PEN_BARDETAIL), 0);
    /* a window: its frame, title bar and borders */
    r.x = x + 16; r.y = py + bh + 8; r.w = pw - 32; r.h = ph - bh - 16;
    if (r.w < 40 || r.h < bh + 2 * by + 4) { oldraw_free(d); return; }
    oldraw_fill(d, &r, OLDRAW_PEN(OLDRAW_PEN_SHADOW));
    in.x = r.x + fr - 1; in.y = r.y + fr - 1; in.w = r.w - 2 * (fr - 1); in.h = r.h - 2 * (fr - 1);
    oldraw_fill(d, &in, OLDRAW_PEN(OLDRAW_PEN_FILL));
    oldraw_frame(d, &in, 0);
    bar.x = in.x + 1; bar.y = in.y + 1; bar.w = in.w - 2; bar.h = bh;
    oldraw_text_box(d, &bar, "A window", (size_t)-1, &sf, OLDRAW_PEN(OLDRAW_PEN_FILLTEXT), 0, OLDRAW_CENTRE);
    in.x += bx; in.y += bh + 1; in.w -= 2 * bx; in.h -= bh + 1 + by;
    if (in.w > 4 && in.h > 4) {
        oldraw_fill(d, &in, OLDRAW_PEN(OLDRAW_PEN_BACKGROUND));
        oldraw_frame(d, &in, 1);
        in.x += 6; in.w -= 12;
        in.h = in.h / 2;
        oldraw_text_box(d, &in, "The quick brown fox: screen font", (size_t)-1, &sf, OLDRAW_PEN(OLDRAW_PEN_TEXT), 0, OLDRAW_LEFT);
        in.y += in.h;
        oldraw_text_box(d, &in, "1> dir SYS:  (system default font)", (size_t)-1, &yf, OLDRAW_PEN(OLDRAW_PEN_TEXT), 0,
                        OLDRAW_LEFT);
    }
    oldraw_flush(d);
    oldraw_free(d);
}

/* ---- the gadgets and the rows ----------------------------------------------------------------------- */

/* The shown profile's gadgets from its row; a kept field shows the shipped value, its gadgets off */
static void show(void)
{
    const gd_profile *p = &rows[cur];
    unsigned k = p->keep;
    set2(G_PROFILE, GTCY_Active, (ULONG)cur, TAG_IGNORE, 0);
    set2(G_SNAME, GTST_String, (ULONG)p->screen.name, GA_Disabled, (k & GD_KEEP_SCREENFONT) != 0);
    set2(G_SSIZE, GTIN_Number, (ULONG)p->screen.ysize, GA_Disabled, (k & GD_KEEP_SCREENFONT) != 0);
    set2(G_SDEF, GTCB_Checked, (k & GD_KEEP_SCREENFONT) != 0, TAG_IGNORE, 0);
    set2(G_YNAME, GTST_String, (ULONG)p->sys.name, GA_Disabled, (k & GD_KEEP_SYSFONT) != 0);
    set2(G_YSIZE, GTIN_Number, (ULONG)p->sys.ysize, GA_Disabled, (k & GD_KEEP_SYSFONT) != 0);
    set2(G_YDEF, GTCB_Checked, (k & GD_KEEP_SYSFONT) != 0, TAG_IGNORE, 0);
    set2(G_INAME, GTST_String, (ULONG)p->icon.name, GA_Disabled, (k & GD_KEEP_ICONFONT) != 0);
    set2(G_ISIZE, GTIN_Number, (ULONG)p->icon.ysize, GA_Disabled, (k & GD_KEEP_ICONFONT) != 0);
    set2(G_IDEF, GTCB_Checked, (k & GD_KEEP_ICONFONT) != 0, TAG_IGNORE, 0);
    set2(G_SET, GTCY_Active, (ULONG)set_index(p->set), GA_Disabled, (k & GD_KEEP_IMAGES) != 0);
    set2(G_SETDEF, GTCB_Checked, (k & GD_KEEP_IMAGES) != 0, TAG_IGNORE, 0);
    set2(G_BAR, GTIN_Number, (ULONG)p->titlebar, GA_Disabled, (k & GD_KEEP_TITLEBAR) != 0);
    set2(G_BARDEF, GTCB_Checked, (k & GD_KEEP_TITLEBAR) != 0, TAG_IGNORE, 0);
    set2(G_FRAME, GTIN_Number, (ULONG)p->frame, GA_Disabled, (k & GD_KEEP_FRAME) != 0);
    set2(G_BX, GTIN_Number, (ULONG)p->border_x, GA_Disabled, (k & GD_KEEP_FRAME) != 0);
    set2(G_BY, GTIN_Number, (ULONG)p->border_y, GA_Disabled, (k & GD_KEEP_FRAME) != 0);
    set2(G_FRAMEDEF, GTCB_Checked, (k & GD_KEEP_FRAME) != 0, TAG_IGNORE, 0);
    set2(G_GREY, GTCB_Checked, (ULONG)p->grey, GA_Disabled, (k & GD_KEEP_GREY) != 0);
    set2(G_GREYDEF, GTCB_Checked, (k & GD_KEEP_GREY) != 0, TAG_IGNORE, 0);
    preview();
}

/* The shown profile's text and number gadgets into its row (the fields not kept) */
static void take(void)
{
    gd_profile *p = &rows[cur];
    if (!win) return;
    if (!(p->keep & GD_KEEP_SCREENFONT)) {
        text_of(G_SNAME, p->screen.name, sizeof p->screen.name);
        p->screen.ysize = clamp(number(G_SSIZE), 5, 64);
    }
    if (!(p->keep & GD_KEEP_SYSFONT)) {
        text_of(G_YNAME, p->sys.name, sizeof p->sys.name);
        p->sys.ysize = clamp(number(G_YSIZE), 5, 64);
    }
    if (!(p->keep & GD_KEEP_ICONFONT)) {
        text_of(G_INAME, p->icon.name, sizeof p->icon.name);
        p->icon.ysize = clamp(number(G_ISIZE), 5, 64);
    }
    if (!(p->keep & GD_KEEP_TITLEBAR)) p->titlebar = clamp(number(G_BAR), 0, 32);
    if (!(p->keep & GD_KEEP_FRAME)) {
        p->frame = clamp(number(G_FRAME), 1, 8);
        p->border_x = clamp(number(G_BX), 0, 32);
        p->border_y = clamp(number(G_BY), 0, 32);
    }
}

/* A Default box: checked, the field is the shipped one again; cleared, it is the page's to change, from the
 * value it shows */
static void toggle_default(unsigned bit, int checked)
{
    gd_profile *p = &rows[cur], s;
    take();
    if (!checked) { p->keep &= ~bit; return; }
    gd_shipped(cur + 1, &s);
    p->keep |= bit;
    if (bit == GD_KEEP_SCREENFONT) p->screen = s.screen;
    if (bit == GD_KEEP_SYSFONT) p->sys = s.sys;
    if (bit == GD_KEEP_ICONFONT) p->icon = s.icon;
    if (bit == GD_KEEP_IMAGES) memcpy(p->set, s.set, sizeof p->set);
    if (bit == GD_KEEP_TITLEBAR) p->titlebar = s.titlebar;
    if (bit == GD_KEEP_FRAME) { p->frame = s.frame; p->border_x = s.border_x; p->border_y = s.border_y; }
    if (bit == GD_KEEP_GREY) p->grey = s.grey;
}

/* A font named without ".font" gets it, as diskfont wants */
static void font_suffix(gd_font *f)
{
    size_t n = strlen(f->name);
    if (n && (n < 5 || strcmp(f->name + n - 5, ".font")) && n + 5 < sizeof f->name) strcat(f->name, ".font");
}

/* What Use or Save does: the gadgets into the row, then the file. 1 done. */
static int put_in_place(int save)
{
    int i;
    take();
    for (i = 0; i < GD_PROFILES; i++) {
        gd_profile *p = &rows[i];
        if (!(p->keep & GD_KEEP_SCREENFONT)) { font_suffix(&p->screen); if (!p->screen.name[0]) p->keep |= GD_KEEP_SCREENFONT; }
        if (!(p->keep & GD_KEEP_SYSFONT)) { font_suffix(&p->sys); if (!p->sys.name[0]) p->keep |= GD_KEEP_SYSFONT; }
        if (!(p->keep & GD_KEEP_ICONFONT)) { font_suffix(&p->icon); if (!p->icon.name[0]) p->keep |= GD_KEEP_ICONFONT; }
    }
    return write_file(save, status);
}

/* The profile of the screen this page is on (gd_profile_of, as GFOpenIntuition chose it) */
static int profile_of_screen(struct Screen *scr)
{
    ULONG id = GetVPModeID(&scr->ViewPort);
    struct DisplayInfo di;
    unsigned rx = 0, ry = 0;
    if (id != (ULONG)INVALID_ID && GetDisplayInfoData(NULL, (UBYTE *)&di, sizeof di, DTAG_DISP, id) >= sizeof di - 16) {
        rx = (unsigned)di.Resolution.x;
        ry = (unsigned)di.Resolution.y;
    }
    return gd_profile_of(id, rx, ry, scr->Width, scr->Height);
}

static struct NewMenu menus[] = {
    { NM_TITLE, (STRPTR)"Project", 0, 0, 0, 0 },
    { NM_ITEM, (STRPTR)"Save", (STRPTR)"S", 0, 0, (APTR)1 },
    { NM_ITEM, (STRPTR)"Use", (STRPTR)"U", 0, 0, (APTR)2 },
    { NM_ITEM, (STRPTR)"Cancel", (STRPTR)"Q", 0, 0, (APTR)3 },
    { NM_END, 0, 0, 0, 0, 0 }
};

static int gui(void)
{
    struct Screen *scr = LockPubScreen(NULL);
    APTR vi = scr ? GetVisualInfoA(scr, NULL) : NULL;
    struct Gadget *glist = NULL, *g;
    struct NewGadget ng;
    struct Menu *menu = NULL;
    struct RastPort *rp;
    int fh, lh, top, row, quit = 0, rc = RETURN_OK, W, X, nmw, nw, cbw, dfx, lw, i;
    if (!vi) { if (scr) UnlockPubScreen(NULL, scr); return RETURN_FAIL; }
    memset(gad, 0, sizeof gad);
    rp = &scr->RastPort;
    here = profile_of_screen(scr);
    cur = here - 1;
    name_profiles();

    fh = scr->Font->ta_YSize;
    lh = fh + 6;
    top = scr->WBorTop + fh + 1 + 8;
    lw = TextLength(rp, (STRPTR)"System default font", 19) + 16;   /* the longest label */
    X = 12 + lw;
    nmw = TextLength(rp, (STRPTR)"GFOSSans.font__", 15) + 16;       /* a font's name */
    nw = TextLength(rp, (STRPTR)"0000", 4) + 16;                    /* a number */
    cbw = 26;                                                       /* a check box */
    dfx = X + nmw + 8 + nw + 12;                                    /* the Default boxes */
    W = dfx + cbw + TextLength(rp, (STRPTR)"Default", 7) + 8 + 12;
    i = X + 3 * (nw + 8) + 12;
    if (W < i) W = i;
    if (W < 12 + lw + TextLength(rp, (STRPTR)"6 Graphics card, large (this screen)", 36) + 40)
        W = 12 + lw + TextLength(rp, (STRPTR)"6 Graphics card, large (this screen)", 36) + 40;
    snprintf(status_text, sizeof status_text, "%s", "Change a profile, then Use or Save.");

    g = CreateContext(&glist);
    memset(&ng, 0, sizeof ng);
    ng.ng_TextAttr = scr->Font;
    ng.ng_VisualInfo = vi;
#define G(kind, id, x, y, w, h, text, flags, ...) \
    (ng.ng_LeftEdge = (x), ng.ng_TopEdge = (y), ng.ng_Width = (w), ng.ng_Height = (h), ng.ng_GadgetText = (STRPTR)(text), \
     ng.ng_Flags = (flags), ng.ng_GadgetID = (id), g = gad[id] = CreateGadget(kind, g, &ng, __VA_ARGS__, TAG_DONE))
#define DEF(id, on) G(CHECKBOX_KIND, id, dfx, row, cbw, lh, "Default", PLACETEXT_RIGHT, GTCB_Checked, (on), GTCB_Scaled, TRUE)
#ifdef LT_ONE
    lt_make(NULL, NULL, 10, top, W - 24, lh, LT_DISPLAY);     /* the frame's sections (GFOpenPrefs) */
    top += lt_space(scr->Font, W - 24, lh);
#endif
    row = top;
    {
        const gd_profile *p = &rows[cur];
        unsigned k = p->keep;
        G(CYCLE_KIND, G_PROFILE, X, row, W - 12 - X, lh, "Profile", PLACETEXT_LEFT, GTCY_Labels, (ULONG)profile_labels,
          GTCY_Active, cur);
        row += lh + 8;
        G(STRING_KIND, G_SNAME, X, row, nmw, lh, "Screen font", PLACETEXT_LEFT, GTST_String, (ULONG)p->screen.name,
          GTST_MaxChars, GD_NAME - 1, GA_Disabled, (k & GD_KEEP_SCREENFONT) != 0);
        G(INTEGER_KIND, G_SSIZE, X + nmw + 8, row, nw, lh, NULL, 0, GTIN_Number, p->screen.ysize, GTIN_MaxChars, 2,
          GA_Disabled, (k & GD_KEEP_SCREENFONT) != 0);
        DEF(G_SDEF, (k & GD_KEEP_SCREENFONT) != 0);
        row += lh + 4;
        G(STRING_KIND, G_YNAME, X, row, nmw, lh, "System default font", PLACETEXT_LEFT, GTST_String, (ULONG)p->sys.name,
          GTST_MaxChars, GD_NAME - 1, GA_Disabled, (k & GD_KEEP_SYSFONT) != 0);
        G(INTEGER_KIND, G_YSIZE, X + nmw + 8, row, nw, lh, NULL, 0, GTIN_Number, p->sys.ysize, GTIN_MaxChars, 2,
          GA_Disabled, (k & GD_KEEP_SYSFONT) != 0);
        DEF(G_YDEF, (k & GD_KEEP_SYSFONT) != 0);
        row += lh + 4;
        G(STRING_KIND, G_INAME, X, row, nmw, lh, "Icon font", PLACETEXT_LEFT, GTST_String, (ULONG)p->icon.name,
          GTST_MaxChars, GD_NAME - 1, GA_Disabled, (k & GD_KEEP_ICONFONT) != 0);
        G(INTEGER_KIND, G_ISIZE, X + nmw + 8, row, nw, lh, NULL, 0, GTIN_Number, p->icon.ysize, GTIN_MaxChars, 2,
          GA_Disabled, (k & GD_KEEP_ICONFONT) != 0);
        DEF(G_IDEF, (k & GD_KEEP_ICONFONT) != 0);
        row += lh + 8;
        G(CYCLE_KIND, G_SET, X, row, nmw + 8 + nw, lh, "Gadget images", PLACETEXT_LEFT, GTCY_Labels, (ULONG)set_labels,
          GTCY_Active, set_index(p->set), GA_Disabled, (k & GD_KEEP_IMAGES) != 0);
        DEF(G_SETDEF, (k & GD_KEEP_IMAGES) != 0);
        row += lh + 4;
        G(INTEGER_KIND, G_BAR, X, row, nw, lh, "Title bar over font", PLACETEXT_LEFT, GTIN_Number, p->titlebar,
          GTIN_MaxChars, 2, GA_Disabled, (k & GD_KEEP_TITLEBAR) != 0);
        DEF(G_BARDEF, (k & GD_KEEP_TITLEBAR) != 0);
        row += lh + 4;
        G(INTEGER_KIND, G_FRAME, X, row, nw, lh, "Frame, borders", PLACETEXT_LEFT, GTIN_Number, p->frame, GTIN_MaxChars, 1,
          GA_Disabled, (k & GD_KEEP_FRAME) != 0);
        G(INTEGER_KIND, G_BX, X + nw + 8, row, nw, lh, NULL, 0, GTIN_Number, p->border_x, GTIN_MaxChars, 2,
          GA_Disabled, (k & GD_KEEP_FRAME) != 0);
        G(INTEGER_KIND, G_BY, X + 2 * (nw + 8), row, nw, lh, NULL, 0, GTIN_Number, p->border_y, GTIN_MaxChars, 2,
          GA_Disabled, (k & GD_KEEP_FRAME) != 0);
        DEF(G_FRAMEDEF, (k & GD_KEEP_FRAME) != 0);
        row += lh + 4;
        G(CHECKBOX_KIND, G_GREY, X, row, cbw, lh, "Grey-level text edges", PLACETEXT_LEFT, GTCB_Checked, p->grey,
          GTCB_Scaled, TRUE, GA_Disabled, (k & GD_KEEP_GREY) != 0);
        DEF(G_GREYDEF, (k & GD_KEEP_GREY) != 0);
        row += lh + 10;
    }
    /* the preview's box: a screen bar and a window, tall enough for a 16-pixel font */
    px = 12; py = row; pw = W - 24; ph = 2 * (fh > 16 ? fh : 16) + 4 * 16 + 24;
    row += ph + 10;
    G(TEXT_KIND, G_STATUS, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)status_text, GTTX_Border, TRUE);
    row += lh + 6;
    {
        static const char *const names[3] = { "Save", "Use", "Cancel" };
        static const int ids[3] = { G_SAVE, G_USE, G_CANCEL };
        int bw = (W - 24 - 2 * 10) / 3;
        for (i = 0; i < 3; i++) G(BUTTON_KIND, ids[i], 12 + i * (bw + 10), row, bw, lh, names[i], 0, GA_Disabled, FALSE);
        row += lh + 8;
    }
    if (!g) { rc = RETURN_FAIL; goto out; }
#define IDCMP_ALL (IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | INTEGERIDCMP | CHECKBOXIDCMP | \
                   CYCLEIDCMP | STRINGIDCMP | TEXTIDCMP)
#ifdef LT_ONE
    win = lt_open(scr, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, scr->BarHeight + 20, W, row - scr->WBorTop - fh - 1,
                  glist, IDCMP_ALL, "GFOpenDisplay 0.1");
#else
    win = OpenWindowTags(NULL, WA_Title, (ULONG)"Display", WA_ScreenTitle, (ULONG)"GFOpenDisplay 0.1", WA_PubScreen, (ULONG)scr,
                         WA_Left, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, WA_Top, scr->BarHeight + 20,
                         WA_InnerWidth, W, WA_InnerHeight, row - scr->WBorTop - fh - 1,
                         WA_Gadgets, (ULONG)glist, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_Activate, TRUE,
                         WA_SmartRefresh, TRUE, WA_NewLookMenus, TRUE, WA_IDCMP, IDCMP_ALL, TAG_DONE);
#endif
    if (!win) { rc = RETURN_FAIL; goto out; }
    if ((menu = CreateMenus(menus, TAG_DONE)) && LayoutMenus(menu, vi, GTMN_NewLookMenus, TRUE, TAG_DONE)) SetMenuStrip(win, menu);
    GT_RefreshWindow(win, NULL);
    preview();
    lt_note_to(status);

    while (!quit) {
        struct IntuiMessage *m;
        WaitPort(win->UserPort);
        while (!quit && (m = GT_GetIMsg(win->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code;
            struct Gadget *gg = (struct Gadget *)m->IAddress;
            GT_ReplyIMsg(m);
            if (cls == IDCMP_CLOSEWINDOW) quit = 1;
            else if (cls == IDCMP_REFRESHWINDOW) { GT_BeginRefresh(win); LT_REFRESH(win); preview(); GT_EndRefresh(win, TRUE); }
            else if (cls == IDCMP_MENUPICK) {
                while (code != MENUNULL && !quit) {
                    struct MenuItem *it = ItemAddress(menu, code);
                    if (!it) break;
                    switch ((ULONG)GTMENUITEM_USERDATA(it)) {
                    case 1: if (put_in_place(1)) quit = 1; break;
                    case 2: if (put_in_place(0)) quit = 1; break;
                    case 3: quit = 1; break;
                    }
                    code = it->NextSelect;
                }
            }
            else if (cls == IDCMP_GADGETUP) {
                int on = (gg->Flags & GFLG_SELECTED) != 0;
                switch (gg->GadgetID) {
                case G_PROFILE: take(); cur = code < GD_PROFILES ? code : cur; show(); break;
                case G_SNAME: case G_SSIZE: case G_YNAME: case G_YSIZE: case G_INAME: case G_ISIZE:
                case G_BAR: case G_FRAME: case G_BX: case G_BY:
                    take(); preview(); break;
                case G_SDEF: toggle_default(GD_KEEP_SCREENFONT, on); show(); break;
                case G_YDEF: toggle_default(GD_KEEP_SYSFONT, on); show(); break;
                case G_IDEF: toggle_default(GD_KEEP_ICONFONT, on); show(); break;
                case G_SETDEF: toggle_default(GD_KEEP_IMAGES, on); show(); break;
                case G_BARDEF: toggle_default(GD_KEEP_TITLEBAR, on); show(); break;
                case G_FRAMEDEF: toggle_default(GD_KEEP_FRAME, on); show(); break;
                case G_GREYDEF: toggle_default(GD_KEEP_GREY, on); show(); break;
                case G_SET:
                    if (code < 4) { strcpy(rows[cur].set, set_names[code]); }
                    break;
                case G_GREY: rows[cur].grey = on; break;
#ifdef LT_ONE
                default: {                          /* another section: the page's changes kept, as Use, when it has any */
                    int to = lt_hit(gg, code);
                    if (to >= 0) { if (LT_DIRTY) put_in_place(0); lt_go(win, to); quit = 1; }
                    break;
                }
#endif
                case G_SAVE: if (put_in_place(1)) quit = 1; break;
                case G_USE: if (put_in_place(0)) quit = 1; break;
                case G_CANCEL: quit = 1; break;
                }
            }
        }
    }
out:
#ifdef LT_ONE
    if (win) { ClearMenuStrip(win); lt_close(win, glist); win = NULL; }
#else
    if (win) { ClearMenuStrip(win); CloseWindow(win); win = NULL; }
#endif
    if (menu) FreeMenus(menu);
    FreeGadgets(glist);
    FreeVisualInfo(vi);
    UnlockPubScreen(NULL, scr);
    return rc;
}

int main(int argc, char **argv)
{
    LONG args[3] = { 0, 0, 0 };
    struct RDArgs *rd = argc > 0 ? ReadArgs((STRPTR)"FROM,USE/S,SAVE/S", args, NULL) : NULL;
    int rc;
    (void)argv;
    (void)version;
    win = NULL;                             /* built into GFOpenPrefs, the statics outlive a run */
    if (!rd && argc > 0) { PrintFault(IoErr(), (STRPTR)"GFOpenDisplay"); return RETURN_FAIL; }
    if (!pf_open_lib()) { if (rd) FreeArgs(rd); return RETURN_FAIL; }
    pf_init(&file);
    load_file(args[0] ? (const char *)args[0] : NULL);
    if (args[1] || args[2]) rc = write_file(args[2] != 0, say_shell) ? RETURN_OK : RETURN_ERROR;
    else rc = gui();
    if (rd) FreeArgs(rd);
    pf_free(&file);
    pf_close_lib();
    return rc;
}
