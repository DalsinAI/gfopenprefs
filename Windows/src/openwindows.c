/* GFOpenWindows: the commodity behind GFOpenPrefs Windows. It watches the input
 * stream and moves, sizes and activates windows with the OS's own calls. One
 * patch (0.7): OpenWindowTagList and OpenWindow, so a window whose place is
 * kept opens there instead of opening where it asked and jumping a second
 * later; stacked with GFOpenLook's patch of the same calls.
 *
 *   snapping      a window dropped near a screen edge or another window's
 *                 edge lines up with it; dropped with the pointer at the
 *                 screen's left or right edge, it fills that half (option)
 *   Amiga+Tab     brings the window at the back to the front and activates
 *                 it; with Shift, sends the front window to the back
 *   double-click  a double-click in a window's title bar (its drag bar)
 *                 brings it to the front, as ClickToFront does for a
 *                 double-click anywhere; one already in front stays
 *   wheel         (0.11, Dale 4 November 2026; wheel.h) the wheel scrolls the
 *                 scroll focus: the scroller last pushed, or beside the place
 *                 last clicked, in the active window (or, wheel.target pointer,
 *                 the one beside the pointer in the window under it), moved as
 *                 a drag of its knob moves it, for a program that doesn't know
 *                 the wheel; one that does gets the wheel's raw keys alone
 *   places        a program's window opens where it was last left (0.7: it
 *                 opens there, placed before Intuition draws it; 0.9.2: also
 *                 when the program gives its own corner in its tags, and
 *                 sized when it asks by its inner size; nothing is moved
 *                 after it opens)
 *   drawers       Workbench drawer windows open at least this big
 *   edges         a resizable window is resized by dragging any edge but
 *                 its title bar: the sides, the bottom and the bottom
 *                 corners, from a few pixels outside the frame (0.5);
 *                 0.6.1: a strip 4 pixels outside the frame, a 5 x 5 square
 *                 on each bottom corner, an outline while the button is held
 *                 and one size at the button-up
 *   drive.title   (0.6) a drive's window is titled with the drive's name only:
 *                 Workbench's "Work  70% full, 453MB free, 62.0MB in use"
 *                 loses its usage part. On by default
 *   drag.solid    (0.8, off unless asked for) a window is dragged by its title
 *                 bar with its contents, not as an outline: GFOpenWindows takes
 *                 the press and moves the window as the pointer moves
 *                 0.8.4: every grip inside the window, none outside (a near miss
 *                 no longer lands on the desktop): 3 pixels in from the sides
 *                 and the bottom, 8 x 8 at the bottom corners; a window with
 *                 a sizing gadget of its own counts as one that can be sized
 *   edges.pointer while one is, the pointer shows the edge held (0.8.4: and
 *                 while it is over one of the active window's grips). Off unless
 *                 asked for: a window's own pointer (WA_Pointer, a busy
 *                 pointer) can't be read back, so it would come back as the
 *                 default one
 *
 *   gestures      (0.9, the action filter: docs/design/action-filter.md) every
 *                 left press goes through one table (ENV:GFOpenPrefs/Gestures,
 *                 built in when there is none): where it was (title bar,
 *                 inside a window, a border gadget, an edge, the backdrop),
 *                 the qualifiers, and what the hand did (a click with no
 *                 move, a double-click, a press and a move past 4 pixels)
 *                 give the actions, decided once. A press GFOpenWindows does
 *                 not take goes on to Intuition at once, so a drag starts
 *                 on the first move; a click fronts the window at its
 *                 release, never at the press; the release ends a drag at
 *                 its own place. With GFOpenIntuition (which drags windows
 *                 with their contents itself) drag.solid leaves the drag to
 *                 Intuition; GFOpenWindows' own solid drag is for the others
 *
 * Settings: ENV:GFOpenPrefs/Windows (GFOpenPrefs Windows writes it, then sends
 * Ctrl-F to the task of the public port "GFOpenWindows" to read it again). Places: ENV:GFOpenPrefs/WindowPlaces,
 * kept in ENVARC: too. Exchange can disable, enable or quit it.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <devices/inputevent.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/classusr.h>
#include <intuition/pointerclass.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/cghooks.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/layers.h>
#include <graphics/clip.h>
#include <libraries/commodities.h>
#include <libraries/keymap.h>
#include <devices/input.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/graphics.h>
#include <proto/commodities.h>
#include <proto/utility.h>
#include <utility/tagitem.h>
#include <exec/semaphores.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../Common/cogplace.h"
#include "gestures.h"
#include "places.h"
#include "wheel.h"
#include "centre_prefs.h"

static const char version[] __attribute__((used)) = "$VER: GFOpenWindows 0.12.2 (7.11.2026) MIT, Copyright (c) 2026 AmiCradle Team";

#define PREFS_ENV "ENV:GFOpenPrefs/Windows"
#define PLACES_ENV "ENV:GFOpenPrefs/WindowPlaces"
#define PLACES_ENVARC "ENVARC:GFOpenPrefs/WindowPlaces"
#define IECLASS_NEWMOUSE_ 0x16                  /* NewMouse drivers' class; OS 3.2 sends RAWKEY 0x7A-0x7D */
#define MAX_SEEN 96
#define MAX_NEVER 16

struct Library *CxBase, *LayersBase;
struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;

/* ---- settings ------------------------------------------------------------ */

static struct {
    int snap, snap_dist, halves, switcher, places, places_wb, drawer_w, drawer_h, dblfront, edges, edge_ptr, drive_title, solid;
    /* 0.11, the wheel (wheel.h): wheel.scroll on|off, wheel.target focus|pointer (Dale, 4 November 2026: the focus,
     * on by default), wheel.lines (3), wheel.self <program> (programs left to scroll themselves). The old "wheel"
     * line (the window under the pointer made active) is read no more: an install that has it gets the focus */
    int wheel_mode;                              /* 0 off, 1 the scroll focus, 2 the window under the pointer */
    int wheel_lines;
    char wself[MAX_NEVER][32];
    int nwself;
    int taller;                                  /* 0.8.2: extra rows in every window's title bar (default 4) */
    /* 0.10 (Dale, 4 November 2026): windows with no snapshot open centred; the snapshot gadget */
    int centre;                                  /* centre on: a window with no snapped place opens centred (on since 0.12,
                                                    centre_prefs.h) */
    int centre_asked;                            /* centre.asked on: one whose program gives its own corner is left there (on) */
    int centre_custom;                           /* centre.custom on: windows on screens that aren't public too (off) */
    int centre_over;                             /* centre.parent on: a window opened while one of its program's own is
                                                    active is centred over that one (requesters, dialogs) (on) */
    int gadget;                                  /* snapshot.gadget on|off; -1 (no line, or one from before 0.12): as centre */
    char key[48];
    char never[MAX_NEVER][32];
    int nnever;
} cfg;
static char log_path[128];                      /* 0.9.2: ENV:OpenWindowsLog names a file: what the look saw */

static char *read_file(const char *path)
{
    BPTR fh = Open((STRPTR)path, MODE_OLDFILE);
    char *buf = NULL;
    if (fh) {
        LONG len;
        Seek(fh, 0, OFFSET_END);
        len = Seek(fh, 0, OFFSET_BEGINNING);
        if (len >= 0 && (buf = AllocVec(len + 1, MEMF_ANY))) {
            if (Read(fh, buf, len) != len) { FreeVec(buf); buf = NULL; }
            else buf[len] = 0;
        }
        Close(fh);
    }
    return buf;
}

static int on_off(const char *s) { return !strncmp(s, "on", 2); }

static void read_prefs(void)
{
    char *text = read_file(PREFS_ENV), *p, *line;
    int cver = 0, cline = -1, gline = -1;        /* 0.12: centre.v, centre and snapshot.gadget as written (centre_prefs.h) */
    memset(&cfg, 0, sizeof cfg);
    cfg.snap = 1; cfg.snap_dist = 12; cfg.halves = 0; cfg.switcher = 1; cfg.wheel_mode = 1; cfg.wheel_lines = 3; cfg.places = 1; cfg.dblfront = 1; cfg.edges = 1; cfg.edge_ptr = 1; cfg.drive_title = 1; cfg.taller = 4;
    cfg.centre = 1; cfg.centre_asked = 1; cfg.centre_custom = 0; cfg.centre_over = 1; cfg.gadget = -1;
    strcpy(cfg.key, "lcommand tab");
    if (!text) return;
    for (p = text; *p; ) {
        char *v;
        line = p;
        while (*p && *p != '\n') p++;
        if (*p) *p++ = 0;
        if (*line == ';' || !*line) continue;
        if (!(v = strchr(line, ' '))) continue;
        *v++ = 0;
        if (!strcmp(line, "snap")) { cfg.snap = on_off(v); if (strchr(v, ' ')) cfg.snap_dist = atoi(strchr(v, ' ') + 1); }
        else if (!strcmp(line, "snap.halves")) cfg.halves = on_off(v);
        else if (!strcmp(line, "switcher")) cfg.switcher = on_off(v);
        else if (!strcmp(line, "switcher.key")) { strncpy(cfg.key, v, sizeof cfg.key - 1); }
        else if (!strcmp(line, "wheel.scroll")) cfg.wheel_mode = on_off(v) ? (cfg.wheel_mode ? cfg.wheel_mode : 1) : 0;
        else if (!strcmp(line, "wheel.target")) { if (cfg.wheel_mode) cfg.wheel_mode = !strncmp(v, "pointer", 7) ? 2 : 1; }
        else if (!strcmp(line, "wheel.lines")) cfg.wheel_lines = atoi(v);
        else if (!strcmp(line, "wheel.self") && cfg.nwself < MAX_NEVER) strncpy(cfg.wself[cfg.nwself++], v, 31);
        else if (!strcmp(line, "doubleclick.front")) cfg.dblfront = on_off(v);
        else if (!strcmp(line, "edges")) cfg.edges = on_off(v);
        else if (!strcmp(line, "edges.pointer")) cfg.edge_ptr = on_off(v);
        else if (!strcmp(line, "drive.title")) cfg.drive_title = on_off(v);
        else if (!strcmp(line, "drag.solid")) cfg.solid = on_off(v);
        else if (!strcmp(line, "title.taller")) cfg.taller = atoi(v);
        else if (!strcmp(line, "places")) cfg.places = on_off(v);
        else if (!strcmp(line, "places.drawers")) cfg.places_wb = on_off(v);
        else if (!strcmp(line, "centre")) cline = on_off(v);
        else if (!strcmp(line, "centre.v")) cver = atoi(v);
        else if (!strcmp(line, "centre.asked")) cfg.centre_asked = on_off(v);
        else if (!strcmp(line, "centre.custom")) cfg.centre_custom = on_off(v);
        else if (!strcmp(line, "centre.parent")) cfg.centre_over = on_off(v);
        else if (!strcmp(line, "snapshot.gadget")) gline = on_off(v);
        else if (!strcmp(line, "never") && cfg.nnever < MAX_NEVER) strncpy(cfg.never[cfg.nnever++], v, 31);
        else if (!strcmp(line, "drawer")) { cfg.drawer_w = atoi(v); if (strchr(v, ' ')) cfg.drawer_h = atoi(strchr(v, ' ') + 1); }
    }
    centre_prefs_resolve(cver, cline, gline, &cfg.centre, &cfg.gadget);
    if (cfg.snap_dist < 2) cfg.snap_dist = 2;
    if (cfg.snap_dist > 64) cfg.snap_dist = 64;
    if (cfg.wheel_lines < 1) cfg.wheel_lines = 1;
    if (cfg.wheel_lines > 30) cfg.wheel_lines = 30;
    if (cfg.taller < 0) cfg.taller = 0;
    if (cfg.taller > 16) cfg.taller = 16;
    FreeVec(text);
}

/* 0.10: the snapshot gadget is on: snapshot.gadget's word, or with no such line as centring is */
#define GADGET_ON (cfg.gadget >= 0 ? cfg.gadget : cfg.centre)
/* 0.10: only snapped places count: with centring on (a window with none opens centred), or with Remember
 * places off and the gadget on (a snapshot is the only way a window keeps a place) */
#define SNAPPED_ONLY (cfg.centre || !cfg.places)

/* ---- the action filter's table (0.9; gestures.h, docs/design/action-filter.md) ---------- */

#define GESTURES_ENV "ENV:GFOpenPrefs/Gestures"
static struct gesture_table gtab;               /* the handler reads it; changed under Forbid */
static int intuition_drags;                     /* Intuition drags windows with their contents itself (GFOpenIntuition) */

/* The built-in table from the settings, or ENV:GFOpenPrefs/Gestures in its place
 * when there is one that reads (a line it can't read: the built-in one, and why). */
static void read_gestures(void)
{
    static struct gesture_table t;
    char *text = read_file(GESTURES_ENV);
    gestures_default(&t, cfg.dblfront, cfg.edges, cfg.snap);
    if (text) {
        int n = gestures_parse(&t, text);
        if (n < 0) {
            char msg[128];   /* the line number and GESTURES_ENV's name: up to 99 bytes (GCC 16 -Werror=format-truncation) */
            snprintf(msg, sizeof msg, "GFOpenWindows: " GESTURES_ENV " line %d can't be read; the built-in table is used\n", -n);
            PutStr((STRPTR)msg);
        }
        FreeVec(text);
    }
    Forbid();
    gtab = t;
    Permit();
}

static void read_settings(void)
{
    const char *id = (const char *)IntuitionBase->LibNode.lib_IdString;
    read_prefs();
    read_gestures();
    if (GetVar((STRPTR)"OpenWindowsLog", (STRPTR)log_path, sizeof log_path, 0) <= 0) log_path[0] = 0;
    intuition_drags = id && strstr(id, "GFOpenIntuition") != NULL;
}

/* ---- taller title bars (0.8.2) ---------------------------------------------------
 *
 * Intuition gives a window's title bar scr->WBorTop + the screen font's height + 1 rows,
 * reading WBorTop from the screen each time a window opens. So the Workbench screen's
 * WBorTop is raised by cfg.taller (4: 2 rows above the text, 2 below, once GFOpenLook
 * centres the title), and every window opened after that has a taller bar. The screen's
 * own bar was laid out when the screen opened and is not changed here. Put back on exit
 * if nothing else has changed it since. */
static struct Screen *tall_scr;
static WORD tall_was, tall_now;

static void taller_on(void)
{
    struct Screen *s = LockPubScreen((STRPTR)"Workbench");
    if (!s) return;
    Forbid();
    if (tall_scr == s && s->WBorTop == tall_now) s->WBorTop = tall_was;     /* the old extra off first */
    else if (tall_scr != s) { tall_scr = s; tall_was = s->WBorTop; }
    tall_now = tall_was + cfg.taller;
    s->WBorTop = tall_now;
    Permit();
    UnlockPubScreen(NULL, s);
}

static void taller_off(void)
{
    struct Screen *s;
    if (!tall_scr) return;
    s = LockPubScreen((STRPTR)"Workbench");
    if (s == tall_scr) { Forbid(); if (s->WBorTop == tall_now) s->WBorTop = tall_was; Permit(); }
    if (s) UnlockPubScreen(NULL, s);
    tall_scr = NULL;
}

/* ---- the input stream (input.device's task: no waiting, no DOS) ------------- */

static struct Task *me;
static struct cogplace cog;                      /* 0.8.1: the cog's request, public by name (cogplace.h) */
static ULONG sig_button, sig_wheel;
/* 0.11, the wheel (wheel.h). The handler's side: the wheel's events the main task is to see (held: taken out
 * of the stream, for the main task to send on), the class of the windows asked so far (WC_SELF, WC_EMULATE:
 * the main task writes it, the handler reads it), and the last left press, for the scroll focus. */
#define WQ 32
static struct wq { struct InputEvent ev; struct Window *w; UBYTE held; } wq[WQ];
static volatile int wq_in, wq_out;
#define WCLS 12
static struct { struct Window *w; UBYTE cls; } wcls[WCLS];
static volatile struct { struct Screen *s; WORD x, y; ULONG gen; } wclick;
static volatile int pass_wheel;                 /* events we put back: let them through */
static volatile int on = 1;

/* The action filter (0.9). The press being watched, the handler's own: where
 * it was and on what, whether GFOpenWindows took it, where the pointer is now
 * and whether it has gone past the threshold. At the release the gesture is
 * decided and its actions go to the main task through the done ring. */
static struct {
    int on, target, take, moved;
    int menu;                                   /* 0.9.1: the menu button came while it was held */
    struct Window *w;
    struct Screen *s;
    struct Layer *top;                          /* 0.9.1: the screen's front layer at the press */
    WORD x0, y0, x, y;
    WORD l, t, wd, ht;                          /* the window's box at the press */
    ULONG secs, micro;
    unsigned qual;
} press;
/* the last click, for a double-click: a second click on the same thing, close in place and time */
static struct { struct Window *w; int target; WORD x, y; ULONG secs, micro; } last_click;
#define DONE_RING 8
static struct done {
    struct Window *w;
    struct Screen *s;
    struct Layer *top;                          /* 0.9.1: the screen's front layer at the press */
    UBYTE kind, target, actions, pad;
    WORD x0, y0, x, y, l, t, wd, ht;
} done_ring[DONE_RING];
static volatile int done_in, done_out;

/* Resizing from an edge (0.5): the handler sees a press on an edge, keeps it
 * from Intuition and tells the main task, which resizes the window with
 * ChangeWindowBox as the pointer moves, until the button comes up. */
#define EDGE_L 1
#define EDGE_R 2
#define EDGE_B 4
#define EDGE_MOVE 8                             /* 0.8: not an edge: the title bar, dragging the window solid */
/* 0.8.3 (the user, 10 October 2026: "mouse behaviour at window edges and corner is odd and inconsistent"): the
 * same reach outside and inside on every side, a corner square big enough to find, and the corner takes in
 * Intuition's own size gadget, so a press there resizes the same way (solid, from here) as a press on an edge. */
/* 0.8.4 (the Team, 11 October 2026: "a mis click outside it removes active status ... an invisible 3 pixel
 * boundary around the two sides and bottom where the mouse pointer changes, and lets us drag"): every grip is
 * inside the window, none outside it, and the pointer shows the grip while it is over one (the window active). */
#define ZONE_IN 3                               /* a side or the bottom: this many pixels inside the frame */
#define ZONE_CORNER 8                           /* a bottom corner: an 8 x 8 square inside it */
#define SIZE_GADGET_W 18                        /* Intuition's size gadget at the bottom right: its box is the corner's too */
static ULONG sig_grab;
static ULONG sig_hover;                         /* 0.8.4: the grip under the pointer changed */
static struct Window *volatile hover_win;       /* the active window whose grip the pointer is over, or NULL */
static volatile int hover_edges;
static struct Window *volatile grab_win;        /* the window being resized, set by the handler */
static volatile int grab_edges, grab_on;
static volatile WORD grab_x0, grab_y0;          /* the pointer at the press, screen relative */
/* 0.9: the pointer as the handler saw it with the drag's latest event, and at
 * the release: the main task follows these, not the screen's MouseX, which by
 * the time it runs may hold moves made after the release (W66: the drag went
 * on past the release by what the hand did after it) */
static volatile WORD grab_px, grab_py, grab_ex, grab_ey;
static volatile ULONG grab_gen;                 /* up by one for each press taken: a new press is never lost to the end of the last */
static volatile ULONG grab_end_gen;             /* the press whose release came (its place: grab_ex, grab_ey) */
static volatile ULONG grab_last;                /* the time stamp (seconds) of the drag's latest event, from the handler */

/* The window whose edge (x, y) is on, and which edges, or NULL. The screen's
 * layers are read front to back under their lock, so none goes while we look;
 * the input handler can't wait for it, so when another task holds it the
 * press is left to Intuition (AttemptSemaphore). The first window whose frame
 * holds the point is the one under the pointer, and only its own edge counts;
 * a point just outside every frame in front of a window belongs to that
 * window's edge. The frame is the window's own box, not its layer's: a
 * GimmeZeroZero window's inner layer and a requester's are inside it. The
 * title bar's height never resizes (it moves the window), nor does a window
 * that can't be resized, a backdrop or a borderless one. */
/* 0.8.4: a window the user may size: Intuition's size gadget, or a sizing gadget of its own (GFOpenBench's
 * drawers have one in place of Intuition's, so their frame stays thin). */
static int resizable(struct Window *w)
{
    struct Gadget *g;
    if (w->Flags & WFLG_SIZEGADGET) return 1;
    for (g = w->FirstGadget; g; g = g->NextGadget)
        if ((g->GadgetType & GTYP_SYSTYPEMASK) == GTYP_SIZING) return 1;
    return 0;
}

static struct Window *edge_at(struct Screen *s, WORD x, WORD y, int *edges)
{
    struct Layer *l;
    struct Window *found = NULL;
    if (!AttemptSemaphore(&s->LayerInfo.Lock))
        return NULL;
    for (l = s->LayerInfo.top_layer; l; l = l->back) {
        struct Window *w = (struct Window *)l->Window;
        WORD L, T, R, B;
        int in, e = 0;
        if (!w || l == s->BarLayer) {           /* the screen's title bar, a menu */
            if (x >= l->bounds.MinX && x <= l->bounds.MaxX && y >= l->bounds.MinY && y <= l->bounds.MaxY) break;
            continue;
        }
        L = w->LeftEdge; T = w->TopEdge; R = L + w->Width - 1; B = T + w->Height - 1;
        in = x >= L && x <= R && y >= T && y <= B;
        if ((w->Flags & (WFLG_BACKDROP | WFLG_BORDERLESS)) || !resizable(w)) {
            if (in) break;                      /* a window that keeps its size */
            continue;
        }
        if (in && y >= T + w->BorderTop) {
            /* the sides and the bottom: a strip inside the frame, below the title bar */
            if (x <= L + ZONE_IN - 1) e |= EDGE_L;
            if (x >= R - ZONE_IN + 1) e |= EDGE_R;
            if (y >= B - ZONE_IN + 1) e |= EDGE_B;
            /* a bottom corner: a square inside it holds both edges */
            if (y >= B - ZONE_CORNER + 1) {
                if (x <= L + ZONE_CORNER - 1) e |= EDGE_L | EDGE_B;
                if (x >= R - ZONE_CORNER + 1) e |= EDGE_R | EDGE_B;
            }
            /* Intuition's size gadget (bottom right, in the bottom border): ours, so both corners feel the same */
            if (x >= R - SIZE_GADGET_W && x <= R && y >= B - (w->BorderBottom > 2 ? w->BorderBottom : 2) - 1 && y <= B)
                e |= EDGE_R | EDGE_B;
        }
        if (e) { *edges = e; found = w; break; }
        if (in) break;                          /* inside it, off its edges: an ordinary press */
    }
    ReleaseSemaphore(&s->LayerInfo.Lock);
    return found;
}

/* 0.9: what a press at (x, y) is on, for the gesture table: the frontmost layer there (read as edge_at reads,
 * under the layers' lock, AttemptSemaphore: the input task never waits), its window, and which part of it:
 * the drag bar (the title bar's drag gadget and no other gadget), a gadget (0.9.1: the border's, close, depth,
 * zoom, size, or one of the program's own, or anything while a requester is up: gestures_target), inside it
 * (GT_BODY), a backdrop window, or the screen's bar. -1 when the layers are busy. */
static int target_at(struct Screen *s, WORD x, WORD y, struct Window **wp)
{
    struct Layer *l;
    int t = GT_BACKDROP;                                    /* nothing there: the screen itself */
    *wp = NULL;
    if (!AttemptSemaphore(&s->LayerInfo.Lock))
        return -1;
    for (l = s->LayerInfo.top_layer; l; l = l->back) {
        struct Window *w = (struct Window *)l->Window;
        if (x < l->bounds.MinX || x > l->bounds.MaxX || y < l->bounds.MinY || y > l->bounds.MaxY) continue;
        if (!w || l == s->BarLayer) { t = GT_SCREENBAR; break; }   /* the screen's title bar, a menu */
        *wp = w;
        if (w->Flags & WFLG_BACKDROP) { t = GT_BACKDROP; break; }
        {
            WORD wx = x - w->LeftEdge, wy = y - w->TopEdge;
            struct Gadget *g;
            struct gesture_hit h;
            memset(&h, 0, sizeof h);
            for (g = w->FirstGadget; g; g = g->NextGadget) {
                /* 0.9.1: a GimmeZeroZero window's own gadgets are placed in its inner part */
                int inner = (w->Flags & WFLG_GIMMEZEROZERO) && !(g->GadgetType & GTYP_GZZGADGET);
                WORD ox = inner ? w->BorderLeft : 0, oy = inner ? w->BorderTop : 0;
                WORD bw = inner ? w->GZZWidth : w->Width, bh = inner ? w->GZZHeight : w->Height;
                WORD gl = ox + g->LeftEdge + ((g->Flags & GFLG_RELRIGHT) ? bw - 1 : 0);
                WORD gt = oy + g->TopEdge + ((g->Flags & GFLG_RELBOTTOM) ? bh - 1 : 0);
                WORD gw = g->Width + ((g->Flags & GFLG_RELWIDTH) ? bw : 0);
                WORD gh = g->Height + ((g->Flags & GFLG_RELHEIGHT) ? bh : 0);
                if (wx < gl || wx >= gl + gw || wy < gt || wy >= gt + gh) continue;
                if ((g->GadgetType & GTYP_SYSTYPEMASK) == GTYP_WDRAGGING && (g->GadgetType & GTYP_SYSGADGET)) h.drag_bar = 1;
                else if (g->GadgetType & GTYP_SYSGADGET) h.border_gadget = 1;
                else if (!(g->Flags & GFLG_DISABLED)) h.own_gadget = 1;   /* 0.9.1: its press is the gadget's */
            }
            h.in_title = (w->Flags & WFLG_DRAGBAR) && wy >= 0 && wy < w->BorderTop;
            h.requester = w->FirstRequest != NULL;
            t = gestures_target(&h);
        }
        break;
    }
    ReleaseSemaphore(&s->LayerInfo.Lock);
    return t;
}

static int wcls_of(struct Window *w)
{
    int i;
    for (i = 0; i < WCLS; i++) if (wcls[i].w == w) return wcls[i].cls;
    return WC_UNKNOWN;
}

static void wq_put(struct InputEvent *ie, struct Window *w, int held)
{
    if (((wq_in + 1) & (WQ - 1)) == wq_out) return;   /* full: this one goes on as it came */
    wq[wq_in].ev = *ie;
    wq[wq_in].ev.ie_NextEvent = NULL;
    wq[wq_in].w = w;
    wq[wq_in].held = (UBYTE)held;
    wq_in = (wq_in + 1) & (WQ - 1);
    if (held) ie->ie_Class = IECLASS_NULL;     /* the main task sends it on */
    Signal(me, sig_wheel);
}

static int over_active(void)
{
    struct Window *w = IntuitionBase->ActiveWindow;
    struct Screen *s = w ? w->WScreen : NULL;
    WORD x, y;
    if (!w || !s || s != IntuitionBase->FirstScreen) return 1;      /* nothing better to offer */
    if (s->LayerInfo.top_layer != w->WLayer) return 0;             /* something may cover it: ask the layers */
    x = s->MouseX; y = s->MouseY;
    return x >= w->LeftEdge && y >= w->TopEdge && x < w->LeftEdge + w->Width && y < w->TopEdge + w->Height;
}

/* The action filter (0.9, docs/design/action-filter.md): a left press. What it is on and the table decide, once,
 * whether GFOpenWindows takes it (an edge to size, or a title bar to drag where Intuition's drag is an outline) or
 * it goes on to Intuition at once (Intuition's own drag then starts on the first move; a press is never held
 * back to see what it becomes). Answers 1 when it took it. */
static int press_begin(struct InputEvent *ie, struct Screen *s)
{
    struct Window *w = NULL;
    int target, edges = 0, take = 0;
    unsigned qual = gestures_qual(ie->ie_Qualifier), move;
    WORD x = s->MouseX, y = s->MouseY;                      /* this handler runs before Intuition: the place of the events before */
    if (ie->ie_Qualifier & IEQUALIFIER_RBUTTON) {           /* 0.9.1: a left press while the menu button is held is the menus' */
        press.on = 0;
        return 0;
    }
    target = gestures_actions(&gtab, GT_EDGE, GB_SELECT, GK_MOVE, qual, &take) && take ? -2 : -1;
    if (target == -2 && (w = edge_at(s, x, y, &edges))) target = GT_EDGE;
    else target = target_at(s, x, y, &w);
    press.on = 0;
    if (target < 0) return 0;                               /* the layers busy: Intuition has it, and no gesture */
    move = gestures_actions(&gtab, target, GB_SELECT, GK_MOVE, qual, &take);
    if (target == GT_TITLE && (move & GA_DRAG) && cfg.solid && !intuition_drags) take = 1;   /* GFOpenWindows' own solid drag */
    if (target != GT_EDGE && target != GT_TITLE) take = 0;  /* only an edge or a title bar is GFOpenWindows' to drag */
    press.on = 1; press.target = target; press.take = take; press.moved = 0; press.menu = 0;
    press.top = s->LayerInfo.top_layer;                     /* 0.9.1: what the press brings up comes in front of it */
    press.w = w; press.s = s; press.x0 = press.x = x; press.y0 = press.y = y; press.qual = qual;
    press.secs = ie->ie_TimeStamp.tv_secs; press.micro = ie->ie_TimeStamp.tv_micro;
    if (w) { press.l = w->LeftEdge; press.t = w->TopEdge; press.wd = w->Width; press.ht = w->Height; }
    if (!take || !w) { press.take = 0; return 0; }
    grab_win = w; grab_edges = target == GT_EDGE ? edges : EDGE_MOVE;
    grab_x0 = grab_px = grab_ex = x; grab_y0 = grab_py = grab_ey = y;
    grab_gen++;
    grab_on = 1; grab_last = ie->ie_TimeStamp.tv_secs;
    ie->ie_Code = IECODE_NOBUTTON;                          /* Intuition sees a move, not the press */
    Signal(me, sig_grab);
    return 1;
}

/* The pointer moved to (x, y) with the button held: past the threshold, the gesture is a move */
static void press_moved(WORD x, WORD y)
{
    press.x = x; press.y = y;
    if (!press.moved && gestures_moved(press.x0, press.y0, x, y)) press.moved = 1;
}

/* The release: the gesture is decided and its actions go to the main task. A click (no move past the threshold)
 * is a double-click when the last click was on the same thing, close in place, within Input prefs' time. The drag
 * itself is over already: Intuition ended its own at this event, GFOpenWindows' ends at this place. */
static void press_end(struct InputEvent *ie)
{
    int kind;
    unsigned a;
    if (!press.on) return;
    press.on = 0;
    kind = press.moved ? GK_MOVE : GK_CLICK;
    if (kind == GK_CLICK) {
        if (last_click.w == press.w && last_click.target == press.target && press.w &&
            last_click.x - press.x0 <= 4 && press.x0 - last_click.x <= 4 && last_click.y - press.y0 <= 4 && press.y0 - last_click.y <= 4 &&
            DoubleClick(last_click.secs, last_click.micro, press.secs, press.micro)) {
            kind = GK_DOUBLE;
            last_click.w = NULL;                            /* a third click starts again */
        } else {
            last_click.w = press.w; last_click.target = press.target; last_click.x = press.x0; last_click.y = press.y0;
            last_click.secs = press.secs; last_click.micro = press.micro;
        }
    } else last_click.w = NULL;
    a = gestures_actions(&gtab, press.target, GB_SELECT, kind, press.qual, NULL);
    /* 0.9.1: a window the press brought up (a popup, a requester, a new drawer) is in front of the screen's layers
     * by now: it stays in front. The main task looks again before it fronts (gestures_done). */
    a = gestures_release(a, press.menu, press.s && press.s->LayerInfo.top_layer != press.top);
    a &= ~(GA_DRAG | GA_RESIZE);                            /* done as the pointer moved */
    if (press.take) a &= ~GA_SNAP;                          /* GFOpenWindows' own drag snaps at its end (resize) */
    if (!a || !press.w || ((done_in + 1) & (DONE_RING - 1)) == done_out) return;
    {
        struct done *d = &done_ring[done_in];
        d->w = press.w; d->s = press.s; d->top = press.top;
        d->kind = (UBYTE)kind; d->target = (UBYTE)press.target; d->actions = (UBYTE)a;
        d->x0 = press.x0; d->y0 = press.y0; d->x = press.x; d->y = press.y;
        d->l = press.l; d->t = press.t; d->wd = press.wd; d->ht = press.ht;
        done_in = (done_in + 1) & (DONE_RING - 1);
    }
    (void)ie;
    Signal(me, sig_button);
}

/* The place an event puts the pointer at on screen s, when it is a placing (IECLASS_NEWPOINTERPOS, as
 * AmiCradle's absolute pointer and tablets send) on that screen: 1 and (x, y) */
static int placed_at(struct InputEvent *ie, struct Screen *s, WORD *x, WORD *y)
{
    if (ie->ie_Class == IECLASS_NEWPOINTERPOS && ie->ie_SubClass == IESUBCLASS_PIXEL && ie->ie_EventAddress) {
        struct IEPointerPixel *pp = (struct IEPointerPixel *)ie->ie_EventAddress;
        if (pp->iepp_Screen != s) return 0;
        *x = pp->iepp_Position.X; *y = pp->iepp_Position.Y;
        return 1;
    }
    return 0;
}

static void custom(CxMsg *msg, CxObj *co)
{
    struct InputEvent *ie = (struct InputEvent *)CxMsgData(msg);
    (void)co;
    if (!on) return;
    for (; ie; ie = ie->ie_NextEvent) {
        struct Screen *s = IntuitionBase->FirstScreen;
        WORD px, py;
        if (ie->ie_Class == IECLASS_RAWMOUSE) {
            WORD mx = s ? s->MouseX + ie->ie_X : 0, my = s ? s->MouseY + ie->ie_Y : 0;   /* a relative mouse's move, near enough */
            if (ie->ie_Code == IECODE_RBUTTON && press.on) press.menu = 1;   /* 0.9.1: the menus' now; it goes on untouched */
            if (grab_on) {                         /* GFOpenWindows' own drag or edge: the button coming up ends it, here */
                grab_px = mx; grab_py = my;
                if (ie->ie_Code == (IECODE_LBUTTON | IECODE_UP_PREFIX)) {
                    grab_ex = mx; grab_ey = my;
                    grab_on = 0; grab_end_gen = grab_gen;
                    ie->ie_Code = IECODE_NOBUTTON;     /* the pointer's last move still goes on */
                    press_end(ie);
                } else if (press.on && (ie->ie_X || ie->ie_Y)) press_moved(mx, my);
                grab_last = ie->ie_TimeStamp.tv_secs;
                Signal(me, sig_grab);              /* the main task follows the place the handler saw */
                continue;
            }
            if (ie->ie_Code == IECODE_NOBUTTON && cfg.edges && cfg.edge_ptr && s && sig_hover) {
                /* 0.8.4: a move: the pointer shows the grip of the active window it is over */
                int e = 0;
                struct Window *hw = edge_at(s, s->MouseX, s->MouseY, &e);
                if (hw != IntuitionBase->ActiveWindow) { hw = NULL; e = 0; }
                if (hw != hover_win || e != hover_edges) { hover_win = hw; hover_edges = e; Signal(me, sig_hover); }
            }
            if (ie->ie_Code == IECODE_LBUTTON && s) {
                if (cfg.wheel_mode == 1) {          /* 0.11: the scroll focus is where the press is (wheel_click) */
                    wclick.s = s; wclick.x = s->MouseX; wclick.y = s->MouseY; wclick.gen++;
                    Signal(me, sig_wheel);
                }
                if (press_begin(ie, s)) continue;
            } else if (ie->ie_Code == (IECODE_LBUTTON | IECODE_UP_PREFIX)) {
                press_end(ie);
            } else if (press.on && (ie->ie_X || ie->ie_Y)) press_moved(mx, my);
        } else if ((ie->ie_Class == IECLASS_POINTERPOS || ie->ie_Class == IECLASS_NEWPOINTERPOS) && s) {
            /* a pointer put somewhere (not a mouse counting): the drag follows it too. 0.8.6: AmiCradle's absolute
             * pointer comes through the open input.device as IECLASS_NEWPOINTERPOS (IESUBCLASS_PIXEL), one a move */
            if (!placed_at(ie, s, &px, &py)) { px = s->MouseX; py = s->MouseY; }
            if (press.on) press_moved(px, py);
            if (grab_on) {
                grab_px = px; grab_py = py;
                grab_last = ie->ie_TimeStamp.tv_secs;
                Signal(me, sig_grab);
            }
        } else if (cfg.wheel_mode && (ie->ie_Class == IECLASS_RAWKEY || ie->ie_Class == IECLASS_NEWMOUSE_) &&
                   (ie->ie_Code & 0x7F) >= RAWKEY_WHEEL_UP && (ie->ie_Code & 0x7F) <= RAWKEY_WHEEL_RIGHT) {
            /* 0.11 (wheel.h): a window asked already goes on at once; the wheel's raw keys go to it either way, and a
             * step is moved for one that doesn't know the wheel. Any other (not asked yet; the window under the
             * pointer, not the active one) is held for the main task, which sends it on. */
            struct Window *aw = IntuitionBase->ActiveWindow;
            int c;
            if (pass_wheel) { pass_wheel--; continue; }
            if (aw && (cfg.wheel_mode == 1 || over_active())) {
                c = wcls_of(aw);
                if (c == WC_SELF) continue;
                if (c == WC_EMULATE) {
                    if (ie->ie_Class == IECLASS_RAWKEY && !(ie->ie_Code & IECODE_UP_PREFIX)) wq_put(ie, aw, 0);
                    continue;
                }
            }
            wq_put(ie, aw, 1);
        }
    }
}

/* ---- windows ---------------------------------------------------------------------------- */

/* The window whose layer is under (x, y) on screen s, or NULL. */
static struct Window *window_at(struct Screen *s, WORD x, WORD y)
{
    struct Layer *l;
    struct Window *w = NULL;
    LockLayerInfo(&s->LayerInfo);
    l = WhichLayer(&s->LayerInfo, x, y);
    if (l) w = (struct Window *)l->Window;
    UnlockLayerInfo(&s->LayerInfo);
    if (w && (w->Flags & WFLG_BACKDROP) && !(w->Flags & WFLG_WBENCHWINDOW)) w = NULL;
    return w;
}

static int is_window(struct Window *w)
{
    struct Screen *s;
    struct Window *x;
    ULONG lock = LockIBase(0);
    int found = 0;
    for (s = IntuitionBase->FirstScreen; s && !found; s = s->NextScreen)
        for (x = s->FirstWindow; x; x = x->NextWindow)
            if (x == w) { found = 1; break; }
    UnlockIBase(lock);
    return found;
}

/* Snapping: the nearest of the screen's and the other windows' edges. */
static void best(WORD *delta, WORD edge, WORD target, int dist)
{
    WORD d = target - edge;
    if (d < 0 ? -d <= dist : d <= dist)
        if (*delta == 0x7fff || (d < 0 ? -d : d) < (*delta < 0 ? -*delta : *delta)) *delta = d;
}

static void snap(struct Window *w)
{
    struct Screen *s = w->WScreen;
    struct Window *o;
    WORD dx = 0x7fff, dy = 0x7fff, l = w->LeftEdge, t = w->TopEdge, r = l + w->Width, b = t + w->Height;
    WORD top = s->BarHeight + 1;
    int dist = cfg.snap_dist;
    best(&dx, l, 0, dist); best(&dx, r, s->Width, dist);
    best(&dy, t, top, dist); best(&dy, t, 0, dist); best(&dy, b, s->Height, dist);
    {
        ULONG lock = LockIBase(0);
        for (o = s->FirstWindow; o; o = o->NextWindow) {
            WORD ol = o->LeftEdge, ot = o->TopEdge, orr = ol + o->Width, ob = ot + o->Height;
            if (o == w || (o->Flags & WFLG_BACKDROP)) continue;
            if (t < ob + dist && b > ot - dist) {                     /* side by side */
                best(&dx, l, orr, dist); best(&dx, r, ol, dist); best(&dx, l, ol, dist); best(&dx, r, orr, dist);
            }
            if (l < orr + dist && r > ol - dist) {                    /* one above the other */
                best(&dy, t, ob, dist); best(&dy, b, ot, dist); best(&dy, t, ot, dist); best(&dy, b, ob, dist);
            }
        }
        UnlockIBase(lock);
    }
    if (dx == 0x7fff) dx = 0;
    if (dy == 0x7fff) dy = 0;
    if (dx || dy) MoveWindow(w, dx, dy);
}

static void halves(struct Window *w, WORD px)
{
    struct Screen *s = w->WScreen;
    WORD top = s->BarHeight + 1, half = s->Width / 2;
    if (!(w->Flags & WFLG_SIZEGADGET)) return;
    if (half < w->MinWidth || s->Height - top < w->MinHeight) return;
    ChangeWindowBox(w, px <= 1 ? 0 : s->Width - half, top, half, s->Height - top);
}

/* ---- the gestures' actions (0.9, the main task) ------------------------------------------------- */

/* 1 when no other window's layer in front of w overlaps it */
static int in_front(struct Window *w)
{
    struct Layer *l;
    int covered = 0;
    LockLayerInfo(&w->WScreen->LayerInfo);
    for (l = w->WLayer->front; l && !covered; l = l->front) {
        struct Window *o = (struct Window *)l->Window;
        if (o && (o->Flags & WFLG_BORDERLESS) && o->Height <= w->WScreen->BarHeight + 1) continue;  /* the title bar's tray */
        covered = l->bounds.MinX <= w->WLayer->bounds.MaxX && l->bounds.MaxX >= w->WLayer->bounds.MinX &&
                  l->bounds.MinY <= w->WLayer->bounds.MaxY && l->bounds.MaxY >= w->WLayer->bounds.MinY;
    }
    UnlockLayerInfo(&w->WScreen->LayerInfo);
    return !covered;
}

/* 0.9.1: 1 when w's program waits on another window of its own that is in front of w and overlaps it: a
 * requester (EasyRequest, ASL, its own dialog) whose loop isn't reading w's port, so w in front would hide
 * it and the program would look stuck (Dale, 18 October: "open files ... wont close"). A program that waits
 * on w's port too (GFOpenBench's drawers share one) or is running isn't held. */
static int own_requester_in_front(struct Window *w)
{
    struct MsgPort *p = w->UserPort;
    struct Task *t = p && (p->mp_Flags & PF_ACTION) == PA_SIGNAL ? (struct Task *)p->mp_SigTask : NULL;
    struct Layer *l;
    int waits_elsewhere, found = 0;
    if (!t) return 0;
    Forbid();
    waits_elsewhere = t->tc_State == TS_WAIT && !(t->tc_SigWait & (1UL << p->mp_SigBit));
    Permit();
    if (!waits_elsewhere) return 0;
    LockLayerInfo(&w->WScreen->LayerInfo);
    for (l = w->WLayer->front; l && !found; l = l->front) {
        struct Window *o = (struct Window *)l->Window;
        if (!o || o == w || !o->UserPort || o->UserPort->mp_SigTask != (APTR)t) continue;
        found = l->bounds.MinX <= w->WLayer->bounds.MaxX && l->bounds.MaxX >= w->WLayer->bounds.MinX &&
                l->bounds.MinY <= w->WLayer->bounds.MaxY && l->bounds.MaxY >= w->WLayer->bounds.MinY;
    }
    UnlockLayerInfo(&w->WScreen->LayerInfo);
    return found;
}

/* What the handler decided at each release, in order: the window to the front or the back, made
 * active, or lined up with an edge after a drag. Intuition has ended its drag before this runs (the
 * input task, at priority 20, takes the release through every handler first); a Intuition that
 * moves a dragged window at the next event (OS 3.2's) gets a frame for it before a snap. */
static void gestures_done(void)
{
    while (done_out != done_in) {
        struct done d = done_ring[done_out];
        struct Window *w = d.w;
        done_out = (done_out + 1) & (DONE_RING - 1);
        if (!on || !w || !is_window(w)) continue;
        if ((d.actions & GA_SNAP) && d.kind == GK_MOVE) {
            if (!intuition_drags) Delay(1);
            if (is_window(w) && w->Width == d.wd && w->Height == d.ht && (w->LeftEdge != d.l || w->TopEdge != d.t)) {
                if (cfg.halves && (d.x <= 1 || d.x >= w->WScreen->Width - 2)) halves(w, d.x);
                else snap(w);
            }
        }
        if (!is_window(w)) continue;
        if ((d.actions & GA_ACTIVATE) && w != IntuitionBase->ActiveWindow) ActivateWindow(w);
        /* 0.9.1: only when nothing came in front since the press: a popup, a requester or a drawer the click
         * opened after the release (its program had the release first) stays in front of it */
        if ((d.actions & GA_FRONT) && w->WScreen->LayerInfo.top_layer == d.top && !in_front(w) &&
            !own_requester_in_front(w))
            WindowToFront(w);
        if (d.actions & GA_BACK) WindowToBack(w);
    }
}

/* ---- resizing from an edge (0.5) ------------------------------------------------------------- */

/* The pointers shown while a window is resized: 16 x 16, the hot spot at the
 * middle (7, 7). '.' is clear, 'X' the outline (the pointer's second colour)
 * and 'o' the fill (its third). */
static const char *const shape_h[16] = {
    "................", "................", "................", "..XXX.....XXX...",
    ".XXoX.....XoXX..", "XXooX.....XooXX.", "XoooXXXXXXXoooXX", "oooooooooooooooX",
    "XoooXXXXXXXoooXX", "XXooX.....XooXX.", ".XXoX.....XoXX..", "..XXX.....XXX...",
    "................", "................", "................", "................",
};
static const char *const shape_v[16] = {
    ".....XXoXX......", "....XXoooXX.....", "...XXoooooXX....", "...XoooooooX....",
    "...XXXXoXXXX....", "......XoX.......", "......XoX.......", "......XoX.......",
    "......XoX.......", "......XoX.......", "...XXXXoXXXX....", "...XoooooooX....",
    "...XXoooooXX....", "....XXoooXX.....", ".....XXoXX......", "......XXX.......",
};
static const char *const shape_d1[16] = {                  /* the bottom right corner */
    "................", ".XXXXXXX........", ".XoooooX........", ".XooooXX........",
    ".XooooXX........", ".XoooooXX.......", ".XoXXoooXX......", ".XXXXXoooXXXXX..",
    ".....XXoooXXoX..", "......XXoooooX..", ".......XXooooX..", ".......XXooooX..",
    ".......XoooooX..", ".......XXXXXXX..", "................", "................",
};
static const char *const shape_d2[16] = {                  /* the bottom left corner */
    "................", ".......XXXXXXX..", ".......XoooooX..", ".......XXooooX..",
    ".......XXooooX..", "......XXoooooX..", ".....XXoooXXoX..", ".XXXXXoooXXXXX..",
    ".XoXXoooXX......", ".XoooooXX.......", ".XooooXX........", ".XooooXX........",
    ".XoooooX........", ".XXXXXXX........", "................", "................",
};
enum { PTR_H, PTR_V, PTR_D1, PTR_D2, PTR_COUNT };
static Object *ptr_obj[PTR_COUNT];

static void make_pointer(int i, const char *const rows[16])
{
    struct BitMap *bm = AllocBitMap(16, 16, 2, BMF_CLEAR, NULL);
    int x, y;
    if (!bm) return;
    for (y = 0; y < 16; y++) {
        UWORD p0 = 0, p1 = 0;
        for (x = 0; x < 16; x++) {
            UWORD bit = (UWORD)(0x8000 >> x);
            if (rows[y][x] == 'X') p1 |= bit;                       /* the second colour: plane 1 only */
            else if (rows[y][x] == 'o') { p0 |= bit; p1 |= bit; }   /* the third: both planes */
        }
        *(UWORD *)(bm->Planes[0] + y * bm->BytesPerRow) = p0;
        *(UWORD *)(bm->Planes[1] + y * bm->BytesPerRow) = p1;
    }
    ptr_obj[i] = NewObject(NULL, (STRPTR)"pointerclass", POINTERA_BitMap, (ULONG)bm, POINTERA_XOffset, -7, POINTERA_YOffset, -7,
                           POINTERA_WordWidth, 1, POINTERA_XResolution, POINTERXRESN_SCREENRES,
                           POINTERA_YResolution, POINTERYRESN_SCREENRESASPECT, TAG_DONE);
    /* pointerclass keeps its own copy of the picture: the bitmap (Chip RAM)
     * may go once NewObject returns, as its autodoc allows (0.10.1, Team 5's
     * chip audit: the four held 256 bytes of Chip RAM for good) */
    FreeBitMap(bm);
}

static void pointers_on(void)
{
    make_pointer(PTR_H, shape_h);
    make_pointer(PTR_V, shape_v);
    make_pointer(PTR_D1, shape_d1);
    make_pointer(PTR_D2, shape_d2);
}

static void pointers_off(void)
{
    int i;
    for (i = 0; i < PTR_COUNT; i++) {
        if (ptr_obj[i]) DisposeObject(ptr_obj[i]);
        ptr_obj[i] = NULL;
    }
}

static struct Window *rs_win;                   /* the window the main task is resizing */
static ULONG rs_gen, rs_gen_done;               /* its press (grab_gen), and the last press done with */
static int rs_edges, rs_ptr;
static WORD rs_l, rs_t, rs_w, rs_h, rs_x0, rs_y0;
static ULONG rs_start;                          /* when the press came (seconds), for the time limit */

/* The size outline (0.6.1): while the button is held only a rectangle is drawn
 * (two pixels, inverted, straight in the screen's bitmap, as Intuition's own
 * sizing does), and the window is sized once, when the button comes up. 0.6
 * called ChangeWindowBox for every move of the pointer: dozens of NEWSIZE and
 * REFRESHWINDOW a second to the program, each one a full redraw of the frame
 * in the look, all through the input task. A program could not keep up and
 * the whole input stream stopped behind it. */
static int ol_on;
static struct Screen *ol_scr;
static WORD ol_l, ol_t, ol_w, ol_h;

static int screen_alive(struct Screen *sc)
{
    struct Screen *x;
    ULONG lock = LockIBase(0);
    int found = 0;
    for (x = IntuitionBase->FirstScreen; x; x = x->NextScreen)
        if (x == sc) { found = 1; break; }
    UnlockIBase(lock);
    return found;
}

static void ol_xor(struct Screen *sc, WORD l, WORD t, WORD w, WORD h)
{
    struct RastPort *rp = &sc->RastPort;
    UBYTE dm = rp->DrawMode;
    WORD th = w < 6 || h < 6 ? 1 : 2;
    SetDrMd(rp, COMPLEMENT);
    RectFill(rp, l, t, l + w - 1, t + th - 1);                      /* top */
    RectFill(rp, l, t + h - th, l + w - 1, t + h - 1);              /* bottom */
    if (h > 2 * th) {
        RectFill(rp, l, t + th, l + th - 1, t + h - th - 1);        /* left */
        RectFill(rp, l + w - th, t + th, l + w - 1, t + h - th - 1); /* right */
    }
    SetDrMd(rp, dm);
}

static void ol_clear(void)
{
    if (ol_on && screen_alive(ol_scr)) ol_xor(ol_scr, ol_l, ol_t, ol_w, ol_h);
    ol_on = 0;
}

static void ol_show(struct Screen *sc, WORD l, WORD t, WORD w, WORD h)
{
    if (ol_on && ol_l == l && ol_t == t && ol_w == w && ol_h == h) return;
    ol_clear();
    ol_xor(sc, l, t, w, h);
    ol_scr = sc; ol_l = l; ol_t = t; ol_w = w; ol_h = h; ol_on = 1;
}

/* A window this drags (drag.solid) is drawn at the look's drag opacity while it moves, as Intuition's own solid drag,
 * on a screen layers.library composites (GFOpenLayers 47.101 and up: SetLayerLook, -258, LAYERLOOK_DRAGGED;
 * gfopengfx include/graphics/openlayers.h). Nothing on a classic screen, or with an older layers.library. */
static void drag_look(struct Window *w, ULONG on)
{
    struct Layer *ly[2];
    int i;
    if (LayersBase->lib_Version != 47 || LayersBase->lib_Revision < 101) return;
    ly[0] = w->WLayer;
    ly[1] = w->BorderRPort && w->BorderRPort->Layer != w->WLayer ? w->BorderRPort->Layer : NULL;    /* GZZ */
    for (i = 0; i < 2; i++)
        if (ly[i]) {
            /* the base through a1, A6 saved by hand (A6 is the frame pointer at -O0) */
            register struct Library *a1 __asm("a1") = LayersBase;
            register struct Layer *a0 __asm("a0") = ly[i];
            register ULONG d0 __asm("d0") = 2, d1 __asm("d1") = on;
            __asm__ volatile("move.l %%a6,-(%%sp)\n\tmove.l %%a1,%%a6\n\tjsr -258(%%a6)\n\tmove.l (%%sp)+,%%a6"
                             : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1) : : "cc", "memory");
        }
}

static void resize_end(void)
{
    ol_clear();
    if (rs_win && (rs_edges & EDGE_MOVE) && is_window(rs_win)) drag_look(rs_win, 0);
    if (rs_win && rs_ptr && is_window(rs_win)) SetWindowPointer(rs_win, WA_Pointer, (ULONG)NULL, TAG_DONE);
    rs_win = NULL;
    rs_ptr = 0;
}

/* The drag is over without a button-up (the release never came): the
 * outline goes and the window is left as it is. The handler always ends a
 * drag at the button-up; this is for the one that never arrives. */
#define DRAG_IDLE_SECS 8
#define DRAG_MAX_SECS 120
static void resize_watch(void)
{
    ULONG secs, micro;
    if (!rs_win || !grab_on) return;
    CurrentTime(&secs, &micro);
    if (secs - rs_start > DRAG_MAX_SECS || (secs - grab_last > DRAG_IDLE_SECS && grab_last <= secs)) {
        Forbid(); if (grab_gen == rs_gen) grab_on = 0; Permit();
        rs_gen_done = rs_gen;
        resize_end();
    }
}

/* The pointer has moved, or the button has come up. The box the window
 * would have: the edge or corner held follows the pointer, the others stay;
 * it keeps to the window's own limits (a MaxWidth of 0 or 65535 is the
 * screen's size) and to the screen. While the button is held the outline
 * shows it; at the button-up the window is sized, once. */
/* 0.8.4: the pointer over a grip of the active window shows which way it sizes. A window's own pointer can't
 * be read back, so leaving the grip gives the window the default one. */
static struct Window *ptr_win;
static int ptr_k = -1;

static void hover(void)
{
    struct Window *hw = hover_win;
    int e = hover_edges, k = -1;
    if (rs_win) return;                          /* a drag shows its own */
    if (hw && e && is_window(hw)) k = !(e & EDGE_B) ? PTR_H : (e & EDGE_R) ? PTR_D1 : (e & EDGE_L) ? PTR_D2 : PTR_V;
    if (k >= 0 && hw == ptr_win && k == ptr_k) return;
    if (ptr_win && (ptr_win != hw || k < 0) && is_window(ptr_win)) SetWindowPointer(ptr_win, TAG_DONE);
    ptr_win = NULL; ptr_k = -1;
    if (k >= 0 && ptr_obj[k]) {
        SetWindowPointer(hw, WA_Pointer, (ULONG)ptr_obj[k], TAG_DONE);
        ptr_win = hw; ptr_k = k;
    }
}

static void resize(void)
{
    for (;;) {
        struct Window *w, *gw;
        struct Screen *s;
        WORD dx, dy, l, t, wd, ht, maxw, maxh, px, py, ex, ey, gx0, gy0;
        ULONG gen, endgen;
        int gedges, ended;
        /* what the handler has, in one piece (it runs in input.device's task: Forbid keeps it out) */
        Forbid();
        gen = grab_gen; endgen = grab_end_gen; gw = grab_win; gedges = grab_edges; gx0 = grab_x0; gy0 = grab_y0;
        px = grab_px; py = grab_py; ex = grab_ex; ey = grab_ey;
        Permit();
        if (!rs_win) {                              /* a new press on an edge, or a title bar to drag */
            if (!gw || gen == rs_gen_done) return;
            w = gw;
            rs_gen = gen;
            if (!is_window(w)) { rs_gen_done = gen; Forbid(); if (grab_gen == gen) grab_on = 0; Permit(); return; }
            rs_win = w; rs_edges = gedges; rs_x0 = gx0; rs_y0 = gy0;
            rs_l = w->LeftEdge; rs_t = w->TopEdge; rs_w = w->Width; rs_h = w->Height;
            { ULONG micro; CurrentTime(&rs_start, &micro); }
            if (w != IntuitionBase->ActiveWindow) ActivateWindow(w);
            if (rs_edges & EDGE_MOVE) drag_look(w, 1);
            if (!(rs_edges & EDGE_MOVE)) {          /* a solid move keeps the window's own pointer */
                int k = !(rs_edges & EDGE_B) ? PTR_H : (rs_edges & EDGE_R) ? PTR_D1 : (rs_edges & EDGE_L) ? PTR_D2 : PTR_V;
                /* only when asked for: the window's own pointer can't be read, so
                 * the end of the drag gives it the default one */
                if (cfg.edge_ptr && ptr_obj[k]) { SetWindowPointer(w, WA_Pointer, (ULONG)ptr_obj[k], TAG_DONE); rs_ptr = 1; }
            }
        }
        w = rs_win;
        if (!is_window(w)) {
            ol_on = 0; rs_win = NULL; rs_ptr = 0; rs_gen_done = rs_gen;
            Forbid(); if (grab_gen == rs_gen) grab_on = 0; Permit();
            continue;
        }
        /* 0.9: the place the handler saw with the drag's last event, or at its release: never one made after it */
        ended = endgen == rs_gen || gen != rs_gen;
        if (endgen == rs_gen) { px = ex; py = ey; }
        s = w->WScreen;
        dx = px - rs_x0;
        dy = py - rs_y0;
        if (rs_edges & EDGE_MOVE) {                 /* 0.8: the window itself follows the pointer, contents and all */
            l = rs_l + dx; t = rs_t + dy;
            if (l < 0) l = 0;
            if (t < 0) t = 0;
            if (l + w->Width > s->Width) l = s->Width - w->Width;
            if (t + w->Height > s->Height) t = s->Height - w->Height;
            if (l != w->LeftEdge || t != w->TopEdge) MoveWindow(w, l - w->LeftEdge, t - w->TopEdge);
            if (ended) {
                rs_gen_done = rs_gen;
                resize_end();
                if (cfg.halves && (px <= 1 || px >= s->Width - 2)) halves(w, px);
                else if (cfg.snap) { Delay(1); if (is_window(w)) snap(w); }   /* after the last move has been done */
                continue;                           /* a press that came after the release: its drag now */
            }
            return;
        }
        l = rs_l; t = rs_t; wd = rs_w; ht = rs_h;
        maxw = w->MaxWidth == 0 || w->MaxWidth > (UWORD)s->Width ? s->Width : (WORD)w->MaxWidth;
        maxh = w->MaxHeight == 0 || w->MaxHeight > (UWORD)s->Height ? s->Height : (WORD)w->MaxHeight;
        if (rs_edges & EDGE_R) wd += dx;
        if (rs_edges & EDGE_L) wd -= dx;
        if (rs_edges & EDGE_B) ht += dy;
        if (wd < w->MinWidth) wd = w->MinWidth;
        if (wd > maxw) wd = maxw;
        if (ht < w->MinHeight) ht = w->MinHeight;
        if (ht > maxh) ht = maxh;
        if (rs_edges & EDGE_L) l = rs_l + rs_w - wd;          /* the right edge stays where it was */
        if (l < 0) { wd += l; l = 0; }
        if (l + wd > s->Width) wd = s->Width - l;
        if (t + ht > s->Height) ht = s->Height - t;
        if (wd < w->MinWidth || ht < w->MinHeight) { wd = w->Width; ht = w->Height; l = w->LeftEdge; }   /* nothing sensible: no change */
        if (ended) {
            rs_gen_done = rs_gen;
            ol_clear();                                          /* the outline goes first, then the one size */
            if (wd != w->Width || ht != w->Height || l != w->LeftEdge)
                ChangeWindowBox(w, l, t, wd, ht);
            resize_end();
            continue;
        }
        ol_show(s, l, t, wd, ht);
        return;
    }
}

/* A window Amiga+Tab offers: not a backdrop, not tiny, and not Workbench's
   own root window (a Workbench window with no close gadget). */
static int switchable(struct Window *w)
{
    if (!w || (w->Flags & WFLG_BACKDROP) || w->Width < 8 || w->Height < 8) return 0;
    if ((w->Flags & WFLG_WBENCHWINDOW) && !(w->Flags & WFLG_CLOSEGADGET)) return 0;
    return 1;
}

/* Amiga+Tab: the screen's windows from front to back, by their layers. */
static void switcher(int back)
{
    struct Screen *s = IntuitionBase->FirstScreen;
    struct Layer *l;
    struct Window *front = NULL, *second = NULL, *rear = NULL;
    if (!s) return;
    LockLayerInfo(&s->LayerInfo);
    for (l = s->LayerInfo.top_layer; l; l = l->back) {
        struct Window *w = (struct Window *)l->Window;
        if (!switchable(w)) continue;
        if (!front) front = w;
        else if (!second) second = w;
        rear = w;
    }
    UnlockLayerInfo(&s->LayerInfo);
    if (!front || front == rear) { if (front) ActivateWindow(front); return; }
    if (back) {                       /* the front one to the back; the one behind it comes up */
        WindowToBack(front);
        ActivateWindow(second);
    } else {                          /* the one at the back to the front */
        WindowToFront(rear);
        ActivateWindow(rear);
    }
}

/* ---- places --------------------------------------------------------------------------------- */

/* The kept places (places.h: keys, the placing rule, the tags). Read by the patch on other tasks: 0.9.2 reads
 * them under Forbid(), and this task changes them only under Forbid() (a few bytes copied, nothing that can
 * wait), so the patch never waits and never misses them. 0.7 to 0.9.1 read them with AttemptSemaphoreShared
 * and opened a window as asked whenever the look held them, which it did across its moves and file writes. */
static struct place places[PLACES_MAX];
static int nplaces, places_dirty;
static ULONG places_clock;                      /* each use: a place's "used" (the file's order: used longest ago first) */
static volatile ULONG looks;                    /* the once-a-second looks so far */
/* every window the look knows: its box at the last look, the key it is known by, fixed when it was first
 * seen (0.9.2: the title it opened with, when the patch saw it open; a title changed later keeps the key),
 * and the key its title gave at the last look (cur) */
static struct { struct Window *w; WORD l, t, wd, ht; char key[64], cur[64]; } seen[MAX_SEEN];
static int nseen;

static void place_log(const char *line)
{
    BPTR fh;
    if (!log_path[0] || !(fh = Open((STRPTR)log_path, MODE_READWRITE))) return;
    Seek(fh, 0, OFFSET_END);
    FPuts(fh, (STRPTR)line);
    Close(fh);
}

/* The program behind a task: its name, or the command a Shell runs. NULL: a
   window with no IDCMP (a console, say), known by its title as "window". */
static void program_of_task(struct Task *t, char *out, int size)
{
    *out = 0;
    if (!t) { strncpy(out, "window", size - 1); return; }
    Forbid();
    if (t->tc_Node.ln_Type == NT_PROCESS && ((struct Process *)t)->pr_CLI) {
        struct CommandLineInterface *cli = BADDR(((struct Process *)t)->pr_CLI);
        UBYTE *bs = cli ? BADDR(cli->cli_CommandName) : NULL;
        if (bs && *bs) {
            int n = *bs < size - 1 ? *bs : size - 1;
            const char *base;
            memcpy(out, bs + 1, n); out[n] = 0;
            base = strrchr(out, '/'); if (!base) base = strchr(out, ':');
            if (base) memmove(out, base + 1, strlen(base + 1) + 1);
        }
    }
    if (!*out && t->tc_Node.ln_Name) { strncpy(out, t->tc_Node.ln_Name, size - 1); out[size - 1] = 0; }
    Permit();
}

/* The program behind a window: the task its IDCMP port signals. */
static void program_of(struct Window *w, char *out, int size)
{
    program_of_task(w->UserPort ? (struct Task *)w->UserPort->mp_SigTask : NULL, out, size);
}

static int never(const char *prog)
{
    int i;
    for (i = 0; i < cfg.nnever; i++)
        if (!stricmp(cfg.never[i], prog)) return 1;
    return 0;
}

/* ---- the wheel (0.11, wheel.h): the main task's side ------------------------------------------- */

static struct MsgPort *wport;                   /* our IntuiMessages come back here */
static struct IOStdReq *wio;                    /* input.device: the held events sent on */
static int wout;                                /* IntuiMessages out */
static struct { struct Window *w; struct Gadget *g[2]; } wfoc[WCLS];   /* each window's scroll focus: [0] down, [1] across */
static int wfoc_next, wcls_next;
static ULONG wclick_done;
static struct { struct Window *w; struct Gadget *g; unsigned dir; int sign, could; LONG before; int armed; } probe;

static void wcls_set(struct Window *w, int c)
{
    int i;
    for (i = 0; i < WCLS; i++) if (wcls[i].w == w) break;
    if (i == WCLS) { i = wcls_next; wcls_next = (wcls_next + 1) % WCLS; }
    Forbid();                                   /* the handler reads the pair */
    wcls[i].w = w; wcls[i].cls = (UBYTE)c;
    Permit();
}

/* windows gone: their class and focus forgotten, so a new window at the same place starts afresh */
static void wheel_prune(void)
{
    int i;
    for (i = 0; i < WCLS; i++) {
        if (wcls[i].w && !is_window(wcls[i].w)) { Forbid(); wcls[i].w = NULL; Permit(); }
        if (wfoc[i].w && !is_window(wfoc[i].w)) wfoc[i].w = NULL;
    }
    if (probe.armed && !is_window(probe.w)) probe.armed = 0;
}

static int wself(struct Window *w)
{
    char prog[48];
    int i;
    if (!cfg.nwself) return 0;
    program_of(w, prog, sizeof prog);
    for (i = 0; i < cfg.nwself; i++) if (!stricmp(cfg.wself[i], prog)) return 1;
    return 0;
}

/* A gadget that is a scroller: WS_VERT, WS_HORIZ or both; 0 not one. A plain prop (GadTools' scrollers, sliders
 * and lists' scrollers are plain props too), or a BOOPSI one that answers PGA_Top, PGA_Total and PGA_Visible
 * (propgclass, and classes built on it). */
static unsigned scroller_of(struct Gadget *g)
{
    if (g->Flags & GFLG_DISABLED) return 0;
    switch (g->GadgetType & GTYP_GTYPEMASK) {
    case GTYP_PROPGADGET: {
        struct PropInfo *pi = (struct PropInfo *)g->SpecialInfo;
        if (!pi) return 0;
        return ((pi->Flags & FREEVERT) ? WS_VERT : 0) | ((pi->Flags & FREEHORIZ) ? WS_HORIZ : 0);
    }
    case GTYP_CUSTOMGADGET: {
        ULONG v = 0;
        if (!GetAttr(PGA_Top, (Object *)g, &v) || !GetAttr(PGA_Total, (Object *)g, &v) || !GetAttr(PGA_Visible, (Object *)g, &v)) return 0;
        if (GetAttr(PGA_Freedom, (Object *)g, &v)) return v == FREEHORIZ ? WS_HORIZ : WS_VERT;
        return g->Width > g->Height ? WS_HORIZ : WS_VERT;
    }
    }
    return 0;
}

/* The window's scrollers, their boxes in the window's own pixels (from its outer top left) */
#define WMAXG 48
static int scrollers(struct Window *w, struct wheel_box *b, struct Gadget **gs)
{
    struct Gadget *g;
    int n = 0;
    ULONG lock = LockIBase(0);
    for (g = w->FirstGadget; g && n < WMAXG; g = g->NextGadget) {
        unsigned f = scroller_of(g);
        WORD W = w->Width, H = w->Height, ox = 0, oy = 0;
        if (!f) continue;
        if ((w->Flags & WFLG_GIMMEZEROZERO) && !(g->Activation & (GACT_RIGHTBORDER | GACT_LEFTBORDER | GACT_TOPBORDER | GACT_BOTTOMBORDER))) {
            W = w->GZZWidth; H = w->GZZHeight; ox = w->BorderLeft; oy = w->BorderTop;
        }
        b[n].l = g->LeftEdge + ((g->Flags & GFLG_RELRIGHT) ? W - 1 : 0) + ox;
        b[n].t = g->TopEdge + ((g->Flags & GFLG_RELBOTTOM) ? H - 1 : 0) + oy;
        b[n].w = g->Width + ((g->Flags & GFLG_RELWIDTH) ? W : 0);
        b[n].h = g->Height + ((g->Flags & GFLG_RELHEIGHT) ? H : 0);
        if (((g->Activation & GACT_RIGHTBORDER) && (f & WS_VERT)) || ((g->Activation & GACT_BOTTOMBORDER) && (f & WS_HORIZ))) f |= WS_BORDER;
        b[n].flags = f;
        gs[n++] = g;
    }
    UnlockIBase(lock);
    return n;
}

static int has_gadget(struct Window *w, struct Gadget *want)
{
    struct Gadget *g;
    ULONG lock = LockIBase(0);
    for (g = w->FirstGadget; g && g != want; g = g->NextGadget) ;
    UnlockIBase(lock);
    return g != NULL;
}

/* A left press came (the handler's wclick): in the window it made active, the scroller pushed or beside it is
 * the scroll focus, down and across; a press beside none leaves the focus as it was */
static void wheel_click(void)
{
    struct wheel_box b[WMAXG];
    struct Gadget *gs[WMAXG];
    struct Screen *s = wclick.s;
    WORD x = wclick.x, y = wclick.y;
    struct Window *w;
    int n, d, i, k;
    if (!s || !(w = window_at(s, x, y)) || w != IntuitionBase->ActiveWindow) return;
    if (!(n = scrollers(w, b, gs))) return;
    for (k = 0; k < WCLS; k++) if (wfoc[k].w == w) break;
    if (k == WCLS) { k = wfoc_next; wfoc_next = (wfoc_next + 1) % WCLS; wfoc[k].w = w; wfoc[k].g[0] = wfoc[k].g[1] = NULL; }
    for (d = 0; d < 2; d++)
        if ((i = wheel_pick(b, n, x - w->LeftEdge, y - w->TopEdge, d ? WS_HORIZ : WS_VERT)) >= 0) wfoc[k].g[d] = gs[i];
}

/* The scroller a step moves in w: the focus (or, under the pointer, the one beside it), else the window's main one */
static struct Gadget *wheel_target(struct Window *w, unsigned dir, int at_pointer)
{
    struct wheel_box b[WMAXG];
    struct Gadget *gs[WMAXG];
    int n = scrollers(w, b, gs), i = -1, k;
    if (!n) return NULL;
    if (at_pointer) i = wheel_pick(b, n, w->MouseX, w->MouseY, dir);
    else
        for (k = 0; k < WCLS; k++)
            if (wfoc[k].w == w && wfoc[k].g[dir == WS_HORIZ])
                for (i = 0; i < n; i++) if (gs[i] == wfoc[k].g[dir == WS_HORIZ] && (b[i].flags & dir)) return gs[i];
    if (i < 0 || i >= n) i = wheel_main(b, n, dir);
    return i >= 0 ? gs[i] : NULL;
}

/* g's view, down or across, in pixels: its box as scrollers() finds it */
static int view_of(struct Window *w, struct Gadget *g, unsigned dir)
{
    struct wheel_box b[WMAXG];
    struct Gadget *gs[WMAXG];
    int n = scrollers(w, b, gs), i;
    for (i = 0; i < n && gs[i] != g; i++) ;
    return i < n ? (dir == WS_HORIZ ? b[i].w : b[i].h) : (dir == WS_HORIZ ? g->Width : g->Height);
}

static int boopsi(struct Gadget *g) { return (g->GadgetType & GTYP_GTYPEMASK) == GTYP_CUSTOMGADGET; }

/* Where a scroller is now (a plain prop's pot, a BOOPSI one's top), and whether it could move up (*up) or down */
static LONG wheel_state(struct Gadget *g, unsigned dir, int *up, int *down)
{
    if (boopsi(g)) {
        ULONG top = 0, total = 0, vis = 0;
        GetAttr(PGA_Top, (Object *)g, &top); GetAttr(PGA_Total, (Object *)g, &total); GetAttr(PGA_Visible, (Object *)g, &vis);
        *up = top > 0; *down = (LONG)top < (LONG)total - (LONG)vis;
        return (LONG)top;
    } else {
        struct PropInfo *pi = (struct PropInfo *)g->SpecialInfo;
        UWORD pot = dir == WS_HORIZ ? pi->HorizPot : pi->VertPot, body = dir == WS_HORIZ ? pi->HorizBody : pi->VertBody;
        *up = body < MAXBODY && pot > 0; *down = body < MAXBODY && pot < MAXPOT;
        return pot;
    }
}

/* An IntuiMessage to w's program, as Intuition sends it while the knob is dragged; it comes back to wport */
static void wheel_imsg(struct Window *w, ULONG cls, UWORD code, UWORD qual, struct Gadget *g)
{
    struct ExtIntuiMessage *m;
    if (!wport || !w->UserPort || !(w->IDCMPFlags & cls)) return;
    if (!(m = AllocVec(sizeof *m, MEMF_PUBLIC | MEMF_CLEAR))) return;
    m->eim_IntuiMessage.ExecMessage.mn_Node.ln_Type = NT_MESSAGE;
    m->eim_IntuiMessage.ExecMessage.mn_ReplyPort = wport;
    m->eim_IntuiMessage.ExecMessage.mn_Length = sizeof(struct IntuiMessage);
    m->eim_IntuiMessage.Class = cls;
    m->eim_IntuiMessage.Code = code;
    m->eim_IntuiMessage.Qualifier = qual;
    m->eim_IntuiMessage.IAddress = g;
    m->eim_IntuiMessage.MouseX = w->MouseX;
    m->eim_IntuiMessage.MouseY = w->MouseY;
    CurrentTime(&m->eim_IntuiMessage.Seconds, &m->eim_IntuiMessage.Micros);
    m->eim_IntuiMessage.IDCMPWindow = w;
    wout++;
    PutMsg(w->UserPort, &m->eim_IntuiMessage.ExecMessage);
}

/* A drag's messages: down (GACT_IMMEDIATE), a move (GACT_FOLLOWMOUSE), up (GACT_RELVERIFY), each only where the
 * window asked for its class */
static void wheel_drag(struct Window *w, struct Gadget *g, UWORD code, UWORD qual)
{
    if (g->Activation & GACT_IMMEDIATE) wheel_imsg(w, IDCMP_GADGETDOWN, code, qual, g);
    if (g->Activation & GACT_FOLLOWMOUSE) wheel_imsg(w, IDCMP_MOUSEMOVE, code, qual, g);
    if (g->Activation & GACT_RELVERIFY) wheel_imsg(w, IDCMP_GADGETUP, code, qual, g);
}

/* One step (sign) on scroller g of w, cfg.wheel_lines lines: as a drag of its knob would move it, and the
 * program told as a drag tells it */
static void wheel_scroll(struct Window *w, struct Gadget *g, unsigned dir, int sign, UWORD qual)
{
    LONG lines = (LONG)cfg.wheel_lines * sign;
    if (!is_window(w) || !has_gadget(w, g)) return;
    if (boopsi(g)) {
        /* propgclass and its kin: OM_UPDATE, as an interconnected gadget would send it; the gadget draws itself and
         * notifies its target (IDCMP_IDCMPUPDATE for a program that takes ICTARGET_IDCMP) */
        ULONG top = 0, total = 0, vis = 0;
        struct TagItem tags[3];
        struct opUpdate up;
        LONG nt;
        struct RastPort *rp = w->RPort;
        int line = rp && rp->TxHeight > 0 ? rp->TxHeight : 8;
        GetAttr(PGA_Top, (Object *)g, &top); GetAttr(PGA_Total, (Object *)g, &total); GetAttr(PGA_Visible, (Object *)g, &vis);
        if (dir == WS_HORIZ && rp && rp->TxWidth > 0) line = rp->TxWidth;
        /* a top counted in items or in pixels: lines into its units by what its view shows */
        lines *= wheel_units((LONG)vis, view_of(w, g, dir), line);
        if ((nt = wheel_top((LONG)top, (LONG)total, (LONG)vis, lines)) == (LONG)top) return;
        tags[0].ti_Tag = PGA_Top; tags[0].ti_Data = (ULONG)nt;
        tags[1].ti_Tag = GA_ID; tags[1].ti_Data = g->GadgetID;
        tags[2].ti_Tag = TAG_DONE;
        up.MethodID = OM_UPDATE; up.opu_AttrList = tags; up.opu_GInfo = NULL; up.opu_Flags = 0;
        DoGadgetMethodA(g, w, NULL, (Msg)&up);
        if (log_path[0]) {
            char line[96];
            ULONG now = 0;
            GetAttr(PGA_Top, (Object *)g, &now);
            snprintf(line, sizeof line, "wheel: top %lu -> %lu (asked %ld of %lu, %lu shown)\n", (unsigned long)top,
                     (unsigned long)now, (long)nt, (unsigned long)total, (unsigned long)vis);
            place_log(line);
        }
        return;
    }
    /* a plain prop, GadTools' scrollers, sliders and lists' scrollers too: the knob moved, then the drag's messages
     * (GT_GetIMsg reads the knob for its own: a scroller's top, a slider's level, a list's place) */
    {
        struct PropInfo *pi = (struct PropInfo *)g->SpecialInfo;
        struct RastPort *rp = w->RPort;
        int view, line, line_h;
        UWORD pot, np;
        view = view_of(w, g, dir);
        line = rp ? (dir == WS_HORIZ ? rp->TxWidth : rp->TxHeight) : 8;
        if (line < 1) line = 8;
        line_h = line;
        pot = dir == WS_HORIZ ? pi->HorizPot : pi->VertPot;
        np = (UWORD)wheel_pot(pot, dir == WS_HORIZ ? pi->HorizBody : pi->VertBody, view, line, (int)lines);
        if (np == pot) return;
        NewModifyProp(g, w, NULL, pi->Flags, dir == WS_HORIZ ? np : pi->HorizPot, dir == WS_HORIZ ? pi->VertPot : np,
                      pi->HorizBody, pi->VertBody, 1);
        if (log_path[0]) {
            char line[96];
            snprintf(line, sizeof line, "wheel: pot %u -> %u (asked %u; view %d, line %d)\n", (unsigned)pot,
                     (unsigned)(dir == WS_HORIZ ? pi->HorizPot : pi->VertPot), (unsigned)np, view, line_h);
            place_log(line);
        }
        wheel_drag(w, g, 0, qual);
    }
}

/* A held event on to Intuition and the rest, past our handler */
static void wheel_send_on(struct InputEvent *ev)
{
    struct InputEvent e = *ev;
    if (!wio) return;
    e.ie_NextEvent = NULL;
    pass_wheel++;
    wio->io_Command = IND_WRITEEVENT;
    wio->io_Data = &e;
    wio->io_Length = sizeof e;
    DoIO((struct IORequest *)wio);
}

static void wheel_activate(struct Window *w)
{
    if (w && w != IntuitionBase->ActiveWindow && !(w->Flags & WFLG_BACKDROP)) {
        ActivateWindow(w);
        Delay(1);                              /* Intuition makes it active on its next event */
    }
}

static void wheel(void)
{
    if (wclick.gen != wclick_done) { wclick_done = wclick.gen; wheel_prune(); wheel_click(); }
    while (wq_out != wq_in) {
        struct wq e = wq[wq_out];
        struct Window *w = e.w;
        unsigned dir = WS_VERT;
        int sign, c, pointer = cfg.wheel_mode == 2, up = 0, down = 0;
        struct Gadget *g;
        UWORD code = e.ev.ie_Code;
        wq_out = (wq_out + 1) & (WQ - 1);
        sign = wheel_step(code & 0x7F, e.ev.ie_Qualifier, &dir);   /* its step's way, for the let-go too */
        if (pointer) {
            struct Screen *s = IntuitionBase->FirstScreen;
            w = s ? window_at(s, s->MouseX, s->MouseY) : NULL;
        }
        if (!w || !is_window(w) || !sign) { if (e.held) wheel_send_on(&e.ev); continue; }
        c = wcls_of(w);
        if (c != WC_SELF && wself(w)) wcls_set(w, c = WC_SELF);
        g = c == WC_SELF ? NULL : wheel_target(w, dir, pointer);
        if (!g) {                              /* the program's own wheel, or nothing here to move: as it came */
            if (e.held) { if (pointer) wheel_activate(w); wheel_send_on(&e.ev); }
            continue;
        }
        if (c == WC_EMULATE) {
            if (e.held && w == IntuitionBase->ActiveWindow) wheel_send_on(&e.ev);
            if (e.ev.ie_Class == IECLASS_RAWKEY && !(code & IECODE_UP_PREFIX)) wheel_scroll(w, g, dir, sign, e.ev.ie_Qualifier);
            continue;
        }
        /* not asked yet: the step goes to the program as it came; did its scroller move by itself? */
        if (pointer) wheel_activate(w);
        if (!probe.armed || probe.w != w) {
            probe.armed = 1; probe.w = w; probe.g = g; probe.dir = dir; probe.sign = sign;
            probe.before = wheel_state(g, dir, &up, &down);
            probe.could = sign > 0 ? down : up;
        }
        wheel_send_on(&e.ev);
        if (e.ev.ie_Class == IECLASS_RAWKEY && (code & IECODE_UP_PREFIX)) {   /* the step's last event */
            LONG now;
            Delay(4);                          /* 80 ms for the program to take it */
            probe.armed = 0;
            if (!is_window(w) || !has_gadget(w, probe.g)) continue;
            now = wheel_state(probe.g, probe.dir, &up, &down);
            c = wheel_probe(now != probe.before, probe.could);
            if (c != WC_UNKNOWN) wcls_set(w, c);
            if (log_path[0]) {                 /* ENV:OpenWindowsLog: what the wheel made of the window */
                char prog[48], line[160];
                program_of(w, prog, sizeof prog);
                snprintf(line, sizeof line, "wheel %s \"%.40s\": %s (scroller %ld -> %ld)\n", prog, w->Title ? (char *)w->Title : "",
                         c == WC_SELF ? "scrolls itself" : c == WC_EMULATE ? "moved by GFOpenWindows" : "at its end, asked again later",
                         (long)probe.before, (long)now);
                place_log(line);
            }
            if (c == WC_EMULATE) wheel_scroll(w, probe.g, probe.dir, probe.sign, e.ev.ie_Qualifier);
        }
    }
}

/* Our IntuiMessages back from the programs */
static void wheel_replies(void)
{
    struct Message *m;
    while (wport && (m = GetMsg(wport))) { FreeVec(m); wout--; }
}

static void read_places(void)
{
    char *text = read_file(PLACES_ENV), *p;
    nplaces = 0;
    if (!text) text = read_file(PLACES_ENVARC);
    if (!text) return;
    for (p = text; *p && nplaces < PLACES_MAX; ) {
        char *line = p;
        while (*p && *p != '\n') p++;
        if (*p) *p++ = 0;
        if (place_parse(line, &places[nplaces])) { places[nplaces].used = nplaces; nplaces++; }
    }
    places_clock = nplaces;                     /* a place used from now on is newer than every one read */
    FreeVec(text);
}

static void write_places(const char *path)
{
    BPTR fh = Open((STRPTR)path, MODE_NEWFILE);
    int order[PLACES_MAX], i;
    if (!fh) {
        BPTR d = CreateDir((STRPTR)(path[0] == 'E' && path[3] == ':' ? "ENV:GFOpenPrefs" : "ENVARC:GFOpenPrefs"));
        if (d) UnLock(d);
        if (!(fh = Open((STRPTR)path, MODE_NEWFILE))) return;
    }
    FPuts(fh, (STRPTR)"; GFOpenWindows: where each program's windows were last left\n");
    place_order(places, nplaces, order);        /* 0.9.2: the one used longest ago first */
    for (i = 0; i < nplaces; i++) {
        const struct place *pl = &places[order[i]];
        FPrintf(fh, (STRPTR)"\"%s\" %ld %ld %ld %ld%s\n", (LONG)pl->key, (LONG)pl->l, (LONG)pl->t, (LONG)pl->w, (LONG)pl->h,
                (LONG)(pl->snap ? " snap" : ""));
    }
    Close(fh);
}

/* A window whose place is kept: a program's (not one never to be touched),
   or a Workbench drawer when "places.drawers" is on (Workbench's own windows
   otherwise keep to its Snapshot; 0.9.2: GFOpenBench's are Workbench's too) */
static int placeable(int wb, int drawer, const char *prog)
{
    if (!prog[0] || never(prog)) return 0;
    if (!wb) return 1;
    return drawer && cfg.places_wb;
}

/* 0.10: a window that may be snapped with the gadget: as above, and a Workbench drawer whatever "Workbench
 * drawers too" says (a snapshot is the user's own choice; Workbench's own Snapshot still writes the icon) */
static int snappable(int wb, int drawer, const char *prog)
{
    if (!prog[0] || never(prog)) return 0;
    return !wb || drawer;
}

/* ---- placed before they open (0.7; 0.9.2) ----------------------------------------------------- */

/* Up to 0.6 a new window was moved to its place at the next once-a-second
 * look, so it opened where it asked and then jumped. From 0.7 OpenWindowTagList
 * and OpenWindow are patched, as GFOpenLook patches them and stacked with it
 * whichever loads first: a window whose place is kept opens there.
 *
 * 0.9.2 (the user, 24 October 2026: "make sure it reads the saved position before it renders the window.
 * Right now it jumps from one location to another"): the program's own WA_Left and WA_Top came after ours,
 * and GFOpenIntuition reads a tag's last (the open ROM): every window that gave its corner in its tags (the
 * Shell, GFOpenBench's drawers, most programs) opened where it asked and the look moved it a second later.
 * Now the window gets a copy of the program's tags without its corner (and without its size when the size
 * is ours) behind ours, and a copy of its NewWindow and ExtNewWindow tags the same way: the program's own are
 * left as they are, and ours are the only ones, read first or last. A window asked for by its inner size
 * (the Shell's CON: window from guest #472, ReAction's) is sized too, by its whole kept size. The look moves
 * nothing any more: a window the patch didn't see open stays where it opened.
 *
 * The patch runs on the opening program's task. It never waits: the places are read under Forbid(). It
 * places nothing while the commodity is disabled, for GFOpenWindows' own windows, or once it has been told
 * to pass everything through (quitting while another program has patched on top of it). */

static APTR old_owtl, old_ow;           /* what the patch replaced */
/* 0.8.4: windows the cog placed: the look leaves them where they opened (it moved them on to their kept place
 * a second later, the window jumping). Written by the patch on the opening task, read by the look. */
#define COG_WINS 4
static struct Window *volatile cog_wins[COG_WINS];
static volatile UBYTE cog_next;
static volatile UWORD patch_in;         /* callers inside the patch now */
static volatile UBYTE patch_pass;       /* 1: place nothing */
/* 0.9.2: the windows the patch saw open, for the look: the title each opened with (the key it is known by,
 * whatever its program calls it later), whether it opened at its kept place, and the look it opened in.
 * Written by the patch on the opening task, read and cleared by the look, both under Forbid(). */
#define OPENED 16
static struct { struct Window *w; char title[40]; UBYTE placed; ULONG look; char why[40]; } opened[OPENED];   /* placed: 1 its place, 2 centred (0.10) */
static UBYTE opened_next;
static void snap_attach(struct Window *w, int from_look);     /* 0.10: the snapshot gadget (below) */

static struct TagItem *tag_of(struct TagItem *tags, struct TagItem *ext, ULONG tag)
{
    struct TagItem *ti = tags ? FindTagItem(tag, tags) : NULL;
    if (!ti && ext) ti = FindTagItem(tag, ext);
    return ti;
}

static ULONG tag_or(struct TagItem *tags, struct TagItem *ext, ULONG tag, ULONG dflt)
{
    struct TagItem *ti = tag_of(tags, ext, tag);
    return ti ? ti->ti_Data : dflt;
}

/* The width and height of the screen a window is about to open on; 0 0 when
   it can't be told (then the window opens as asked). */
static void opening_screen(struct NewWindow *nw, struct TagItem *tags, struct TagItem *ext, WORD *sw, WORD *sh)
{
    struct Screen *s = (struct Screen *)tag_or(tags, ext, WA_CustomScreen, 0), *locked = NULL;
    *sw = *sh = 0;
    if (!s) s = (struct Screen *)tag_or(tags, ext, WA_PubScreen, 0);
    if (!s && nw && (nw->Type & SCREENTYPE) == CUSTOMSCREEN) s = nw->Screen;
    if (!s) {
        STRPTR name = (STRPTR)tag_or(tags, ext, WA_PubScreenName, 0);
        locked = LockPubScreen(name);           /* NULL: the default public screen */
        s = locked;
    }
    if (s) { *sw = s->Width; *sh = s->Height; }
    if (locked) UnlockPubScreen(NULL, locked);
}

/* The kept place for a window this program is opening, keyed as the look keys
   it; 1 when there is one. A window opened without IDCMP is keyed later by the
   task whose port the program attaches (Workbench and many programs share one
   port that way), or as "window" when it never gets one: both are tried. */
static int place_for(const char *title, int has_idcmp, int wbflag, struct place *pl)
{
    char prog[32], key[64];
    int pass, found = 0;
    for (pass = 0; pass < 2 && !found; pass++) {
        int wb, drawer, i, ok_auto, ok_snap;
        if (pass == 1 && has_idcmp) break;
        program_of_task(pass == 0 ? FindTask(NULL) : NULL, prog, sizeof prog);
        wb = place_workbench(prog, wbflag);
        drawer = wb && title && strncmp(title, "Workbench", 9);
        /* 0.10: a place the look kept counts unless only snapshots do; a snapshot counts for any window it can be taken of */
        ok_auto = !SNAPPED_ONLY && placeable(wb, drawer, prog);
        ok_snap = snappable(wb, drawer, prog);
        if (!ok_auto && !ok_snap) continue;
        place_key(title, prog, key, sizeof key);
        Forbid();                               /* the look changes them only under Forbid() too */
        if ((i = place_index(places, &nplaces, key, 0)) >= 0 && (places[i].snap ? ok_snap : ok_auto)) { *pl = places[i]; found = 1; }
        Permit();
    }
    return found;
}

/* The program's tags and NewWindow with the plan's corner (and size), in copies: the program's own stay as
 * they are. 1 when made; 0 (no memory, a list too long) and the window opens as asked. */
static int with_plan(struct NewWindow *nw, struct TagItem *tags, struct TagItem *ext, const struct place_plan *p,
                     struct ExtNewWindow *copy, struct NewWindow **pass_nw, struct TagItem **use,
                     struct TagItem **buf, struct TagItem **extbuf)
{
    int n = place_tags_count(tags), m = ext ? place_tags_count(ext) : 0;
    if (n < 0 || m < 0) return 0;
    if (!(*buf = AllocVec((n + 6) * sizeof(struct TagItem), MEMF_ANY))) return 0;
    if (place_tags(*buf, n + 6, p, tags, 1) < 0) goto fail;
    if (nw) {
        memcpy(copy, nw, (nw->Flags & WFLG_NW_EXTENDED) ? sizeof(struct ExtNewWindow) : sizeof(struct NewWindow));
        copy->LeftEdge = p->l; copy->TopEdge = p->t;
        if (p->size) { copy->Width = p->w; copy->Height = p->h; }
        if (ext) {
            if (!(*extbuf = AllocVec((m + 1) * sizeof(struct TagItem), MEMF_ANY))) goto fail;
            if (place_tags(*extbuf, m + 1, p, ext, 0) < 0) goto fail;
            copy->Extension = *extbuf;
        }
        *pass_nw = (struct NewWindow *)copy;
    }
    *use = *buf;
    return 1;
fail:
    if (*buf) FreeVec(*buf);
    if (*extbuf) FreeVec(*extbuf);
    *buf = *extbuf = NULL;
    return 0;
}

/* ---- centred until snapped (0.10) --------------------------------------------------------------
 *
 * Dale, 4 November 2026: "a setting that makes windows open in the centre of the screen until their location
 * is stored via snap". With centre on, a window with no snapped place (the gadget's, or a Workbench drawer's
 * icon snapshot) opens centred in the screen's free area, placed before it opens as a kept place is (no jump).
 * Left as they ask: backdrops, borderless windows and windows with no drag bar (popups, docks, full-screen
 * views); a window as big as the screen either way; one on a screen that isn't public (a game's or a demo's
 * own) unless centre.custom; one whose program gives a corner of its own (place_asked: not 0,0 and not the
 * left edge just under the screen's bar) unless centre.asked is off; a Workbench drawer whose icon keeps a
 * place. A window a program opens while one of its own windows is active (a requester, a dialog, ASL's) is
 * centred over that window with centre.parent (on), and kept in the free area. */

/* What centring needs of the screen a window is about to open on */
struct open_scr { struct Screen *s; WORD sw, sh, bar, font, bl, br, bt, bb; int pub; };

/* 1 for Workbench's screen and every screen on Intuition's list of public screens (a game's or a demo's own is
 * on neither; its Flags can't say: PUBLICSCREEN is a screen type, and CUSTOMSCREEN's bits hold it too) */
static int screen_public(struct Screen *s)
{
    struct List *l;
    struct Node *n;
    int pub = 0;
    if (!s) return 0;
    if ((s->Flags & SCREENTYPE) == WBENCHSCREEN) return 1;
    if ((l = LockPubScreenList())) {
        for (n = l->lh_Head; n->ln_Succ; n = n->ln_Succ)
            if (((struct PubScreenNode *)n)->psn_Screen == s) { pub = 1; break; }
        UnlockPubScreenList();
    }
    return pub;
}

static int opening_screen_info(struct NewWindow *nw, struct TagItem *tags, struct TagItem *ext, struct open_scr *o)
{
    struct Screen *s = (struct Screen *)tag_or(tags, ext, WA_CustomScreen, 0), *locked = NULL;
    memset(o, 0, sizeof *o);
    if (!s) s = (struct Screen *)tag_or(tags, ext, WA_PubScreen, 0);
    if (!s && nw && (nw->Type & SCREENTYPE) == CUSTOMSCREEN) s = nw->Screen;
    if (!s) {
        STRPTR name = (STRPTR)tag_or(tags, ext, WA_PubScreenName, 0);
        locked = LockPubScreen(name);           /* NULL: the default public screen */
        s = locked;
    }
    if (s) {
        o->s = s; o->sw = s->Width; o->sh = s->Height; o->bar = s->BarHeight;
        o->font = s->Font ? s->Font->ta_YSize : 8;
        o->bl = s->WBorLeft; o->br = s->WBorRight; o->bt = s->WBorTop; o->bb = s->WBorBottom;
        /* a public screen: Workbench, or one a program made public (a game's or a demo's own is neither) */
        o->pub = locked != NULL || screen_public(s);
    }
    if (locked) UnlockPubScreen(NULL, locked);
    return s != NULL && o->sw > 0 && o->sh > 0;
}

/* Docks along a screen's edge (GFOpenDock's shelf), noted by the look for the patch, under Forbid() */
#define DOCKS 4
static struct { struct Screen *s; WORD l, t, w, h; } docks[DOCKS];
static int ndocks;

/* 1 when (nw, tags) were given a centred corner in copies (with_plan) */
static int centre_window(struct NewWindow *nw, struct TagItem *tags, struct TagItem *ext, ULONG flags, const char *title,
                         int wbflag, struct ExtNewWindow *copy, struct NewWindow **pass_nw, struct TagItem **use,
                         struct TagItem **buf, struct TagItem **extbuf, char *why, int whysize)
{
    struct open_scr o;
    struct place_area area;
    struct place_plan p;
    char prog[32];
    WORD w, h, l, t, pl = 0, pt = 0, pw = 0, ph = 0;
    int wb, drawer, given, k;
    struct Task *me_now = FindTask(NULL);
    why[0] = 0;
    if ((flags & (WFLG_BACKDROP | WFLG_BORDERLESS)) || !(flags & WFLG_DRAGBAR)) return 0;
    program_of_task(me_now, prog, sizeof prog);
    if (!prog[0] || never(prog)) return 0;
    if (!opening_screen_info(nw, tags, ext, &o)) return 0;
    if (!o.pub && !cfg.centre_custom) { strncpy(why, "a screen that isn't public", whysize - 1); return 0; }
    /* its outer size: as asked, or from its inner size and the screen's borders (near enough to centre it) */
    w = (WORD)tag_or(tags, ext, WA_Width, nw ? (ULONG)(LONG)nw->Width : 0);
    h = (WORD)tag_or(tags, ext, WA_Height, nw ? (ULONG)(LONG)nw->Height : 0);
    if (tag_of(tags, ext, WA_InnerWidth))
        w = (WORD)tag_or(tags, ext, WA_InnerWidth, 0) + o.bl + ((flags & WFLG_SIZEGADGET) && o.br < 18 ? 18 : o.br);
    if (tag_of(tags, ext, WA_InnerHeight))
        h = (WORD)tag_or(tags, ext, WA_InnerHeight, 0) + o.bt + o.font + 1 + o.bb;
    place_area_init(&area, o.sw, o.sh, o.bar);
    if (place_too_big(&area, o.sw, o.sh, w, h)) { strncpy(why, "as big as the screen", whysize - 1); return 0; }
    given = nw != NULL || tag_of(tags, ext, WA_Left) || tag_of(tags, ext, WA_Top);
    l = (WORD)tag_or(tags, ext, WA_Left, nw ? (ULONG)(LONG)nw->LeftEdge : 0);
    t = (WORD)tag_or(tags, ext, WA_Top, nw ? (ULONG)(LONG)nw->TopEdge : 0);
    /* GFOpenBench marks its drawers WFLG_WBENCHWINDOW only after they open: known here by its name (OpenBench before
     * the GF names, as OpenUp 0.8 installs it) */
    wb = place_workbench(prog, wbflag) || !stricmp(prog, "OpenBench");
    drawer = wb && title && strncmp(title, "Workbench", 9);
    if (drawer) {
        /* GFOpenBench opens a drawer whose icon keeps no place at ((width - w) / 2, (height - h) / 3)
         * (gfopengadtools wb_drawer.c); anywhere else is the place its icon's DrawerData keeps (Snapshot) */
        if (l != (o.sw - w) / 2 || t != (o.sh - h) / 3) { strncpy(why, "its icon keeps its place", whysize - 1); return 0; }
    } else if (cfg.centre_asked && place_asked(given, l, t, o.bar)) {
        strncpy(why, "its program gives its own place", whysize - 1);
        return 0;
    }
    if (cfg.centre_over && !wb) {
        /* a requester or dialog: over the window of its own program that is active */
        Forbid();
        {
            struct Window *a = IntuitionBase->ActiveWindow;
            if (a && a->WScreen == o.s && !(a->Flags & WFLG_BACKDROP) && a->UserPort && a->UserPort->mp_SigTask == (APTR)me_now) {
                pl = a->LeftEdge; pt = a->TopEdge; pw = a->Width; ph = a->Height;
            }
        }
        Permit();
    }
    Forbid();
    for (k = 0; k < ndocks; k++)
        if (docks[k].s == o.s) place_area_reserve(&area, o.sw, o.sh, docks[k].l, docks[k].t, docks[k].w, docks[k].h);
    Permit();
    place_centre(&area, o.sw, o.sh, w, h, pl, pt, pw, ph, &l, &t);
    p.l = l; p.t = t; p.w = w; p.h = h; p.size = 0;
    strncpy(why, pw ? "centred over its program's window" : "centred", whysize - 1);
    return with_plan(nw, tags, ext, &p, copy, pass_nw, use, buf, extbuf);
}

/* OpenWindowTagList(nw, tags) with the window's kept place, when it has one. */
static struct Window *__attribute__((used)) ow_open_window(struct NewWindow *nw, struct TagItem *tags, APTR fn)
{
    struct TagItem *use = tags, *ext = NULL, *buf = NULL, *extbuf = NULL;
    struct ExtNewWindow copy;
    struct NewWindow *pass_nw = nw;
    struct Window *win;
    const char *title = NULL;
    int cogged = 0, saw = 0, placed = 0;
    char why[40];
    why[0] = 0;
    patch_in++;
    if (nw && (nw->Flags & WFLG_NW_EXTENDED)) ext = ((struct ExtNewWindow *)nw)->Extension;
    /* 0.8.1: a window the cog asked for: at the screen's top right, COGPLACE_GAP from its top and right */
    if (!patch_pass && FindTask(NULL) != me && cog.armed) {
        ULONG flags = 0, secs, micros;
        int take = 0;
        flags = tag_or(tags, ext, WA_Flags, nw ? nw->Flags : 0);
        if (tag_or(tags, ext, WA_DragBar, 0)) flags |= WFLG_DRAGBAR;
        if (tag_or(tags, ext, WA_Backdrop, 0)) flags |= WFLG_BACKDROP;
        CurrentTime(&secs, &micros);
        Forbid();
        if (cog.armed && FindTask(NULL) != cog.from && (flags & WFLG_DRAGBAR) && !(flags & WFLG_BACKDROP)) {
            if (secs <= cog.until) take = 1;
            cog.armed = 0;
        }
        Permit();
        if (take) {
            WORD sw, sh, wd;
            struct Screen *ps = (struct Screen *)tag_or(tags, ext, WA_CustomScreen, 0);
            opening_screen(nw, tags, ext, &sw, &sh);
            if (!ps) ps = IntuitionBase->FirstScreen;
            wd = (WORD)tag_or(tags, ext, WA_Width, nw ? (ULONG)(LONG)nw->Width : 0);
            if (tag_or(tags, ext, WA_InnerWidth, 0))
                wd = (WORD)tag_or(tags, ext, WA_InnerWidth, 0) + (ps ? ps->WBorLeft + ps->WBorRight : 8) +
                     ((flags & WFLG_SIZEGADGET) || tag_or(tags, ext, WA_SizeGadget, 0) ? 14 : 0);
            if (sw > 0 && wd > 0) {
                struct place_plan p;
                p.l = sw - wd - COGPLACE_GAP; p.t = COGPLACE_GAP; p.w = p.h = 0; p.size = 0;
                if (p.l < 0) p.l = 0;
                cogged = with_plan(nw, tags, ext, &p, &copy, &pass_nw, &use, &buf, &extbuf);
            }
        }
    }
    if (!cogged && !patch_pass && on && (cfg.places || cfg.centre || GADGET_ON) && FindTask(NULL) != me) {
        ULONG flags, idcmp;
        struct place pl;
        int found = 0;
        flags = tag_or(tags, ext, WA_Flags, nw ? nw->Flags : 0);
        if (tag_or(tags, ext, WA_Backdrop, 0)) flags |= WFLG_BACKDROP;
        if (tag_or(tags, ext, WA_Borderless, 0)) flags |= WFLG_BORDERLESS;
        if (tag_or(tags, ext, WA_GimmeZeroZero, 0)) flags |= WFLG_GIMMEZEROZERO;
        if (tag_or(tags, ext, WA_SizeGadget, 0)) flags |= WFLG_SIZEGADGET;
        if (tag_or(tags, ext, WA_DragBar, 0)) flags |= WFLG_DRAGBAR;
        title = (const char *)tag_or(tags, ext, WA_Title, nw ? (ULONG)nw->Title : 0);
        idcmp = tag_or(tags, ext, WA_IDCMP, nw ? nw->IDCMPFlags : 0);
        /* the same windows the look keeps: not backdrops, not GZZ, a drag bar or a title */
        if (place_candidate((flags & (WFLG_BACKDROP | WFLG_GIMMEZEROZERO)) != 0, (flags & WFLG_DRAGBAR) != 0, title)) {
            saw = 1;
            found = place_for(title, idcmp != 0, (flags & WFLG_WBENCHWINDOW) != 0, &pl);
        }
        if (found) {
            struct place_ask a;
            struct place_plan p;
            WORD sw, sh;
            a.w = (WORD)tag_or(tags, ext, WA_Width, nw ? (ULONG)(LONG)nw->Width : 0);
            a.h = (WORD)tag_or(tags, ext, WA_Height, nw ? (ULONG)(LONG)nw->Height : 0);
            a.inner = tag_of(tags, ext, WA_InnerWidth) || tag_of(tags, ext, WA_InnerHeight);
            a.sizeable = (flags & WFLG_SIZEGADGET) != 0;
            a.minw = (WORD)tag_or(tags, ext, WA_MinWidth, nw ? (ULONG)(LONG)nw->MinWidth : 0);
            a.minh = (WORD)tag_or(tags, ext, WA_MinHeight, nw ? (ULONG)(LONG)nw->MinHeight : 0);
            a.maxw = (WORD)tag_or(tags, ext, WA_MaxWidth, nw ? (ULONG)nw->MaxWidth : 0);
            a.maxh = (WORD)tag_or(tags, ext, WA_MaxHeight, nw ? (ULONG)nw->MaxHeight : 0);
            opening_screen(nw, tags, ext, &sw, &sh);
            if (sw > 0 && sh > 0) {
                place_plan(&pl, &a, sw, sh, &p);
                placed = with_plan(nw, tags, ext, &p, &copy, &pass_nw, &use, &buf, &extbuf);
            }
        } else if (cfg.centre) {
            /* 0.10: no snapped place: centred, unless it is one centring leaves alone (centre_window) */
            if (centre_window(nw, tags, ext, flags, title, (flags & WFLG_WBENCHWINDOW) != 0, &copy, &pass_nw, &use, &buf, &extbuf,
                              why, sizeof why))
                placed = 2;
            else if (!why[0]) strcpy(why, "left as it asks");
        }
    }
    {
        /* the registers are set here, after every other call */
        register struct Window *res __asm("d0");
        register struct NewWindow *a0 __asm("a0") = pass_nw;
        register struct TagItem *a1 __asm("a1") = use;
        register APTR a6 __asm("a6") = IntuitionBase;
        __asm volatile ("jsr (%4)" : "=r"(res), "+r"(a0), "+r"(a1), "+r"(a6) : "a"(fn) : "d1", "memory", "cc");
        win = res;
    }
    if (win && (cogged || saw)) {
        int k = 0;
        Forbid();
        if (cogged) {
            cog_wins[cog_next] = win;
            cog_next = (UBYTE)((cog_next + 1) % COG_WINS);
        } else {
            opened[opened_next].w = win;
            if (title) while (title[k] && k < (int)sizeof opened[0].title - 1) { opened[opened_next].title[k] = title[k]; k++; }
            opened[opened_next].title[k] = 0;
            opened[opened_next].placed = (UBYTE)placed;
            memcpy(opened[opened_next].why, why, sizeof opened[0].why);
            opened[opened_next].look = looks;
            opened_next = (UBYTE)((opened_next + 1) % OPENED);
        }
        Permit();
    }
    if (buf) FreeVec(buf);
    if (extbuf) FreeVec(extbuf);
    /* 0.10: the snapshot gadget, as the window opens (on the opening task, as the program would add one) */
    if (win && !cogged && !patch_pass && on && GADGET_ON && FindTask(NULL) != me) snap_attach(win, 0);
    patch_in--;
    return win;
}

__asm(
"_ow_owtl:\n"
"   movem.l d2-d7/a2-a6,-(sp)\n"
"   move.l _old_owtl,-(sp)\n move.l a1,-(sp)\n move.l a0,-(sp)\n"
"   jsr _ow_open_window\n"
"   lea 12(sp),sp\n"
"   movem.l (sp)+,d2-d7/a2-a6\n"
"   rts\n"
/* OpenWindow(nw a0): as OpenWindowTagList(nw, NULL), through what OpenWindowTagList was */
"_ow_ow:\n"
"   movem.l d2-d7/a2-a6,-(sp)\n"
"   move.l _old_owtl,-(sp)\n clr.l -(sp)\n move.l a0,-(sp)\n"
"   jsr _ow_open_window\n"
"   lea 12(sp),sp\n"
"   movem.l (sp)+,d2-d7/a2-a6\n"
"   rts\n");
void ow_owtl(void), ow_ow(void);

static void places_patch_on(void)
{
    patch_pass = 0;                         /* also after a switch-off that couldn't take the calls back */
    if (old_owtl) return;
    Forbid();
    old_owtl = SetFunction((struct Library *)IntuitionBase, -606, (APTR)ow_owtl);
    old_ow = SetFunction((struct Library *)IntuitionBase, -204, (APTR)ow_ow);
    Permit();
}

/* 1 when Intuition has its own calls back and nobody is still inside ours; 0
   when another program patched on top (ours must stay, passing through). */
static int places_patch_off(void)
{
    int ok = 1, i;
    if (!old_owtl) return 1;
    Forbid();
    if (SetFunction((struct Library *)IntuitionBase, -606, old_owtl) != (APTR)ow_owtl) {
        SetFunction((struct Library *)IntuitionBase, -606, (APTR)ow_owtl);
        ok = 0;
    } else if (SetFunction((struct Library *)IntuitionBase, -204, old_ow) != (APTR)ow_ow) {
        SetFunction((struct Library *)IntuitionBase, -204, (APTR)ow_ow);
        SetFunction((struct Library *)IntuitionBase, -606, (APTR)ow_owtl);
        ok = 0;
    }
    Permit();
    if (!ok) { patch_pass = 1; return 0; }
    old_owtl = old_ow = NULL;
    for (i = 0; i < 250 && patch_in; i++) Delay(1);      /* a window opening now finishes in our code */
    return patch_in == 0;
}

/* ---- the snapshot gadget (0.10) ----------------------------------------------------------------
 *
 * Dale, 4 November 2026: "we need a snap icon similar to the way MUI does it". A gadget in the title bar,
 * left of the depth and zoom gadgets, as MUI's snapshot gadget sits by MorphOS' (WA_ExtraGadget_Snapshot):
 * a click keeps the window's place and size (a snapped place, "snap" in WindowPlaces); a click on a snapped
 * window's pin forgets it (0.12.2), and so does Shift and a click (MUI's unsnapshot). Its pin shows which: hollow and standing, nothing kept; solid
 * and pushed in, snapped. With centring on, a window that is not snapped opens centred.
 *
 * - On a window with a drag bar and a depth gadget, not a backdrop or borderless one, on a public screen, of
 *   a program GFOpenWindows may keep (never lines), and on Workbench's drawers (not its root window).
 * - The tile is the window's own depth gadget's image (the theme's, Intuition's, GFOpenLook's: whatever draws
 *   it, active and inactive, held and not, on RTG and on the chipset), drawn at the gadget's place; the depth
 *   glyph is then covered with the tile's own column left of it, and the pin drawn in the shadow pen.
 * - It goes into the window's list just before the drag bar, after close, depth and zoom: it is drawn after
 *   them and takes a press before the drag bar does. It sends the program nothing: the press is the gadget's
 *   own (GM_GOACTIVE, GM_HANDLEINPUT), and at the release over it (GM_GOINACTIVE) the main task is told.
 * - GFOpenWindows takes its gadgets out again when the gadget is switched off and when it quits; one on a
 *   window that closed is disposed of at the next look. */

static struct IClass *snap_class;
struct snapdata { UBYTE snapped, pressed, forget, act; };
#define SNAP_RING 8
static struct { struct Window *w; struct Gadget *g; UBYTE what; } snap_ring[SNAP_RING];  /* what: 1 snap, 2 forget */
static volatile int snap_in, snap_out;
static ULONG sig_snap;
#define SNAP_GADS 128
static struct { struct Window *w; struct Gadget *g; } sgads[SNAP_GADS];   /* changed under Forbid() */
static int nsgads;

/* The pin, 7 x 8 cells (bit 6 the left): its outline in the shadow pen and, standing (nothing kept), its inside in
 * the shine pen, as the depth glyph's boxes are drawn (dark lines round a light inside); pushed in a row and solid
 * dark when snapped */
static const UBYTE pin_open[8] = { 0x3e, 0x22, 0x14, 0x14, 0x7f, 0x08, 0x08, 0x08 };
static const UBYTE pin_open_in[8] = { 0x00, 0x1c, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00 };
static const UBYTE pin_shut[8] = { 0x00, 0x3e, 0x3e, 0x1c, 0x7f, 0x08, 0x08, 0x00 };

static int is_snap_gadget(struct Gadget *g)
{
    return snap_class && (g->GadgetType & GTYP_GTYPEMASK) == GTYP_CUSTOMGADGET && OCLASS(g) == snap_class;
}

static struct Gadget *depth_gadget(struct Window *w)
{
    struct Gadget *g;
    for (g = w->FirstGadget; g; g = g->NextGadget)
        if ((g->GadgetType & GTYP_SYSGADGET) && (g->GadgetType & GTYP_SYSTYPEMASK) == GTYP_WDEPTH) return g;
    return NULL;
}

static void snap_draw(struct Window *w, struct Gadget *g, struct RastPort *rp, struct DrawInfo *dri, int in)
{
    struct snapdata *d = INST_DATA(snap_class, g);
    struct Gadget *dg = depth_gadget(w);
    WORD x0 = (g->Flags & GFLG_RELRIGHT) ? w->Width - 1 + g->LeftEdge : g->LeftEdge, y0 = g->TopEdge;
    WORD x1 = x0 + g->Width - 1, y1 = y0 + g->Height - 1, u, gx, gy, r, c, aw, ah;
    int active = (w->Flags & WFLG_WINDOWACTIVE) != 0;
    const UBYTE *bits = d->snapped ? pin_shut : pin_open;
    ULONG apen = GetAPen(rp), mode = GetDrMd(rp);
    UWORD shadow = dri ? dri->dri_Pens[SHADOWPEN] : 1;
    if (g->Width < 6 || g->Height < 6) return;
    SetDrMd(rp, JAM1);
    if (dg && (dg->Flags & GFLG_GADGIMAGE) && dg->GadgetRender) {
        struct Image *im = (struct Image *)dg->GadgetRender;
        ULONG st = active ? (in ? IDS_SELECTED : IDS_NORMAL) : (in ? IDS_INACTIVESELECTED : IDS_INACTIVENORMAL);
        WORD ix0, ix1, cx;
        DrawImageState(rp, im, x0, y0, st, dri);
        /* the depth glyph covered with the tile's own column left of it (the tile's bands run across) */
        ix0 = x0 + im->LeftEdge; ix1 = ix0 + im->Width - 1;
        cx = ix0 + 2;
        for (c = cx + 1; c <= ix1 - 2; c++) ClipBlit(rp, cx, y0 + 1, rp, c, y0 + 1, 1, g->Height - 2, 0xc0);
    } else {
        /* no image to borrow: a plain tile in the bar's pens */
        SetAPen(rp, dri ? dri->dri_Pens[active ? FILLPEN : BACKGROUNDPEN] : 0);
        RectFill(rp, x0, y0, x1, y1);
        SetAPen(rp, dri ? dri->dri_Pens[in ? SHADOWPEN : SHINEPEN] : 2);
        Move(rp, x0, y1); Draw(rp, x0, y0); Draw(rp, x1, y0);
        SetAPen(rp, dri ? dri->dri_Pens[in ? SHINEPEN : SHADOWPEN] : 1);
        Draw(rp, x1, y1); Draw(rp, x0 + 1, y1);
    }
    /* the pin: 7 x 8 cells of u pixels, in the middle, a pixel down and right while held */
    aw = g->Width - 6; ah = g->Height - 4;
    u = ah / 9; if (u * 8 > aw) u = aw / 8; if (u < 1) u = 1;
    gx = x0 + (g->Width - 7 * u) / 2 + in; gy = y0 + (g->Height - 8 * u) / 2 + in;
    SetAPen(rp, shadow);
    for (r = 0; r < 8; r++)
        for (c = 0; c < 7; c++)
            if (bits[r] & (0x40 >> c)) RectFill(rp, gx + c * u, gy + r * u, gx + c * u + u - 1, gy + r * u + u - 1);
    if (!d->snapped) {
        SetAPen(rp, dri ? dri->dri_Pens[SHINEPEN] : 2);
        for (r = 0; r < 8; r++)
            for (c = 0; c < 7; c++)
                if (pin_open_in[r] & (0x40 >> c)) RectFill(rp, gx + c * u, gy + r * u, gx + c * u + u - 1, gy + r * u + u - 1);
    }
    SetAPen(rp, apen);
    SetDrMd(rp, mode);
}

static void snap_redraw(struct GadgetInfo *gi, struct Gadget *g, int in)
{
    struct RastPort *rp;
    if (gi && gi->gi_Window && (rp = ObtainGIRPort(gi))) {
        snap_draw(gi->gi_Window, g, rp, gi->gi_DrInfo, in);
        ReleaseGIRPort(rp);
    }
}

/* The class's dispatcher: on Intuition's input task (press, release, drawing) and on whichever task refreshes
 * the window. It never waits and never calls DOS. */
static ULONG snap_dispatch(struct IClass *cl __asm("a0"), Object *o __asm("a2"), Msg msg __asm("a1"))
{
    struct Gadget *g = (struct Gadget *)o;
    struct snapdata *d;
    switch (msg->MethodID) {
    case OM_NEW: {
        ULONG r = CallHookPkt(&cl->cl_Super->cl_Dispatcher, o, msg);
        if (r) memset(INST_DATA(cl, (Object *)r), 0, sizeof(struct snapdata));
        return r;
    }
    case GM_RENDER: {
        struct gpRender *m = (struct gpRender *)msg;
        d = INST_DATA(cl, o);
        if (m->gpr_GInfo && m->gpr_GInfo->gi_Window && m->gpr_RPort)
            snap_draw(m->gpr_GInfo->gi_Window, g, m->gpr_RPort, m->gpr_GInfo->gi_DrInfo, d->pressed);
        return 0;
    }
    case GM_HITTEST:
        return GMR_GADGETHIT;
    case GM_GOACTIVE: {
        struct gpInput *m = (struct gpInput *)msg;
        d = INST_DATA(cl, o);
        if (!m->gpi_IEvent) return GMR_NOREUSE;             /* ActivateGadget(): nothing to do */
        d->pressed = 1; d->act = 0;
        d->forget = (m->gpi_IEvent->ie_Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)) != 0;
        snap_redraw(m->gpi_GInfo, g, 1);
        return GMR_MEACTIVE;
    }
    case GM_HANDLEINPUT: {
        struct gpInput *m = (struct gpInput *)msg;
        struct InputEvent *ie = m->gpi_IEvent;
        int inside = m->gpi_Mouse.X >= 0 && m->gpi_Mouse.Y >= 0 && m->gpi_Mouse.X < g->Width && m->gpi_Mouse.Y < g->Height;
        d = INST_DATA(cl, o);
        if (ie && ie->ie_Class == IECLASS_RAWMOUSE) {
            if (ie->ie_Code == SELECTUP) {
                /* 0.12.2: a toggle: a pushed-in pin unsnaps (place_pin_click) */
                d->act = (UBYTE)place_pin_click(inside, d->snapped,
                                                d->forget || (ie->ie_Qualifier & (IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)));
                return GMR_NOREUSE;
            }
            if (ie->ie_Code == MENUDOWN) { d->act = 0; return GMR_NOREUSE; }
            if (inside != d->pressed) { d->pressed = (UBYTE)inside; snap_redraw(m->gpi_GInfo, g, inside); }
        }
        return GMR_MEACTIVE;
    }
    case GM_GOINACTIVE: {
        struct gpGoInactive *m = (struct gpGoInactive *)msg;
        d = INST_DATA(cl, o);
        d->pressed = 0;
        snap_redraw(m->gpgi_GInfo, g, 0);
        if (d->act && m->gpgi_GInfo && m->gpgi_GInfo->gi_Window) {
            Forbid();
            if ((snap_in + 1) % SNAP_RING != snap_out) {
                snap_ring[snap_in].w = m->gpgi_GInfo->gi_Window; snap_ring[snap_in].g = g; snap_ring[snap_in].what = d->act;
                snap_in = (snap_in + 1) % SNAP_RING;
            }
            Permit();
            Signal(me, sig_snap);
        }
        d->act = 0;
        return 0;
    }
    }
    return CallHookPkt(&cl->cl_Super->cl_Dispatcher, o, msg);
}

static volatile UBYTE snap_accept;              /* 1: snap_attach may add gadgets (read by the patch) */
static int snap_all;                            /* the next look adds the gadget to every window open */

static void snap_class_on(void)
{
    if (!snap_class && (snap_class = MakeClass(NULL, (ClassID)"gadgetclass", NULL, sizeof(struct snapdata), 0)))
        snap_class->cl_Dispatcher.h_Entry = (ULONG (*)())snap_dispatch;
    if (!snap_accept && snap_class) snap_all = 1;
    snap_accept = snap_class != NULL;
}

/* The key a window is kept by, as the look knows it (the title it opened with), on the main task */
static void window_key(struct Window *w, char *key, int size)
{
    char prog[32], title[sizeof opened[0].title];
    int j, k;
    for (j = 0; j < nseen; j++)
        if (seen[j].w == w) { strncpy(key, seen[j].key, size - 1); key[size - 1] = 0; return; }
    title[0] = 0;
    if (w->Title) { strncpy(title, (const char *)w->Title, sizeof title - 1); title[sizeof title - 1] = 0; }
    Forbid();
    for (k = 0; k < OPENED; k++) if (opened[k].w == w) memcpy(title, opened[k].title, sizeof title);
    Permit();
    program_of(w, prog, sizeof prog);
    place_key(title, prog, key, size);
}

static int key_snapped(const char *key)
{
    int i, s;
    Forbid();
    i = place_index(places, &nplaces, key, 0);
    s = i >= 0 && places[i].snap;
    Permit();
    return s;
}

/* 0.12.1: 1 when w is shaped as a system requester Intuition opened itself (place_sysreq): its own gadgets */
static int sysreq_window(struct Window *w)
{
    UWORD types[PLACE_SYSREQ_GADS], act[PLACE_SYSREQ_GADS];
    struct Gadget *x;
    int n = 0;
    for (x = w->FirstGadget; x; x = x->NextGadget) {
        if (x->GadgetType & GTYP_SYSGADGET) continue;
        if (n == PLACE_SYSREQ_GADS) return 0;
        types[n] = x->GadgetType; act[n] = x->Activation; n++;
    }
    return place_sysreq(w->Flags, w->MenuStrip != NULL, n, types, act);
}

/* The snapshot gadget onto w, when it is one to have it. from_look 0: on the opening task, from the patch,
 * just after the window opened; 1: on the main task, for a window that opened without the patch seeing it. */
static void snap_attach(struct Window *w, int from_look)
{
    struct Gadget *dg, *x, *g;
    struct Screen *s = w->WScreen;
    char prog[32], key[64];
    WORD minl = 0x7fff;
    int pos, dpos = -1, k, ok = 0, wb, drawer;
    if (!snap_accept || !snap_class || (w->Flags & (WFLG_BACKDROP | WFLG_BORDERLESS))) return;
    if (!screen_public(s)) return;
    /* 0.12.1: never onto a system requester (Team 5's lab, 5 November 2026: the look gave a dos requester over a
     * Shell one); the patch never sees those open, so only the look's windows are asked */
    if (from_look && sysreq_window(w)) return;
    if (from_look) program_of(w, prog, sizeof prog);
    else program_of_task(FindTask(NULL), prog, sizeof prog);
    wb = place_workbench(prog, (w->Flags & WFLG_WBENCHWINDOW) != 0);
    drawer = wb && w->Title && strncmp((const char *)w->Title, "Workbench", 9);
    if (!snappable(wb, drawer, prog) || !(dg = depth_gadget(w))) return;
    for (x = w->FirstGadget, pos = 0; x; x = x->NextGadget, pos++) {
        if (is_snap_gadget(x)) return;                          /* it has one */
        if ((x->GadgetType & GTYP_SYSGADGET) && (x->GadgetType & GTYP_SYSTYPEMASK) == GTYP_WDRAGGING && dpos < 0) dpos = pos;
        /* the title bar's gadgets on the right: depth, zoom, and any a program or the system put there */
        if ((x->Flags & GFLG_RELRIGHT) && !(x->Flags & GFLG_RELWIDTH) && x->TopEdge < w->BorderTop &&
            ((x->Activation & GACT_TOPBORDER) || (x->GadgetType & GTYP_SYSGADGET)) && x->LeftEdge < minl)
            minl = x->LeftEdge;
    }
    if (dpos < 0 || minl == 0x7fff) return;                     /* no drag bar: nothing to drag, nothing to snap */
    g = (struct Gadget *)NewObject(snap_class, NULL, GA_RelRight, (ULONG)(LONG)(minl - dg->Width), GA_Top, (ULONG)(LONG)dg->TopEdge,
                                   GA_Width, (ULONG)dg->Width, GA_Height, (ULONG)dg->Height, GA_TopBorder, TRUE,
                                   GA_GZZGadget, (ULONG)((w->Flags & WFLG_GIMMEZEROZERO) != 0), TAG_DONE);
    if (!g) return;
    if (from_look) window_key(w, key, sizeof key);
    else place_key((const char *)w->Title, prog, key, sizeof key);
    ((struct snapdata *)INST_DATA(snap_class, g))->snapped = (UBYTE)key_snapped(key);
    Forbid();
    for (k = 0; k < nsgads; k++) if (sgads[k].w == w && sgads[k].g) break;
    if (k == nsgads && nsgads < SNAP_GADS) { sgads[nsgads].w = w; sgads[nsgads].g = g; nsgads++; ok = 1; }
    Permit();
    if (!ok) { DisposeObject(g); return; }
    AddGList(w, g, (UWORD)dpos, 1, NULL);
    RefreshGList(g, w, NULL, 1);
}

/* 1 when w is open and g is in its list */
static int snap_alive(struct Window *w, struct Gadget *g)
{
    struct Screen *s;
    struct Window *x;
    struct Gadget *y;
    int found = 0;
    ULONG lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && !found; s = s->NextScreen)
        for (x = s->FirstWindow; x; x = x->NextWindow)
            if (x == w) {
                for (y = w->FirstGadget; y; y = y->NextGadget) if (y == g) { found = 1; break; }
                break;
            }
    UnlockIBase(lock);
    return found;
}

/* Each gadget's pin as its window's place is now; the gadgets of windows that closed disposed of */
static void snap_sweep(int redraw)
{
    int k;
    char key[64];
    for (k = 0; k < nsgads; ) {
        struct Window *w = sgads[k].w;
        struct Gadget *g = sgads[k].g;
        if (!snap_alive(w, g)) {
            Forbid(); sgads[k] = sgads[--nsgads]; Permit();
            DisposeObject(g);
            continue;
        }
        if (redraw) {
            struct snapdata *d = INST_DATA(snap_class, g);
            UBYTE now;
            window_key(w, key, sizeof key);
            now = (UBYTE)key_snapped(key);
            if (now != d->snapped) { d->snapped = now; RefreshGList(g, w, NULL, 1); }
        }
        k++;
    }
}

/* Every gadget out of its window (the gadget switched off, or GFOpenWindows quitting); 1 when the class is gone */
static int snap_off(void)
{
    int i;
    snap_accept = 0;                                            /* no new ones, from the patch or the look */
    for (i = 0; i < 250 && patch_in; i++) Delay(1);             /* one being added now finishes first */
    while (nsgads > 0) {
        struct Window *w = sgads[nsgads - 1].w;
        struct Gadget *g = sgads[nsgads - 1].g;
        if (snap_alive(w, g) && RemoveGList(w, g, 1) >= 0) RefreshWindowFrame(w);
        Forbid(); nsgads--; Permit();
        DisposeObject(g);
    }
    if (snap_class && FreeClass(snap_class)) snap_class = NULL;
    return snap_class == NULL;
}

/* A click on a gadget: the window's place snapped, or forgotten (a snapped pin, or Shift); both files written at once */
static void snap_do(void)
{
    char key[64], line[160];
    while (snap_out != snap_in) {
        struct Window *w;
        struct Gadget *g;
        int what, i;
        Forbid();
        w = snap_ring[snap_out].w; g = snap_ring[snap_out].g; what = snap_ring[snap_out].what;
        snap_out = (snap_out + 1) % SNAP_RING;
        Permit();
        if (!snap_alive(w, g)) continue;
        window_key(w, key, sizeof key);
        Forbid();
        if (what == 1) {
            i = place_index(places, &nplaces, key, 1);
            places[i].l = w->LeftEdge; places[i].t = w->TopEdge; places[i].w = w->Width; places[i].h = w->Height;
            places[i].snap = 1;
            places[i].used = ++places_clock;
        } else if ((i = place_index(places, &nplaces, key, 0)) >= 0)
            place_remove(places, &nplaces, i);
        Permit();
        write_places(PLACES_ENV);
        write_places(PLACES_ENVARC);
        places_dirty = 0;
        snprintf(line, sizeof line, "%s \"%s\" %d %d %d %d\n", what == 1 ? "snapped" : "forgot", key,
                 w->LeftEdge, w->TopEdge, w->Width, w->Height);
        place_log(line);
    }
    snap_sweep(1);
}

/* ---- drive windows: the name only (0.6) --------------------------------------------------------- */

/* Workbench titles a drive's window "Work  70% full, 453MB free, 62.0MB in use"
 * (workbench.library's format; the two spaces, the percentage and its
 * percent sign are the part to find). Cut in place, at the first of the two
 * spaces, in the string Workbench made: its memory is its own and stays so.
 * 1 when something was cut. */
static int cut_usage(char *t)
{
    char *p;
    for (p = t; *p; p++) {
        char *q = p + 2;
        if (p[0] != ' ' || p[1] != ' ' || *q < '0' || *q > '9') continue;
        while (*q >= '0' && *q <= '9') q++;
        if (*q == '%' && p > t) { *p = 0; return 1; }
    }
    return 0;
}

static void drive_titles(void)
{
    struct Window *fix[8], *w;
    struct Screen *s;
    int n = 0, i;
    ULONG lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s; s = s->NextScreen)
        for (w = s->FirstWindow; w && n < 8; w = w->NextWindow)
            if ((w->Flags & WFLG_WBENCHWINDOW) && w->Title && cut_usage((char *)w->Title)) fix[n++] = w;
    UnlockIBase(lock);
    for (i = 0; i < n; i++)
        if (is_window(fix[i])) SetWindowTitles(fix[i], fix[i]->Title, (STRPTR)-1);      /* the same string: drawn again */
}

/* 0.10: the docks along each screen's edge, for centring: the borderless windows of GFOpenDock's task (its port,
   GFOpenDock, or OpenDock on an OpenUp before the GF names), noted under Forbid() for the patch */
static void find_docks(void)
{
    static const char *const names[] = { "GFOpenDock", "OpenDock" };
    struct MsgPort *port = NULL;
    struct Task *dock = NULL;
    struct Screen *s;
    struct Window *w;
    int k, n = 0;
    struct { struct Screen *s; WORD l, t, w, h; } got[DOCKS];
    Forbid();
    for (k = 0; k < 2 && !port; k++) port = FindPort((STRPTR)names[k]);
    if (port) dock = (struct Task *)port->mp_SigTask;
    Permit();
    if (dock) {
        ULONG lock = LockIBase(0);
        for (s = IntuitionBase->FirstScreen; s; s = s->NextScreen)
            for (w = s->FirstWindow; w && n < DOCKS; w = w->NextWindow)
                if ((w->Flags & WFLG_BORDERLESS) && w->UserPort && w->UserPort->mp_SigTask == (APTR)dock) {
                    got[n].s = s; got[n].l = w->LeftEdge; got[n].t = w->TopEdge; got[n].w = w->Width; got[n].h = w->Height;
                    n++;
                }
        UnlockIBase(lock);
    }
    Forbid();
    for (k = 0; k < n; k++) { docks[k].s = got[k].s; docks[k].l = got[k].l; docks[k].t = got[k].t; docks[k].w = got[k].w; docks[k].h = got[k].h; }
    ndocks = n;
    Permit();
}

/* Once a second: every window's place is noted, and kept when it was moved or sized; a Workbench drawer
   is made at least the drawer size. 0.9.2: nothing is moved to its place here any more: the patch places a
   window before it opens, and a window it didn't see open (a system requester Intuition opens itself, one
   opened before GFOpenWindows started) stays where it opened. ENV:OpenWindowsLog, when set, names a file
   that gets a line for each new window and each place kept. 0.10: with "centre on" no place is kept here
   (only the snapshot gadget keeps one); a snapped place is never moved; the docks are noted for centring and
   the snapshot gadget goes onto windows the patch didn't see open. */
static void watch(void)
{
    /* static: Run gives a commodity a 4 KB stack, and this is 12 KB */
    static struct { struct Window *w; WORD l, t, wd, ht, sw, sh; ULONG flags; char prog[32]; char key[64], cur[64]; int wb, drawer, keep; } now[MAX_SEEN];
    char line[200];
    int n = 0, i, j, k, busy;
    struct Screen *s;
    struct Window *w;
    ULONG lock;
    if (cfg.drive_title) drive_titles();
    if (cfg.centre) find_docks();
    if (nsgads) snap_sweep(0);                  /* 0.10: the gadgets of windows that closed */
    if (!cfg.places && !cfg.drawer_w && !cfg.centre && !GADGET_ON) return;
    busy = patch_in != 0;                       /* a window opening through the patch now: new ones wait a look */
    lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s; s = s->NextScreen)
        for (w = s->FirstWindow; w && n < MAX_SEEN; w = w->NextWindow) {
            if (!place_candidate((w->Flags & (WFLG_BACKDROP | WFLG_GIMMEZEROZERO)) != 0, (w->Flags & WFLG_DRAGBAR) != 0, (const char *)w->Title))
                continue;
            now[n].w = w; now[n].l = w->LeftEdge; now[n].t = w->TopEdge; now[n].wd = w->Width; now[n].ht = w->Height;
            now[n].sw = s->Width; now[n].sh = s->Height; now[n].flags = w->Flags;
            {   /* the program's name while the window is sure to be there; the key from a copy
                   of it (GCC 16 can't tell now[n].prog from now[n].key: -Wrestrict) */
                char prog[sizeof now[0].prog];
                program_of(w, prog, sizeof prog);
                memcpy(now[n].prog, prog, sizeof prog);
                now[n].wb = place_workbench(prog, (w->Flags & WFLG_WBENCHWINDOW) != 0);
                /* a Workbench drawer, not its root window, takes a place when the switch says */
                now[n].drawer = now[n].wb && w->Title && strncmp((const char *)w->Title, "Workbench", 9);
                place_key((const char *)w->Title, prog, now[n].key, sizeof now[n].key);
                memcpy(now[n].cur, now[n].key, sizeof now[n].cur);
            }
            n++;
        }
    UnlockIBase(lock);
    looks++;
    for (i = 0; i < n; i++) {
        int known = -1;
        now[i].keep = 1;
        for (j = 0; j < nseen; j++) if (seen[j].w == now[i].w) { known = j; break; }
        if (known >= 0) {
            /* another window in the memory of one that closed since the last look: the patch noted the new one
             * opening; or, one it didn't see, its title and its box both changed at once */
            int moved = seen[known].l != now[i].l || seen[known].t != now[i].t || seen[known].wd != now[i].wd || seen[known].ht != now[i].ht;
            Forbid();
            for (k = 0; k < OPENED; k++) if (opened[k].w == now[i].w) known = -1;
            Permit();
            if (known >= 0 && moved && strcmp(seen[known].cur, now[i].cur)) known = -1;
        }
        if (known >= 0) {
            memcpy(now[i].key, seen[known].key, sizeof now[i].key);     /* the key it was first known by */
            /* 0.10: not with only snapshots counting, and never over a snapped place (it stays where it was snapped) */
            if (!SNAPPED_ONLY && placeable(now[i].wb, now[i].drawer, now[i].prog) && !key_snapped(now[i].key) &&
                (seen[known].l != now[i].l || seen[known].t != now[i].t || seen[known].wd != now[i].wd || seen[known].ht != now[i].ht)) {
                /* the user moved or sized it: remember that */
                int p;
                Forbid();
                p = place_index(places, &nplaces, now[i].key, 1);
                places[p].l = now[i].l; places[p].t = now[i].t; places[p].w = now[i].wd; places[p].h = now[i].ht;
                places[p].used = ++places_clock;
                Permit();
                places_dirty = 1;
                snprintf(line, sizeof line, "look %lu: kept \"%s\" %d %d %d %d\n", (unsigned long)looks, now[i].key,
                         now[i].l, now[i].t, now[i].wd, now[i].ht);
                place_log(line);
            }
            /* 0.10: the gadget just switched on (or GFOpenWindows just started): onto the windows open already. Later,
             * only the patch adds it: a window it didn't see open is one Intuition opened inside itself (a system
             * requester), which no place could ever be given to */
            if (snap_all && GADGET_ON && is_window(now[i].w)) snap_attach(now[i].w, 1);
        } else {
            /* a new window (the first look only notes the windows already open) */
            int rec = -1, cogged = 0, placed = 0, p = -1;
            char title[sizeof opened[0].title], why[sizeof opened[0].why];
            if (busy && looks > 1) { now[i].keep = 0; continue; }   /* the patch may not have noted it yet */
            Forbid();
            for (k = 0; k < COG_WINS; k++)
                if (cog_wins[k] == now[i].w) { cog_wins[k] = NULL; cogged = 1; }
            for (k = 1; k <= OPENED && rec < 0; k++) {               /* the newest first: a window's memory used again */
                int x = (opened_next + OPENED - k) % OPENED;
                if (opened[x].w == now[i].w) rec = x;
            }
            why[0] = 0;
            if (rec >= 0) {
                memcpy(title, opened[rec].title, sizeof title);
                placed = opened[rec].placed;
                memcpy(why, opened[rec].why, sizeof why);
                opened[rec].w = NULL;
            }
            Permit();
            if (rec >= 0) place_key(title, now[i].prog, now[i].key, sizeof now[i].key);   /* the title it opened with */
            if (looks > 1 && (cfg.places || cfg.centre || GADGET_ON) && snappable(now[i].wb, now[i].drawer, now[i].prog) &&
                (p = place_index(places, &nplaces, now[i].key, 0)) >= 0) {
                /* 0.10: a place that counts now: a snapped one, or one the look kept when those count */
                if (places[p].snap || (!SNAPPED_ONLY && placeable(now[i].wb, now[i].drawer, now[i].prog))) {
                    Forbid();
                    places[p].used = ++places_clock;
                    Permit();
                } else p = -1;
            }
            if (looks > 1 && log_path[0]) {
                int mine = (cfg.places || cfg.centre || GADGET_ON) && snappable(now[i].wb, now[i].drawer, now[i].prog);
                snprintf(line, sizeof line, "look %lu: new \"%s\" at %d %d %d %d, %s%s%s%s\n", (unsigned long)looks, now[i].key,
                         now[i].l, now[i].t, now[i].wd, now[i].ht,
                         cogged ? "placed by the cog" : !mine ? (now[i].wb ? "a Workbench window: Workbench's" : "not one GFOpenWindows keeps") :
                         rec < 0 ? "not seen opening, left where it opened" :
                         placed == 2 ? "centred before it opened" : placed ? "placed before it opened" :
                         p >= 0 ? "opened as asked" : "no place kept",
                         p >= 0 && (places[p].l != now[i].l || places[p].t != now[i].t) ? " (not at its kept place)" : "",
                         why[0] ? ": " : "", why);
                place_log(line);
            }
            if (looks > 1 && !cogged && p < 0 && now[i].wb && cfg.drawer_w && (now[i].flags & WFLG_SIZEGADGET) &&
                now[i].prog[0] && !stricmp(now[i].prog, "Workbench")) {
                /* a Workbench drawer with no place of its own: at least the drawer size */
                WORD wd = now[i].wd < cfg.drawer_w ? cfg.drawer_w : now[i].wd, ht = now[i].ht < cfg.drawer_h ? cfg.drawer_h : now[i].ht;
                WORD l = now[i].l, t = now[i].t;
                if (wd > now[i].sw) wd = now[i].sw;
                if (ht > now[i].sh) ht = now[i].sh;
                if (l + wd > now[i].sw) l = now[i].sw - wd;
                if (t + ht > now[i].sh) t = now[i].sh - ht;
                if (wd != now[i].wd || ht != now[i].ht) {
                    if (is_window(now[i].w)) ChangeWindowBox(now[i].w, l, t, wd, ht);
                    now[i].l = l; now[i].t = t; now[i].wd = wd; now[i].ht = ht;
                }
            }
        }
    }
    /* what the patch noted for windows that closed before a look saw them */
    Forbid();
    for (k = 0; k < OPENED; k++)
        if (opened[k].w && opened[k].look + 2 < looks) opened[k].w = NULL;
    Permit();
    nseen = 0;
    for (i = 0; i < n; i++) {
        if (!now[i].keep) continue;
        seen[nseen].w = now[i].w; seen[nseen].l = now[i].l; seen[nseen].t = now[i].t; seen[nseen].wd = now[i].wd; seen[nseen].ht = now[i].ht;
        memcpy(seen[nseen].key, now[i].key, sizeof seen[0].key);
        memcpy(seen[nseen].cur, now[i].cur, sizeof seen[0].cur);
        nseen++;
    }
    if (looks > 1) snap_all = 0;
    if (places_dirty && looks % 2 == 0) write_places(PLACES_ENV);
    if (places_dirty && looks % 10 == 0) { write_places(PLACES_ENV); write_places(PLACES_ENVARC); places_dirty = 0; }
}

/* ---- the commodity --------------------------------------------------------------------------- */

static struct NewBroker nb = {
    NB_VERSION, (STRPTR)"GFOpenWindows", (STRPTR)"GFOpenWindows 0.11", (STRPTR)"Snapping, edges, Amiga+Tab, wheel, window places, centring",
    NBU_UNIQUE | NBU_NOTIFY, 0, 0, NULL, 0
};
static CxObj *broker, *hot_fwd, *hot_back;
#define EV_FWD 1
#define EV_BACK 2

static CxObj *hotkey(const char *desc, struct MsgPort *port, LONG id)
{
    CxObj *f = CxFilter((STRPTR)desc), *snd, *tr;
    if (!f) return NULL;
    snd = CxSender(port, id);
    tr = CxTranslate(NULL);
    AttachCxObj(f, snd);
    AttachCxObj(f, tr);
    if (CxObjError(f)) { DeleteCxObjAll(f); return NULL; }
    AttachCxObj(broker, f);
    return f;
}

static void hotkeys(struct MsgPort *port)
{
    char back[64];
    const char *last;
    if (hot_fwd) { DeleteCxObjAll(hot_fwd); hot_fwd = NULL; }
    if (hot_back) { DeleteCxObjAll(hot_back); hot_back = NULL; }
    if (!cfg.switcher) return;
    hot_fwd = hotkey(cfg.key, port, EV_FWD);
    last = strrchr(cfg.key, ' ');
    if (last) snprintf(back, sizeof back, "%.*s shift%s", (int)(last - cfg.key), cfg.key, last);
    else snprintf(back, sizeof back, "shift %s", cfg.key);
    hot_back = hotkey(back, port, EV_BACK);
}

int main(void)
{
    struct MsgPort *port = NULL, *tport = NULL;
    struct timerequest *tr = NULL;
    CxObj *cust;
    LONG bsig = -1, wsig = -1, gsig = -1;
    int quit = 0, timer = 0;
    (void)version;
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39);
    LayersBase = OpenLibrary((STRPTR)"layers.library", 39);
    CxBase = OpenLibrary((STRPTR)"commodities.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 39);
    if (!IntuitionBase || !LayersBase || !CxBase || !GfxBase) goto out;
    me = FindTask(NULL);
    if ((bsig = AllocSignal(-1)) < 0 || (wsig = AllocSignal(-1)) < 0 || (gsig = AllocSignal(-1)) < 0) goto out;
    sig_button = 1UL << bsig; sig_wheel = 1UL << wsig; sig_grab = 1UL << gsig;
    { BYTE hsig = AllocSignal(-1); if (hsig >= 0) sig_hover = 1UL << hsig; }
    { BYTE ssig = AllocSignal(-1); if (ssig >= 0) sig_snap = 1UL << ssig; }   /* 0.10: a click on a snapshot gadget */
    pointers_on();
    /* 0.11, the wheel: a port for our IntuiMessages' replies, input.device for the held events */
    if ((wport = CreateMsgPort()) && (wio = (struct IOStdReq *)CreateIORequest(wport, sizeof *wio)) &&
        OpenDevice((STRPTR)"input.device", 0, (struct IORequest *)wio, 0)) { DeleteIORequest((struct IORequest *)wio); wio = NULL; }
    if (!(port = CreateMsgPort())) goto out;
    nb.nb_Port = port;
    if (!(broker = CxBroker(&nb, NULL))) goto out;    /* already running: NBU_UNIQUE */
    port->mp_Node.ln_Name = (char *)"GFOpenWindows";    /* GFOpenPrefs Windows finds this task by it */
    port->mp_Node.ln_Pri = 0;
    AddPort(port);
    read_settings();
    read_places();
    if (GADGET_ON && sig_snap) snap_class_on();        /* 0.10: before the patch, so the first windows get it */
    taller_on();                                       /* 0.8.2: taller title bars from here on */
    places_patch_on();                                 /* 0.8.1: always: the cog's placing goes through it too */
    cog.sem.ss_Link.ln_Name = (char *)COGPLACE_NAME;
    cog.sem.ss_Link.ln_Pri = 0;
    AddSemaphore(&cog.sem);
    cust = CxCustom(custom, 0);
    if (!cust) goto out;
    AttachCxObj(broker, cust);
    hotkeys(port);
    ActivateCxObj(broker, 1);
    if ((tport = CreateMsgPort()) && (tr = (struct timerequest *)CreateIORequest(tport, sizeof *tr)))
        timer = !OpenDevice((STRPTR)TIMERNAME, UNIT_VBLANK, (struct IORequest *)tr, 0);
    if (timer) { tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 1; tr->tr_time.tv_micro = 0; SendIO((struct IORequest *)tr); }

    while (!quit) {
        ULONG got = Wait((1UL << port->mp_SigBit) | sig_button | sig_wheel | sig_grab | sig_hover | sig_snap | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F |
                         (wport ? 1UL << wport->mp_SigBit : 0) |
                         (timer ? 1UL << tport->mp_SigBit : 0));
        CxMsg *m;
        if (got & SIGBREAKF_CTRL_C) quit = 1;
        if (got & SIGBREAKF_CTRL_F) {
            read_settings(); hotkeys(port); taller_on();
            places_patch_on();                   /* 0.8.1: always on: the kept places follow cfg.places inside it */
            if (GADGET_ON && sig_snap) snap_class_on();   /* 0.10: read live: on, the next look adds it to open windows */
            else if (snap_class) snap_off();             /* off: taken out of every window now */
        }
        if (got & sig_snap) snap_do();
        if (got & sig_wheel) wheel();
        wheel_replies();
        if (got & sig_grab) resize();
        if (got & sig_hover) hover();
        if (got & sig_button) gestures_done();
        while ((m = (CxMsg *)GetMsg(port))) {
            ULONG type = CxMsgType(m), id = CxMsgID(m);
            ReplyMsg((struct Message *)m);
            if (type == CXM_IEVENT) {
                if (on && (id == EV_FWD || id == EV_BACK)) switcher(id == EV_BACK);
            } else if (type == CXM_COMMAND) {
                switch (id) {
                case CXCMD_DISABLE: ActivateCxObj(broker, 0); on = 0; break;
                case CXCMD_ENABLE: ActivateCxObj(broker, 1); on = 1; break;
                case CXCMD_KILL: quit = 1; break;
                case CXCMD_UNIQUE:                    /* started again: read the settings again, stay */
                    read_settings(); hotkeys(port); taller_on();
                    places_patch_on();               /* 0.8.1: always on */
                    if (GADGET_ON && sig_snap) snap_class_on();
                    else if (snap_class) snap_off();
                    break;
                }
            }
        }
        if (quit && snap_class && !snap_off()) {
            /* 0.10: a gadget of ours is still somewhere (its class can't go): our code must stay */
            quit = 0;
            on = 0;
            ActivateCxObj(broker, 0);
            PutStr((STRPTR)"GFOpenWindows: a snapshot gadget is still in use; GFOpenWindows stays loaded\n");
        }
        if (quit && !places_patch_off()) {
            /* Another program patched OpenWindowTagList or OpenWindow after us: our
               code must stay. Everything passes through it; the rest of GFOpenWindows stops. */
            quit = 0;
            on = 0;
            ActivateCxObj(broker, 0);
            PutStr((STRPTR)"GFOpenWindows: another program has patched the same Intuition calls; "
                           "GFOpenWindows stays loaded, passing every window through\n");
        }
        if (timer && CheckIO((struct IORequest *)tr)) {
            WaitIO((struct IORequest *)tr);
            resize_watch();
            if (on) watch();
            wheel_prune();
            tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 1; tr->tr_time.tv_micro = 0;
            SendIO((struct IORequest *)tr);
        }
    }
    if (places_dirty) { write_places(PLACES_ENV); write_places(PLACES_ENVARC); }
out:
    if (snap_class) snap_off();
    taller_off();
    if (cog.sem.ss_Link.ln_Name) { Forbid(); cog.armed = 0; RemSemaphore(&cog.sem); Permit(); }
    if (broker) ActivateCxObj(broker, 0);           /* no new presses on an edge */
    grab_on = 0;
    resize_end();
    if (timer) { AbortIO((struct IORequest *)tr); WaitIO((struct IORequest *)tr); CloseDevice((struct IORequest *)tr); }
    if (tr) DeleteIORequest((struct IORequest *)tr);
    if (tport) DeleteMsgPort(tport);
    if (broker) DeleteCxObjAll(broker);
    if (port) {
        struct Message *m;
        if (port->mp_Node.ln_Name) RemPort(port);
        while ((m = GetMsg(port))) ReplyMsg(m);
        DeleteMsgPort(port);
    }
    pointers_off();
    if (gsig >= 0) FreeSignal(gsig);
    if (wsig >= 0) FreeSignal(wsig);
    if (wport) {
        /* our IntuiMessages still out: back within two seconds, or left where they are (a program not reading) */
        int k;
        for (k = 0; wout > 0 && k < 100; k++) { wheel_replies(); if (wout > 0) Delay(1); }
        if (wio) { CloseDevice((struct IORequest *)wio); DeleteIORequest((struct IORequest *)wio); }
        if (!wout) DeleteMsgPort(wport);
    }
    if (bsig >= 0) FreeSignal(bsig);
    if (CxBase) CloseLibrary(CxBase);
    if (LayersBase) CloseLibrary(LayersBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
