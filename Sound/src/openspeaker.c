/* GFOpenSpeaker 0.1: the speaker on the menu bar (Sound/DESIGN.md, "The
 * speaker on the menu bar"; Dale, 6 October 2026: "a menu bar speaker and
 * volume control").
 *
 * A small window on the Workbench screen's menu bar, at its far end, that
 * shows the volume. A click opens the levels: the volume, Paula's level,
 * AHI's level and mute, the same sliders as Sound prefs, and a button to
 * open Sound prefs. The mouse wheel over the speaker or its levels turns the
 * volume up and down, and the middle button mutes. A click on the speaker
 * leaves the active window active, as a click on GFOpenMenus' bar does.
 *
 * With the ACAHI board (AmiCradle) the levels are the board's LEVELS,
 * which the PC's mixer applies and its mixer page moves too: the speaker
 * follows LEVEL_SEQ five times a second. On a real Amiga the volume is
 * AHI's output volume, and Paula's level is the speakers' knob.
 *
 * Where the menu bar is: GFOpenMenus' settings (ENV:GFOpenMenus/Menus, read
 * with the Menus editor's om_prefs.c): the screen's title bar, or
 * GFOpenMenus' own bar at an edge, where the speaker sits at the bar's end.
 * It asks GFOpenMenus for that room by writing its width to
 * ENV:GFOpenMenus/Tray and sending GFOpenMenus Ctrl-F; the file goes when the
 * speaker does.
 *
 *   GFOpenSpeaker     (started by "Sound USE" at boot when Sound prefs say
 *                    to show it; Ctrl-C quits, Ctrl-F reads the settings
 *                    again and quits when the speaker is switched off)
 *
 * 0.1.9 (Dale, 11 November 2026: the speaker looked thinner than its neighbours, its waves cramped): on the tray's
 * grid (Common/traygrid.h, as GFOpenTitle 0.7.10), and drawn again 11 rows high as the network icons, its waves
 * 1-pixel arcs as the Wi-Fi's, centred on the same row.
 *
 * 0.1.7 (Dale, 1 November 2026: "yes" to the review's idle ticks): after 2 s with no input the tick is 1 s, not
 * 200 ms, and the first input brings it back at once (Common/idlepace.h); the 2 s checks and the save 2 s after
 * the wheel count time, not ticks, so they keep their times at either pace.
 *
 * 0.1.6 (the user, 24 October 2026, 21:39: "remove the percentage from volume and improve spacing on the
 * sys icon tray"): the speaker alone on the bar, its level only in the levels it opens (and the cross when
 * muted), with the same room each side as GFOpenTitle 0.7.3's tray icons (TRAY_PAD); on the screen's title bar
 * the row ends at the screen's depth gadget, as GFOpenTitle's does (bar_end()).
 *
 * 0.1.3 (the Team, 11 October 2026: the speaker's "55%" was seen in a white
 * box on the black bar): on the screen's title bar it is always painted in
 * the bar's own colour, never the bar pen's white. When reading the bar's
 * colour fails it keeps the colours last read well, or the theme's screenbar
 * colour before any reading has worked (Common/barpaint.h, bp_paint_kept()).
 * While its levels are open it shows pressed in a lighter shade of the bar,
 * not the fill pen's white. When the bar's height changes (GFOpenLook makes it
 * taller as it starts) its window is made again for the new height, as
 * GFOpenTitle 0.6.11's are.
 *
 * 0.1.8 (4 November 2026): also a public port named "OpenSpeaker", the name from before the GF rename, which the
 * ROM's own title-bar items (OpenKick's OpenBar) look for to leave the speaker to it (GFOpenTitle 0.7.9 does the same
 * with "OpenTitle").
 *
 * 0.2 (12 November 2026, Dale: GFOpenTitle owns the system bar, TitleBar/docs/system-bar.md): while GFOpenTitle 0.8
 * runs, the speaker is an item in its row (taskspace version 2, TitleBar/include/taskspace.h), drawn by GFOpenTitle
 * like every other icon there; the speaker has no window of its own over the bar then. Its pictures are the ones
 * it drew (muted, one wave, two waves), and the help under it says the volume. GFOpenTitle says where it put it
 * (TSE_PLACE), so the wheel, the middle button and a click there still come here first, as before. Without
 * GFOpenTitle (or on GFOpenMenus' own bar) it has its own window as before, and it moves into the row when
 * GFOpenTitle starts (checked every 2 s) and back out when GFOpenTitle ends (TSE_GONE).
 *
 * 0.3 (14 November 2026, Dale, 03:14: "the speaker icon in the systray is wrong, and it should now be a commodity"):
 *   - A commodity as the OS's are: Exchange's Show and Hide open and close its levels, Disable takes the speaker
 *     off the bar and Enable puts it back, Kill quits. The tool types (or the Shell's KEY=VALUE words) CX_PRIORITY
 *     (default 10), CX_POPUP (the levels open at the start) and MUTE (a hotkey that mutes and unmutes, none by
 *     default), src/sp_cx.h.
 *   - Its item goes through gfopensysbar.library (SysBar/) while the bar has an owner there (GFOpenTitle 0.10), so
 *     GFOpenTitle draws it as the other tray icons: 16 x 11, in the bar's text colour, now five pictures (muted, off,
 *     one, two and three waves for the volume's thirds; round arcs, not the 0.2 steps). The taskspace (an older
 *     GFOpenTitle) gets four of them, and our own window draws the same pictures where neither is there.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <devices/inputevent.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <libraries/commodities.h>
#include <graphics/gfxbase.h>
#include <graphics/layers.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/commodities.h>
#include <proto/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <proto/gfopensysbar.h>              /* 0.3: SysBar/include */

#include <string.h>
#include <stdio.h>

#include "sp_core.h"
#include "sp_amiga.h"
#include "om_prefs.h"
#include "wbclose.h"
#include "popuptheme.h"
#include "barpaint.h"
#include "../../Common/prefsapp.h"           /* where GFOpenPrefs is (SYS:Preferences) */
#include "../../Common/idlepace.h"           /* the tick slowed when nobody is there */
#include "../../Common/traygrid.h"          /* 0.1.9: the tray's grid, as GFOpenTitle 0.7.10 */
#include "taskspace.h"                       /* 0.2: the speaker as an item in GFOpenTitle's row */
#include "sp_cx.h"                           /* 0.3: the commodity's options and the pictures */

#define SP_OLD_PORT "OpenSpeaker"            /* 0.1.8: SP_SPEAKER_PORT before the GF rename */
static struct MsgPort *old_port;

const char version[] __attribute__((used)) = "$VER: GFOpenSpeaker 0.3.1 (14.11.2026) GFOpenPrefs, AmiCradle Team";

struct IntuitionBase *IntuitionBase;
struct Library *CyberGfxBase;
struct GfxBase *GfxBase;
struct Library *GadToolsBase, *LayersBase, *CxBase;
struct Library *SysBarBase, *IconBase;       /* 0.3: gfopensysbar.library (when there), icon.library for the tool types */

#define MENUS_ENV "ENV:GFOpenMenus/Menus"
#define TRAY_PARENT "ENV:GFOpenMenus"
#define TRAY_DIR  "ENV:GFOpenMenus/Tray"        /* the tray: a file per program on the menu bar's end, "width order" */
#define TRAY_MINE "ENV:GFOpenMenus/Tray/Speaker"
#define MY_ORDER  0                           /* lower is nearer the bar's end: GFOpenTitle's cog (-5) is rightmost, then the speaker, the clock (10) left of it */
#define SOUND_SECTION "SOUND"                /* 0.1.5: GFOpenPrefs at its Sound page (SYS:Preferences/GFOpenPrefs: prefsapp.h) */
#define NM_WHEEL_UP   0x7A                   /* the mouse wheel, as NewMouse and OS 3.2 send it */
#define NM_WHEEL_DOWN 0x7B
#define IECLASS_NEWMOUSE_ 0x16

static sp_settings cur;
static volatile ULONG *board;
static ULONG seen_seq;
static int bar_edge = OM_BAR_TITLE;

static struct Screen *scr;
static struct DrawInfo *dri;
static struct Window *spk;                   /* the speaker */
static WORD sx, sy, sw, sh;
static WORD made_bh;                         /* 0.1.3: the bar's height the speaker's window was made for */

static struct Window *pop;                   /* its levels */
static int wb_down;                         /* Workbench is shutting down or shut: 1 our windows are closed, 2 to open them again */
static struct Gadget *pop_glist;
static APTR vi;
static struct Window *before;                /* the window that was active when the levels opened */

/* from the input handler to the task */
static struct Task *me;
static ULONG sig_input, sig_wake;
/* The tick: 200 ms, and 1 s after 2 s with no input; sig_wake brings it back at the first input */
static struct idle_pace pace;
static volatile WORD wheel_steps;            /* up is positive */
static volatile UBYTE want_mute, want_toggle, want_close;
static volatile WORD hx0, hy0, hx1, hy1;     /* the speaker on the screen, for the handler */
static volatile WORD px0, py0, px1, py1;     /* its levels, while open */
static struct Screen *volatile hscr;

/* ---- the levels ------------------------------------------------------------------------------- */

static int level(void) { return cur.muted ? 0 : cur.volume; }

static void levels_out(int final)
{
    if (board) {
        sp_board_set(board, sp_levels_word(&cur));
        seen_seq = sp_board_seq(board);
    } else if (final) {
        sp_volume_to_units(&cur);            /* AHI hears it when a program next opens it */
        sp_store(&cur, 0);
    }
}

/* the board's levels, when the PC's mixer page or Sound prefs moved them: 1 when they changed */
static int levels_in(void)
{
    ULONG seq;
    if (!board || (seq = sp_board_seq(board)) == seen_seq) return 0;
    seen_seq = seq;
    sp_from_levels_word(&cur, sp_board_levels(board));
    return 1;
}

/* ---- the speaker -------------------------------------------------------------------------------- */

#define PRESSED_SHADE 56                     /* while the levels are open: 56/256 of the way to white on a dark bar */

/* The theme's colour for the bar (its screenbar), ARGB; 0 with no theme. */
static ULONG theme_bar(void)
{
    ogt_rgb c;
    return pt_colour("screenbar", &c) ? pt_argb(c) : 0;
}

/* The speaker's background, and the pen for it on that. On the screen's title bar: the bar's own colour, a shade
 * lighter while the levels are open, never the bar pen's white (0.1.3). On GFOpenMenus' own bar: its pen, as before,
 * and the theme's accent (or the fill pen without a theme) while the levels are open. */
static LONG paint_back(void)
{
    struct RastPort *rp = spk->RPort;
    if (bar_edge == OM_BAR_TITLE) {
        ULONG keep = theme_bar();
        int shade = pop ? PRESSED_SHADE : 0;
        bp_paint_kept(spk, keep, dri->dri_Pens[BARBLOCKPEN], 0, 0, sw, sh, shade);
        return bp_text_pen_kept(scr, keep, shade, dri->dri_Pens[BARDETAILPEN]);
    }
    if (pop) {
        SetAPen(rp, pt_ok ? pt_hot_pen() : dri->dri_Pens[FILLPEN]);
        RectFill(rp, 0, 0, sw - 1, sh - 1);
        return pt_ok ? pt_hot_text_pen() : dri->dri_Pens[FILLTEXTPEN];
    }
    bp_paint(spk, dri->dri_Pens[BARBLOCKPEN], 0, 0, sw, sh);
    return bp_text_pen(scr, dri->dri_Pens[BARDETAILPEN]);
}

/* ---- 0.2: the speaker in GFOpenTitle's row ---------------------------------------------------- */
#define TS_NAME "Speaker"                    /* also its tray file, as before */
#define TS_EVENTS "GFOpenSpeaker.bar"        /* our port for GFOpenTitle's events */
#define PIC_W SPX_W
#define PIC_H SPX_H
#define BAR_TASKSPACE 1                      /* in_bar: GFOpenTitle's taskspace item */
#define BAR_LIBRARY   2                      /* 0.3: gfopensysbar.library's item, drawn by the bar's owner */
static int in_bar;                           /* 0: our own window; else BAR_*, we have no window */
static struct SysBarItem *sb_item;           /* 0.3: our item in the library while in_bar is BAR_LIBRARY */
static UBYTE pics[SPX_STATES * PIC_W * PIC_H];
static struct MsgPort *ts_events;            /* public, TS_EVENTS */
static int ts_shown = -1;                    /* the picture last sent */
static char ts_help[48];

static int pic_state(void) { return spx_state(cur.muted, cur.volume); }

/* 0.3: the taskspace takes four pictures (TS_STATES): muted, off, then two waves and three; one wave shows as two */
static const UBYTE ts_pics_of[TS_STATES] = { SPX_MUTED, SPX_OFF, SPX_WAVES2, SPX_WAVES3 };
static int ts_state(int st) { return st == SPX_WAVES3 ? 3 : st >= SPX_WAVES1 ? 2 : st; }

static void help_text(char *t, int size)
{
    if (level() == 0) strlcpy(t, cur.muted ? "Sound: muted" : "Sound: off", size);
    else snprintf(t, size, "Sound: volume %d%%", cur.volume);
}

/* one message to GFOpenTitle and its reply; the result, or -1 when GFOpenTitle isn't there */
static LONG ts_send(UWORD command)
{
    static UBYTE tpics[TS_STATES * PIC_W * PIC_H];
    struct TaskspaceMsg m;
    struct MsgPort *reply, *p;
    int i;
    if (!(reply = CreateMsgPort())) return -1;
    memset(&m, 0, sizeof m);
    m.tsm_Message.mn_ReplyPort = reply;
    m.tsm_Message.mn_Length = sizeof m;
    m.tsm_Magic = TASKSPACE_MAGIC;
    m.tsm_Version = TASKSPACE_VERSION;
    m.tsm_Command = command;
    strcpy(m.tsm_Name, TS_NAME);
    m.tsm_Order = MY_ORDER;
    m.tsm_State = ts_state(pic_state());
    m.tsm_Width = PIC_W; m.tsm_Height = PIC_H; m.tsm_States = TS_STATES;
    for (i = 0; i < TS_STATES; i++) spx_pic(tpics + i * PIC_W * PIC_H, ts_pics_of[i]);
    m.tsm_Pixels = tpics;
    help_text(m.tsm_Help, sizeof m.tsm_Help);
    strcpy(m.tsm_Port, TS_EVENTS);
    Forbid();
    if ((p = FindPort((STRPTR)TASKSPACE_PORT))) PutMsg(p, &m.tsm_Message);
    Permit();
    if (p) WaitPort(reply), GetMsg(reply);
    DeleteMsgPort(reply);
    if (!p) return -1;
    if (m.tsm_Result == TSR_OK) { ts_shown = pic_state(); strcpy(ts_help, m.tsm_Help); }
    return m.tsm_Result;
}

/* 0.3: the item in gfopensysbar.library: added, its picture and help changed, or removed. SBR_OK, or why not. */
static LONG sb_send(UWORD command)
{
    char help[SB_HELPLEN];
    LONG err = SBR_UNKNOWN;
    if (!SysBarBase) return SBR_UNKNOWN;
    help_text(help, sizeof help);
    if (command == TSC_REMOVE) {
        if (sb_item) SB_RemItem(sb_item);
        sb_item = NULL;
        return SBR_OK;
    }
    if (command == TSC_ADD && !sb_item) {
        struct TagItem t[] = {
            { SBA_Name, (ULONG)TS_NAME }, { SBA_Help, (ULONG)help }, { SBA_Order, MY_ORDER },
            { SBA_Width, PIC_W }, { SBA_Height, PIC_H }, { SBA_States, SPX_STATES }, { SBA_Pixels, (ULONG)pics },
            { SBA_State, (ULONG)pic_state() }, { SBA_Port, (ULONG)ts_events }, { SBA_Error, (ULONG)&err }, { TAG_DONE, 0 } };
        if (!(sb_item = SB_AddItemA(t))) return err;
    } else {
        struct TagItem t[] = { { SBA_State, (ULONG)pic_state() }, { SBA_Help, (ULONG)help }, { TAG_DONE, 0 } };
        if (!sb_item || (err = SB_SetItemA(sb_item, t)) != SBR_OK) return err;
    }
    ts_shown = pic_state();
    strlcpy(ts_help, help, sizeof ts_help);
    return SBR_OK;
}

/* the item, in whichever row we are in */
static void bar_send(UWORD command)
{
    if (in_bar == BAR_LIBRARY) sb_send(command);
    else if (in_bar == BAR_TASKSPACE) ts_send(command);
}

/* who draws the bar's row: BAR_LIBRARY (gfopensysbar.library has an owner: GFOpenTitle 0.10), BAR_TASKSPACE (an
 * older GFOpenTitle's port), or 0 (nobody: our own window) */
static int bar_owner_here(void)
{
    struct MsgPort *p;
    if (SysBarBase && SB_OwnerTask()) return BAR_LIBRARY;
    Forbid();
    p = FindPort((STRPTR)TASKSPACE_PORT);
    Permit();
    return p ? BAR_TASKSPACE : 0;
}

static void draw_speaker(void)
{
    struct RastPort *rp;
    LONG fg;
    WORD cy, x, h;
    if (in_bar) {                            /* 0.2: GFOpenTitle draws it: only a changed picture or help is sent */
        char t[48];
        help_text(t, sizeof t);
        if (pic_state() != ts_shown || strcmp(t, ts_help)) bar_send(TSC_SET);
        return;
    }
    if (!spk) return;
    rp = spk->RPort;
    fg = paint_back();
    SetAPen(rp, fg);
    h = sh;                                                      /* the bar's rows: the line under them isn't ours */
    cy = (h - (bar_edge == OM_BAR_TITLE ? 1 : 0)) / 2;           /* 0.1.9: centred as GFOpenTitle's network icons */
    x = TRAY_PAD;                                                /* 0.1.9: in the tray's TRAY_ICON box, 11 rows as the network icons */
    {                                                            /* 0.3: the same pictures as in the row */
        const UBYTE *px = pics + (ULONG)pic_state() * PIC_W * PIC_H;
        int r, c;
        for (r = 0; r < PIC_H; r++)
            for (c = 0; c < PIC_W; c++)
                if (px[r * PIC_W + c]) {
                    int run = c;
                    while (run < PIC_W && px[r * PIC_W + run]) run++;
                    RectFill(rp, x + c, cy - PIC_H / 2 + r, x + run - 1, cy - PIC_H / 2 + r);
                    c = run;
                }
    }
}

/* GFOpenMenus' settings: where the bar is (the screen's title bar when GFOpenMenus doesn't run or is off) */
static int read_bar_edge(void)
{
    om_prefs p;
    BPTR fh;
    char buf[2048];
    LONG n;
    struct MsgPort *port;
    Forbid();
    port = FindPort((STRPTR)"GFOpenMenus");
    Permit();
    if (!port) return OM_BAR_TITLE;
    om_defaults(&p);
    if ((fh = Open((STRPTR)MENUS_ENV, MODE_OLDFILE))) {
        n = Read(fh, buf, sizeof buf - 1);
        Close(fh);
        buf[n > 0 ? n : 0] = 0;
        om_parse(&p, buf);
    }
    return p.enabled ? p.bar : OM_BAR_TITLE;
}

static void tell_openmenus(void)
{
    struct MsgPort *p;
    Forbid();
    if ((p = FindPort((STRPTR)"GFOpenMenus")) && p->mp_SigTask) Signal((struct Task *)p->mp_SigTask, SIGBREAKF_CTRL_F);
    Permit();
}

static void tray(int width)
{
    BPTR fh, lock;
    char t[24];
    if (width > 0) {
        if ((lock = Lock((STRPTR)TRAY_PARENT, ACCESS_READ))) UnLock(lock);   /* a fresh system has neither yet */
        else if ((lock = CreateDir((STRPTR)TRAY_PARENT))) UnLock(lock);
        if ((lock = Lock((STRPTR)TRAY_DIR, ACCESS_READ))) UnLock(lock);
        else if ((lock = CreateDir((STRPTR)TRAY_DIR))) UnLock(lock);
        if ((fh = Open((STRPTR)TRAY_MINE, MODE_NEWFILE))) { LONG n = snprintf(t, sizeof t, "%d %d\n", width, MY_ORDER); Write(fh, t, n); Close(fh); }
    } else DeleteFile((STRPTR)TRAY_MINE);
    tell_openmenus();
}

/* The room the tray's other programs nearer the bar's end take (none by default: the speaker is order 0):
 * their widths in pixels, and how many they are. */
static int tray_before(int *count)
{
    struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, NULL);
    BPTR lock = Lock((STRPTR)TRAY_DIR, ACCESS_READ);
    int sum = 0;
    *count = 0;
    if (fib && lock && Examine(lock, fib)) {
        while (ExNext(lock, fib)) {
            char path[sizeof TRAY_DIR "/" + sizeof fib->fib_FileName], buf[24];
            BPTR fh;
            int w = 0, order = 0;
            LONG n;
            if (fib->fib_DirEntryType > 0 || !strcmp((const char *)fib->fib_FileName, "Speaker")) continue;
            snprintf(path, sizeof path, TRAY_DIR "/%s", (const char *)fib->fib_FileName);
            if (!(fh = Open((STRPTR)path, MODE_OLDFILE))) continue;
            n = Read(fh, buf, sizeof buf - 1);
            Close(fh);
            buf[n > 0 ? n : 0] = 0;
            if (sscanf(buf, "%d %d", &w, &order) < 1 || w <= 0 || w > 400) continue;
            if (order < MY_ORDER || (order == MY_ORDER && strcmp((const char *)fib->fib_FileName, "Speaker") < 0)) { sum += w; (*count)++; }
        }
    }
    if (lock) UnLock(lock);
    if (fib) FreeDosObject(DOS_FIB, fib);
    return sum;
}

/* 0.1.6: where the tray's row ends on the screen's title bar: the screen's depth gadget, its own width and
 * 2 pixels, as GFOpenTitle 0.7.3 has it, or the width the bar draws it at */
static WORD bar_end(void)
{
    struct Gadget *g;
    for (g = scr->FirstGadget; g; g = g->NextGadget)
        if ((g->GadgetType & GTYP_SYSTYPEMASK) == GTYP_SDEPTH) {
            /* its left edge, as openframes reads it: from the right edge when GFLG_RELRIGHT */
            LONG x = g->LeftEdge + ((g->Flags & GFLG_RELRIGHT) ? scr->Width - 1 : 0), end = scr->Width - x;
            if (end > 0 && end < 3 * scr->BarHeight) return (WORD)end + 2;
        }
    /* no such gadget: the open ROM's Intuition draws the depth gadget into the bar itself, 23 pixels wide on a
     * hires screen and 17 on a lores one, as OS 3.2's SDEPTHIMAGE is */
    return ((scr->Flags & SCREENHIRES) ? 23 : 17) + 2;
}

/* where the speaker goes: the bar's far end (GFOpenMenus keeps 2 bar heights for the screen's depth gadget) */
static void place(void)
{
    WORD bh = scr->BarHeight + 1;
    int n, off = tray_before(&n);            /* after any program with a lower order than ours */
    sw = TRAY_CELL;                          /* 0.1.6: no level beside it; 0.1.9: the tray's cell */
    switch (bar_edge) {
    case OM_BAR_BOTTOM: sh = bh; sx = scr->Width - sw - off; sy = scr->Height - bh; break;
    case OM_BAR_LEFT:   sh = bh; sx = 0; sy = scr->Height - bh * (n + 1); break;
    case OM_BAR_RIGHT:  sh = bh; sx = scr->Width - sw; sy = scr->Height - bh * (n + 1); break;
    case OM_BAR_TOP:    sh = bh; sx = scr->Width - 2 * scr->BarHeight - sw - off; sy = 0; break;
    default:            sh = scr->BarHeight; sx = scr->Width - bar_end() - sw - off; sy = 0; break;   /* in the title bar, above its line */
    }
    hx0 = sx; hy0 = sy; hx1 = sx + sw; hy1 = sy + sh;
}

static int speaker_ours(struct Window *w) { return w == spk || w == pop; }
static int scr_locked;                      /* 0.2: in the row we have no window to keep the screen open: a lock does */

static void close_speaker(void)
{
    hscr = NULL;
    pt_free();
    bp_release();
    if (in_bar) { bar_send(TSC_REMOVE); in_bar = 0; ts_shown = -1; }
    if (spk) { CloseWindow(spk); spk = NULL; }
    if (dri) { FreeScreenDrawInfo(scr, dri); dri = NULL; }
    if (scr && scr_locked) UnlockPubScreen(NULL, scr);
    scr_locked = 0;
    scr = NULL;
}

static int open_speaker(void)
{
    struct Screen *s = LockPubScreen((STRPTR)"Workbench");
    if (!s) return 0;
    scr = s;
    made_bh = s->BarHeight;
    if ((dri = GetScreenDrawInfo(s))) {
        bar_edge = read_bar_edge();
        pt_load(s);
        place();
        bp_sample(s, bar_edge == OM_BAR_TITLE, speaker_ours);
        /* 0.2: GFOpenTitle's row when it runs (on the screen's title bar); our box until it says where it put us */
        if (bar_edge == OM_BAR_TITLE && ts_events && (in_bar = bar_owner_here())) {
            if ((in_bar == BAR_LIBRARY ? sb_send(TSC_ADD) == SBR_OK : ts_send(TSC_ADD) == TSR_OK) == 0) in_bar = 0;
        }
        if (in_bar) {
            scr_locked = 1;                  /* kept locked: no window of ours holds it */
            hscr = s;
            return 1;
        }
        spk = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)s, WA_Left, sx, WA_Top, sy, WA_Width, sw, WA_Height, sh,
                             WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE,
                             WA_IDCMP, IDCMP_REFRESHWINDOW, TAG_DONE);
        if (spk) WindowToBack(spk);              /* under every other window, as the bar's title is: a window over the bar covers it */
    }
    UnlockPubScreen(NULL, s);                /* the window keeps the screen open */
    if (!spk) { close_speaker(); return 0; }
    SetFont(spk->RPort, dri->dri_Font);
    draw_speaker();
    hscr = s;
    tray(sw);                                /* in the title bar too: the clock and others keep clear of us */
    return 1;
}

/* ---- its levels ------------------------------------------------------------------------------------ */

enum { P_MUTE, P_VOL, P_PAULA, P_AHI, P_NOTE, P_PREFS, P_COUNT };
static struct Gadget *pg[P_COUNT];

#define PSET(id, ...) do { if (pg[id]) GT_SetGadgetAttrs(pg[id], pop, NULL, __VA_ARGS__, TAG_DONE); } while (0)

static void pop_show(void)
{
    PSET(P_MUTE, GTCB_Checked, cur.muted);
    PSET(P_VOL, GTSL_Level, cur.volume);
    PSET(P_PAULA, GTSL_Level, cur.paula);
    PSET(P_AHI, GTSL_Level, cur.ahi);
}

static void pop_draw_frame(void)
{
    struct RastPort *rp = pop->RPort;
    WORD w = pop->Width, h = pop->Height, fh = dri->dri_Font->tf_YSize;
    if (pt_ok) {
        pt_panel(rp, w, h);                  /* the theme's menu colour and a slim frame, as GFOpenMenus' menus */
        SetAPen(rp, pt_text_pen());
    } else {
        SetAPen(rp, dri->dri_Pens[BACKGROUNDPEN]);
        RectFill(rp, 1, 1, w - 2, h - 2);
        DrawBevelBox(rp, 0, 0, w, h, GT_VisualInfo, (ULONG)vi, TAG_DONE);
        SetAPen(rp, dri->dri_Pens[TEXTPEN]);
    }
    SetDrMd(rp, JAM1);
    Move(rp, 10, 6 + rp->TxBaseline);
    Text(rp, (STRPTR)"Sound", 5);
    (void)fh;
}

/* the window that was active before the levels opened, active again, if it is still open */
static void give_back(void)
{
    struct Window *w;
    ULONG lock;
    if (!before || IntuitionBase->ActiveWindow == before) return;
    lock = LockIBase(0);
    for (w = scr->FirstWindow; w && w != before; w = w->NextWindow) ;
    UnlockIBase(lock);
    if (w) ActivateWindow(w);
}

static void pop_close(int activate_before)
{
    if (!pop) return;
    px0 = py0 = px1 = py1 = 0;
    CloseWindow(pop); pop = NULL;
    FreeGadgets(pop_glist); pop_glist = NULL;
    if (vi) { FreeVisualInfo(vi); vi = NULL; }
    sp_save_levels(&cur, 1);                 /* where the knob was left, kept */
    if (activate_before) give_back();
    before = NULL;
    draw_speaker();
}

static void pop_open(void)
{
    struct NewGadget ng;
    struct Gadget *g;
    WORD fh = dri->dri_Font->tf_YSize, lh = fh + 6, W = 260, L = 10, X = 96, row, x, y, H;
    if (pop || !(vi = GetVisualInfoA(scr, NULL))) return;
    memset(pg, 0, sizeof pg);
    memset(&ng, 0, sizeof ng);
    ng.ng_TextAttr = scr->Font;
    ng.ng_VisualInfo = vi;
    g = CreateContext(&pop_glist);
#define PG(kind, id, gx, gy, gw, gh, text, flags, ...) \
    (ng.ng_LeftEdge = (gx), ng.ng_TopEdge = (gy), ng.ng_Width = (gw), ng.ng_Height = (gh), ng.ng_GadgetText = (STRPTR)(text), \
     ng.ng_Flags = (flags), ng.ng_GadgetID = (id), g = pg[id] = CreateGadget(kind, g, &ng, __VA_ARGS__, TAG_DONE))
#define PSLIDER(id, gy, text) \
    PG(SLIDER_KIND, id, X, gy, W - X - 64, lh - 2, text, PLACETEXT_LEFT, GTSL_Min, 0, GTSL_Max, 100, GTSL_Level, 0, \
       GTSL_LevelFormat, (ULONG)"%ld%%", GTSL_MaxLevelLen, 6, GTSL_LevelPlace, PLACETEXT_RIGHT, GA_RelVerify, TRUE, GA_Immediate, TRUE)
    PG(CHECKBOX_KIND, P_MUTE, W - 84, 4, 26, fh + 3, "Mute", PLACETEXT_RIGHT, GTCB_Scaled, TRUE);
    row = fh + 14;
    PSLIDER(P_VOL, row, board ? "Volume" : "Volume (AHI)"); row += lh + 2;
    if (board) {
        PSLIDER(P_PAULA, row, "Paula"); row += lh + 2;
        PSLIDER(P_AHI, row, "AHI"); row += lh + 4;
        PG(TEXT_KIND, P_NOTE, L, row, W - 2 * L, lh - 2, NULL, 0, GTTX_Text, (ULONG)"Plays on: your speakers");
    } else {
        row += 2;
        PG(TEXT_KIND, P_NOTE, L, row, W - 2 * L, lh - 2, NULL, 0, GTTX_Text, (ULONG)"Amiga sound (Paula): your speakers' knob");
    }
    row += lh + 4;
    PG(BUTTON_KIND, P_PREFS, L, row, W - 2 * L, lh, "Sound prefs...", 0, GA_Disabled, FALSE);
    row += lh + 8;
    H = row;
    if (!g) { FreeGadgets(pop_glist); pop_glist = NULL; FreeVisualInfo(vi); vi = NULL; return; }
    /* beside the speaker, towards the middle of the screen */
    x = sx + sw - W; if (x < 0) x = 0;
    if (bar_edge == OM_BAR_LEFT) x = 0;
    y = (bar_edge == OM_BAR_BOTTOM || bar_edge == OM_BAR_LEFT || bar_edge == OM_BAR_RIGHT) ? sy - H : sy + sh + (bar_edge == OM_BAR_TITLE ? 1 : 0);
    if (y < 0) y = 0;
    before = IntuitionBase->ActiveWindow;
    pop = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Left, x, WA_Top, y, WA_Width, W, WA_Height, H,
                         WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE,   /* the program you are in stays active */
                         WA_IDCMP, IDCMP_RAWKEY | IDCMP_REFRESHWINDOW | SLIDERIDCMP | BUTTONIDCMP | CHECKBOXIDCMP,
                         TAG_DONE);
    if (!pop) { FreeGadgets(pop_glist); pop_glist = NULL; FreeVisualInfo(vi); vi = NULL; before = NULL; return; }
    SetFont(pop->RPort, dri->dri_Font);
    pop_draw_frame();
    AddGList(pop, pop_glist, (UWORD)~0, -1, NULL);
    RefreshGList(pop_glist, pop, NULL, -1);
    GT_RefreshWindow(pop, NULL);
    pop_show();
    px0 = x; py0 = y; px1 = x + W; py1 = y + H;
    draw_speaker();
}

static void open_sound_prefs(void)
{
    char cmd[64];
    BPTR in = Open((STRPTR)"NIL:", MODE_OLDFILE), out = Open((STRPTR)"NIL:", MODE_NEWFILE);
    snprintf(cmd, sizeof cmd, "\"%s\" " SOUND_SECTION, op_app_path(1));
    if (in && out && SystemTags((STRPTR)cmd, SYS_Input, in, SYS_Output, out, SYS_Asynch, TRUE, NP_StackSize, 32768, TAG_DONE) != -1)
        return;
    if (in) Close(in);
    if (out) Close(out);
}

/* 1 when the speaker should go */
static int pop_events(void)
{
    struct IntuiMessage *m;
    int close = 0, prefs = 0;
    while (pop && (m = GT_GetIMsg(pop->UserPort))) {
        ULONG cls = m->Class;
        UWORD code = m->Code;
        struct Gadget *gg = (struct Gadget *)m->IAddress;
        GT_ReplyIMsg(m);
        if (cls == IDCMP_RAWKEY && code == 0x45) close = 2;            /* Esc */
        else if (cls == IDCMP_REFRESHWINDOW) { GT_BeginRefresh(pop); pop_draw_frame(); GT_EndRefresh(pop, TRUE); }
        else if ((cls == IDCMP_MOUSEMOVE || cls == IDCMP_GADGETUP || cls == IDCMP_GADGETDOWN) && gg) {
            int final = cls == IDCMP_GADGETUP;
            switch (gg->GadgetID) {
            case P_VOL: cur.volume = (WORD)code; levels_out(final); break;
            case P_PAULA: cur.paula = (WORD)code; levels_out(final); break;
            case P_AHI: cur.ahi = (WORD)code; levels_out(final); break;
            case P_MUTE: cur.muted = (gg->Flags & GFLG_SELECTED) != 0; levels_out(1); break;
            case P_PREFS: prefs = 1; close = 2; break;
            }
            draw_speaker();
            if (final && !close) give_back();   /* a click on a gadget activated the levels: back to the program */
        }
    }
    if (close) pop_close(close == 2);
    if (prefs) open_sound_prefs();
    return 0;
}

/* ---- the input handler: the wheel and the buttons over the speaker or its levels ------------------ */

/* 0.2: GFOpenTitle's events: where it put the speaker, a click it saw, or that it is ending */
static void bar_events(void)
{
    struct TaskspaceEvent *e;
    int gone = 0;
    if (!ts_events) return;
    while ((e = (struct TaskspaceEvent *)GetMsg(ts_events))) {
        struct SysBarEvent *b = (struct SysBarEvent *)e;
        if (e->tse_Message.mn_Length == sizeof *b && b->sbe_Magic == SBE_MAGIC) {   /* 0.3: the library's: not replied */
            if (in_bar == BAR_LIBRARY && b->sbe_Item == sb_item && (b->sbe_Kind == SBE_PLACE || b->sbe_Kind == SBE_CLICK)) {
                sx = b->sbe_Left; sy = b->sbe_Top; sw = b->sbe_Width; sh = b->sbe_Height;
                Disable(); hx0 = sx; hy0 = sy; hx1 = sx + sw; hy1 = sy + sh; Enable();
                if (b->sbe_Kind == SBE_CLICK) want_toggle = 1;
            }
            SB_DoneEvent(b);
            continue;
        }
        if (e->tse_Magic == TASKSPACE_MAGIC && in_bar == BAR_TASKSPACE) {
            if (e->tse_Kind == TSE_PLACE || e->tse_Kind == TSE_CLICK) {
                sx = e->tse_Left; sy = e->tse_Top; sw = e->tse_Width; sh = e->tse_Height;
                Disable(); hx0 = sx; hy0 = sy; hx1 = sx + sw; hy1 = sy + sh; Enable();
            }
            if (e->tse_Kind == TSE_CLICK) want_toggle = 1;   /* only when the handler didn't take it first */
            if (e->tse_Kind == TSE_GONE) gone = 1;
        }
        ReplyMsg(&e->tse_Message);
    }
    if (want_toggle) { want_toggle = 0; if (pop) pop_close(1); else pop_open(); }
    if (gone) {                              /* GFOpenTitle is ending: our own window again */
        in_bar = 0; ts_shown = -1;
        if (pop) pop_close(0);
        close_speaker();
        if (!open_speaker()) wb_down = 2;
    }
}

static int over(WORD x0, WORD y0, WORD x1, WORD y1, struct Screen *s)
{
    return x1 > x0 && s->MouseX >= x0 && s->MouseX < x1 && s->MouseY >= y0 && s->MouseY < y1;
}

static void custom(CxMsg *msg, CxObj *co)
{
    struct InputEvent *ie = (struct InputEvent *)CxMsgData(msg), *e;
    struct Screen *s = IntuitionBase->FirstScreen;
    (void)co;
    for (e = ie; e; e = e->ie_NextEvent)
        if (idle_pace_is_input(e->ie_Class)) {
            if (idle_pace_input(&pace)) Signal(me, sig_wake);   /* ticking slowly: back to full speed now */
            break;
        }
    if (!hscr || s != hscr) return;
    for (; ie; ie = ie->ie_NextEvent) {
        int on_spk = over(hx0, hy0, hx1, hy1, s), on_pop = over(px0, py0, px1, py1, s);
        UWORD code = ie->ie_Code;
        if ((ie->ie_Class == IECLASS_RAWKEY || ie->ie_Class == IECLASS_NEWMOUSE_) && (code == NM_WHEEL_UP || code == NM_WHEEL_DOWN) &&
            (on_spk || on_pop)) {
            wheel_steps += code == NM_WHEEL_UP ? 1 : -1;
            ie->ie_Class = IECLASS_NULL;
            Signal(me, sig_input);
        } else if (ie->ie_Class == IECLASS_RAWMOUSE && on_spk) {
            if (code == IECODE_MBUTTON) { want_mute = 1; Signal(me, sig_input); }
            else if (code == IECODE_LBUTTON) { want_toggle = 1; Signal(me, sig_input); }
            if (code == IECODE_MBUTTON || code == IECODE_LBUTTON || code == (IECODE_MBUTTON | IECODE_UP_PREFIX) ||
                code == (IECODE_LBUTTON | IECODE_UP_PREFIX))
                ie->ie_Class = IECLASS_NULL;     /* ours: the active window keeps the activation */
        } else if (ie->ie_Class == IECLASS_RAWMOUSE && on_pop && code == IECODE_MBUTTON) {
            want_mute = 1; ie->ie_Class = IECLASS_NULL; Signal(me, sig_input);
        } else if (ie->ie_Class == IECLASS_RAWMOUSE && px1 > px0 && !on_pop && (code == IECODE_LBUTTON || code == IECODE_RBUTTON)) {
            want_close = 1; Signal(me, sig_input);    /* a click anywhere else closes the levels, and goes on to where it was */
        }
    }
}

static struct NewBroker nb = {
    NB_VERSION, (STRPTR)"GFOpenSpeaker", (STRPTR)"GFOpenSpeaker 0.3", (STRPTR)"The speaker on the menu bar: volume and mute",
    NBU_UNIQUE | NBU_NOTIFY, COF_SHOW_HIDE, SPX_PRI_DEFAULT, NULL, 0   /* 10: before GFOpenWindows, whose wheel would activate the window under the pointer */
};

#define CX_ID_MUTE 1                         /* 0.3: the MUTE hotkey's events */
static spx_opts opts;
static int disabled;                         /* 0.3: Exchange's Disable: off the bar until Enable */

/* 0.3: the tool types (from Workbench: our icon's, WBStartup's copy included) or the Shell's KEY=VALUE words */
static void read_options(int argc, char **argv)
{
    spx_defaults(&opts);
    if (argc == 0 && argv) {
        struct WBStartup *wb = (struct WBStartup *)argv;
        struct DiskObject *dob;
        BPTR old;
        if (!(IconBase = OpenLibrary((STRPTR)"icon.library", 37)) || wb->sm_NumArgs < 1) return;
        old = CurrentDir(wb->sm_ArgList[0].wa_Lock);
        if ((dob = GetDiskObject((STRPTR)wb->sm_ArgList[0].wa_Name))) {
            char **tt;
            for (tt = (char **)dob->do_ToolTypes; tt && *tt; tt++) spx_word(&opts, *tt);
            FreeDiskObject(dob);
        }
        CurrentDir(old);
    } else {
        int i;
        for (i = 1; i < argc; i++) spx_word(&opts, argv[i]);
    }
}

/* the MUTE hotkey under the broker: its events come to the broker's port as CX_ID_MUTE, and go no further */
static void add_hotkey(CxObj *broker, struct MsgPort *port)
{
    CxObj *f, *snd, *tr;
    if (!opts.mute[0] || !(f = CxFilter((STRPTR)opts.mute))) return;
    snd = CxSender(port, CX_ID_MUTE);
    tr = CxTranslate(NULL);
    if (snd) AttachCxObj(f, snd);
    if (tr) AttachCxObj(f, tr);
    if (CxObjError(f) || !snd || !tr) { DeleteCxObjAll(f); return; }
    AttachCxObj(broker, f);
}

/* ---- the task ---------------------------------------------------------------------------------- */

int main(int argc, char **argv)
{
    struct MsgPort *port = NULL, *tport = NULL;
    struct timerequest *tr = NULL;
    CxObj *broker = NULL, *cust;
    LONG s1 = -1, s2 = -1;
    int quit = 0, timer = 0, due;
    ULONG asked = 200000, save_in = 0;      /* save_in: us until the levels are saved (0: nothing to save) */
    ULONG clock_us = 0;                     /* time ticked, in a 4 s round: the 2 s checks follow it at either pace */
    struct MsgPort *other;
    Forbid();
    other = FindPort((STRPTR)SP_SPEAKER_PORT);
    if (other && other->mp_SigTask) Signal((struct Task *)other->mp_SigTask, SIGBREAKF_CTRL_F);   /* one speaker: that one reads again */
    Permit();
    if (other) return RETURN_OK;
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 39);
    GadToolsBase = OpenLibrary((STRPTR)"gadtools.library", 39);
    LayersBase = OpenLibrary((STRPTR)"layers.library", 39);
    CyberGfxBase = OpenLibrary((STRPTR)"cybergraphics.library", 41);   /* the panel's exact colours */
    CxBase = OpenLibrary((STRPTR)"commodities.library", 39);
    if (!IntuitionBase || !GfxBase || !GadToolsBase || !LayersBase || !CxBase) goto out;
    read_options(argc, argv);                /* 0.3 */
    nb.nb_Pri = (BYTE)opts.pri;
    SysBarBase = OpenLibrary((STRPTR)SYSBARNAME, SYSBAR_VERSION);   /* 0.3: none is fine: the taskspace or our window */
    {
        int i;
        for (i = 0; i < SPX_STATES; i++) spx_pic(pics + i * PIC_W * PIC_H, i);
    }
    wbc_start();
    me = FindTask(NULL);
    if ((s1 = AllocSignal(-1)) < 0 || (s2 = AllocSignal(-1)) < 0) goto out;
    sig_input = 1UL << s1; sig_wake = 1UL << s2;
    idle_pace_init(&pace, 200000, 1000000);
    sp_load(&cur, NULL);
    if (!cur.speaker) goto out;
    board = sp_board();
    if (board) seen_seq = sp_board_seq(board);
    if (!(port = CreateMsgPort())) goto out;
    nb.nb_Port = port;
    if (!(broker = CxBroker(&nb, NULL))) goto out;
    port->mp_Node.ln_Name = (char *)SP_SPEAKER_PORT;       /* Sound prefs finds this task by it */
    AddPort(port);
    Forbid();
    if (!FindPort((STRPTR)SP_OLD_PORT) && (old_port = CreateMsgPort())) {  /* 0.1.8: the ROM's OpenBar finds it */
        old_port->mp_Node.ln_Name = (char *)SP_OLD_PORT;
        old_port->mp_Node.ln_Pri = -128;
        AddPort(old_port);
    }
    Permit();
    if ((ts_events = CreateMsgPort())) {     /* 0.2: GFOpenTitle's events come here */
        ts_events->mp_Node.ln_Name = (char *)TS_EVENTS;
        AddPort(ts_events);
    }
    if (!(cust = CxCustom(custom, 0))) goto out;
    AttachCxObj(broker, cust);
    add_hotkey(broker, port);                /* 0.3 */
    if ((tport = CreateMsgPort()) && (tr = (struct timerequest *)CreateIORequest(tport, sizeof *tr)))
        timer = !OpenDevice((STRPTR)TIMERNAME, UNIT_VBLANK, (struct IORequest *)tr, 0);
    if (!timer || !open_speaker()) goto out;
    tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 0; tr->tr_time.tv_micro = 200000;
    SendIO((struct IORequest *)tr);
    ActivateCxObj(broker, 1);
    if (opts.popup) pop_open();              /* 0.3: CX_POPUP=YES */
    while (!quit) {
        ULONG got = Wait((1UL << port->mp_SigBit) | (1UL << tport->mp_SigBit) | sig_input | sig_wake | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F |
                         (spk ? 1UL << spk->UserPort->mp_SigBit : 0) | (pop ? 1UL << pop->UserPort->mp_SigBit : 0) | wbc_sigmask() |
                         (ts_events ? 1UL << ts_events->mp_SigBit : 0));
        CxMsg *cm;
        if (got & SIGBREAKF_CTRL_C) quit = 1;
        /* Workbench is shutting down (Font prefs' Use, a screen mode): no window, no lock on its screen until it is back */
        switch (wbc_take()) {
        case WBC_CLOSE: if (pop) pop_close(0); close_speaker(); wb_down = 1; wbc_closed(); break;
        /* 0.1.4: only when the speaker is down; a refused CloseWorkBench and the OpenWorkBench after it
         * say "open" twice, and the second opened a second speaker */
        case WBC_OPEN:  if (wb_down) wb_down = 2; break;
        }
        if (wb_down == 2 && !disabled && open_speaker()) wb_down = 0;
        if (got & SIGBREAKF_CTRL_F) {                       /* Sound prefs changed the settings */
            sp_load(&cur, NULL);
            if (!cur.speaker) quit = 1;
            else draw_speaker();
        }
        while ((cm = (CxMsg *)GetMsg(port))) {
            ULONG type = CxMsgType(cm), id = CxMsgID(cm);
            ReplyMsg((struct Message *)cm);
            if (type == CXM_COMMAND && id == CXCMD_KILL) quit = 1;
            else if (type == CXM_COMMAND && id == CXCMD_UNIQUE) { sp_load(&cur, NULL); draw_speaker(); }
            /* 0.3: Exchange's other commands */
            else if (type == CXM_COMMAND && id == CXCMD_APPEAR) { if (!pop && !disabled && !wb_down) pop_open(); }
            else if (type == CXM_COMMAND && id == CXCMD_DISAPPEAR) { if (pop) pop_close(0); }
            else if (type == CXM_COMMAND && id == CXCMD_DISABLE && !disabled) {
                ActivateCxObj(broker, 0);
                disabled = 1;
                if (pop) pop_close(0);
                if (spk) tray(0);
                close_speaker();             /* off the bar: our window or our item goes */
            }
            else if (type == CXM_COMMAND && id == CXCMD_ENABLE && disabled) {
                ActivateCxObj(broker, 1);
                disabled = 0;
                if (!wb_down && !open_speaker()) wb_down = 2;
            }
            else if (type == CXM_IEVENT && id == CX_ID_MUTE && !disabled) {   /* the MUTE hotkey */
                cur.muted = !cur.muted;
                levels_out(1);
                save_in = 2000000;
                if (pop) pop_show();
                draw_speaker();
            }
        }
        if (spk) {
            struct IntuiMessage *m;
            while ((m = (struct IntuiMessage *)GetMsg(spk->UserPort))) {
                if (m->Class == IDCMP_REFRESHWINDOW) { BeginRefresh(spk); draw_speaker(); EndRefresh(spk, TRUE); }
                ReplyMsg((struct Message *)m);
            }
        }
        if (pop) pop_events();
        bar_events();
        if (got & sig_input) {
            WORD steps;
            Disable(); steps = wheel_steps; wheel_steps = 0; Enable();
            if (want_mute) { want_mute = 0; cur.muted = !cur.muted; levels_out(1); save_in = 2000000; }
            if (steps) {
                int v = cur.volume + steps * cur.wheel;
                cur.volume = v < 0 ? 0 : v > 100 ? 100 : v;
                if (steps > 0) cur.muted = 0;               /* turning it up unmutes, as a knob does */
                levels_out(!board);
                save_in = 2000000;                          /* kept when the wheel has been still for 2 s */
            }
            if (want_close) { want_close = 0; if (pop) pop_close(0); }
            if (want_toggle) { want_toggle = 0; if (pop) pop_close(1); else pop_open(); }
            if (pop) pop_show();
            draw_speaker();
        }
        /* the timer; input while it is out for the idle 1 s: cut short, and ticking at full speed from now */
        due = CheckIO((struct IORequest *)tr) != NULL;
        if (!due && (got & sig_wake) && pace.slow) { AbortIO((struct IORequest *)tr); due = 2; }
        if (due) {
            ULONG elapsed = due == 2 ? 0 : asked, before_us = clock_us;
            WaitIO((struct IORequest *)tr);
            clock_us = (clock_us + elapsed) % 4000000;          /* elapsed is at most 1 s */
            /* 0.1.3: the bar changed height (GFOpenLook makes it taller as it starts): the speaker made again for it */
            if ((spk || in_bar) && !wb_down && scr->BarHeight != made_bh) {
                if (pop) pop_close(0);
                close_speaker();
                if (!open_speaker()) wb_down = 2;    /* tried again as Workbench's return is */
            }
            if (levels_in()) { draw_speaker(); if (pop) pop_show(); }
            if (save_in) { save_in = elapsed < save_in ? save_in - elapsed : 0; if (!save_in) sp_save_levels(&cur, 1); }
            if (clock_us / 2000000 != before_us / 2000000 && !pop && !wb_down && !disabled) {   /* every 2 s: GFOpenMenus' bar moved? */
                int e = read_bar_edge();
                if (e != bar_edge) { close_speaker(); open_speaker(); }
                else if (spk && ts_events && bar_edge == OM_BAR_TITLE && bar_owner_here()) {   /* 0.2: GFOpenTitle came: into its row */
                    close_speaker(); tray(0);
                    if (!open_speaker()) wb_down = 2;
                }
                else if (in_bar && bar_owner_here() != in_bar) {   /* GFOpenTitle went without a word (0.3: or the row is
                                                                    * now another's): our own window, or the new row */
                    close_speaker();                 /* out of the old row first (a gone port's send does nothing) */
                    if (!open_speaker()) wb_down = 2;
                }
                else if (spk) {                     /* the clock came or went, or changed its width */
                    WORD ox = sx, oy = sy;
                    place();
                    if (ox != sx || oy != sy) { ChangeWindowBox(spk, sx, sy, sw, sh); draw_speaker(); }
                }
            }
            /* the theme changed the bar (every 2 s, a second after the bar's check) */
            if (spk && (clock_us + 1000000) % 4000000 / 2000000 != (before_us + 1000000) % 4000000 / 2000000 &&
                bp_sample(scr, bar_edge == OM_BAR_TITLE, speaker_ours)) draw_speaker();
            Forbid();                                 /* the input handler marks input between our reads otherwise */
            asked = idle_pace_tick(&pace, elapsed);
            Permit();
            tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 0; tr->tr_time.tv_micro = asked;
            SendIO((struct IORequest *)tr);
        }
    }
out:
    if (broker) DeleteCxObjAll(broker);
    if (pop) pop_close(0);
    if (save_in) sp_save_levels(&cur, 1);
    wbc_stop();
    if (spk) { close_speaker(); tray(0); }
    if (in_bar) close_speaker();             /* 0.2: our item out of GFOpenTitle's row */
    if (ts_events) {
        struct Message *m;
        RemPort(ts_events);
        while ((m = GetMsg(ts_events))) {          /* the library's events are given back, never replied (Marcus) */
            struct SysBarEvent *b = (struct SysBarEvent *)m;
            if (m->mn_Length == sizeof *b && b->sbe_Magic == SBE_MAGIC) SB_DoneEvent(b);
            else ReplyMsg(m);
        }
        DeleteMsgPort(ts_events);
    }
    if (timer) { if (!CheckIO((struct IORequest *)tr)) AbortIO((struct IORequest *)tr); WaitIO((struct IORequest *)tr); CloseDevice((struct IORequest *)tr); }
    if (tr) DeleteIORequest((struct IORequest *)tr);
    if (tport) DeleteMsgPort(tport);
    if (old_port) {
        struct Message *m;
        RemPort(old_port);
        while ((m = GetMsg(old_port))) ReplyMsg(m);
        DeleteMsgPort(old_port);
    }
    if (port) {
        struct Message *m;
        if (port->mp_Node.ln_Name) RemPort(port);
        while ((m = GetMsg(port))) ReplyMsg(m);
        DeleteMsgPort(port);
    }
    if (s2 >= 0) FreeSignal(s2);
    if (s1 >= 0) FreeSignal(s1);
    if (SysBarBase) CloseLibrary(SysBarBase);   /* 0.3: after close_speaker took our item out */
    if (IconBase) CloseLibrary(IconBase);
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (CxBase) CloseLibrary(CxBase);
    if (LayersBase) CloseLibrary(LayersBase);
    if (GadToolsBase) CloseLibrary(GadToolsBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    return RETURN_OK;
}
