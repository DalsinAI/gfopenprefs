/* GFOpenTitle 0.1: the Workbench screen's title bar as Dale drew it on
 * 6 October 2026: AmiCradle's logo at the far left, Workbench's title with
 * the free chip and fast memory after it, and the day, date and time at the
 * right end, just left of GFOpenSpeaker. Optionally the display's border
 * around the screen is black.
 *
 * 0.5 (8 October 2026): a cog at the bar's far end, right of GFOpenSpeaker and
 * left of the screen's depth gadget. A click lists the Open settings editors
 * that are installed, and "All Prefs..." (the SYS:Prefs drawer); choosing
 * one starts it as a double-click on its icon does.
 *
 * The free memory is Workbench's own: its title format (the WBTF chunk of
 * Sys/Workbench.prefs, as Prefs/Workbench writes it) becomes
 *   "GFOpenBench 0.1 - Chip %mcek KB, Fast %mfek KB, GPU = 1, CPU = 1 (m68k) + 8 (m68k)"
 * (0.6.9; both in whole KB, as the Amiga always showed them: the user, 10
 * October 2026; the earlier Workbench ones are still known as ours and replaced)
 * in ENV:, so IPrefs shows it and it updates as Workbench does; spaces
 * before it leave room for the logo.
 *
 * The logo and the clock are two small borderless windows in the title bar,
 * as GFOpenSpeaker is. The clock is in GFOpenMenus' tray (ENV:GFOpenMenus/Tray/Clock,
 * "width order", order 10: left of the speaker, order 0), and goes where
 * GFOpenMenus' bar is when GFOpenMenus puts it at an edge. A click on either
 * leaves the active window active.
 *
 *   GFOpenTitle        (started at boot; Ctrl-C quits, Ctrl-F reads
 *                     ENV:GFOpenPrefs/TitleBar again; since 0.7 the cog
 *                     stays, so it no longer quits when nothing else is on)
 *
 * 0.6 (8 October 2026): the taskspace. Other programs put icons in the row
 * beside the cog, the speaker and the network icons through GFOpenTitle's
 * port (include/taskspace.h); GFOpenTitle draws them, lights them under the
 * pointer, shows their help under them and tells the program of a click.
 * Tata is the first.
 *
 * 0.6.13 (11 October 2026): the cog asks GFOpenBench first (Common/obmsg.h), so an editor built into GFOpenBench
 *   runs there; Workbench's OpenWorkbenchObject, then Run, when GFOpenBench isn't running.
 * 0.6.12 (11 October 2026): the title's room for the logo is worked out from the logo's width and the bar's
 *   font (Common/barlogo.h, shared with GFOpenBench), and made again when the bar changes height: the bigger
 *   logo covered the title's first letter. The cores say what they run: "+ 8 (m68k)", "+ 8 (m68k, x86-64)".
 * 0.6.10 (11 October 2026): the counts read "GPU = 1, CPU = 1 (m68k)", as the user put them.
 * 0.6.6 (10 October 2026): the editor the cog starts opens at the screen's top right, 20 pixels in (with
 *   GFOpenWindows 0.8.1); Network (GFOpenSocketControl) in the cog's list.
 * 0.6.5 (10 October 2026): the cog's list has Date and time (GFOpenDateTime)
 *   and Printers (GFOpenPrint's Printers and Queue), each when it is there.
 * 0.6.4 (10 October 2026): a double-click on the date and time opens
 * GFOpenDateTime, the date, time and region editor (the OS's Time editor until
 * it is installed).
 *
 * 0.6.3 (10 October 2026): fast memory in whole KB like chip memory, as the
 * Amiga always showed it, and the processors after it: " - 1 CPU + 8 cores"
 * (gfopenmulticore.library's board), or " - 1 CPU".
 *
 * 0.6.2 (10 October 2026): the bar's icons sit under windows that overlap the
 * bar (they are windows of their own and are no longer raised over the
 * others), are painted over the bar's own gradient, not a white box (the
 * colours are read from the screen's bar, Common/barpaint.h), and are
 * centred in the bar.
 *
 * 0.7.5 (25 October 2026, the user, 24 October, 21:25: "make the title bar translucent until moused over then it goes
 *   its blue colour"): the see-through bar, as far as GFOpenTitle can take it. Where the screen can (composited by
 *   GFOpenLayers, 32-bit, something behind the bar to see, and Intuition taking the bar's opacity: TB_ICTRL_BAROPACITY,
 *   proposed to Team 4, not in Intuition yet), the bar's background rests at seethrough.opacity (40%) and goes solid
 *   while the pointer is on the bar or a menu is open (tb_fade.h: 150 ms in, 300 ms out after 300 ms); GFOpenTitle's
 *   own bar windows keep their glyphs opaque and let the bar show round them. Elsewhere, and everywhere until
 *   Intuition takes it, the bar is solid as before. The seethrough line of ENV:GFOpenPrefs/TitleBar switches it.
 * 0.7.4 (25 October 2026, the user, 00:12: "perhaps m68k/x64 since its running on" one): the cores read
 *   "+ 10 (m68k/x64)", 68k code on the host's x86-64 cores, from the board's ISAS (Common/barlogo.h).
 * 0.7.3 (24 October 2026, the user, 21:39: "improve spacing on the sys icon tray"): every tray icon has the same
 *   room each side (TRAY_PAD, 6 pixels), so the gaps between the clock, the network, the taskspace's icons, the
 *   speaker and the cog are even; on the screen's title bar the row ends at the screen's depth gadget (its own
 *   width and 2 pixels, bar_end()), not 2 bar heights from the edge.
 * 0.7 (17 October 2026, Dale, 08:26: "The cog should be present on all standard screens so Workbench screen
 *   has it all the time"): the cog is always on the Workbench screen's bar (the "cog on|off" line of
 *   ENV:GFOpenPrefs/TitleBar is kept but read no more), and its list is GFOpenPrefs's sections, each running
 *   GFOpenPrefs SECTION (SYS:Preferences/GFOpenPrefs, else SYS:Prefs/GFOpenPrefs), then Printers, Network, GFOpenUp Setup
 *   and Tata when they are there, and All Prefs.... The cog alone is also on every other public screen that
 *   shows its title bar (in the public screen list, not private, SHOWTITLE), as a visitor window while the
 *   screen has a window of its own, so its owner can close it; its list opens GFOpenPrefs on that screen
 *   (PUBSCREEN=). Never on a custom screen that isn't public (a game's), nor on one with no title bar.
 *
 * 0.7.6 (25 October 2026, Team 4's bar-opacity agent): the check that GFOpenLayers has ComposeLayerInfo asked for
 *   a 264-byte table (lib_NegSize); 47.106's is 258, so the see-through bar was never tried. It asks for 258.
 * 0.7.7 (27 October 2026, Dale, 07:39 Cyprus): a pointer that hasn't moved since GFOpenTitle started doesn't count as on
 *   the bar (tb_fade.h, tbp_on_bar). After a boot the pointer sits at the screen's top left corner, on the bar, so the
 *   see-through bar stayed solid until someone moved the mouse (Instance-32, 26 October). The menus, the cog's list
 *   and the menu button still make it solid as before.
 *
 * 0.7.8 (3 November 2026, Dale: "re-write the screen call to allow passing an optional icon for the screen"): where
 *   Intuition takes SA_TitleIcon (GFOpenIntuition, IntuitionControlA on the Workbench screen) the logo is the screen's
 *   own, drawn by Intuition as part of the bar: no logo window, no spaces before the title, and nothing to cover, race
 *   or leave behind when another window's title is shown (a drawer's menu turned the logo into a white block on the
 *   see-through bar). The logo window stays for an Intuition that doesn't take it. Cleared again as GFOpenTitle quits.
 *
 * 0.7.9 (4 November 2026, Dale, 20:50: "the title-bar clock shows Wed 4 Nov twice", "white blobs"): GFOpenTitle also
 *   holds a public port named "OpenTitle", the name from before the GF rename, which the ROM's own title-bar items
 *   (OpenKick's OpenBar) look for to step aside. With only "GFOpenTitle" there, OpenBar drew its clock, network,
 *   speaker and cog a second time over the bar, in a white box.
 *
 * 0.7.10 (11 November 2026, Dale, 17:56: "look at the desktop, particularly the screen depth gadget and the speaker next
 *   to it ... they appear 'stuck on'. Make them seem more integrated, with perhaps a wee bit more space. Also we need a
 *   reboot / power cycle option, and a safe eject for USB"): every tray icon on one grid (Common/traygrid.h: a 16-pixel
 *   box with 7 pixels each side, so the gaps are even and a little wider; the two network icons a cell each, where
 *   the gap between them was half the others). The cog's list ends with an "Eject" row for each USB stick mounted
 *   (gfopenusb's C:USBEject, then "safe to remove" in plain words) and the power menu: Reboot (warm, ColdReboot),
 *   Restart the machine... and Power off... (asked first, then accontrol.device 1.5's ACCMD_POWER to the host).
 *
 * 0.8 (12 November 2026, Dale, 00:57-01:01: GFOpenTitle owns the system bar; every commodity is shown; the time in the
 *   centre of the screen title, the right end for the icons and the screen's depth gadget): the cog, the network
 *   icons and every program's icon are drawn in ONE window, the row (no item has a window of its own over the bar any
 *   more, which is what made them look stuck on). Each item draws into a bitmap of its own, as it drew into its
 *   window before, and the bitmap is copied into the row; a click on the row goes to the item under the pointer. On
 *   the screen's title bar the clock is a window of its own in the bar's centre. TitleBar/docs/system-bar.md.
 *
 * 0.8.1 (12 November 2026, Dale, 15:01: "the fonts are still a bit yuk"): the clock, the cog's and the tray's menus,
 *   the help line and the panel draw their text anti-aliased through GFOpenLayout 19's OLUI_AmigaText, from the
 *   screen font's outline face at its height (sans-serif when it has none); Text() as before where it can't
 *   (Common/aatext.h). Boxes take the wider of the two widths.
 *
 * 0.9 (12 November 2026, CM4 item 17, Dale, 17:01: "lets make sure the new screen title and commodities handling makes
 *   it in"): the bar's right end is, left to right, the memory and the time, the systray, then Prefs and Power, then the
 *   screen's depth gadget. The memory reads "3 GB RAM (0.3 MB in use)" and the time "Nov 12 1059" (tb_format.h); both
 *   are in the row now, no longer in the title (which keeps "GFOpenBench 0.1" and the processors) or the centre. The
 *   systray replaces Exchange: an item for each commodity running (commodities.library's CopyBrokerList, read every
 *   2 s), but those that have taskspace icons of their own; a click shows or hides its window, the menu button gives
 *   Show, Hide, Active, Information and Remove (BrokerCommand), and the pointer resting on it gives its title and
 *   description. The cog is Prefs alone, with "All commodities..." listing every broker; the power menu (the USB
 *   sticks' Eject, Reboot, Restart, Power off) is an icon of its own beside it. The systray and the Prefs and Power
 *   pair each sit on a shaded band of the bar's own colour, and an open item is a deeper shade, not the fill pen.
 *
 * 0.10 (14 November 2026, Dale, 03:08: "go ahead" on designs/SYSTEM-BAR-LIBRARY-20261114.md): GFOpenTitle owns
 *   gfopensysbar.library's bar (SB_Own) when the library is there, and shows its picture items in the row beside the
 *   taskspace's icons, drawn the same way: their help under them, their state, a click and their place sent back to
 *   the item's port (SBE_CLICK, SBE_PLACE). The items are the library's, so they are there again when GFOpenTitle
 *   starts again. Text items and items' menus come in a later step. The taskspace port works as before.
 *
 * 0.11 (14 November 2026, Dale, 03:44: "make sure the new gfopensystray is that CX Exchange replacement. it still
 *   looks like the network, sound aren't commodities"): the systray is where commodities live (src/tb_owner.h).
 *   The menu button on a picture item (GFOpenSpeaker's, GFOpenNetwork's, any program's) opens its program's
 *   broker's menu, as on a letter: Show, Hide, Active, Information, Remove. A disabled broker leaves the strip, as a
 *   disabled GFOpenSpeaker or GFOpenNetwork takes its own icons away; every broker, ours and third-party, active or
 *   not, stays in the cog's "All commodities..." to enable again. commodities.library is untouched, so Exchange
 *   lists the same brokers and its messages do the same.
 * 0.11.1 (Marcus's review of 0.11): the menu button is taken over a picture item only when its program has a broker;
 *   on any other it goes on to the screen's menus.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <exec/errors.h>
#include <exec/execbase.h>
#include <libraries/commodities.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <graphics/videocontrol.h>
#include <graphics/layers.h>
#include <utility/date.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <libraries/configvars.h>
#include <libraries/expansionbase.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/utility.h>
#include <proto/bsdsocket.h>
#include <exec/io.h>
#include <dos/dostags.h>
#include <devices/timer.h>
#include <dos/notify.h>
#include <workbench/workbench.h>
#include <proto/wb.h>
#include <devices/inputevent.h>
#include <proto/input.h>
#include <proto/commodities.h>
#include <proto/gfopensysbar.h>              /* 0.10: SysBar/include */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tb_prefs.h"
#include "tb_fade.h"
#include "tb_format.h"
#include "tb_shapes.h"
#include "tb_sbsync.h"                     /* 0.10 */
#include "tb_owner.h"                      /* 0.11 */
#include <gfopenlayout_draw.h>
#include "om_prefs.h"
#include "logo.h"
#include "logo_big.h"
#include "taskspace.h"
#include "aatext.h"
#include "barpaint.h"
#include "wbclose.h"
#include "popuptheme.h"
#include "cogplace.h"
#include "barlogo.h"
#include "obmsg.h"
#include "../../Common/traygrid.h"     /* 0.7.10: every tray icon in a TRAY_ICON box, TRAY_PAD each side */
#include "../../Common/prefsapp.h"           /* where GFOpenPrefs is (SYS:Preferences) */

const char version[] __attribute__((used)) = "$VER: GFOpenTitle 0.11.1 (14.11.2026) GFOpenPrefs, AmiCradle Team";

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *LayersBase, *UtilityBase, *SocketBase, *WorkbenchBase, *CyberGfxBase, *CxBase;
struct Library *SysBarBase;                     /* 0.10: gfopensysbar.library, when it is there */

#define MENUS_ENV  "ENV:GFOpenMenus/Menus"
#define TRAY_DIR   "ENV:GFOpenMenus/Tray"
#define CLOCK_ORDER 10                         /* left of the network icons (5) and GFOpenSpeaker (0) */
#define NET_ORDER   5
#define COG_ORDER   (-5)                       /* the far end: right of GFOpenSpeaker (0), left of the depth gadget */
#define BAND_SHADE 36                          /* 0.9: the bands: 256ths of the way from the bar's colour to white (or black) */
#define BAND_HOT   84                          /* an item whose menu is open */
#define CX_ORDER    3                          /* 0.9: the commodities, between the network icons and GFOpenSpeaker */
#define NET_TOOL   "SYS:Tools/Commodities/GFOpenSocketControl"
#define WIFI_DEVICE "gfopensocketwifi.device"    /* GFOpenSocket's: the PC's (or the card's) Wi-Fi */
#define WIFI_STATUS 2
#define WB_ENV     "ENV:Sys/Workbench.prefs"
/* 0.6.9: the screen says what it is (the user, 10 October 2026: "GFOpenBench x.x - ChipMem, FastMem,
 * GPU count, CPU count"); the GPUs and CPUs follow, written once, as the format has no tokens for them */
#define OPENBENCH_VERSION "0.1"
#define TITLE_MEM  "GFOpenBench " OPENBENCH_VERSION " - Chip %mcek KB, Fast %mfek KB"
#define TITLE_PLAIN "GFOpenBench " OPENBENCH_VERSION
#define TITLE_MEM_068 "Workbench %r  Chip %mcek KB  Fast %mfek KB"   /* GFOpenTitle 0.6.3 to 0.6.8: still ours */
#define TITLE_MEM_062 "Workbench %r  Chip %mcek KB  Fast %mfem MB"   /* GFOpenTitle 0.6.2 and before */
#define TITLE_PLAIN_068 "Amiga Workbench %r"
#define LOGO_PAD   "         "                 /* a title with room for the logo starts with these (0.6.12: and as many
                                                * more as the logo needs, Common/barlogo.h) */

static tb_prefs prefs;
static struct Screen *scr;
static int icon_in_bar;                        /* 0.7.8: Intuition draws the logo now (SA_TitleIcon) */
static struct DrawInfo *dri;
static struct Window *logo_w, *clock_w;
static WORD cx, cy, cw, ch;                    /* the clock window's place */
static int bar_edge = OM_BAR_TITLE;
static int border_set;                         /* we made the border black */
static struct Window *last_other;              /* the window that was active before a click on ours */
static char shown[132];
static int wb_down;                            /* Workbench is shutting down or shut: 1 our windows are closed, 2 to open them again */

/* the network icons: LAN and Wi-Fi, from GFOpenSocket */
static struct Window *net_w, *pop_w;
static WORD nx, ny, nw, nh;
static int lan_state;                          /* 0 no bsdsocket.library, 1 no address, 2 connected */
static int wifi_state = -1;                    /* -1 no Wi-Fi to show, 0 not connected, 1 connected */
static int wifi_signal, pop_ticks;
static char lan_addr[20], wifi_ssid[34];
static int net_drawn = -100;                   /* what the icons show, to draw only on change */

/* the cog and its list of settings editors */
static struct Window *cog_w, *menu_w;
static WORD gx, gy, gw, gh;
static int cog_drawn = -1;                     /* 0 plain, 1 the pointer over it, 2 its list open */
static int cog_skip;                           /* the click that closed the list was on the cog: don't open it again */
static int menu_hot = -1;

static LONG pens[64];
static ULONG pen_rgb[64];
static int npens;

/* ---- files --------------------------------------------------------------------------------- */

static char *read_file(const char *path, LONG *lenp)
{
    BPTR fh = Open((STRPTR)path, MODE_OLDFILE);
    char *buf = NULL;
    LONG len = 0;
    if (fh) {
        Seek(fh, 0, OFFSET_END);
        len = Seek(fh, 0, OFFSET_BEGINNING);
        if (len >= 0 && (buf = AllocVec(len + 1, MEMF_ANY | MEMF_CLEAR))) {
            if (Read(fh, buf, len) != len) { FreeVec(buf); buf = NULL; }
        }
        Close(fh);
    }
    if (lenp) *lenp = buf ? len : 0;
    return buf;
}

static void read_prefs(void)
{
    char *t = read_file(TB_ENV, NULL);
    if (!t) t = read_file(TB_ENVARC, NULL);
    tb_parse(&prefs, t);
    if (t) FreeVec(t);
}

/* ---- the tray and GFOpenMenus' settings, read when they change (0.7.2) --------------------------
 * Every 2 s GFOpenTitle places its windows, and each place read ENV:GFOpenMenus/Tray afresh, every file
 * in it, four times a pass, and ENV:GFOpenMenus/Menus once (Team 4's GFOpenMonitor Calls view, 24 Oct
 * 2026). Now both are read once and again only when a DOS notification says they changed, or when
 * GFOpenTitle changes the tray itself. Without the notifications they are read every time, as before. */
#define TRAY_MAX 24
static struct { char name[32]; int w, order; } tray_tab[TRAY_MAX];
static int tray_n, tray_ok, edge_ok, edge_bar;
static struct NotifyRequest tray_nr, menus_nr;
static BYTE watch_sig = -1;
static int tray_watched, menus_watched;

static void watch_start(void)
{
    struct Task *me = FindTask(NULL);
    if ((watch_sig = AllocSignal(-1)) < 0) return;
    tray_nr.nr_Name = (STRPTR)TRAY_DIR;
    tray_nr.nr_Flags = NRF_SEND_SIGNAL;
    tray_nr.nr_stuff.nr_Signal.nr_Task = me;
    tray_nr.nr_stuff.nr_Signal.nr_SignalNum = watch_sig;
    tray_watched = StartNotify(&tray_nr) != 0;
    menus_nr.nr_Name = (STRPTR)MENUS_ENV;
    menus_nr.nr_Flags = NRF_SEND_SIGNAL;
    menus_nr.nr_stuff.nr_Signal.nr_Task = me;
    menus_nr.nr_stuff.nr_Signal.nr_SignalNum = watch_sig;
    menus_watched = StartNotify(&menus_nr) != 0;
}

static void watch_stop(void)
{
    if (tray_watched) EndNotify(&tray_nr);
    if (menus_watched) EndNotify(&menus_nr);
    tray_watched = menus_watched = 0;
    if (watch_sig >= 0) FreeSignal(watch_sig);
    watch_sig = -1;
}

static ULONG watch_mask(void) { return watch_sig >= 0 ? 1UL << watch_sig : 0; }

/* a notification came: what it covers is read again when next asked for */
static void watch_changed(void) { tray_ok = 0; edge_ok = 0; }

static int read_bar_edge(void)
{
    struct MsgPort *port;
    Forbid();
    port = FindPort((STRPTR)"GFOpenMenus");
    Permit();
    if (!port) return OM_BAR_TITLE;
    if (!edge_ok || !menus_watched) {
        om_prefs p;
        char *t;
        om_defaults(&p);
        if ((t = read_file(MENUS_ENV, NULL))) { om_parse(&p, t); FreeVec(t); }
        edge_bar = p.enabled ? p.bar : OM_BAR_TITLE;
        edge_ok = 1;
    }
    return edge_bar;
}

static void tell_openmenus(void)
{
    struct MsgPort *p;
    Forbid();
    if ((p = FindPort((STRPTR)"GFOpenMenus")) && p->mp_SigTask) Signal((struct Task *)p->mp_SigTask, SIGBREAKF_CTRL_F);
    Permit();
}

static void tray(const char *name, int width, int order)
{
    BPTR fh, lock;
    char t[24], path[64];
    if (width > 0) {
        if ((lock = Lock((STRPTR)TRAY_DIR, ACCESS_READ))) UnLock(lock);
        else {                                 /* ENV:GFOpenMenus is there only once GFOpenMenus has settings */
            if ((lock = CreateDir((STRPTR)"ENV:GFOpenMenus"))) UnLock(lock);
            if ((lock = CreateDir((STRPTR)TRAY_DIR))) UnLock(lock);
        }
        snprintf(path, sizeof path, TRAY_DIR "/%s", name);
        if ((fh = Open((STRPTR)path, MODE_NEWFILE))) {
            LONG n = snprintf(t, sizeof t, "%d %d\n", width, order);
            Write(fh, t, n);
            Close(fh);
        }
    } else {
        snprintf(path, sizeof path, TRAY_DIR "/%s", name);
        DeleteFile((STRPTR)path);
    }
    tray_ok = 0;
    tell_openmenus();
}

/* the tray's files, "width order" each, into tray_tab (0.7.2: once, and again when they change) */
static void tray_load(void)
{
    struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, NULL);
    BPTR lock = Lock((STRPTR)TRAY_DIR, ACCESS_READ);
    tray_n = 0;
    if (fib && lock && Examine(lock, fib)) {
        while (ExNext(lock, fib) && tray_n < TRAY_MAX) {
            char path[sizeof TRAY_DIR "/" + sizeof fib->fib_FileName], *t;
            int w = 0, order = 0;
            const char *name = (const char *)fib->fib_FileName;
            if (fib->fib_DirEntryType > 0 || strlen(name) >= sizeof tray_tab[0].name) continue;
            snprintf(path, sizeof path, TRAY_DIR "/%s", name);
            if (!(t = read_file(path, NULL))) continue;
            if (sscanf(t, "%d %d", &w, &order) < 1) w = 0;
            FreeVec(t);
            if (w <= 0 || w > 400) continue;
            strcpy(tray_tab[tray_n].name, name);
            tray_tab[tray_n].w = w;
            tray_tab[tray_n].order = order;
            tray_n++;
        }
    }
    if (lock) UnLock(lock);
    if (fib) FreeDosObject(DOS_FIB, fib);
    tray_ok = 1;
}

/* the room the tray programs nearer the bar's end take, and how many they are */
static int tray_before(const char *mine, int my_order, int *count)
{
    int i, sum = 0;
    *count = 0;
    if (!tray_ok || !tray_watched) tray_load();
    for (i = 0; i < tray_n; i++) {
        const char *name = tray_tab[i].name;
        int order = tray_tab[i].order;
        if (!strcmp(name, mine)) continue;
        if (order < my_order || (order == my_order && strcmp(name, mine) < 0)) { sum += tray_tab[i].w; (*count)++; }
    }
    return sum;
}

/* ---- Workbench's title format (the WBTF chunk of ENV:Sys/Workbench.prefs) ------------------ */

static ULONG get32(const UBYTE *p) { return ((ULONG)p[0] << 24) | ((ULONG)p[1] << 16) | ((ULONG)p[2] << 8) | p[3]; }
static void put32(UBYTE *p, ULONG v) { p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v; }

/* The title format Workbench.prefs has now (NULL when none) into fmt. */
static int wb_title(char *fmt, int size)
{
    LONG len, i;
    UBYTE *d = (UBYTE *)read_file(WB_ENV, &len);
    int found = 0;
    fmt[0] = 0;
    if (d && len >= 12 && !memcmp(d, "FORM", 4) && !memcmp(d + 8, "PREF", 4)) {
        for (i = 12; i + 8 <= len; ) {
            ULONG n = get32(d + i + 4);
            if (i + 8 + n > (ULONG)len) break;
            if (!memcmp(d + i, "WBTF", 4)) {
                int k = n < (ULONG)size - 1 ? n : size - 1;
                memcpy(fmt, d + i + 8, k);
                fmt[k] = 0;
                found = 1;
            }
            i += 8 + n + (n & 1);
        }
    }
    if (d) FreeVec(d);
    return found;
}

/* Writes ENV:Sys/Workbench.prefs with fmt as its title format (NULL: none), keeping its other chunks. */
static void wb_set_title(const char *fmt)
{
    LONG len, i, o = 12, flen = fmt ? strlen(fmt) + 1 : 0;
    UBYTE *d = (UBYTE *)read_file(WB_ENV, &len), *out;
    BPTR fh;
    if (!d || len < 12 || memcmp(d, "FORM", 4) || memcmp(d + 8, "PREF", 4)) {
        static const UBYTE prhd[14] = { 'P', 'R', 'H', 'D', 0, 0, 0, 6, 0, 0, 0, 0, 0, 0 };
        if (d) FreeVec(d);
        if (!fmt) return;
        len = 12 + 14;
        if (!(d = AllocVec(len, MEMF_CLEAR))) return;
        memcpy(d, "FORM", 4); memcpy(d + 8, "PREF", 4); memcpy(d + 12, prhd, 14);
    }
    if (!(out = AllocVec(len + flen + 16, MEMF_CLEAR))) { FreeVec(d); return; }
    memcpy(out, d, 12);
    for (i = 12; i + 8 <= len; ) {
        ULONG n = get32(d + i + 4), all = 8 + n + (n & 1);
        if (i + all > (ULONG)len) break;
        if (memcmp(d + i, "WBTF", 4)) { memcpy(out + o, d + i, all); o += all; }
        i += all;
    }
    if (fmt) {
        memcpy(out + o, "WBTF", 4);
        put32(out + o + 4, flen);
        memcpy(out + o + 8, fmt, flen);
        o += 8 + flen + (flen & 1);
    }
    put32(out + 4, o - 8);
    if ((fh = Open((STRPTR)WB_ENV, MODE_NEWFILE))) { Write(fh, out, o); Close(fh); }   /* IPrefs is notified and shows it */
    FreeVec(out);
    FreeVec(d);
}

/* How many processors the machine has, after the memory (the user, 10 October 2026: "a - with number of
 * cpus / cores"): the main CPU, and the cores gfopenmulticore.library's board gives (OMC_CoreCount, its first
 * call, -30; 0 without a board or without the library). Written into the title once, at the start and on
 * Ctrl-F, as Workbench's title format has no token for it. */
/* The cores the CPU cores board presents through AutoConfig (the user, 10 October 2026: "the cpu count
 * should read the amount of cores presented via autoconfig"): an AmiCradle Team ($DA15) product 7 board carrying
 * the ACSV block, CLASS ($20) 2, UNITS ($24) its cores. 0 without the board. */
struct ExpansionBase *ExpansionBase;   /* opened for the look, closed after (the type proto/expansion.h declares) */
static ULONG board_isas;                       /* 0.6.12: what the cores run (ISAS, $28; Common/barlogo.h) */
static ULONG board_cores(void)
{
    ULONG n = 0;
    board_isas = 1;
    ExpansionBase = (struct ExpansionBase *)OpenLibrary((STRPTR)"expansion.library", 36);
    if (ExpansionBase) {
        struct ConfigDev *cd = FindConfigDev(NULL, 0xDA15, 7);
        if (cd && cd->cd_BoardAddr) {
            volatile ULONG *r = (volatile ULONG *)cd->cd_BoardAddr;
            if (r[0] == 0x41435356UL && r[0x20 / 4] == 2) { n = r[0x24 / 4]; board_isas = r[0x28 / 4] | 1; }
        }
        CloseLibrary((struct Library *)ExpansionBase);
        ExpansionBase = NULL;
    }
    return n > 1024 ? 0 : n;
}

static ULONG omc_cores(void)
{
    struct Library *omc = OpenLibrary((STRPTR)"gfopenmulticore.library", 0);
    ULONG n = 0;
    if (omc) {
        register ULONG d0 __asm("d0");
        register struct Library *a6 __asm("a6") = omc;
        __asm volatile ("jsr -30(%%a6)" : "=r" (d0) : "r" (a6) : "d1", "a0", "a1", "cc", "memory");
        n = d0;
        CloseLibrary(omc);
    }
    return n > 1024 ? 0 : n;
}

/* The GPUs (0.6.9): AmiCradle's graphics boards, ACRTG (AmiCradle Team $DA15, up to four), counted
 * through AutoConfig as the cores are: product 9, the 16 MiB board, and product 15, the 64 MiB one (0.6.15;
 * before, a 64 MiB machine showed GPU = 0). 0 on a machine without one. */
static ULONG board_gpus(void)
{
    ULONG n = 0;
    ExpansionBase = (struct ExpansionBase *)OpenLibrary((STRPTR)"expansion.library", 36);
    if (ExpansionBase) {
        struct ConfigDev *cd = NULL;
        while ((cd = FindConfigDev(cd, 0xDA15, 9)) && n < 16) n++;
        cd = NULL;
        while ((cd = FindConfigDev(cd, 0xDA15, 15)) && n < 16) n++;
        CloseLibrary((struct Library *)ExpansionBase);
        ExpansionBase = NULL;
    }
    return n;
}

/* ", GPU = 1, CPU = 1 (m68k) + 8 (m68k)": after the memory (0.6.10, the user, 10 October 2026: "GPU = 1,
 * CPU = 1 (m68k)"); the cores only when the board or gfopenmulticore.library gives them */
static void cpus_text(char *buf, int size)
{
    ULONG n = board_cores(), g = board_gpus();
    if (!n) { n = omc_cores(); board_isas = 1; }  /* a provider without the board: 68k cores */
    if (n) snprintf(buf, size, ", GPU = %lu, CPU = 1 (m68k) + %lu (%s)", (unsigned long)g, (unsigned long)n, barlogo_isas(board_isas));
    else snprintf(buf, size, ", GPU = %lu, CPU = 1 (m68k)", (unsigned long)g);
}

static int ours(const char *fmt)
{
    const char *f = fmt;
    size_t m = strlen(TITLE_MEM);
    while (*f == ' ') f++;
    if (!strncmp(f, TITLE_MEM, m) && (!f[m] || !strncmp(f + m, ", ", 2))) return 1;   /* with or without the GPUs and CPUs */
    m = strlen(TITLE_MEM_068);                       /* 0.6.3 to 0.6.8's, replaced by the GFOpenBench one */
    if (!strncmp(f, TITLE_MEM_068, m) && (!f[m] || !strncmp(f + m, " - ", 3))) return 1;
    m = strlen(TITLE_PLAIN);                         /* 0.9's: GFOpenBench's line and the processors */
    if (!strncmp(f, TITLE_PLAIN, m) && !strncmp(f + m, ", GPU", 5)) return 1;
    return !strcmp(f, TITLE_MEM_062) || !strcmp(f, TITLE_PLAIN) || !strcmp(f, TITLE_PLAIN_068);
}

static void apply_title(void)
{
    char now[160], want[160], cpus[48], pad[BARLOGO_MAX_SPACES + 1];
    int have = wb_title(now, sizeof now);
    /* 0.9: the memory is in the row now; the title is GFOpenBench's line and the processors, whatever the memory's
     * switch says (Workbench's own format would show its own memory again) */
    if (have && !ours(now) && now[0]) {
        /* someone's own title format: keep it, only make room for the logo (0.7.8: none needed when Intuition
         * draws it: the room made before goes) */
        if (icon_in_bar) {
            const char *f = now;
            if (strncmp(now, LOGO_PAD, strlen(LOGO_PAD))) return;
            while (*f == ' ') f++;
            snprintf(want, sizeof want, "%s", f);
        } else {
            if (!prefs.logo || !strncmp(now, LOGO_PAD, strlen(LOGO_PAD))) return;
            snprintf(want, sizeof want, "%s%s", barlogo_pad(scr, pad), now);
        }
    } else {
        cpus_text(cpus, sizeof cpus);
        snprintf(want, sizeof want, "%s%s%s", prefs.logo && !icon_in_bar ? barlogo_pad(scr, pad) : "", TITLE_PLAIN, cpus);
    }
    if (!have || strcmp(now, want)) wb_set_title(want);
}

/* ---- the border ------------------------------------------------------------------------------ */

static void apply_border(void)
{
    if (!scr || !scr->ViewPort.ColorMap) return;
    if (prefs.border_black == border_set) return;
    VideoControlTags(scr->ViewPort.ColorMap, prefs.border_black ? VTAG_BORDERBLANK_SET : VTAG_BORDERBLANK_CLR, 0, TAG_DONE);
    border_set = prefs.border_black;
    MakeScreen(scr);
    RethinkDisplay();
}

/* ---- the bar's own colours ------------------------------------------------------------------ */

static int ts_is(struct Window *w);

static int bar_ours(struct Window *o)
{
    return o == logo_w || o == clock_w || o == pop_w || o == menu_w || ts_is(o);   /* ts_is: the row and the help too */
}

/* 0.7.5: the see-through bar's state (the rest of it is before close_all) */
static int see_live;                           /* the bar is see-through now: the screen can, and the setting is on */
static int see_level = 255;                    /* the bar's background opacity as last given to Intuition */
static int see_tray_bg = -1;                   /* the alpha of our bar windows' background: 0, or 255 while solid */

/* 0.7.5: read only while the bar is solid; see-through, the screen shows it mixed with what is behind it, and the
 * colours read last stay (a theme changed then is seen once the bar is solid again) */
static int bar_sample_now(void) { return see_live && see_level != 255 ? 0 : bp_sample(scr, bar_edge == OM_BAR_TITLE, bar_ours); }

/* the pen for text and glyphs on the bar: light on a dark bar, dark on a light one */
static LONG bar_text(void) { return bp_text_pen(scr, dri->dri_Pens[BARDETAILPEN]); }

/* the bar's colour behind a box of a bar window, not the pen's white */
static void bar_paint(struct Window *win, WORD x, WORD y, WORD pw, WORD ph)
{
    bp_paint(win, dri->dri_Pens[BARBLOCKPEN], x, y, pw, ph);
}

/* ---- 0.7.5: our bar windows on the see-through bar ---------------------------------------------
 * Each bar window (the logo, the clock, the network icons, the cog, the taskspace's icons) is drawn as before, in the
 * bar's colours, and then, while the bar is see-through, keyed: a pixel in its row's bar colour (bp_paint's) takes
 * see_tray_bg, any other (text, glyphs, the logo, a lit edge) is opaque. The window's layer has GFOpenLayers'
 * LAYERLOOK_ALPHA, so its own alpha counts. see_tray_bg is 0 while the bar is less than solid: the bar itself
 * shows round the glyphs, at whatever opacity it has; and 255 once it is solid, so the window covers the bar as
 * before (a title long enough to run under the tray stays hidden). Pens and RECTFMT_ARGB leave a GFOpenRTG 32-bit
 * pixel's alpha 0, so the alpha goes in with RECTFMT_RAW, the bitmap's own pixels, which keeps it. */
#ifndef RECTFMT_RAW
#define RECTFMT_RAW 5                          /* CyberGraphX 43: the bitmap's own pixel format */
#endif
#define SEE_LOOK_ALPHA 8UL                     /* gfopengfx include/graphics/openlayers.h: LAYERLOOK_ALPHA */

/* GFOpenLayers 47.101 and up: SetLayerLook(layer, what, value) (-258, gfopengfx include/graphics/openlayers.h); ~0
 * for a classic layer. Only once see_screen() has found GFOpenLayers. A6 saved by hand, as GFOpenWindows does. */
static ULONG layer_look(struct Layer *l, ULONG what, ULONG value)
{
    register struct Library *a1 __asm("a1") = LayersBase;
    register struct Layer *a0 __asm("a0") = l;
    register ULONG d0 __asm("d0") = what, d1 __asm("d1") = value;
    __asm__ volatile("move.l %%a6,-(%%sp)\n\tmove.l %%a1,%%a6\n\tjsr -258(%%a6)\n\tmove.l (%%sp)+,%%a6"
                     : "+r"(d0), "+r"(d1), "+r"(a0), "+r"(a1) : : "cc", "memory");
    return d0;
}

struct slot;
static struct slot *slot_of(struct Window *w);
static void slot_show(struct slot *s);
static void see_key_one(struct Window *w);
static void see_key(struct Window *w)
{
    struct slot *s = slot_of(w);
    see_key_one(w);
    if (s) slot_show(s);
}

static void see_key_one(struct Window *w)
{
    ULONG *px, *p, bga;
    WORD x, y, ww, wh;
    if (!see_live || !w) return;
    ww = w->Width;
    wh = w->Height;
    if (ww <= 0 || wh <= 0 || !(px = AllocVec((ULONG)ww * wh * 4, MEMF_ANY))) return;
    bga = (ULONG)(see_tray_bg & 255) << 24;
    ReadPixelArray(px, 0, 0, ww * 4, w->RPort, 0, 0, ww, wh, RECTFMT_ARGB);
    for (y = 0, p = px; y < wh; y++) {
        ULONG bg = bp_row_kept(w->TopEdge + y, 0) & 0xffffffUL;
        for (x = 0; x < ww; x++, p++) {
            ULONG c = *p & 0xffffffUL;
            *p = c | (c == bg ? bga : 0xff000000UL);
        }
    }
    WritePixelArray(px, 0, 0, ww * 4, w->RPort, 0, 0, ww, wh, RECTFMT_RAW);
    FreeVec(px);
}

/* a bar window just opened: its own alpha counts while the bar is see-through (it is keyed as it is drawn) */
static struct Window *see_new(struct Window *w)
{
    if (w && w->WLayer && see_live) layer_look(w->WLayer, SEE_LOOK_ALPHA, 1);
    return w;
}

/* ---- the logo -------------------------------------------------------------------------------- */

static LONG pen_for(ULONG rgb)
{
    int i;
    for (i = 0; i < npens; i++) if (pen_rgb[i] == rgb) return pens[i];
    if (npens == 64) return dri->dri_Pens[BARDETAILPEN];
    pen_rgb[npens] = rgb;
    pens[npens] = ObtainBestPen(scr->ViewPort.ColorMap, (rgb >> 16) * 0x01010101UL, ((rgb >> 8) & 255) * 0x01010101UL,
                                (rgb & 255) * 0x01010101UL, OBP_Precision, PRECISION_IMAGE, TAG_DONE);
    if (pens[npens] < 0) pens[npens] = dri->dri_Pens[BARDETAILPEN];
    return pens[npens++];
}

static void free_pens(void)
{
    int i;
    for (i = 0; i < npens; i++) if (pens[i] >= 0 && scr) ReleasePen(scr->ViewPort.ColorMap, pens[i]);
    npens = 0;
}

#define LOGO_W 17                              /* the logo's window: the logo is 16 wide, its picture 15 */

/* 0.6.11 (the Team, 11 October 2026: "bigger ... aspect correct but 2 pixels smaller than the title bar and
 * centre aligned"): the logo at the bar's height less 2, its width to match, a pixel's gap above and below,
 * each pixel the average of the ones of logo_big it covers, laid over the bar's own colour by its alpha.
 * Needs cybergraphics (the bar's colours); else the 16 x 16 picture as before. */
#if LOGO_BIG_W != BARLOGO_PIC_W || LOGO_BIG_H != BARLOGO_PIC_H
#error "logo_big.h and Common/barlogo.h disagree on the logo's picture"
#endif
static WORD logo_size(WORD bar_h, WORD *w)
{
    return barlogo_size(bar_h, w);        /* GFOpenBench leaves room for the same (Common/barlogo.h) */
}

static WORD logo_window_w(void)
{
    WORD w;
    if (!CyberGfxBase || !bp_n) return LOGO_W;
    (void)w;
    return barlogo_window_w(scr->BarHeight);
}

static int draw_logo_big(void)
{
    static ULONG row[LOGO_BIG_W + 8];
    struct RastPort *rp = logo_w->RPort;
    WORD th, tw, x, y, top;
    if (!CyberGfxBase || !bp_n) return 0;
    th = logo_size(logo_w->Height, &tw);
    if (tw + 4 > logo_w->Width) return 0;
    top = (logo_w->Height - th) / 2;
    for (y = 0; y < logo_w->Height; y++) {
        WORD sy = logo_w->TopEdge + y;
        ULONG bar = bp_rgb[sy < 0 ? 0 : sy >= bp_n ? bp_n - 1 : sy];
        for (x = 0; x < logo_w->Width; x++) row[x] = bar;
        if (y >= top && y < top + th) {
            LONG y0 = (LONG)(y - top) * LOGO_BIG_H / th, y1 = (LONG)(y - top + 1) * LOGO_BIG_H / th;
            if (y1 <= y0) y1 = y0 + 1;
            for (x = 0; x < tw; x++) {
                LONG x0 = (LONG)x * LOGO_BIG_W / tw, x1 = (LONG)(x + 1) * LOGO_BIG_W / tw, i, j, n = 0;
                ULONG a = 0, r = 0, g = 0, b = 0, br = (bar >> 16) & 255, bg = (bar >> 8) & 255, bb = bar & 255;
                if (x1 <= x0) x1 = x0 + 1;
                for (j = y0; j < y1; j++)
                    for (i = x0; i < x1; i++) {
                        ULONG p = logo_big[j * LOGO_BIG_W + i], pa = p >> 24;
                        a += pa; r += ((p >> 16) & 255) * pa; g += ((p >> 8) & 255) * pa; b += (p & 255) * pa; n++;
                    }
                if (!a) continue;
                /* the average colour (by alpha) over the bar, by the average alpha */
                r /= a; g /= a; b /= a; a /= n;
                r = (r * a + br * (255 - a) + 127) / 255; g = (g * a + bg * (255 - a) + 127) / 255; b = (b * a + bb * (255 - a) + 127) / 255;
                row[2 + x] = 0xff000000UL | (r << 16) | (g << 8) | b;
            }
        }
        WritePixelArray(row, 0, 0, logo_w->Width * 4, rp, 0, y, logo_w->Width, 1, RECTFMT_ARGB);
    }
    return 1;
}

static void draw_logo(void)
{
    struct RastPort *rp;
    WORD x, y, oy;
    if (!logo_w) return;
    if (draw_logo_big()) { see_key(logo_w); return; }
    rp = logo_w->RPort;
    bar_paint(logo_w, 0, 0, logo_w->Width, logo_w->Height);
    /* the picture as it is (16 x 16, rows 4 to 12 used), its used rows centred in the bar's colour rows */
    oy = (logo_w->Height - 9) / 2 - 4;
    for (y = 4; y < 13; y++)
        for (x = 0; x < 16; x++) {
            long c = logo16[y * 16 + x];
            if (c < 0) continue;
            SetAPen(rp, pen_for((ULONG)c));
            WritePixel(rp, 1 + x, oy + y);
        }
    see_key(logo_w);
}

/* ---- the clock ------------------------------------------------------------------------------- */

/* 0.9: every MemHeader's size, and how much of it isn't free: "3 GB RAM (0.3 MB in use)" */
static void memory_text(char *t, int size)
{
    struct MemHeader *mh;
    unsigned long long total = 0, avail;
    Forbid();
    for (mh = (struct MemHeader *)SysBase->MemList.lh_Head; mh->mh_Node.ln_Succ; mh = (struct MemHeader *)mh->mh_Node.ln_Succ)
        total += (ULONG)mh->mh_Upper - (ULONG)mh->mh_Lower;
    Permit();
    avail = AvailMem(MEMF_ANY);
    tb_memory(total, total > avail ? total - avail : 0, t, size);
}

/* 0.9: "Nov 12 1059", or "1059" with the date off (tb_format.h) */
static void time_text(char *t, int size)
{
    struct DateStamp ds;
    struct ClockData cd;
    DateStamp(&ds);
    Amiga2Date(ds.ds_Days * 86400 + ds.ds_Minute * 60 + ds.ds_Tick / TICKS_PER_SECOND, &cd);
    tb_clock(cd.month, cd.mday, cd.hour, cd.min, prefs.date, t, size);
}

#define CLOCK_GAP (3 * TRAY_PAD)               /* between the memory and the time */

/* the memory's words and the time's, each "" when switched off */
static void clock_parts(char *m, char *t, int size)
{
    m[0] = t[0] = 0;
    if (prefs.memory) memory_text(m, size);
    if (prefs.clock) time_text(t, size);
}

/* the room both need: measured on their words with every digit a 0, so it changes only with the digits' count */
static WORD clock_width(void)
{
    static char last[132];
    static WORD last_w;
    struct RastPort rp;
    char m[64], t[64], z[132];
    WORD w = 2 * TRAY_PAD;
    clock_parts(m, t, sizeof m);
    snprintf(z, sizeof z, "%s|%s", m, t);
    tb_zeros(z, z, sizeof z);
    if (last_w && !strcmp(z, last)) return last_w;
    rp = scr->RastPort;                        /* 0.8.1: the screen's, so the outline text is measured as drawn */
    SetFont(&rp, dri->dri_Font);
    tb_zeros(m, m, sizeof m);
    tb_zeros(t, t, sizeof t);
    if (m[0]) w += aat_length(&rp, m, strlen(m));
    if (t[0]) w += aat_length(&rp, t, strlen(t));
    if (m[0] && t[0]) w += CLOCK_GAP;
    strcpy(last, z);
    last_w = w;
    return w;
}

static void place(void);
static void slot_move(struct Window *w, WORD x, WORD y, WORD ww, WORD wh);

static void draw_clock(int force)
{
    struct RastPort *rp;
    char m[64], t[64], both[132];
    WORD h, w;
    if (!clock_w || !slot_of(clock_w)) return;
    clock_parts(m, t, sizeof m);
    snprintf(both, sizeof both, "%s|%s", m, t);
    if (!force && !strcmp(both, shown)) return;
    strcpy(shown, both);
    if (clock_width() != cw) {                 /* more digits: the room grows (the others move at the next place) */
        place();
        slot_move(clock_w, cx, cy, cw, ch);
        tray("Clock", cw, CLOCK_ORDER);
    }
    rp = clock_w->RPort;
    bar_paint(clock_w, 0, 0, cw, ch);
    SetAPen(rp, bar_text());
    SetDrMd(rp, JAM1);
    h = ch - (bar_edge == OM_BAR_TITLE ? 1 : 0);
    if (m[0]) aat_box(rp, scr, TRAY_PAD, 0, h, m, strlen(m));
    if (t[0]) {
        w = aat_length(rp, t, strlen(t));
        aat_box(rp, scr, cw - TRAY_PAD - w, 0, h, t, strlen(t));
    }
    see_key(clock_w);
}

/* 0.7.3: the room each tray icon has on each side, so the gaps between them are even (0.7.10: Common/traygrid.h) */

/* 0.7.3: where the tray's row ends on the screen's title bar: the screen's depth gadget, its own width and
 * 2 pixels (2 bar heights before, which left a wide gap after the cog), or the width the bar draws it at */
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

/* where the clock goes: left of the tray programs nearer the bar's end */
static void place(void)
{
    WORD bh = scr->BarHeight + 1;
    int n, off = tray_before("Clock", CLOCK_ORDER, &n);
    cw = clock_width();
    switch (bar_edge) {
    case OM_BAR_BOTTOM: ch = bh; cx = scr->Width - cw - off; cy = scr->Height - bh; break;
    case OM_BAR_LEFT:   ch = bh; cx = 0; cy = scr->Height - bh * (n + 1); break;
    case OM_BAR_RIGHT:  ch = bh; cx = scr->Width - cw; cy = scr->Height - bh * (n + 1); break;
    case OM_BAR_TOP:    ch = bh; cx = scr->Width - 2 * scr->BarHeight - cw - off; cy = 0; break;
    default:            ch = scr->BarHeight; cx = scr->Width - bar_end() - cw - off; cy = 0; break;   /* 0.9: in the row */
    }
}

/* ---- the windows ----------------------------------------------------------------------------- */

/* a bar window sits in front of the bar and the Workbench backdrop but under every other window:
 * a window that overlaps the bar covers the bar's icons, as it covers the bar's title */
static struct Window *to_back(struct Window *w)
{
    if (w) WindowToBack(w);
    return w;
}

static struct Window *bar_window(WORD x, WORD y, WORD w, WORD h)
{
    return to_back(OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Left, x, WA_Top, y, WA_Width, w, WA_Height, h,
                          WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE,
                          WA_IDCMP, IDCMP_REFRESHWINDOW | IDCMP_ACTIVEWINDOW, TAG_DONE));
}

/* ---- 0.8: the row: one window for every item at the bar's right end ----------------------------
 * An item (the cog, the network icons, a program's icon) is a slot: a bitmap of the item's size, with a RastPort
 * on it, and a Window record that is never opened. The item's code draws through slot.win.RPort and reads the
 * record's place (LeftEdge, TopEdge, Width, Height, on the screen) exactly as it drew into its own window before;
 * see_key() then copies the slot into the row. The row is one borderless window over all the slots' boxes; it is
 * made again when the boxes change. A click on the row is the click of the slot under the pointer. */
#define SLOTS (2 + TS_ICONS)
struct slot { struct Window win; struct RastPort rp; struct BitMap *bm; int on; };
static struct slot slots[SLOTS];
static struct Window *row_w;
static WORD rx, ry, rw, rh;

static struct slot *slot_of(struct Window *w)
{
    int i;
    for (i = 0; i < SLOTS; i++) if (w && w == &slots[i].win && slots[i].on) return &slots[i];
    return NULL;
}

static void slot_show(struct slot *s)
{
    if (row_w && s->on && s->bm)
        BltBitMapRastPort(s->bm, 0, 0, row_w->RPort, s->win.LeftEdge - rx, s->win.TopEdge - ry, s->win.Width, s->win.Height, 0xc0);
}

static void row_redraw(void);
static void bar_paint(struct Window *win, WORD x, WORD y, WORD pw, WORD ph);
static void band(struct Window *win, WORD x, WORD w, WORD h, int shade);
static void see_key(struct Window *w);
static struct Window *see_new(struct Window *w);

/* the row over every slot's box, made again when that changed */
static void row_layout(void)
{
    WORD x0 = 0x7fff, y0 = 0x7fff, x1 = -1, y1 = -1;
    int i;
    for (i = 0; i < SLOTS; i++) {
        struct Window *w = &slots[i].win;
        if (!slots[i].on) continue;
        if (w->LeftEdge < x0) x0 = w->LeftEdge;
        if (w->TopEdge < y0) y0 = w->TopEdge;
        if (w->LeftEdge + w->Width > x1) x1 = w->LeftEdge + w->Width;
        if (w->TopEdge + w->Height > y1) y1 = w->TopEdge + w->Height;
    }
    if (x1 < 0) { if (row_w) { CloseWindow(row_w); row_w = NULL; } return; }
    if (row_w && x0 == rx && y0 == ry && x1 - x0 == rw && y1 - y0 == rh) return;
    if (row_w) { CloseWindow(row_w); row_w = NULL; }
    rx = x0; ry = y0; rw = x1 - x0; rh = y1 - y0;
    if (!(row_w = see_new(to_back(OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Left, rx, WA_Top, ry, WA_Width, rw,
                                 WA_Height, rh, WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE,
                                 WA_IDCMP, IDCMP_REFRESHWINDOW | IDCMP_ACTIVEWINDOW | IDCMP_MOUSEBUTTONS, TAG_DONE)))))
        return;
    row_redraw();
}

/* the row's own background between the slots, then every slot as last drawn */
static void row_show(void)
{
    int i;
    if (!row_w) return;
    bar_paint(row_w, 0, 0, rw, rh);
    see_key(row_w);
    for (i = 0; i < SLOTS; i++) slot_show(&slots[i]);
}

static void slot_free(struct slot *s)
{
    if (s->bm) { WaitBlit(); FreeBitMap(s->bm); s->bm = NULL; }
}

/* the slot's box on the screen; its bitmap made again when the size changed */
static int slot_box(struct slot *s, WORD x, WORD y, WORD w, WORD h)
{
    if (!s->bm || w != s->win.Width || h != s->win.Height) {
        slot_free(s);
        if (w <= 0 || h <= 0 || !(s->bm = AllocBitMap(w, h, GetBitMapAttr(scr->RastPort.BitMap, BMA_DEPTH), BMF_CLEAR,
                                                     scr->RastPort.BitMap))) return 0;
        s->rp.BitMap = s->bm;
    }
    s->win.LeftEdge = x; s->win.TopEdge = y; s->win.Width = w; s->win.Height = h;
    row_layout();
    return 1;
}

/* an item's "window": a free slot at that box, or NULL */
static struct Window *slot_open(WORD x, WORD y, WORD w, WORD h)
{
    int i;
    struct slot *s;
    for (i = 0; i < SLOTS && slots[i].on; i++) ;
    if (i == SLOTS) return NULL;
    s = &slots[i];
    memset(s, 0, sizeof *s);
    InitRastPort(&s->rp);
    SetFont(&s->rp, dri->dri_Font);
    s->win.RPort = &s->rp;
    s->win.WScreen = scr;
    s->on = 1;
    if (!slot_box(s, x, y, w, h)) { s->on = 0; row_layout(); return NULL; }
    return &s->win;
}

static void slot_close(struct Window *w)
{
    struct slot *s = slot_of(w);
    if (!s) return;
    s->on = 0;
    slot_free(s);
    row_layout();
    row_show();
}

/* ChangeWindowBox for an item */
static void slot_move(struct Window *w, WORD x, WORD y, WORD ww, WORD wh)
{
    struct slot *s = slot_of(w);
    if (s) { slot_box(s, x, y, ww, wh); row_redraw(); }
}

/* the slot under the pointer, as the item's window it was */
static struct Window *row_item(void)
{
    int i;
    WORD x = scr->MouseX, y = scr->MouseY;
    for (i = 0; i < SLOTS; i++) {
        struct Window *w = &slots[i].win;
        if (slots[i].on && x >= w->LeftEdge && x < w->LeftEdge + w->Width && y >= w->TopEdge && y < w->TopEdge + w->Height) return w;
    }
    return NULL;
}


/* ---- the network icons ----------------------------------------------------------------------- */

struct wifi_row {                              /* gfopensocketwifi.device's record (acnetwork.h), 64 bytes */
    char ssid[33];
    UBYTE bssid[6], signal, security, flags, reserved0[2];
    ULONG channel, rate_mbps;
    UBYTE reserved1[12];
};
struct wifi_call { ULONG command; const char *ssid; struct wifi_row *networks; ULONG capacity; LONG result; ULONG error; };

/* the PC's (or the card's) Wi-Fi: -1 none to show, 0 not connected, 1 connected */
static int wifi_poll(void)
{
    struct MsgPort *port = CreateMsgPort();
    struct IOStdReq *req = port ? (struct IOStdReq *)CreateIORequest(port, sizeof *req) : NULL;
    struct wifi_row row;
    struct wifi_call call;
    int state = -1;
    if (req && !OpenDevice((STRPTR)WIFI_DEVICE, 0, (struct IORequest *)req, 0)) {
        register struct Device *a6 __asm("a6") = req->io_Device;
        register struct wifi_call *a0 __asm("a0") = &call;
        register LONG d0 __asm("d0");
        memset(&call, 0, sizeof call);
        memset(&row, 0, sizeof row);
        call.command = WIFI_STATUS;
        call.networks = &row;
        call.capacity = 1;
        __asm volatile ("jsr -42(%%a6)" : "=r"(d0), "+r"(a0) : "r"(a6) : "d1", "a1", "cc", "memory");
        (void)d0;
        if (!call.error) {
            state = call.result > 0;
            if (state) { strncpy(wifi_ssid, row.ssid, sizeof wifi_ssid - 1); wifi_ssid[sizeof wifi_ssid - 1] = 0; wifi_signal = row.signal; }
        }
        CloseDevice((struct IORequest *)req);
    }
    if (req) DeleteIORequest((struct IORequest *)req);
    if (port) DeleteMsgPort(port);
    return state;
}

static void net_poll(void)
{
    ULONG id;
    if (!SocketBase) SocketBase = OpenLibrary((STRPTR)"bsdsocket.library", 4);
    if (!SocketBase) { lan_state = 0; lan_addr[0] = 0; }
    else if ((id = gethostid()) != 0 && id != 0x7f000001UL) {
        struct in_addr a;
        a.s_addr = id;
        lan_state = 2;
        strncpy(lan_addr, (const char *)Inet_NtoA(a.s_addr), sizeof lan_addr - 1);
        lan_addr[sizeof lan_addr - 1] = 0;
    } else { lan_state = 1; lan_addr[0] = 0; }
    wifi_state = wifi_poll();
}

static WORD net_width(void) { return (wifi_state >= 0 ? 2 : 1) * TRAY_CELL; }   /* 0.7.10: a cell each, as every tray icon */

static void net_place(void)
{
    WORD bh = scr->BarHeight + 1;
    int n, off = tray_before("Network", NET_ORDER, &n);
    nw = net_width();
    switch (bar_edge) {
    case OM_BAR_BOTTOM: nh = bh; nx = scr->Width - nw - off; ny = scr->Height - bh; break;
    case OM_BAR_LEFT:   nh = bh; nx = 0; ny = scr->Height - bh * (n + 1); break;
    case OM_BAR_RIGHT:  nh = bh; nx = scr->Width - nw; ny = scr->Height - bh * (n + 1); break;
    case OM_BAR_TOP:    nh = bh; nx = scr->Width - 2 * scr->BarHeight - nw - off; ny = 0; break;
    default:            nh = scr->BarHeight; nx = scr->Width - bar_end() - nw - off; ny = 0; break;
    }
}

/* LAN: a box above two, joined; filled when connected, hollow when not, crossed with no network */
static void draw_lan(struct RastPort *rp, WORD x, WORD cy, int state)
{
    WORD t = cy - 5;
    if (state == 2) { RectFill(rp, x + 5, t, x + 10, t + 3); RectFill(rp, x, t + 7, x + 4, t + 10); RectFill(rp, x + 11, t + 7, x + 15, t + 10); }
    else {
        Move(rp, x + 5, t); Draw(rp, x + 10, t); Draw(rp, x + 10, t + 3); Draw(rp, x + 5, t + 3); Draw(rp, x + 5, t);
        Move(rp, x, t + 7); Draw(rp, x + 4, t + 7); Draw(rp, x + 4, t + 10); Draw(rp, x, t + 10); Draw(rp, x, t + 7);
        Move(rp, x + 11, t + 7); Draw(rp, x + 15, t + 7); Draw(rp, x + 15, t + 10); Draw(rp, x + 11, t + 10); Draw(rp, x + 11, t + 7);
    }
    Move(rp, x + 7, t + 4); Draw(rp, x + 7, t + 5); Move(rp, x + 2, t + 5); Draw(rp, x + 13, t + 5);
    Move(rp, x + 2, t + 5); Draw(rp, x + 2, t + 6); Move(rp, x + 13, t + 5); Draw(rp, x + 13, t + 6);
    if (state == 0) { Move(rp, x, t); Draw(rp, x + 15, t + 10); Move(rp, x + 15, t); Draw(rp, x, t + 10); }
}

/* Wi-Fi: a dot and up to three arcs by the signal; only the dot and a cross when not connected */
static void draw_wifi(struct RastPort *rp, WORD x, WORD cy, int state, int signal)
{
    WORD ox = x + 8, oy = cy + 4;
    int arcs = !state ? 0 : signal > 66 ? 3 : signal > 33 ? 2 : 1;
    RectFill(rp, ox - 1, oy - 1, ox, oy);
    for (int k = 1; k <= 3; k++) {
        int r = 3 * k + 1;
        if (k > arcs) continue;
        for (int dx = -r; dx <= r; dx++) {
            int dy = 0;
            while ((dy + 1) * (dy + 1) + dx * dx <= r * r) dy++;
            if (dx * dx <= dy * dy) WritePixel(rp, ox + dx, oy - dy);
        }
    }
    if (!state) { Move(rp, ox + 2, oy - 8); Draw(rp, ox + 7, oy - 3); Move(rp, ox + 7, oy - 8); Draw(rp, ox + 2, oy - 3); }
}

static void draw_net(int force)
{
    struct RastPort *rp;
    int key = lan_state * 1000 + (wifi_state + 1) * 100 + (wifi_state > 0 ? (wifi_signal > 66 ? 3 : wifi_signal > 33 ? 2 : 1) : 0);
    WORD cy;
    if (!net_w || !slot_of(net_w) || (!force && key == net_drawn)) return;
    net_drawn = key;
    rp = net_w->RPort;
    bar_paint(net_w, 0, 0, nw, nh);
    band(net_w, 0, nw, nh, BAND_SHADE);         /* 0.9: the systray's band */
    SetAPen(rp, bar_text());
    cy = (nh - (bar_edge == OM_BAR_TITLE ? 1 : 0)) / 2;
    draw_lan(rp, TRAY_PAD, cy, lan_state);
    if (wifi_state >= 0) draw_wifi(rp, TRAY_CELL + TRAY_PAD, cy, wifi_state, wifi_signal);
    see_key(net_w);
}

static struct Window *net_window(WORD x, WORD y, WORD w, WORD h)
{
    return to_back(OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Left, x, WA_Top, y, WA_Width, w, WA_Height, h,
                          WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE,
                          WA_IDCMP, IDCMP_REFRESHWINDOW | IDCMP_ACTIVEWINDOW | IDCMP_MOUSEBUTTONS, TAG_DONE));
}

/* 0.8: GFOpenNetwork (the network as a commodity, Dale 12 November 2026) runs: its icons are items in the row and
 * ours are not drawn */
static int net_elsewhere(void)
{
    struct MsgPort *p;
    Forbid();
    p = FindPort((STRPTR)"GFOpenNetwork.bar");
    Permit();
    return p != NULL;
}

static void net_open(void)
{
    net_poll();
    net_place();
    if ((net_w = slot_open(nx, ny, nw, nh))) {
        net_drawn = -100;
        draw_net(1);
        tray("Network", nw, NET_ORDER);
    }
}

/* the panel under the icons: what the network is, and a button for its settings */
#define POP_W 230
static void pop_lines(char l[3][64])
{
    if (lan_state == 0) strcpy(l[0], "Network: GFOpenSocket isn't running");
    else if (lan_state == 1) strcpy(l[0], "Network: not connected");
    else snprintf(l[0], 64, "Network: connected, %s", lan_addr);
    if (wifi_state < 0) strcpy(l[1], "Wi-Fi: none");
    else if (!wifi_state) strcpy(l[1], "Wi-Fi: not connected");
    else snprintf(l[1], 64, "Wi-Fi: %s, signal %d%%", wifi_ssid, wifi_signal);
    strcpy(l[2], "Network settings...");
}

static void draw_pop(void)
{
    struct RastPort *rp;
    char l[3][64];
    WORD fh, y, i;
    if (!pop_w) return;
    rp = pop_w->RPort;
    fh = rp->TxHeight;
    pop_lines(l);
    if (pt_ok) pt_panel(rp, pop_w->Width, pop_w->Height);       /* the theme's menu colour and a slim frame */
    else {
        SetAPen(rp, dri->dri_Pens[BACKGROUNDPEN]);
        RectFill(rp, 0, 0, pop_w->Width - 1, pop_w->Height - 1);
        SetAPen(rp, dri->dri_Pens[SHADOWPEN]);
        Move(rp, 0, 0); Draw(rp, pop_w->Width - 1, 0); Draw(rp, pop_w->Width - 1, pop_w->Height - 1); Draw(rp, 0, pop_w->Height - 1); Draw(rp, 0, 0);
    }
    SetAPen(rp, pt_ok ? pt_text_pen() : dri->dri_Pens[TEXTPEN]);
    SetDrMd(rp, JAM1);
    for (i = 0, y = 6; i < 2; i++, y += fh + 4) aat_box(rp, scr, 8, y, fh, l[i], strlen(l[i]));
    /* the button */
    if (pt_ok) {
        SetAPen(rp, pt_hot_pen());
        RectFill(rp, 8, y, pop_w->Width - 9, y + fh + 5);
        SetAPen(rp, pt_hot_text_pen());
    } else {
        SetAPen(rp, dri->dri_Pens[SHINEPEN]);
        Move(rp, 8, y + fh + 5); Draw(rp, 8, y); Draw(rp, pop_w->Width - 9, y);
        SetAPen(rp, dri->dri_Pens[SHADOWPEN]);
        Draw(rp, pop_w->Width - 9, y + fh + 5); Draw(rp, 8, y + fh + 5);
        SetAPen(rp, dri->dri_Pens[TEXTPEN]);
    }
    aat_box(rp, scr, (pop_w->Width - aat_length(rp, l[2], strlen(l[2]))) / 2, y + 3, fh, l[2], strlen(l[2]));
}

static void pop_close(void)
{
    if (pop_w) { CloseWindow(pop_w); pop_w = NULL; }
    if (last_other) ActivateWindow(last_other);
}

/* 0.8.1: the panel as wide as its longest line (outline text is wider than Text()'s), POP_W at least */
static WORD pop_width(void)
{
    struct RastPort rp = scr->RastPort;
    char l[3][64];
    WORD w = POP_W, tw;
    int i;
    SetFont(&rp, dri->dri_Font);
    pop_lines(l);
    for (i = 0; i < 2; i++)
        if ((tw = aat_length(&rp, l[i], strlen(l[i])) + 16) > w) w = tw;
    return w;
}

static void pop_open(void)
{
    WORD fh = dri->dri_Font->tf_YSize, h = 3 * (fh + 4) + 14, pw = pop_width(), x = nx + nw - pw, y = ny + nh;
    if (x < 0) x = 0;
    if (bar_edge == OM_BAR_BOTTOM) y = ny - h;
    if (y + h > scr->Height) y = scr->Height - h;
    net_poll();
    draw_net(0);
    if ((pop_w = net_window(x, y, pw, h))) {
        SetFont(pop_w->RPort, dri->dri_Font);
        draw_pop();
        pop_ticks = 0;
    }
}

static void pop_click(WORD my)
{
    WORD fh = pop_w->RPort->TxHeight, by = 6 + 2 * (fh + 4);
    if (my >= by && my <= by + fh + 5) {
        BPTR in = Open((STRPTR)"NIL:", MODE_OLDFILE), out = Open((STRPTR)"NIL:", MODE_NEWFILE);
        /* asynchronous: both handles are the new process's, closed when it ends */
        if (!in || !out || SystemTags((STRPTR)NET_TOOL, SYS_Input, in, SYS_Output, out, SYS_Asynch, TRUE, SYS_UserShell, TRUE,
                                      NP_StackSize, 16384, TAG_DONE) == -1) {
            if (in) Close(in);
            if (out) Close(out);
        }
    }
    pop_close();
}

/* ---- the cog: the settings editors --------------------------------------------------------- */

/* The cog, drawn at three sizes; the largest that fits the bar is used. */
static const char *const cog9[9] = {
    "....#....", ".#.###.#.", "..##.##..", ".##...##.", "###...###", ".##...##.", "..##.##..", ".#.###.#.", "....#....",
};
static const char *const cog11[11] = {
    "....###....", ".##.###.##.", ".#########.", "..##...##..", "###.....###", "###.....###",
    "###.....###", "..##...##..", ".#########.", ".##.###.##.", "....###....",
};
static const char *const cog13[13] = {
    ".....###.....", ".##..###..##.", ".###########.", "..#########..", "..###...###..", "####.....####", "####.....####",
    "####.....####", "..###...###..", "..#########..", ".###########.", ".##..###..##.", ".....###.....",
};
/* 0.9: Power's glyph, the power symbol, at the cog's sizes */
static const char *const pw9[9] = {
    "....#....", ".#..#..#.", "#...#...#", "#...#...#", "#.......#", "#.......#", ".#.....#.", "..#####..", ".........",
};
static const char *const pw11[11] = {
    ".....#.....", "..#..#..#..", ".#...#...#.", "#....#....#", "#....#....#", "#.........#",
    "#.........#", "#.........#", ".#.......#.", "..#.....#..", "...#####...",
};
static const char *const pw13[13] = {
    "......#......", "...#..#..#...", "..##..#..##..", ".##...#...##.", ".#....#....#.", "#.....#.....#", "#...........#",
    "#...........#", "#...........#", ".#.........#.", ".##.......##.", "..###...###..", "....#####....",
};

/* ---- 0.9: the commodities (the systray), as last read ------------------------------------------------
 * commodities.library's private calls that Exchange uses (-186 CopyBrokerList, -192 FreeBrokerList, -198
 * BrokerCommand), in OS 3.2's and GFOpenCommodities' library alike, as GFOpenExchange calls them (gfopenup
 * src/openexchange/exchange.c). No change to commodities.library. */
#ifndef COF_ACTIVE
#define COF_ACTIVE 2
#endif
struct BrokerCopy {
    struct Node bc_Node;
    char bc_Name[CBD_NAMELEN];
    char bc_Title[CBD_TITLELEN];
    char bc_Descr[CBD_DESCRLEN];
    LONG bc_Task;
    LONG bc_Port;
    UWORD bc_Dummy;
    ULONG bc_Flags;
};

static LONG cx_CopyBrokerList(struct List *l)
{
    register struct Library *a6 __asm("a6") = CxBase;
    register struct List *a0 __asm("a0") = l;
    register LONG d0 __asm("d0");
    __asm__ volatile("jsr -186(%%a6)" : "=r"(d0), "+r"(a0) : "r"(a6) : "d1", "a1", "cc", "memory");
    return d0;
}

static void cx_FreeBrokerList(struct List *l)
{
    register struct Library *a6 __asm("a6") = CxBase;
    register struct List *a0 __asm("a0") = l;
    __asm__ volatile("jsr -192(%%a6)" : "+r"(a0) : "r"(a6) : "d0", "d1", "a1", "cc", "memory");
}

static LONG cx_BrokerCommand(const char *name, LONG id)
{
    register struct Library *a6 __asm("a6") = CxBase;
    register const char *a0 __asm("a0") = name;
    register LONG d0 __asm("d0") = id;
    __asm__ volatile("jsr -198(%%a6)" : "+r"(d0), "+r"(a0) : "r"(a6) : "d1", "a1", "cc", "memory");
    return d0;
}

static void cx_copy(char *to, const char *from, int size)
{
    int i;
    for (i = 0; i < size - 1 && from[i]; i++) to[i] = from[i];
    to[i] = 0;
}

#define CX_ALL 24                              /* the brokers "All commodities..." lists */
#define CX_MAX 12                              /* the systray's items */
struct cx_item { char name[CBD_NAMELEN], title[CBD_TITLELEN], descr[CBD_DESCRLEN]; ULONG flags; struct Task *task; };
static struct cx_item cx_all[CX_ALL];
static int cx_all_n;
static int cx_tray[CX_MAX], cx_n;              /* the systray's items: indexes into cx_all */
static struct Window *cx_w;                    /* the systray's slot */
/* 0.9.2: the cog's and the systray's places, published by the main task (rb_publish); rb_func reads only these,
 * never a Window, which the main task closes and opens again (Team 5's review of 0.9.1) */
static struct tb_rect rb_cog, rb_cx;
static struct tb_rect rb_ts[TS_ICONS];          /* 0.11: the picture items' places, for their brokers' menus */
static WORD xx, xy, xw, xh;
static int cx_drawn = -1;
static void draw_cx(int force);
static void cx_sync(int read);

/* 0.9: who a menu is for: Prefs (the cog), Power, or a systray item (OW_CX + its place in the systray) */
#define OW_PREFS 1
#define OW_POWER 2
#define OW_CX    TBO_CX
#define OW_TS    TBO_TS                        /* 0.11: a picture item's (OW_TS + its slot): its program's broker */
static int menu_owner;
#define COG_GAP 6                              /* before Prefs and Power: their band apart from the systray's */

/* 0.9.3: what 0.9 added to the bar (the hot band, the cog's and power's glyphs, the raised cell, Active's tick, a
 * commodity's box) draws through GFOpenLayout (gfopenlayout_draw.h), not with graphics.library (the Team's drawing
 * rule, 13 November 2026; Team 5's review of 0.9.2). An oldraw on a slot's window, for one redraw. */
static oldraw *od_open(struct Window *win)
{
    return win ? oldraw_new_rastport(win->RPort, win->WScreen, win->Width, win->Height) : NULL;
}

/* a pen the bar already has (its text pen, the theme's menu pens) as an oldraw colour: the pen's own RGB, which
 * oldraw gives back as that pen (a palette screen) or draws as it is (true colour) */
static oldraw_colour pen_colour(LONG pen)
{
    ULONG rgb[3];
    if (!scr) return OLDRAW_PEN(OLDRAW_PEN_TEXT);
    GetRGB32(scr->ViewPort.ColorMap, (ULONG)pen, 1, rgb);
    return OLDRAW_RGB(((rgb[0] >> 24) << 16) | ((rgb[1] >> 24) << 8) | (rgb[2] >> 24));
}

static void od_boxes(oldraw *d, const struct tb_box *b, int n, oldraw_colour c)
{
    int i;
    for (i = 0; i < n; i++) {
        oldraw_rect r;
        r.x = b[i].x; r.y = b[i].y; r.w = b[i].w; r.h = b[i].h;
        oldraw_fill(d, &r, c);
    }
}

/* 0.9: a band of the bar's own colour, a shade lighter (darker on a light bar), across x..x+w-1 of an item */
static void band(struct Window *win, WORD x, WORD w, WORD h, int shade)
{
    WORD top = h > 12 ? 2 : 1, bh = h - 2 * top - (bar_edge == OM_BAR_TITLE ? 1 : 0);
    if (w <= 0 || bh <= 0) return;
    if (CyberGfxBase && bp_n) bp_paint_kept(win, 0, dri->dri_Pens[BARBLOCKPEN], x, top, w, bh, shade);
    else if (shade >= BAND_HOT) {
        oldraw *d = od_open(win);
        if (d) {
            oldraw_rect r;
            r.x = x; r.y = top; r.w = w; r.h = bh;
            oldraw_fill(d, &r, OLDRAW_PEN(OLDRAW_PEN_FILL));
            oldraw_free(d);
        }
    }
}

/* a glyph's '#' rows, centred in the box x..x+w-1, 0..h-1, in c */
static void glyph(oldraw *d, oldraw_colour c, const char *const *rows, int size, WORD x, WORD w, WORD h)
{
    struct tb_box b[64];
    WORD ox = x + (w - size) / 2, oy = (h - size) / 2;
    if (oy < 0) oy = 0;
    od_boxes(d, b, tb_glyph_boxes(rows, size, ox, oy, h, b, 64), c);
}

/* raised under the pointer, as the taskspace's icons are: a one-pixel bevel over x+1..x+w-2 */
static void raised(oldraw *d, WORD x, WORD w, WORD h)
{
    oldraw_rect r;
    r.x = x + 1; r.y = 0; r.w = w - 2; r.h = h;
    oldraw_frame(d, &r, 0);
}

/* 0.7.10 (Dale, 11 November 2026: "we need a reboot / power cycle option. Also ... a safe eject for USB"): the
 * cog's list ends with the power menu, and above it an "Eject" row for each USB stick mounted (usb_sticks()) */
#define PW_REBOOT  1                           /* warm: the Amiga's own ColdReboot */
#define PW_RESTART 2                           /* cold: the host starts the machine again (accontrol.device) */
#define PW_OFF     3                           /* the card shuts down; on a PC the instance closes */
#define USB_MAX    6                           /* the sticks the list shows */
static struct { char dev[32], name[32], label[48]; } usb[USB_MAX];
static int usb_n;

/* The cog's list (0.7): GFOpenPrefs's sections (section: GFOpenPrefs SECTION; 0.7.1: by the pages' names, as
 * GFOpenPrefs lists them), then the other settings programs
 * that are there (needs, or path when NULL), as in GFOpenUpMenu's Tools > Preferences (gfopenup
 * src/openupmenu.c). */
static const struct { const char *label, *path, *section, *needs; int power; } cog_items[] = {
    { "Theme",         NULL, "THEME",    NULL },
    { "Screen Mode",   NULL, "SCREENMODE", NULL },
    { "Windows",       NULL, "WINDOWS",  NULL },
    { "Title Bar",     NULL, "TITLEBAR", NULL },
    { "Menus",         NULL, "MENUS",    NULL },
    { "IControl",      NULL, "ICONTROL", NULL },
    { "Dock",          NULL, "DOCK",     NULL },
    { "Sound",         NULL, "SOUND",    NULL },
    { "Input",         NULL, "INPUT",    NULL },
    { "Gamepads",      NULL, "GAMEPADS", NULL },
    { "Date & Time",   NULL, "DATETIME", NULL },
    { "Serial",        NULL, "SERIAL",   NULL },
    { "File Types",    NULL, "TYPES",    NULL },
    { "System",        NULL, "SYSTEM",   NULL },
    { "Blanker",       NULL, "BLANKER",  NULL },
    { NULL,            NULL, NULL,       NULL },                          /* a line */
    { "Printers",      "SYS:Utilities/GFOpenPrint/Printers and Queue", NULL, NULL },  /* 0.6.5: GFOpenPrint's printers */
    { "Network",       "SYS:Tools/Commodities/GFOpenSocketControl", NULL, NULL },     /* 0.6.6: GFOpenSocket's settings and status */
    { "GFOpenUp Setup",  "SYS:Prefs/GFOpenUp-Setup", NULL, NULL },
    { "Tata",          "SYS:Prefs/GFOpenTata",     NULL, NULL },            /* 0.6: Tata's window, when Tata is installed */
    { NULL,            NULL, NULL,       NULL },                          /* a line */
    { "All Prefs...",  "SYS:Prefs",              NULL, NULL },
    { NULL,            NULL, NULL,       NULL },                          /* a line; then the USB sticks (0.7.10) */
    { "Reboot",        NULL, NULL, NULL, PW_REBOOT },                     /* 0.7.10: the power menu */
    { "Restart the machine...", NULL, NULL, NULL, PW_RESTART },
    { "Power off...",  NULL, NULL, NULL, PW_OFF },
};
#define COG_SECTIONS 15                        /* the first ones: GFOpenPrefs's */
#define COG_ITEMS (int)(sizeof cog_items / sizeof cog_items[0])
#define COG_PAD 10
#define COG_POWER_LINE (COG_ITEMS - 4)         /* the line before Reboot: Prefs' list ends there, Power's starts after it */

/* 0.9: the rows the commodities' menus have (and "All commodities..."), past cog_items and the USB sticks */
#define XR_BASE 1000
#define XR_MAX  (CX_ALL + 4)
enum { XA_LINE, XA_SHOW, XA_HIDE, XA_ACTIVE, XA_INFO, XA_REMOVE, XA_ALLCX, XA_BROKER };
static struct { char label[CBD_TITLELEN + 4]; int act, tick; } xrow[XR_MAX];
static int xr_n;
static char menu_cx[CBD_NAMELEN];              /* the broker a commodity's menu is for */

static int menu_idx[COG_ITEMS + USB_MAX + XR_MAX + 2], menu_n; /* what the list shows: cog_items, usb[], xrow[] */
static WORD menu_y[COG_ITEMS + USB_MAX + XR_MAX + 3], menu_rh;

/* a row's words: a cog_items entry's, a USB stick's "Eject", or past them a commodity row's (NULL: a line) */
static const char *row_label(int idx)
{
    if (idx >= XR_BASE) return xrow[idx - XR_BASE].act == XA_LINE ? NULL : xrow[idx - XR_BASE].label;
    return idx < COG_ITEMS ? cog_items[idx].label : usb[idx - COG_ITEMS].label;
}
static void usb_sticks(void);
static const char *menu_title;

static int exists(const char *path)
{
    struct Process *me = (struct Process *)FindTask(NULL);
    APTR was = me->pr_WindowPtr;
    BPTR l;
    me->pr_WindowPtr = (APTR)-1;               /* no "please insert volume" for a place that isn't there */
    l = Lock((STRPTR)path, ACCESS_READ);
    me->pr_WindowPtr = was;
    if (l) UnLock(l);
    return l != 0;
}

/* GFOpenPrefs: SYS:Preferences', else an older disk's SYS:Prefs one (Common/prefsapp.h); NULL when neither is there */
static const char *prefs_app(void)
{
    return op_app_path(0);
}

/* GFOpenPrefs at a section, on the public screen pub (NULL: the default), as a process of its own */
static void prefs_start(const char *pub, const char *section)
{
    const char *app = prefs_app();
    char cmd[240];
    BPTR in, out;
    if (!app) return;
    if (pub) snprintf(cmd, sizeof cmd, "\"%s\" PUBSCREEN=\"%s\" %s", app, pub, section);
    else snprintf(cmd, sizeof cmd, "\"%s\" %s", app, section);
    in = Open((STRPTR)"NIL:", MODE_OLDFILE);
    out = Open((STRPTR)"NIL:", MODE_NEWFILE);
    if (!in || !out || SystemTags((STRPTR)cmd, SYS_Input, in, SYS_Output, out, SYS_Asynch, TRUE, SYS_UserShell, TRUE,
                                  NP_StackSize, 32768, TAG_DONE) == -1) {
        if (in) Close(in);
        if (out) Close(out);
    }
}

static int cog_size(void)
{
    int h = scr->BarHeight - 2;
    return h >= 13 ? 13 : h >= 11 ? 11 : 9;
}

/* 0.9: Prefs and Power, one item of the row: a gap, then a cell each */
static void cog_place(void)
{
    WORD bh = scr->BarHeight + 1;
    int n, off = tray_before("Cog", COG_ORDER, &n);
    gw = COG_GAP + 2 * TRAY_CELL;
    switch (bar_edge) {
    case OM_BAR_BOTTOM: gh = bh; gx = scr->Width - gw - off; gy = scr->Height - bh; break;
    case OM_BAR_LEFT:   gh = bh; gx = 0; gy = scr->Height - bh * (n + 1); break;
    case OM_BAR_RIGHT:  gh = bh; gx = scr->Width - gw; gy = scr->Height - bh * (n + 1); break;
    case OM_BAR_TOP:    gh = bh; gx = scr->Width - 2 * scr->BarHeight - gw - off; gy = 0; break;
    default:            gh = scr->BarHeight; gx = scr->Width - bar_end() - gw - off; gy = 0; break;
    }
}

/* the cell under the pointer: OW_PREFS, OW_POWER or 0 */
static int cog_cell(void)
{
    WORD x = scr->MouseX - gx - COG_GAP, y = scr->MouseY;
    if (!cog_w || IntuitionBase->FirstScreen != scr || y < gy || y >= gy + gh || x < 0 || x >= 2 * TRAY_CELL) return 0;
    return x < TRAY_CELL ? OW_PREFS : OW_POWER;
}

/* the systray's item under the pointer, or -1 */
static int cx_over(void)
{
    WORD x = scr->MouseX - xx, y = scr->MouseY;
    if (!cx_w || IntuitionBase->FirstScreen != scr || y < xy || y >= xy + xh || x < 0 || x >= cx_n * TRAY_CELL) return -1;
    return x / TRAY_CELL;
}

static int ts_at_pointer(void);

/* whose menu a click here opens: OW_PREFS, OW_POWER, OW_CX + k, OW_TS + slot (0.11), or 0 */
static int owner_at_pointer(void)
{
    int k = cog_cell();
    return tbo_owner(k, k ? -1 : cx_over(), k || cx_over() >= 0 ? -1 : ts_at_pointer());
}

/* Prefs (the cog) and Power on their band; raised under the pointer; a deeper shade while its menu is open (0.9:
 * no longer the fill pen, Dale: "the colour on the prefs bit needs sorting") */
static void draw_cog(int force)
{
    oldraw *d;
    int hot = cog_cell(), open = menu_w && menu_owner <= OW_POWER ? menu_owner : 0, state = hot + 3 * open, size, c;
    if (!cog_w || !slot_of(cog_w) || (!force && state == cog_drawn)) return;
    cog_drawn = state;
    bar_paint(cog_w, 0, 0, gw, gh);
    band(cog_w, COG_GAP, 2 * TRAY_CELL, gh, BAND_SHADE);
    size = cog_size();
    if ((d = od_open(cog_w))) {
        oldraw_colour ink = pen_colour(bar_text());
        for (c = OW_PREFS; c <= OW_POWER; c++) {
            WORD x = COG_GAP + (c - 1) * TRAY_CELL;
            if (open == c) band(cog_w, x, TRAY_CELL, gh, BAND_HOT);
            else if (hot == c) raised(d, x, TRAY_CELL, gh);
            if (c == OW_PREFS) glyph(d, ink, size == 13 ? cog13 : size == 11 ? cog11 : cog9, size, x, TRAY_CELL, gh);
            else glyph(d, ink, size == 13 ? pw13 : size == 11 ? pw11 : pw9, size, x, TRAY_CELL, gh);
        }
        oldraw_free(d);
    }
    see_key(cog_w);
}

static void cog_open(void)
{
    cog_place();
    if ((cog_w = slot_open(gx, gy, gw, gh))) {
        cog_drawn = -1;
        draw_cog(1);
        tray("Cog", gw, COG_ORDER);
    }
}

static void draw_menu(void)
{
    struct RastPort *rp;
    WORD w, h;
    LONG ink;
    int i;
    if (!menu_w) return;
    rp = menu_w->RPort;
    w = menu_w->Width; h = menu_w->Height;
    if (pt_ok) pt_panel(rp, w, h);              /* the theme's menu colour and a slim frame, as GFOpenMenus' menus */
    else {
        SetAPen(rp, dri->dri_Pens[BARBLOCKPEN]);
        RectFill(rp, 1, 1, w - 2, h - 2);
        SetAPen(rp, dri->dri_Pens[SHADOWPEN]);
        Move(rp, 0, 0); Draw(rp, w - 1, 0); Draw(rp, w - 1, h - 1); Draw(rp, 0, h - 1); Draw(rp, 0, 0);
    }
    SetDrMd(rp, JAM1);
    for (i = 0; i < menu_n; i++) {
        const char *t = row_label(menu_idx[i]);
        WORD y0 = menu_y[i], y1 = menu_y[i + 1] - 1;
        if (!t) {
            if (pt_ok) {
                SetAPen(rp, pt_frame_pen());
                Move(rp, 4, (y0 + y1) / 2); Draw(rp, w - 5, (y0 + y1) / 2);
                continue;
            }
            SetAPen(rp, dri->dri_Pens[SHADOWPEN]);
            Move(rp, 4, (y0 + y1) / 2); Draw(rp, w - 5, (y0 + y1) / 2);
            SetAPen(rp, dri->dri_Pens[SHINEPEN]);
            Move(rp, 4, (y0 + y1) / 2 + 1); Draw(rp, w - 5, (y0 + y1) / 2 + 1);
            continue;
        }
        if (i == menu_hot) { SetAPen(rp, pt_ok ? pt_hot_pen() : dri->dri_Pens[FILLPEN]); RectFill(rp, 2, y0, w - 3, y1); }
        ink = pt_ok ? (i == menu_hot ? pt_hot_text_pen() : pt_text_pen()) : dri->dri_Pens[i == menu_hot ? FILLTEXTPEN : BARDETAILPEN];
        SetAPen(rp, ink);
        aat_box(rp, scr, COG_PAD, y0, y1 - y0 + 1, t, strlen(t));
        if (menu_idx[i] >= XR_BASE && xrow[menu_idx[i] - XR_BASE].tick) {   /* 0.9: Active's tick */
            oldraw *d = od_open(menu_w);
            if (d) {
                struct tb_box b[18];
                od_boxes(d, b, tb_tick_boxes((y0 + y1) / 2, b, 18), pen_colour(ink));
                oldraw_free(d);
            }
        }
    }
}

static int menu_row(WORD my)
{
    int i;
    for (i = 0; i < menu_n; i++)
        if (my >= menu_y[i] && my < menu_y[i + 1]) return row_label(menu_idx[i]) ? i : -1;
    return -1;
}

static void menu_close(int give_back)
{
    if (!menu_w) return;
    CloseWindow(menu_w);
    menu_w = NULL;
    menu_hot = -1;
    if (give_back && last_other) ActivateWindow(last_other);
    draw_cog(1);
    draw_cx(1);
}

static void xr_add(const char *label, int act, int tick)
{
    unsigned n = (unsigned)xr_n, i;
    if (n >= XR_MAX) return;
    if (!label) label = "";
    for (i = 0; i < sizeof xrow[0].label - 1 && label[i]; i++) xrow[n].label[i] = label[i];
    xrow[n].label[i] = 0;
    xrow[n].act = act;
    xrow[n].tick = tick;
    xr_n = n + 1;
}

static int cx_find(const char *name);

/* the box the menu hangs from: its right edge, top and height (0.9: Prefs, Power or a systray item) */
static int ts_box(int i, WORD *right, WORD *top, WORD *h);
static void owner_box(int owner, WORD *right, WORD *top, WORD *h)
{
    if (owner >= OW_TS && owner < OW_TS + TS_ICONS && ts_box(owner - OW_TS, right, top, h)) return;   /* 0.11 */
    if (owner >= OW_CX) { *right = xx + (owner - OW_CX + 1) * TRAY_CELL; *top = xy; *h = xh; }
    else { *right = gx + COG_GAP + (owner == OW_POWER ? 2 : 1) * TRAY_CELL; *top = gy; *h = gh; }
}

/* 0.9: the menu for owner: Prefs' (GFOpenPrefs' sections, the other settings, "All commodities...", All Prefs...),
 * Power's (the USB sticks, Reboot, Restart, Power off), a commodity's (cx: Show, Hide, Active, Information,
 * Remove), or every broker's (all) */
#define MK_PREFS 1
#define MK_POWER 2
#define MK_CX    3
#define MK_CXALL 4
static void menu_open(int kind, int owner, const char *cx)
{
    struct RastPort trp;
    WORD W = 0, H, x, y, right, top, bh;
    int i, k;
    InitRastPort(&trp);
    SetFont(&trp, dri->dri_Font);
    menu_rh = dri->dri_Font->tf_YSize + 4;
    menu_n = 0;
    xr_n = 0;
    H = 3;
#define ROW(at) do { const char *t_ = row_label(at); menu_y[menu_n] = H; menu_idx[menu_n++] = (at); \
                     if (t_) { WORD tw_ = aat_length(&trp, t_, strlen(t_)); if (tw_ > W) W = tw_; } H += t_ ? menu_rh : 6; } while (0)
    if (kind == MK_PREFS) {
        const char *app = prefs_app();
        for (i = 0; i < COG_POWER_LINE; i++) {
            const char *t = cog_items[i].label;
            if (t && (cog_items[i].section ? !app : !exists(cog_items[i].needs ? cog_items[i].needs : cog_items[i].path))) continue;
            if (!t && !menu_n) continue;               /* no line at the top */
            if (t && !strcmp(t, "All Prefs...") && CxBase) { xr_add("All commodities...", XA_ALLCX, 0); ROW(XR_BASE + xr_n - 1); }
            ROW(i);
        }
    } else if (kind == MK_POWER) {
        usb_sticks();
        for (k = 0; k < usb_n; k++) ROW(COG_ITEMS + k);
        if (usb_n) ROW(COG_POWER_LINE);
        for (i = COG_POWER_LINE + 1; i < COG_ITEMS; i++) ROW(i);
    } else if (kind == MK_CX) {
        int b = cx_find(cx);
        ULONG f = b >= 0 ? cx_all[b].flags : 0;
        cx_copy(menu_cx, cx, sizeof menu_cx);
        if (f & COF_SHOW_HIDE) {
            xr_add("Show", XA_SHOW, 0); ROW(XR_BASE + xr_n - 1);
            xr_add("Hide", XA_HIDE, 0); ROW(XR_BASE + xr_n - 1);
            xr_add(NULL, XA_LINE, 0); ROW(XR_BASE + xr_n - 1);
        }
        xr_add("Active", XA_ACTIVE, (f & COF_ACTIVE) != 0); ROW(XR_BASE + xr_n - 1);
        xr_add("Information...", XA_INFO, 0); ROW(XR_BASE + xr_n - 1);
        xr_add(NULL, XA_LINE, 0); ROW(XR_BASE + xr_n - 1);
        xr_add("Remove", XA_REMOVE, 0); ROW(XR_BASE + xr_n - 1);
    } else {
        for (k = 0; k < cx_all_n && xr_n < XR_MAX; k++) {
            xr_add(cx_all[k].name, XA_BROKER, (cx_all[k].flags & COF_ACTIVE) != 0);
            ROW(XR_BASE + xr_n - 1);
        }
        if (!cx_all_n) { xr_add("No commodities running", XA_LINE + 100, 0); ROW(XR_BASE + xr_n - 1); }
    }
#undef ROW
    if (!menu_n) return;
    menu_y[menu_n] = H;
    H += 3;
    W += 2 * COG_PAD;
    if (W < 120) W = 120;
    owner_box(owner, &right, &top, &bh);
    x = right - W;                                      /* under the item, towards the middle of the screen */
    if (bar_edge == OM_BAR_LEFT) x = 0;
    if (x < 0) x = 0;
    y = (bar_edge == OM_BAR_BOTTOM || bar_edge == OM_BAR_LEFT || bar_edge == OM_BAR_RIGHT) ? top - H : top + bh + (bar_edge == OM_BAR_TITLE ? 1 : 0);
    if (y < 0) y = 0;
    if (y + H > scr->Height) y = scr->Height - H;
    menu_hot = -1;
    menu_owner = owner;
    {   /* the bar keeps the title it shows now while the list is active */
        static char title[120];
        strncpy(title, scr->Title ? (const char *)scr->Title : "", sizeof title - 1);
        title[sizeof title - 1] = 0;
        menu_title = title;
    }
    /* active while open, so the pointer lights the entries and a click elsewhere closes it */
    menu_w = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Left, x, WA_Top, y, WA_Width, W, WA_Height, H,
                            WA_Borderless, TRUE, WA_Activate, TRUE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE, WA_ReportMouse, TRUE,
                            WA_ScreenTitle, (ULONG)menu_title,
                            WA_IDCMP, IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_INACTIVEWINDOW | IDCMP_RAWKEY,
                            TAG_DONE);
    if (menu_w) {
        SetFont(menu_w->RPort, dri->dri_Font);
        draw_menu();
    }
    draw_cog(1);
    draw_cx(1);
}

/* starts an editor as a double-click on its icon does (its icon's stack and tool types count) */
/* 0.6.6: the window the editor opens goes to the screen's top right, by the cog (GFOpenWindows 0.8.1, cogplace.h) */
static void cog_arm(void)
{
    struct cogplace *cp;
    ULONG secs, micros;
    CurrentTime(&secs, &micros);
    Forbid();
    if ((cp = (struct cogplace *)FindSemaphore((STRPTR)COGPLACE_NAME))) {
        cp->until = secs + COGPLACE_SECS;
        cp->from = FindTask(NULL);
        cp->right = cp->top = COGPLACE_GAP;
        cp->armed = 1;
    }
    Permit();
}

static void cog_start(const char *path)
{
    BPTR in, out;
    char cmd[80];
    cog_arm();
    if (ob_open(path) > 0) return;                     /* 0.6.13: GFOpenBench opens it (a built-in runs inside it) */
    if (!WorkbenchBase) WorkbenchBase = OpenLibrary((STRPTR)"workbench.library", 44);
    if (WorkbenchBase && OpenWorkbenchObjectA((STRPTR)path, NULL)) return;
    if (!strcmp(path, "SYS:Prefs")) return;             /* a drawer needs Workbench */
    snprintf(cmd, sizeof cmd, "\"%s\"", path);
    in = Open((STRPTR)"NIL:", MODE_OLDFILE);
    out = Open((STRPTR)"NIL:", MODE_NEWFILE);
    if (!in || !out || SystemTags((STRPTR)cmd, SYS_Input, in, SYS_Output, out, SYS_Asynch, TRUE, SYS_UserShell, TRUE,
                                  NP_StackSize, 16384, TAG_DONE) == -1) {
        if (in) Close(in);
        if (out) Close(out);
    }
}

/* ---- 0.7.10: the USB sticks' Eject, and the power menu ------------------------------------------ */

static void bstr_copy(BSTR b, char *out, int max)
{
    const UBYTE *p = BADDR(b);
    int n = p ? p[0] : 0, i;
    if (n > max - 1) n = max - 1;
    for (i = 0; i < n; i++) out[i] = (char)p[1 + i];
    out[n] = 0;
}

/* the USB sticks mounted now: DOS's drives on gfopenusbacfs.device (Cradle's folder sticks and the sticks the
 * host mounted) or usbdisk.device (a stick on the Amiga's own USB), each with its volume's name when one is in */
static void usb_sticks(void)
{
    struct DosList *dl, *e, *v;
    usb_n = 0;
    dl = LockDosList(LDF_DEVICES | LDF_VOLUMES | LDF_READ);
    for (e = dl; usb_n < USB_MAX && (e = NextDosEntry(e, LDF_DEVICES)) != NULL; ) {
        struct DeviceNode *dn = (struct DeviceNode *)e;
        struct FileSysStartupMsg *fssm;
        char exec_dev[40];
        if ((ULONG)dn->dn_Startup <= 1024 || !dn->dn_Task) continue;     /* not a disk, or nothing mounted on it */
        fssm = BADDR(dn->dn_Startup);
        bstr_copy(fssm->fssm_Device, exec_dev, sizeof exec_dev);
        if (strcmp(exec_dev, "gfopenusbacfs.device") && strcmp(exec_dev, "usbdisk.device")) continue;
        bstr_copy(dn->dn_Name, usb[usb_n].dev, sizeof usb[usb_n].dev);
        strcpy(usb[usb_n].name, usb[usb_n].dev);
        for (v = dl; (v = NextDosEntry(v, LDF_VOLUMES)) != NULL; )
            if (v->dol_Task == dn->dn_Task) { bstr_copy(v->dol_Name, usb[usb_n].name, sizeof usb[usb_n].name); break; }
        snprintf(usb[usb_n].label, sizeof usb[usb_n].label, "Eject %s", usb[usb_n].name);
        usb_n++;
    }
    UnLockDosList(LDF_DEVICES | LDF_VOLUMES | LDF_READ);
}

static LONG ask(const char *title, const char *text, const char *buttons)
{
    struct EasyStruct es;
    es.es_StructSize = sizeof es;
    es.es_Flags = 0;
    es.es_Title = (UBYTE *)title;
    es.es_TextFormat = (UBYTE *)text;
    es.es_GadgetFormat = (UBYTE *)buttons;
    return EasyRequestArgs(NULL, &es, NULL, NULL);
}

/* a stick let go (gfopenusb's C:USBEject: the volume stops, the host syncs and unmounts it), then in plain words
 * whether it can be pulled out */
static void usb_eject(int k)
{
    char cmd[80], said[160];
    BPTR in, out;
    LONG rc = -1, n = 0;
    snprintf(cmd, sizeof cmd, "C:USBEject DRIVE=\"%s:\"", usb[k].dev);
    in = Open((STRPTR)"NIL:", MODE_OLDFILE);
    out = Open((STRPTR)"T:GFOpenTitle.eject", MODE_NEWFILE);
    if (in && out) rc = SystemTags((STRPTR)cmd, SYS_Input, in, SYS_Output, out, SYS_UserShell, TRUE, NP_StackSize, 8192, TAG_DONE);
    if (in) Close(in);
    if (out) Close(out);
    said[0] = 0;
    if ((out = Open((STRPTR)"T:GFOpenTitle.eject", MODE_OLDFILE))) {
        n = Read(out, said, sizeof said - 1);
        Close(out);
        said[n > 0 ? n : 0] = 0;
        while (n > 0 && (said[n - 1] == '\n' || said[n - 1] == ' ')) said[--n] = 0;
    }
    DeleteFile((STRPTR)"T:GFOpenTitle.eject");
    if (rc == 0) {
        char t[80];
        snprintf(t, sizeof t, "%s is safe to remove.", usb[k].name);
        ask("USB", t, "OK");
    } else if (rc == -1)
        ask("USB", "C:USBEject isn't here (GFOpenUSB), so the stick can't be ejected from the bar.", "OK");
    else
        ask("USB", said[0] ? said : "The stick wasn't ejected.", "OK");
}

#define ACC_CMD_POWER   (CMD_NONSTD + 14)      /* accontrol.device 1.5's ACCMD_POWER (amicradle-guest accontrol.h) */
#define ACC_ERR_NOPOWER 48

/* the host restarts the machine (1) or powers it off (2): accontrol.device to the bridge */
static void power_host(ULONG action)
{
    struct MsgPort *port = CreateMsgPort();
    struct IOStdReq *io = port ? (struct IOStdReq *)CreateIORequest(port, sizeof *io) : NULL;
    BYTE err = -1;
    if (io && !OpenDevice((STRPTR)"accontrol.device", 0, (struct IORequest *)io, 0)) {
        io->io_Command = ACC_CMD_POWER;
        io->io_Offset = action;
        err = DoIO((struct IORequest *)io);
        CloseDevice((struct IORequest *)io);
    }
    if (io) DeleteIORequest((struct IORequest *)io);
    if (port) DeleteMsgPort(port);
    if (err == ACC_ERR_NOPOWER || err == -1 || err == IOERR_OPENFAIL)
        ask("Power", action == 1 ? "This machine can't be restarted from here." : "This machine can't be powered off from here.", "OK");
    else if (err)
        ask("Power", action == 1 ? "The host didn't restart the machine." : "The host didn't power the machine off.", "OK");
}

static void power(int what)
{
    if (what == PW_REBOOT) {                   /* warm: the Amiga starts again, as Ctrl-Amiga-Amiga */
        Delay(TICKS_PER_SECOND);               /* the disks' last writes first */
        ColdReboot();
    } else if (what == PW_RESTART) {
        if (ask("Restart the machine", "Restart the machine from cold?\nWork not saved in open programs is lost.", "Restart|Cancel"))
            power_host(1);
    } else if (what == PW_OFF) {
        if (ask("Power off", "Power the machine off?\nWork not saved in open programs is lost.", "Power off|Cancel"))
            power_host(2);
    }
}

static void cx_pick(int r);

static void menu_pick(int row)
{
    const char *path;
    int at;
    if (row < 0 || row >= menu_n) return;
    at = menu_idx[row];
    if (at >= XR_BASE) { cx_pick(at - XR_BASE); return; }                        /* 0.9 */
    if (at >= COG_ITEMS) { menu_close(1); usb_eject(at - COG_ITEMS); return; }     /* 0.7.10 */
    if (cog_items[at].power) { menu_close(1); power(cog_items[at].power); return; }
    if (cog_items[menu_idx[row]].section) {   /* 0.7: GFOpenPrefs at its section, by the cog */
        const char *section = cog_items[menu_idx[row]].section;
        menu_close(1);
        cog_arm();
        prefs_start(NULL, section);
        return;
    }
    if (!(path = cog_items[menu_idx[row]].path)) return;
    menu_close(1);
    cog_start(path);
}

/* the list's messages; first, so a click on the cog that closed it isn't taken as one to open it again */
static void menu_events(void)
{
    struct IntuiMessage *m;
    while (menu_w && (m = (struct IntuiMessage *)GetMsg(menu_w->UserPort))) {
        ULONG cls = m->Class;
        UWORD code = m->Code;
        WORD my = m->MouseY, mx = m->MouseX;
        int inside = mx >= 0 && mx < menu_w->Width && my >= 0 && my < menu_w->Height;
        ReplyMsg((struct Message *)m);
        if (cls == IDCMP_REFRESHWINDOW) { BeginRefresh(menu_w); draw_menu(); EndRefresh(menu_w, TRUE); }
        else if (cls == IDCMP_MOUSEMOVE) {
            int hot = inside ? menu_row(my) : -1;
            if (hot != menu_hot) { menu_hot = hot; draw_menu(); }
        }
        else if (cls == IDCMP_MOUSEBUTTONS && code == SELECTUP && inside) menu_pick(menu_row(my));
        else if (cls == IDCMP_INACTIVEWINDOW) {          /* a click elsewhere */
            cog_skip = owner_at_pointer() == menu_owner ? menu_owner : 0;   /* on the item: it closed the menu */
            menu_close(0);
        }
        else if (cls == IDCMP_RAWKEY) {
            int i = menu_hot;
            if (code == 0x45) menu_close(1);                                  /* Esc */
            else if (code == 0x44 || code == 0x43) menu_pick(menu_hot);       /* Return, Enter */
            else if (code == 0x4C || code == 0x4D) {                          /* up, down */
                do i = code == 0x4D ? (i + 1) % menu_n : (i <= 0 ? menu_n - 1 : i - 1);
                while (!row_label(menu_idx[i]));
                menu_hot = i;
                draw_menu();
            }
        }
    }
}

/* ---- the taskspace: other programs' icons (include/taskspace.h) --------------------------- */

struct ts_icon {
    char name[16], help[48], port[24];
    WORD order;
    UWORD state, w, h, states;
    UWORD version;                             /* 0.8: the protocol the program spoke; 2 hears TSE_PLACE */
    struct SysBarItem *sbi;                    /* 0.10: a gfopensysbar.library item's, else NULL (a taskspace icon) */
    struct Task *task;                         /* 0.10: the library item's program */
    int seen;                                  /* 0.10: still in the library's list at the last read */
    UBYTE pixels[SB_STATES * SB_MAXW * SB_MAXH];   /* 0.10: room for the library's 16 x 16 too */
    ULONG rgb[12];
    struct Window *win;
    WORD x, y, bw, bh;
    int drawn;                                 /* what is drawn: state * 2 + lit, -1 nothing yet */
};
static struct ts_icon *ts[TS_ICONS];
static struct MsgPort *ts_reply;               /* where the programs reply our events */
static int ts_out;                             /* events not replied yet */
static struct Window *help_w;                  /* the help under an icon */
static int help_for = -1, hover_for = -1, hover_ticks;

static int ts_find(const char *name)
{
    int i;
    for (i = 0; i < TS_ICONS; i++) if (ts[i] && !strcmp(ts[i]->name, name)) return i;
    return -1;
}

static void ts_place(struct ts_icon *t)
{
    WORD bh = scr->BarHeight + 1;
    int n, off = tray_before(t->name, t->order, &n);
    t->bw = TRAY_ROOM(t->w);
    switch (bar_edge) {
    case OM_BAR_BOTTOM: t->bh = bh; t->x = scr->Width - t->bw - off; t->y = scr->Height - bh; break;
    case OM_BAR_LEFT:   t->bh = bh; t->x = 0; t->y = scr->Height - bh * (n + 1); break;
    case OM_BAR_RIGHT:  t->bh = bh; t->x = scr->Width - t->bw; t->y = scr->Height - bh * (n + 1); break;
    case OM_BAR_TOP:    t->bh = bh; t->x = scr->Width - 2 * scr->BarHeight - t->bw - off; t->y = 0; break;
    default:            t->bh = scr->BarHeight; t->x = scr->Width - bar_end() - t->bw - off; t->y = 0; break;
    }
}

static int ts_over(const struct ts_icon *t)
{
    WORD x = scr->MouseX, y = scr->MouseY;
    return t->win && IntuitionBase->FirstScreen == scr && x >= t->x && x < t->x + t->bw && y >= t->y && y < t->y + t->bh;
}

/* 0.11: the picture item's slot under the pointer, or -1 */
static int ts_at_pointer(void)
{
    int i;
    for (i = 0; i < TS_ICONS; i++) if (ts[i] && ts_over(ts[i])) return i;
    return -1;
}

/* 0.11: the box a picture item's menu hangs from; 0 when the slot is empty */
static int ts_box(int i, WORD *right, WORD *top, WORD *h)
{
    if (i < 0 || i >= TS_ICONS || !ts[i] || !ts[i]->win) return 0;
    *right = ts[i]->x + ts[i]->bw; *top = ts[i]->y; *h = ts[i]->bh;
    return 1;
}

/* 0.11: the program behind a picture item: the library item's task, or the taskspace port's */
static struct Task *ts_task(const struct ts_icon *t)
{
    struct MsgPort *p;
    struct Task *task = NULL;
    if (t->sbi) return t->task;
    if (!t->port[0]) return NULL;
    Forbid();
    if ((p = FindPort((STRPTR)t->port))) task = (struct Task *)p->mp_SigTask;
    Permit();
    return task;
}

/* the picture in the bar's pens and the program's colours; raised under the pointer, as the cog is */
static void ts_draw(struct ts_icon *t, int force)
{
    struct RastPort *rp;
    int lit, key, x, y;
    WORD h, ox, oy;
    const UBYTE *px;
    if (!t->win || !slot_of(t->win)) return;
    lit = ts_over(t);
    key = t->state * 2 + lit;
    if (!force && key == t->drawn) return;
    t->drawn = key;
    rp = t->win->RPort;
    h = t->bh;
    bar_paint(t->win, 0, 0, t->bw, t->bh);
    band(t->win, 0, t->bw, t->bh, BAND_SHADE);  /* 0.9: the systray's band */
    if (lit) {
        SetAPen(rp, dri->dri_Pens[SHINEPEN]);
        Move(rp, 1, h - 1); Draw(rp, 1, 0); Draw(rp, t->bw - 2, 0);
        SetAPen(rp, dri->dri_Pens[SHADOWPEN]);
        Draw(rp, t->bw - 2, h - 1); Draw(rp, 2, h - 1);
    }
    ox = (t->bw - t->w) / 2;
    oy = (h - t->h) / 2;
    if (oy < 0) oy = 0;
    px = t->pixels + (ULONG)t->state * t->w * t->h;
    for (y = 0; y < t->h && oy + y < t->bh; y++)
        for (x = 0; x < t->w; ) {
            UBYTE c = px[y * t->w + x];
            int run;
            LONG pen;
            if (!c) { x++; continue; }
            for (run = x; run < t->w && px[y * t->w + run] == c; run++) ;
            pen = c == 1 ? bar_text() : c == 2 ? dri->dri_Pens[SHINEPEN] : c == 3 ? dri->dri_Pens[SHADOWPEN]
                : pen_for(t->rgb[(c - 4) & 15]);
            SetAPen(rp, pen);
            RectFill(rp, ox + x, oy + y, ox + run - 1, oy + y);
            x = run;
        }
    see_key(t->win);
}

static int ts_tell(struct ts_icon *t, UWORD kind);
static void rb_publish(void);
static void ts_open_one(struct ts_icon *t)
{
    if (!scr || t->win) return;
    ts_place(t);
    if ((t->win = slot_open(t->x, t->y, t->bw, t->bh))) {
        t->drawn = -1;
        ts_draw(t, 1);
        tray(t->name, t->bw, t->order);
        if (t->version >= 2) ts_tell(t, TSE_PLACE);
        rb_publish();                          /* 0.11: the menu button over it */
    }
}

static void help_close(void)
{
    if (help_w) { CloseWindow(help_w); help_w = NULL; }
    help_for = -1;
}

static void ts_close_one(struct ts_icon *t, int forget_place)
{
    int i;
    for (i = 0; i < TS_ICONS; i++) if (ts[i] == t && help_for == i) help_close();
    if (t->win) { slot_close(t->win); t->win = NULL; rb_publish(); }
    if (forget_place) tray(t->name, 0, t->order);
}

static void ts_drop(int i)
{
    if (!ts[i]) return;
    ts_close_one(ts[i], 1);
    FreeVec(ts[i]);
    ts[i] = NULL;
    if (hover_for == i) hover_for = -1;
}

/* the help under an icon, after the pointer has rested on it a moment (0.9: any words, under any item's box) */
static int help_show(const char *text, WORD right, WORD top, WORD bh)
{
    struct RastPort trp;
    WORD w, h, x, y;
    if (!text[0] || help_w) return 0;
    InitRastPort(&trp);
    SetFont(&trp, dri->dri_Font);
    w = aat_length(&trp, text, strlen(text)) + 12;
    h = dri->dri_Font->tf_YSize + 6;
    if (w > scr->Width) w = scr->Width;
    x = right - w;
    if (x < 0) x = 0;
    y = bar_edge == OM_BAR_BOTTOM ? top - h : top + bh + (bar_edge == OM_BAR_TITLE ? 1 : 0);
    if (y + h > scr->Height) y = scr->Height - h;
    if (!(help_w = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Left, x, WA_Top, y, WA_Width, w, WA_Height, h,
                                  WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE, TAG_DONE)))
        return 0;
    SetFont(help_w->RPort, dri->dri_Font);
    SetAPen(help_w->RPort, dri->dri_Pens[BARBLOCKPEN]);
    RectFill(help_w->RPort, 1, 1, w - 2, h - 2);
    SetAPen(help_w->RPort, dri->dri_Pens[SHADOWPEN]);
    Move(help_w->RPort, 0, 0); Draw(help_w->RPort, w - 1, 0); Draw(help_w->RPort, w - 1, h - 1); Draw(help_w->RPort, 0, h - 1); Draw(help_w->RPort, 0, 0);
    SetAPen(help_w->RPort, dri->dri_Pens[BARDETAILPEN]);
    SetDrMd(help_w->RPort, JAM1);
    aat_box(help_w->RPort, scr, 6, 3, help_w->RPort->TxHeight, text, strlen(text));
    return 1;
}

static void help_open(int i)
{
    struct ts_icon *t = ts[i];
    if (t && t->win && help_show(t->help, t->x + t->bw, t->y, t->bh)) help_for = i;
}

static void cx_help(int k);

/* every tenth of a second while the pointer is on the bar: the light and the help */
static void ts_hover(void)
{
    int i, over = -1;
    for (i = 0; i < TS_ICONS; i++) if (ts[i]) { ts_draw(ts[i], 0); if (ts_over(ts[i])) over = i; }
    if (over < 0 && (i = cx_over()) >= 0) over = OW_CX + i;     /* 0.9: a commodity's title and description */
    draw_cx(0);
    if (over != hover_for) { hover_for = over; hover_ticks = 0; if (help_w) help_close(); }
    else if (over >= 0 && ++hover_ticks == 6) { if (over >= OW_CX) cx_help(over - OW_CX); else help_open(over); }
}

/* tells the program; 0 when its port is gone (it ended without TSC_REMOVE) */
static int ts_tell(struct ts_icon *t, UWORD kind)
{
    struct TaskspaceEvent *e;
    struct MsgPort *p;
    if (t->sbi) {                              /* 0.10: through the library, which checks the item is still there */
        WORD box[4];
        int k = tbs_event(kind);
        if (!k || !SysBarBase) return 1;
        box[0] = t->x; box[1] = t->y; box[2] = t->bw; box[3] = t->bh;
        SB_LockList();
        SB_SendEvent(t->sbi, (ULONG)k, 0, box);
        SB_UnlockList();
        return 1;                              /* gone items leave at the next read of the list */
    }
    if (!t->port[0]) return 1;
    if (!ts_reply || !(e = AllocVec(sizeof *e, MEMF_PUBLIC | MEMF_CLEAR))) return 1;
    e->tse_Message.mn_ReplyPort = ts_reply;
    e->tse_Message.mn_Length = sizeof *e;
    e->tse_Magic = TASKSPACE_MAGIC;
    e->tse_Kind = kind;
    strcpy(e->tse_Name, t->name);
    e->tse_Left = t->x; e->tse_Top = t->y; e->tse_Width = t->bw; e->tse_Height = t->bh;
    Forbid();
    if ((p = FindPort((STRPTR)t->port))) { PutMsg(p, &e->tse_Message); ts_out++; }
    Permit();
    if (!p) FreeVec(e);
    return p != NULL;
}

static void ts_replies(void)
{
    struct Message *m;
    while (ts_reply && (m = GetMsg(ts_reply))) { FreeVec(m); ts_out--; }
}

/* a program's message on GFOpenTitle's port */
static void ts_command(struct TaskspaceMsg *m)
{
    int i;
    if (m->tsm_Message.mn_Length < sizeof *m || m->tsm_Magic != TASKSPACE_MAGIC || m->tsm_Version < 1) { m->tsm_Result = TSR_BADMSG; return; }
    m->tsm_Name[sizeof m->tsm_Name - 1] = 0;
    i = ts_find(m->tsm_Name);
    m->tsm_Result = TSR_OK;
    switch (m->tsm_Command) {
    case TSC_ADD: {
        struct ts_icon *t;
        ULONG n = (ULONG)m->tsm_Width * m->tsm_Height;
        if (!m->tsm_Name[0] || !m->tsm_Pixels || !m->tsm_Width || !m->tsm_Height || m->tsm_Width > TS_MAXW || m->tsm_Height > TS_MAXH
            || !m->tsm_States || m->tsm_States > TS_STATES) { m->tsm_Result = TSR_BADMSG; return; }
        if (i < 0) for (i = 0; i < TS_ICONS && ts[i]; i++) ;
        if (i == TS_ICONS) { m->tsm_Result = TSR_FULL; return; }
        if (ts[i]) ts_close_one(ts[i], 0);
        else if (!(ts[i] = AllocVec(sizeof *t, MEMF_ANY | MEMF_CLEAR))) { m->tsm_Result = TSR_NOMEM; return; }
        t = ts[i];
        memset(t, 0, sizeof *t);
        strcpy(t->name, m->tsm_Name);
        strlcpy(t->help, m->tsm_Help, sizeof t->help);  /* the sender may fill the field: always ended */
        strlcpy(t->port, m->tsm_Port, sizeof t->port);
        t->order = m->tsm_Order;
        t->version = m->tsm_Version;
        t->w = m->tsm_Width; t->h = m->tsm_Height; t->states = m->tsm_States;
        t->state = m->tsm_State < t->states ? m->tsm_State : 0;
        memcpy(t->pixels, m->tsm_Pixels, n * t->states);
        memcpy(t->rgb, m->tsm_RGB, sizeof t->rgb);
        ts_open_one(t);
        cx_sync(1);                            /* 0.9: its commodity leaves the systray */
        break;
    }
    case TSC_SET:
        if (i < 0) { m->tsm_Result = TSR_UNKNOWN; return; }
        if (m->tsm_State < ts[i]->states) ts[i]->state = m->tsm_State;
        m->tsm_Help[sizeof m->tsm_Help - 1] = 0;
        if (m->tsm_Help[0] && strcmp(m->tsm_Help, ts[i]->help)) {
            strcpy(ts[i]->help, m->tsm_Help);
            if (help_for == i) help_close();
        }
        ts_draw(ts[i], 0);
        break;
    case TSC_REMOVE:
        if (i < 0) { m->tsm_Result = TSR_UNKNOWN; return; }
        ts_drop(i);
        cx_sync(1);
        break;
    default:
        m->tsm_Result = TSR_BADMSG;
    }
}

static void port_events(struct MsgPort *port)
{
    struct Message *m;
    while ((m = GetMsg(port))) {
        ts_command((struct TaskspaceMsg *)m);
        ReplyMsg(m);
    }
}

/* GFOpenTitle is ending: each program hears its icon has gone, and has two seconds to reply */
static void ts_end(void)
{
    int i, n;
    for (i = 0; i < TS_ICONS; i++) if (ts[i]) { ts_tell(ts[i], TSE_GONE); ts_drop(i); }
    for (n = 0; n < 40 && ts_out > 0; n++) { Delay(TICKS_PER_SECOND / 20); ts_replies(); }
}

static int ts_count(void)
{
    int i, n = 0;
    for (i = 0; i < TS_ICONS; i++) if (ts[i]) n++;
    return n;
}

/* 0.10: gfopensysbar.library's picture items into the row, beside the taskspace's: new ones opened, changed ones
 * drawn again, gone ones closed. A name the taskspace already has keeps the taskspace's icon. */
static BYTE sb_sig = -1;
static int sb_owner;

static void sb_sync(void)
{
    struct SysBarItem *it;
    int i, changed = 0, help_drop = 0;
    if (!sb_owner) return;
    for (i = 0; i < TS_ICONS; i++) if (ts[i]) ts[i]->seen = 0;
    /* under the lock only the fields are copied; windows are drawn, closed and opened after it */
    SB_LockList();
    for (it = SB_NextItem(NULL); it; it = SB_NextItem(it)) {
        struct ts_icon *t;
        ULONG n = 0;
        int ok = it->si_Pixels && it->si_Width && it->si_Width <= SB_MAXW && it->si_Height && it->si_Height <= SB_MAXH
                 && it->si_States && it->si_States <= SB_STATES;
        int fr, moved = 0, at = ok ? ts_find(it->si_Name) : -1;
        if (ok) n = (ULONG)it->si_Width * it->si_Height * it->si_States;
        for (fr = 0; fr < TS_ICONS && ts[fr]; fr++) ;
        t = at >= 0 ? ts[at] : NULL;
        switch (tbs_plan(at, t && t->sbi, fr < TS_ICONS, ok,
                         t && t->w == it->si_Width && t->order == it->si_Order,
                         t && t->h == it->si_Height && t->states == it->si_States && !memcmp(t->pixels, it->si_Pixels, n)
                         && !memcmp(t->rgb, it->si_RGB, sizeof t->rgb))) {
        case TBS_NEW:
            if (!(t = AllocVec(sizeof *t, MEMF_ANY | MEMF_CLEAR))) break;
            ts[fr] = t;
            strlcpy(t->name, it->si_Name, sizeof t->name);
            t->version = 2;                    /* it hears its place */
            changed = 1;
            /* fall through: its fields, as for a moved one */
        case TBS_MOVE:
            moved = 1;                         /* closed and opened again after the lock, below */
            changed = 1;
            /* fall through */
        case TBS_REDRAW:
            t->w = it->si_Width; t->h = it->si_Height; t->states = it->si_States; t->order = it->si_Order;
            memcpy(t->pixels, it->si_Pixels, n);
            memcpy(t->rgb, it->si_RGB, sizeof t->rgb);
            t->drawn = -1;
            /* fall through */
        case TBS_KEEP:
            if (tbs_rebind((void **)&t->sbi, (void **)&t->task, it, it->si_Task)) changed = 1;
            t->seen = moved ? 2 : 1;
            if (strcmp(t->help, it->si_Help)) {
                strlcpy(t->help, it->si_Help, sizeof t->help);
                if (at >= 0 && help_for == at) help_drop = 1;
            }
            t->state = it->si_State < t->states ? it->si_State : 0;
            break;
        default:                               /* TBS_SKIP, TBS_FULL */
            break;
        }
    }
    SB_UnlockList();
    /* opening a window sends SBE_PLACE, and SB_LockList there may take out items */
    if (help_drop) help_close();
    for (i = 0; i < TS_ICONS; i++) {
        if (!ts[i]) continue;
        switch (tbs_after(ts[i]->sbi != NULL, ts[i]->seen, ts[i]->win != NULL)) {
        case TBA_DROP: ts_drop(i); changed = 1; break;
        case TBA_REOPEN: ts_close_one(ts[i], 1); /* fall through */
        case TBA_OPEN: ts_open_one(ts[i]); break;
        case TBA_DRAW: ts_draw(ts[i], 0); break;
        default: break;
        }
    }
    if (changed) cx_sync(1);                   /* a commodity with an item leaves the systray, or comes back */
}

static void sb_start(void)
{
    if (!(SysBarBase = OpenLibrary((STRPTR)SYSBARNAME, SYSBAR_VERSION))) return;
    if ((sb_sig = AllocSignal(-1)) < 0) return;
    sb_owner = SB_Own((ULONG)sb_sig) == SBR_OK;
    sb_sync();
}

static ULONG sb_mask(void) { return sb_owner ? 1UL << sb_sig : 0; }

/* the items stay in the library for the next owner; only our windows go (ts_end) */
static void sb_stop(void)
{
    if (sb_owner) SB_Disown();
    sb_owner = 0;
    if (sb_sig >= 0) FreeSignal(sb_sig);
    sb_sig = -1;
    if (SysBarBase) CloseLibrary(SysBarBase);
    SysBarBase = NULL;
}

static int ts_is(struct Window *w)
{
    int i;
    if (w && w == help_w) return 1;
    for (i = 0; i < TS_ICONS; i++) if (ts[i] && ts[i]->win == w && w) return 1;
    if (w && w == row_w) return 1;
    return 0;
}

/* ---- 0.9: the systray: an item for each commodity --------------------------------------------------- */

static int cx_find(const char *name)
{
    int i;
    for (i = 0; i < cx_all_n; i++) if (!strcmp(cx_all[i].name, name)) return i;
    return -1;
}

/* a commodity with taskspace icons of its own (GFOpenSpeaker, GFOpenNetwork) is shown by them, not again */
static int cx_has_icons(struct Task *task)
{
    int i, has = 0;
    Forbid();
    for (i = 0; i < TS_ICONS && !has; i++) {
        struct MsgPort *p;
        if (ts[i] && ts[i]->sbi && ts[i]->task == task) has = 1;          /* 0.10 */
        else if (ts[i] && ts[i]->port[0] && (p = FindPort((STRPTR)ts[i]->port)) && (struct Task *)p->mp_SigTask == task) has = 1;
    }
    Permit();
    return has;
}

/* the brokers now, into cx_all and the systray's cx_tray: 1 when what the systray shows changed */
static int cx_read(void)
{
    static struct cx_item now[CX_ALL];
    struct List l;
    struct Node *n;
    int count = 0, i, tray_was[CX_MAX], n_was = cx_n, changed;
    if (!CxBase) return 0;
    l.lh_Head = (struct Node *)&l.lh_Tail; l.lh_Tail = NULL; l.lh_TailPred = (struct Node *)&l.lh_Head; l.lh_Type = 0;
    cx_CopyBrokerList(&l);
    for (n = l.lh_Head; n->ln_Succ && count < CX_ALL; n = n->ln_Succ) {
        struct BrokerCopy *b = (struct BrokerCopy *)n;
        struct cx_item *e = &now[count++];
        memset(e, 0, sizeof *e);
        cx_copy(e->name, b->bc_Name, CBD_NAMELEN);
        cx_copy(e->title, b->bc_Title, CBD_TITLELEN);
        cx_copy(e->descr, b->bc_Descr, CBD_DESCRLEN);
        e->flags = b->bc_Flags & (COF_ACTIVE | COF_SHOW_HIDE);
        e->task = (struct Task *)b->bc_Task;
    }
    cx_FreeBrokerList(&l);
    memcpy(tray_was, cx_tray, sizeof tray_was);
    changed = count != cx_all_n || memcmp(now, cx_all, count * sizeof now[0]);
    memcpy(cx_all, now, count * sizeof now[0]);
    cx_all_n = count;
    cx_n = 0;
    for (i = 0; i < count && cx_n < CX_MAX; i++)
        if (tbo_in_strip(!strcmp(now[i].name, TB_PORT), (now[i].flags & COF_ACTIVE) != 0, cx_has_icons(now[i].task)))
            cx_tray[cx_n++] = i;                /* 0.11: a disabled one is off the bar, in Commodities... */
    if (changed) rb_publish();                  /* 0.11.1: which picture items have a broker for the menu button */
    return changed || cx_n != n_was || memcmp(tray_was, cx_tray, cx_n * sizeof cx_tray[0]);
}

static void cx_place(void)
{
    WORD bh = scr->BarHeight + 1;
    int n, off = tray_before("Commodities", CX_ORDER, &n);
    xw = cx_n * TRAY_CELL;
    switch (bar_edge) {
    case OM_BAR_BOTTOM: xh = bh; xx = scr->Width - xw - off; xy = scr->Height - bh; break;
    case OM_BAR_LEFT:   xh = bh; xx = 0; xy = scr->Height - bh * (n + 1); break;
    case OM_BAR_RIGHT:  xh = bh; xx = scr->Width - xw; xy = scr->Height - bh * (n + 1); break;
    case OM_BAR_TOP:    xh = bh; xx = scr->Width - 2 * scr->BarHeight - xw - off; xy = 0; break;
    default:            xh = scr->BarHeight; xx = scr->Width - bar_end() - xw - off; xy = 0; break;
    }
}

/* a commodity's letter: its name's first, after "GFOpen" (GFOpenWindows: W) */
static char cx_letter(const char *name)
{
    char c;
    if (!strncmp(name, "GFOpen", 6) && name[6]) name += 6;
    else if (!strncmp(name, "Open", 4) && name[4]) name += 4;
    c = name[0] ? name[0] : '?';
    return c >= 'a' && c <= 'z' ? c - 32 : c;
}

/* each item a letter in a rounded box (no box while the commodity is inactive), on the systray's band; raised under
 * the pointer, a deeper shade while its menu is open */
static void draw_cx(int force)
{
    struct RastPort *rp;
    oldraw *d;
    oldraw_colour ink;
    int hot = cx_over(), open = menu_w && menu_owner >= OW_CX ? menu_owner - OW_CX : -1, state = (hot + 1) * 64 + open + 1, k;
    WORD size;
    if (!cx_w || !slot_of(cx_w) || (!force && state == cx_drawn)) return;
    cx_drawn = state;
    rp = cx_w->RPort;
    bar_paint(cx_w, 0, 0, xw, xh);
    band(cx_w, 0, xw, xh, BAND_SHADE);
    size = cog_size();
    d = od_open(cx_w);
    ink = pen_colour(bar_text());
    for (k = 0; k < cx_n; k++) {
        const struct cx_item *c = &cx_all[cx_tray[k]];
        WORD x = k * TRAY_CELL, ox = x + (TRAY_CELL - size) / 2, oy = (xh - (bar_edge == OM_BAR_TITLE ? 1 : 0) - size) / 2, lw;
        char letter = cx_letter(c->name);
        if (oy < 0) oy = 0;
        if (open == k) band(cx_w, x, TRAY_CELL, xh, BAND_HOT);
        else if (hot == k && d) raised(d, x, TRAY_CELL, xh);
        if ((c->flags & COF_ACTIVE) && d) {
            struct tb_box b[8];
            od_boxes(d, b, tb_rbox_boxes(ox, oy, size, b, 8), ink);
        }
        SetAPen(rp, bar_text());                /* the letter: the bar's text, as its other words */
        SetDrMd(rp, JAM1);
        lw = aat_length(rp, &letter, 1);
        aat_box(rp, scr, ox + (size - lw + 1) / 2, oy, size, &letter, 1);
    }
    if (d) oldraw_free(d);
    see_key(cx_w);
}

/* the systray after the brokers changed: its slot opened, moved or closed, and the row placed again */
static void place_all(void);
static void cx_sync(int read)
{
    if (!scr || (read && !cx_read())) return;
    if (help_for >= OW_CX) help_close();
    if (!cx_n) {
        if (cx_w) { tb_rect_set(&rb_cx, 0, 0, 0, 0); slot_close(cx_w); cx_w = NULL; tray("Commodities", 0, CX_ORDER); place_all(); }
        return;
    }
    cx_place();
    if (!cx_w) {
        if (!(cx_w = slot_open(xx, xy, xw, xh))) return;
        tray("Commodities", xw, CX_ORDER);
        place_all();
    } else if (cx_w->Width != xw || cx_w->LeftEdge != xx || cx_w->TopEdge != xy || cx_w->Height != xh) {
        slot_move(cx_w, xx, xy, xw, xh);
        tray("Commodities", xw, CX_ORDER);
        place_all();
    }
    cx_drawn = -1;
    draw_cx(1);
}

static void cx_help(int k)
{
    char t[CBD_TITLELEN + CBD_DESCRLEN + 4];
    const struct cx_item *c;
    if (k < 0 || k >= cx_n) return;
    c = &cx_all[cx_tray[k]];
    snprintf(t, sizeof t, "%s%s%s", c->title[0] ? c->title : c->name, c->descr[0] ? " - " : "", c->descr);
    if (help_show(t, xx + (k + 1) * TRAY_CELL, xy, xh)) help_for = OW_CX + k;
}

/* the commodity's task has a window open: on Workbench's screen, or any other */
static int cx_shown(struct Task *task)
{
    struct Screen *s;
    struct Window *w;
    int found = 0;
    ULONG lock;
    if (!task) return 0;
    lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && !found; s = s->NextScreen)
        for (w = s->FirstWindow; w && !found; w = w->NextWindow)
            if (w->UserPort && (struct Task *)w->UserPort->mp_SigTask == task) found = 1;
    UnlockIBase(lock);
    return found;
}

/* a click on Prefs, Power or a systray item (menu: with the menu button) */
static void owner_click(int owner, int menu)
{
    help_close();
    if (owner == OW_PREFS) menu_open(MK_PREFS, owner, NULL);
    else if (owner == OW_POWER) menu_open(MK_POWER, owner, NULL);
    else if (owner >= OW_TS && owner < OW_TS + TS_ICONS) {   /* 0.11: a picture item: its program's broker's menu */
        const void *tasks[CX_ALL];
        int k, b;
        struct ts_icon *t = ts[owner - OW_TS];
        for (k = 0; k < cx_all_n; k++) tasks[k] = cx_all[k].task;
        if (t && (b = tbo_broker_of(tasks, cx_all_n, ts_task(t))) >= 0) menu_open(MK_CX, owner, cx_all[b].name);
    }
    else if (owner >= OW_CX && owner - OW_CX < cx_n) {
        const struct cx_item *c = &cx_all[cx_tray[owner - OW_CX]];
        if (!menu && (c->flags & COF_SHOW_HIDE))   /* its window shown, or hidden when it is */
            cx_BrokerCommand(c->name, cx_shown(c->task) ? CXCMD_DISAPPEAR : CXCMD_APPEAR);
        else menu_open(MK_CX, owner, c->name);
    }
}

/* 0.9.1: the menu button on Prefs, Power and the systray. The row never stays the active window, and Intuition
 * gives the menu button to the active one (GFOpenBench's menus), so the row never saw it. A commodity of our own
 * (CxCustom, in input.device's task) takes the press and its release while the pointer is on the cog's or the
 * systray's slot, and signals the loop. Nothing else is touched: every other event goes on. */
static struct Screen *volatile rb_scr;         /* the bar's screen while the windows are open; NULL: take nothing */
static struct Task *rb_task;
static BYTE rb_sig = -1;
static volatile UBYTE rb_held;                 /* we took the press: the release is ours too */
static CxObj *rb_broker;
static struct MsgPort *rb_port;


static void rb_publish(void)
{
    Forbid();
    if (cog_w && slot_of(cog_w)) tb_rect_set(&rb_cog, cog_w->LeftEdge, cog_w->TopEdge, cog_w->Width, cog_w->Height);
    else tb_rect_set(&rb_cog, 0, 0, 0, 0);
    if (cx_w && slot_of(cx_w)) tb_rect_set(&rb_cx, cx_w->LeftEdge, cx_w->TopEdge, cx_w->Width, cx_w->Height);
    else tb_rect_set(&rb_cx, 0, 0, 0, 0);
    {
        int i;
        const void *tasks[CX_ALL];
        int k;
        for (k = 0; k < cx_all_n; k++) tasks[k] = cx_all[k].task;
        /* only an item whose program has a broker: on any other the menu button goes on to the screen's menus (Marcus) */
        for (i = 0; i < TS_ICONS; i++)
            if (ts[i] && ts[i]->win && slot_of(ts[i]->win) && tbo_broker_of(tasks, cx_all_n, ts_task(ts[i])) >= 0)
                tb_rect_set(&rb_ts[i], ts[i]->x, ts[i]->y, ts[i]->bw, ts[i]->bh);
            else tb_rect_set(&rb_ts[i], 0, 0, 0, 0);
    }
    Permit();
}

static void rb_clear(void)
{
    Forbid();
    rb_scr = NULL;
    tb_rect_set(&rb_cog, 0, 0, 0, 0);
    tb_rect_set(&rb_cx, 0, 0, 0, 0);
    {
        int i;
        for (i = 0; i < TS_ICONS; i++) tb_rect_set(&rb_ts[i], 0, 0, 0, 0);
    }
    Permit();
}

static void rb_func(CxMsg *m, CxObj *co)
{
    struct InputEvent *ie = (struct InputEvent *)CxMsgData(m);
    struct Screen *s = rb_scr;
    (void)co;
    if (!ie || ie->ie_Class != IECLASS_RAWMOUSE) return;
    if (ie->ie_Code == IECODE_RBUTTON) {
        if (!s || IntuitionBase->FirstScreen != s) return;
        if (!tb_rect_hit(&rb_cog, s->MouseX, s->MouseY) && !tb_rect_hit(&rb_cx, s->MouseX, s->MouseY)) {
            int i, on = 0;                     /* 0.11: or a picture item */
            for (i = 0; i < TS_ICONS && !on; i++) on = tb_rect_hit(&rb_ts[i], s->MouseX, s->MouseY);
            if (!on) return;
        }
        rb_held = 1;
        DisposeCxMsg(m);
        Signal(rb_task, 1UL << rb_sig);
    } else if (ie->ie_Code == (IECODE_RBUTTON | IECODE_UP_PREFIX) && rb_held) {
        rb_held = 0;
        DisposeCxMsg(m);
    }
}

static void rb_start(void)
{
    struct NewBroker nb;
    CxObj *c;
    if (!CxBase || !(rb_port = CreateMsgPort())) return;
    if ((rb_sig = AllocSignal(-1)) < 0) { DeleteMsgPort(rb_port); rb_port = NULL; return; }
    rb_task = FindTask(NULL);
    memset(&nb, 0, sizeof nb);
    nb.nb_Version = NB_VERSION;
    nb.nb_Name = (STRPTR)"GFOpenTitle";        /* cx_read leaves it out of the systray */
    nb.nb_Title = (STRPTR)"GFOpenTitle";
    nb.nb_Descr = (STRPTR)"The title bar's menu button";
    nb.nb_Unique = NBU_UNIQUE;
    nb.nb_Pri = 50;
    nb.nb_Port = rb_port;
    if (!(rb_broker = CxBroker(&nb, NULL))) return;
    if (!(c = CxCustom(rb_func, 0))) { DeleteCxObjAll(rb_broker); rb_broker = NULL; return; }
    AttachCxObj(rb_broker, c);
    if (CxObjError(rb_broker)) { DeleteCxObjAll(rb_broker); rb_broker = NULL; return; }
    ActivateCxObj(rb_broker, 1);
}

static void rb_stop(void)
{
    struct Message *m;
    rb_clear();
    if (rb_broker) DeleteCxObjAll(rb_broker);
    rb_broker = NULL;
    if (rb_port) { while ((m = GetMsg(rb_port))) ReplyMsg(m); DeleteMsgPort(rb_port); }
    rb_port = NULL;
    if (rb_sig >= 0) FreeSignal(rb_sig);
    rb_sig = -1;
}

/* the loop's part: Exchange's commands answered (nothing to do), and a press on an item opens its menu */
static void rb_events(ULONG sig)
{
    struct Message *m;
    if (rb_port) while ((m = GetMsg(rb_port))) ReplyMsg(m);
    if (rb_sig < 0 || !(sig & (1UL << rb_sig)) || !scr) return;
    {
        int o = owner_at_pointer();
        if (menu_w) menu_close(1);
        else if (o) owner_click(o, 1);
    }
}

static ULONG rb_mask(void)
{
    return (rb_sig >= 0 ? 1UL << rb_sig : 0) | (rb_port ? 1UL << rb_port->mp_SigBit : 0);
}

/* a commodity's words, in a requester ("%" in them stays as it is) */
static void cx_info(const struct cx_item *c)
{
    struct EasyStruct es;
    const char *args[2];
    es.es_StructSize = sizeof es;
    es.es_Flags = 0;
    es.es_Title = (UBYTE *)(c->title[0] ? c->title : c->name);
    es.es_TextFormat = (UBYTE *)"%s\n\n%s";
    es.es_GadgetFormat = (UBYTE *)"OK";
    args[0] = c->title[0] ? c->title : c->name;
    args[1] = c->descr[0] ? c->descr : "No description.";
    EasyRequestArgs(NULL, &es, NULL, (APTR)args);
}

/* a row of a commodity's menu, or of "All commodities..." */
static void cx_pick(int r)
{
    int act = xrow[r].act, owner = menu_owner, b;
    char name[CBD_NAMELEN];
    if (act == XA_ALLCX) { menu_close(0); menu_open(MK_CXALL, owner, NULL); return; }
    if (act == XA_BROKER) { strcpy(name, xrow[r].label); menu_close(0); menu_open(MK_CX, owner, name); return; }
    strcpy(name, menu_cx);
    menu_close(1);
    if ((b = cx_find(name)) < 0) return;          /* it ended meanwhile */
    switch (act) {
    case XA_SHOW:   cx_BrokerCommand(name, CXCMD_APPEAR); break;
    case XA_HIDE:   cx_BrokerCommand(name, CXCMD_DISAPPEAR); break;
    case XA_ACTIVE: cx_BrokerCommand(name, (cx_all[b].flags & COF_ACTIVE) ? CXCMD_DISABLE : CXCMD_ENABLE); break;
    case XA_INFO:   cx_info(&cx_all[b]); break;
    case XA_REMOVE: cx_BrokerCommand(name, CXCMD_KILL); break;
    }
    if (act == XA_ACTIVE || act == XA_REMOVE) { Delay(TICKS_PER_SECOND / 10); cx_sync(1); }   /* seen at once, not in 2 s */
}

/* ---- 0.7.5: the see-through bar ------------------------------------------------------------------
 * The bar is Intuition's: it draws the background, the title and the depth gadget into the screen's BarLayer with
 * pens, and nothing tells a program when it draws them again. GFOpenLayers can make a layer translucent
 * (SetLayerOpacity) or let its own alpha count (LAYERLOOK_ALPHA), but either from outside would fade the title and
 * the depth gadget with the background, or lose them (pens leave a 32-bit pixel's alpha 0). So the bar's
 * background opacity is asked of Intuition, with a call proposed to Team 4 on 25 October 2026 and not in
 * Intuition yet:
 *
 *   IntuitionControlA(screen, tags) with TB_ICTRL_BAROPACITY: ti_Data 0-255 is how solid the bar's background (its
 *   fill or gradient and its lines) is; the title, its shadow and the depth gadget stay opaque, and 255 is the bar
 *   as it is today. ~0 asks and changes nothing. It answers 0x100 | the opacity before when the bar takes it, and
 *   0 when it can't. A change composes the bar again and draws no more than the bar.
 *
 * Today's Intuition answers 0 for a screen (its IntuitionControlA takes windows only), so see_screen() says no
 * and the bar stays as it was. The rest is ready for it: the setting, the fade and its timing, and our windows.
 *
 * See-through only where all of these hold, checked when the windows open (and the last every 2 s):
 *   - the Workbench screen's own title bar (not GFOpenMenus' bar at an edge), shown;
 *   - GFOpenLayers composites the screen (47.102's LAYERS_COMPOSE_WHY answers 0): RTG, not 8-bit, the switch on;
 *   - 32 bits a pixel, where a layer's own alpha counts (our windows' glyphs stay opaque on the clear bar);
 *   - Intuition takes TB_ICTRL_BAROPACITY;
 *   - a layer behind the bar covers it all, so there is something to see. GFOpenBench's backdrop window starts
 *     under the bar (gfopenbench desktop/openbench.c), so with it alone there is only the compositor's black.
 * Elsewhere (AGA, PAL, 8 or 16-bit, a screen classic layers draw) nothing here runs and the bar is solid.
 *
 * The cost at rest is today's: no extra wake. Each wake the loop already has (a tenth of a second while the pointer
 * is on the bar, half a second otherwise) also looks at the pointer's height, at GFOpenTitle's own lists and at the
 * menu button (input.device's PeekQualifier): no calls into the screen. Only while the bar fades does the loop
 * wake every TBF_STEP_MS (40 ms): at most 4 steps in, then one after the grace, then 8 steps out; each is one
 * IntuitionControlA. */
#define TB_ICTRL_BAROPACITY (TAG_USER + 0x41430003UL)  /* proposed: after openintuition's WA_Opacity (+1), WA_AlphaSurface (+2) */
static int see_ok;                             /* this screen can (see_screen), found when the windows open */
static tb_fade see_fade;
static ULONG see_s, see_m;                     /* when the fade last stepped (CurrentTime) */
static struct IOStdReq input_io;               /* input.device while see-through: PeekQualifier, for the menu button */

static ULONG bar_opacity(ULONG level)
{
    struct TagItem t[2];
    t[0].ti_Tag = TB_ICTRL_BAROPACITY;
    t[0].ti_Data = level;
    t[1].ti_Tag = TAG_DONE;
    t[1].ti_Data = 0;
    return IntuitionControlA(scr, t);
}

/* 0.7.8: the logo as the screen's own (GFOpenIntuition's SA_TitleIcon, through IntuitionControlA): 1 when Intuition
 * took it (or let it go, on == 0); 0 from an Intuition without it, which keeps the logo window */
#define TB_SA_TITLEICON (TAG_USER + 0x41430004UL)
struct tb_title_icon { UWORD w, h; const ULONG *argb; };
static const struct tb_title_icon tb_icon = { LOGO_BIG_W, LOGO_BIG_H, logo_big };

static ULONG bar_icon(int on)
{
    struct TagItem t[2];
    if (!scr || IntuitionBase->LibNode.lib_Version < 48) return 0;   /* GFOpenIntuition only */
    t[0].ti_Tag = TB_SA_TITLEICON;
    t[0].ti_Data = on ? (ULONG)&tb_icon : 0;
    t[1].ti_Tag = TAG_DONE;
    t[1].ti_Data = 0;
    return IntuitionControlA(scr, t);
}

static int see_screen(void)
{
    ULONG why;
    if (!scr || bar_edge != OM_BAR_TITLE || !(scr->Flags & SHOWTITLE) || !scr->BarLayer || !CyberGfxBase) return 0;
    if (LayersBase->lib_Version != 47 || LayersBase->lib_Revision < 102 || LayersBase->lib_NegSize < 258) return 0;   /* 0.7.6: 47.106's table is 258 bytes */
    {   /* ComposeLayerInfo(li, NULL, LAYERS_COMPOSE_WHY) (-246, GFOpenLayers 47.102): 0 while it composites. As
         * GFOpenPrefs' Windows page asks it; A6 saved by hand */
        register struct Library *d1 __asm("d1") = LayersBase;
        register void *a0 __asm("a0") = &scr->LayerInfo;
        register void *a1 __asm("a1") = NULL;
        register ULONG d0 __asm("d0") = 2;
        __asm__ volatile("move.l %%a6,-(%%sp)\n\tmove.l %%d1,%%a6\n\tjsr -246(%%a6)\n\tmove.l (%%sp)+,%%a6"
                         : "+r"(d0), "+r"(a0), "+r"(a1), "+r"(d1) : : "cc", "memory");
        why = d0;
    }
    if (why != 0) return 0;
    if (GetCyberMapAttr(scr->RastPort.BitMap, CYBRMATTR_BPPIX) != 4) return 0;
    /* a composited screen means AmiCradle's own Intuition (only it asks layers to composite), so the call is there */
    if (IntuitionBase->LibNode.lib_Version < 47) return 0;
    return bar_opacity(~0UL) != 0;
}

/* a layer behind the bar that covers it all: something to see through it */
static int see_backed(void)
{
    struct Layer *l;
    int ok = 0;
    LockLayerInfo(&scr->LayerInfo);
    for (l = scr->BarLayer->back; l && !ok; l = l->back)
        ok = l->bounds.MinX <= 0 && l->bounds.MinY <= 0 && l->bounds.MaxX >= scr->Width - 1 && l->bounds.MaxY >= scr->BarHeight;
    UnlockLayerInfo(&scr->LayerInfo);
    return ok;
}

/* the bar is wanted solid: the pointer on it, our list or panel open, or the menu button held */
static tb_ptr see_ptr;                         /* 0.7.7: where the pointer was at the first look, and whether it moved */

static int see_hot(void)
{
    if (menu_w || pop_w) return 1;
    /* 0.7.7: the pointer counts only once it has moved since GFOpenTitle first looked (tbp_on_bar): after a boot it
     * rests at the top left corner, on the bar */
    if (IntuitionBase->FirstScreen == scr
        && tbp_on_bar(&see_ptr, scr->MouseX, scr->MouseY, scr->MouseY >= 0 && scr->MouseY <= scr->BarHeight)) return 1;
    if (input_io.io_Device) {
        struct Library *InputBase = (struct Library *)input_io.io_Device;   /* PeekQualifier's base, as GFOpenBench has it */
        (void)InputBase;
        if (PeekQualifier() & IEQUALIFIER_RBUTTON) return 1;
    }
    return 0;
}

/* 0.8: the row's background and every item drawn again (the row was made again, or the keying changed) */
static void row_redraw(void)
{
    int i;
    if (!row_w) return;
    bar_paint(row_w, 0, 0, rw, rh);
    see_key(row_w);
    /* 0.9: only slots still on. slot_close() lays the row out again before its item forgets the slot, and a row made
     * again draws every item: one whose bitmap was just freed drew into freed memory (lab, 12 November 2026: the
     * machine stopped when the memory came back on, the clock's slot at the row's left end) */
    if (clock_w && slot_of(clock_w)) { shown[0] = 0; draw_clock(1); }
    if (net_w && slot_of(net_w)) { net_drawn = -100; draw_net(1); }
    if (cog_w && slot_of(cog_w)) { cog_drawn = -1; draw_cog(1); }
    if (cx_w && slot_of(cx_w)) { cx_drawn = -1; draw_cx(1); }
    for (i = 0; i < TS_ICONS; i++) if (ts[i] && ts[i]->win && slot_of(ts[i]->win)) { ts[i]->drawn = -1; ts_draw(ts[i], 1); }
}

static void see_looks(ULONG on)
{
    struct Window *const ws[2] = { logo_w, row_w };   /* 0.9: the clock is in the row */
    int i;
    for (i = 0; i < 2; i++) if (ws[i]) layer_look(ws[i]->WLayer, SEE_LOOK_ALPHA, on);
}

static void see_rekey(void)
{
    if (logo_w) see_key(logo_w);
    row_redraw();
}

/* solid, as before: our windows' colours are as they were drawn, and their alpha no longer counts */
static void see_off(void)
{
    if (!see_live) return;
    see_live = 0;
    bar_opacity(255);
    see_looks(0);
    see_level = 255;
    see_tray_bg = -1;
    if (input_io.io_Device) CloseDevice((struct IORequest *)&input_io);
    input_io.io_Device = NULL;
}

/* the bar at level; our windows clear before it fades, and opaque once it is solid */
static void see_apply(int level)
{
    int bg = level >= 255 ? 255 : 0;
    if (!bg && see_tray_bg != 0) { see_tray_bg = 0; see_rekey(); }
    if (level != see_level) {
        if (!bar_opacity((ULONG)level)) { see_off(); see_ok = 0; return; }    /* Intuition no longer takes it */
        see_level = level;
    }
    if (bg && see_tray_bg != 255) { see_tray_bg = 255; see_rekey(); }
}

static void see_on(void)
{
    memset(&input_io, 0, sizeof input_io);
    if (OpenDevice((STRPTR)"input.device", 0, (struct IORequest *)&input_io, 0)) input_io.io_Device = NULL;
    see_live = 1;
    see_level = 255;                           /* as the bar is */
    tbf_init(&see_fade, prefs.see_opacity * 255 / 100, see_hot());
    CurrentTime(&see_s, &see_m);
    see_tray_bg = see_fade.level >= 255 ? 255 : 0;
    see_rekey();                               /* the alpha in first: it counts once the look says so, with no flash */
    see_looks(1);
    see_apply(see_fade.level);
}

/* fresh: the windows have just opened (the screen, the setting or GFOpenMenus' bar may have changed) */
static void see_check(int fresh)
{
    int want;
    if (fresh) see_ok = prefs.seethrough && see_screen();
    want = see_ok && prefs.seethrough && see_backed();
    if (want && !see_live) see_on();
    else if (!want && see_live) see_off();
}

/* each wake: the fade's next step */
static void see_tick(void)
{
    ULONG s, m;
    LONG ds, dt;
    if (!see_live) return;
    CurrentTime(&s, &m);
    ds = (LONG)(s - see_s);
    dt = ds < 0 ? 0 : ds > 1 ? TBF_DT_MAX : ds * 1000 + ((LONG)m - (LONG)see_m) / 1000;
    see_s = s;
    see_m = m;
    see_apply(tbf_step(&see_fade, (int)(dt < 0 ? 0 : dt), see_hot()));
}

static void close_all(void)
{
    rb_clear();                                /* 0.9.2: the menu button left alone before any slot closes */
    see_off();                                 /* 0.7.5: the bar solid again while the screen is still ours */
    if (clock_w) { slot_close(clock_w); clock_w = NULL; tray("Clock", 0, CLOCK_ORDER); }
    if (pop_w) { CloseWindow(pop_w); pop_w = NULL; }
    if (net_w) { slot_close(net_w); net_w = NULL; tray("Network", 0, NET_ORDER); }
    if (menu_w) { CloseWindow(menu_w); menu_w = NULL; }
    if (cog_w) { slot_close(cog_w); cog_w = NULL; tray("Cog", 0, COG_ORDER); }
    if (cx_w) { slot_close(cx_w); cx_w = NULL; tray("Commodities", 0, CX_ORDER); }   /* 0.9 */
    if (logo_w) { CloseWindow(logo_w); logo_w = NULL; }
    {
        int i;
        help_close();
        for (i = 0; i < TS_ICONS; i++) if (ts[i]) ts_close_one(ts[i], 1);
        if (row_w) { CloseWindow(row_w); row_w = NULL; }
    }
    if (border_set) { prefs.border_black = 0; apply_border(); }
    free_pens();
    pt_free();
    bp_release();
    if (dri) { FreeScreenDrawInfo(scr, dri); dri = NULL; }
    if (icon_in_bar) { bar_icon(0); icon_in_bar = 0; }   /* 0.7.8: the screen's logo goes with us */
    if (scr) { UnlockPubScreen(NULL, scr); scr = NULL; }
}

static WORD made_bh;                           /* 0.6.11: the bar's height the windows were made for */

static int open_all(void)
{
    if (!(scr = LockPubScreen((STRPTR)"Workbench"))) return 0;
    made_bh = scr->BarHeight;
    if (!(dri = GetScreenDrawInfo(scr))) { close_all(); return 0; }
    icon_in_bar = prefs.logo && bar_icon(1) == 1;   /* 0.7.8 */
    bar_edge = read_bar_edge();
    bar_sample_now();
    pt_load(scr);
    if (prefs.clock || prefs.memory) {         /* 0.9: the memory and the time, an item of the row */
        place();
        if ((clock_w = slot_open(cx, cy, cw, ch))) {
            shown[0] = 0;
            draw_clock(1);
            tray("Clock", cw, CLOCK_ORDER);
        }
    }
    if (prefs.network && !net_elsewhere()) net_open();   /* 0.8 */
    cog_open();                                /* 0.7: always (Dale, 17 Oct 2026); prefs.cog is read no more */
    {
        int i;
        for (i = 0; i < TS_ICONS; i++) if (ts[i]) ts_open_one(ts[i]);
    }
    cx_read();                                 /* 0.9: the systray, then everything where the tray now says */
    cx_sync(0);
    place_all();
    apply_border();
    see_check(1);                              /* 0.7.5 */
    return 1;
}

/* ---- 0.7: the cog on the other public screens -------------------------------------------------- */

#define OS_MAX 4
static struct { struct Screen *s; struct Window *w; WORD bh, gw; char name[MAXPUBSCREENNAME + 1]; } os_cog[OS_MAX];

/* another window than ours on s: the screen is in use, our cog may stay */
static int os_used(struct Screen *s, struct Window *ours)
{
    struct Window *w;
    int n = 0;
    ULONG lock = LockIBase(0);
    for (w = s->FirstWindow; w; w = w->NextWindow) if (w != ours) n++;
    UnlockIBase(lock);
    return n > 0;
}

static void os_draw(int i)
{
    struct DrawInfo *d = GetScreenDrawInfo(os_cog[i].s);
    struct RastPort *rp = os_cog[i].w->RPort;
    WORD size = os_cog[i].bh - 2 >= 13 ? 13 : os_cog[i].bh - 2 >= 11 ? 11 : 9, ox, oy, x, y;
    const char *const *rows = size == 13 ? cog13 : size == 11 ? cog11 : cog9;
    if (!d) return;
    SetAPen(rp, d->dri_Pens[BARBLOCKPEN]);
    RectFill(rp, 0, 0, os_cog[i].gw - 1, os_cog[i].bh - 1);
    SetAPen(rp, d->dri_Pens[BARDETAILPEN]);
    ox = (os_cog[i].gw - size) / 2;
    oy = (os_cog[i].bh - size) / 2 > 0 ? (os_cog[i].bh - size) / 2 : 0;
    for (y = 0; y < size && oy + y < os_cog[i].bh; y++)
        for (x = 0; x < size; ) {
            WORD run;
            if (rows[y][x] != '#') { x++; continue; }
            for (run = x; run < size && rows[y][run] == '#'; run++) ;
            RectFill(rp, ox + x, oy + y, ox + run - 1, oy + y);
            x = run;
        }
    FreeScreenDrawInfo(os_cog[i].s, d);
}

static void os_close(int i)
{
    if (os_cog[i].w) CloseWindow(os_cog[i].w);
    os_cog[i].w = NULL;
    os_cog[i].s = NULL;
    os_cog[i].name[0] = 0;
}

/* the public screens that show a title bar, but the Workbench's: a cog on each while it has a window of its own */
static void os_poll(int on)
{
    struct List *l;
    struct PubScreenNode *n;
    struct Screen *seen[OS_MAX];
    char names[OS_MAX][MAXPUBSCREENNAME + 1];
    int k = 0, i, j;
    if (on && (l = LockPubScreenList())) {
        for (n = (struct PubScreenNode *)l->lh_Head; n->psn_Node.ln_Succ && k < OS_MAX;
             n = (struct PubScreenNode *)n->psn_Node.ln_Succ) {
            struct Screen *s = n->psn_Screen;
            if (!s || s == scr || (s->Flags & SCREENTYPE) == WBENCHSCREEN || !(s->Flags & SHOWTITLE) || s->BarHeight < 7 ||
                (n->psn_Flags & PSNF_PRIVATE) || !n->psn_Node.ln_Name)
                continue;
            seen[k] = s;
            strncpy(names[k], n->psn_Node.ln_Name, MAXPUBSCREENNAME);
            names[k][MAXPUBSCREENNAME] = 0;
            k++;
        }
        UnlockPubScreenList();
    }
    for (i = 0; i < OS_MAX; i++) {
        int there = 0;
        if (!os_cog[i].s) continue;
        for (j = 0; j < k; j++) if (seen[j] == os_cog[i].s && !strcmp(names[j], os_cog[i].name)) there = 1;
        if (!there || !os_used(os_cog[i].s, os_cog[i].w) || os_cog[i].s->BarHeight != os_cog[i].bh) os_close(i);
    }
    for (j = 0; j < k; j++) {
        struct Screen *s;
        int f = -1;
        for (i = 0; i < OS_MAX; i++) { if (os_cog[i].s == seen[j]) break; if (!os_cog[i].s && f < 0) f = i; }
        if (i < OS_MAX || f < 0 || !(s = LockPubScreen((STRPTR)names[j]))) continue;
        if (s == seen[j] && os_used(s, NULL)) {
            WORD bh = s->BarHeight, gw = TRAY_CELL;
            os_cog[f].w = OpenWindowTags(NULL, WA_PubScreen, (ULONG)s, WA_Left, s->Width - 2 * bh - gw, WA_Top, 0,
                                         WA_Width, gw, WA_Height, bh, WA_Borderless, TRUE, WA_Activate, FALSE,
                                         WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE, WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_REFRESHWINDOW,
                                         TAG_DONE);
            if (os_cog[f].w) {
                os_cog[f].s = s; os_cog[f].bh = bh; os_cog[f].gw = gw;
                strcpy(os_cog[f].name, names[j]);
                WindowToBack(os_cog[f].w);
                os_draw(f);
            }
        }
        UnlockPubScreen(NULL, s);
    }
}

/* the list under a cog on another screen, until a pick or a click elsewhere: the section picked, or -1 */
static int os_menu(struct Screen *s, WORD right, WORD top)
{
    struct DrawInfo *d = GetScreenDrawInfo(s);
    struct Window *w;
    struct RastPort trp;
    WORD rh, W = 0, H, i, hot = -1, pick = -1, done = 0;
    if (!d) return -1;
    InitRastPort(&trp);
    SetFont(&trp, d->dri_Font);
    rh = d->dri_Font->tf_YSize + 4;
    for (i = 0; i < COG_SECTIONS; i++) {
        WORD tw = aat_length(&trp, cog_items[i].label, strlen(cog_items[i].label));
        if (tw > W) W = tw;
    }
    W += 2 * COG_PAD;
    H = COG_SECTIONS * rh + 6;
    if (right < W) right = W;
    if (top + H > s->Height) top = s->Height - H > 0 ? s->Height - H : 0;
    w = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)s, WA_Left, right - W, WA_Top, top, WA_Width, W, WA_Height, H,
                       WA_Borderless, TRUE, WA_Activate, TRUE, WA_RMBTrap, TRUE, WA_ReportMouse, TRUE, WA_SmartRefresh, TRUE,
                       WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_INACTIVEWINDOW | IDCMP_RAWKEY, TAG_DONE);
    if (!w) { FreeScreenDrawInfo(s, d); return -1; }
    SetFont(w->RPort, d->dri_Font);
    SetDrMd(w->RPort, JAM1);
    for (;;) {
        struct IntuiMessage *m;
        struct RastPort *rp = w->RPort;
        SetAPen(rp, d->dri_Pens[BARBLOCKPEN]);
        RectFill(rp, 1, 1, W - 2, H - 2);
        SetAPen(rp, d->dri_Pens[SHADOWPEN]);
        Move(rp, 0, 0); Draw(rp, W - 1, 0); Draw(rp, W - 1, H - 1); Draw(rp, 0, H - 1); Draw(rp, 0, 0);
        for (i = 0; i < COG_SECTIONS; i++) {
            WORD y = 3 + i * rh;
            if (i == hot) { SetAPen(rp, d->dri_Pens[FILLPEN]); RectFill(rp, 2, y, W - 3, y + rh - 1); }
            SetAPen(rp, d->dri_Pens[i == hot ? FILLTEXTPEN : BARDETAILPEN]);
            aat_box(rp, s, COG_PAD, y, rh, cog_items[i].label, strlen(cog_items[i].label));
        }
        if (done) break;
        WaitPort(w->UserPort);
        while ((m = (struct IntuiMessage *)GetMsg(w->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code;
            WORD mx = m->MouseX, my = m->MouseY, row = -1;
            ReplyMsg((struct Message *)m);
            if (mx >= 0 && mx < W && my >= 3 && my < H - 3 && (my - 3) / rh < COG_SECTIONS) row = (my - 3) / rh;
            if (cls == IDCMP_MOUSEMOVE) hot = row;
            else if (cls == IDCMP_MOUSEBUTTONS && code == SELECTUP && row >= 0) { pick = row; done = 1; }
            else if (cls == IDCMP_MOUSEBUTTONS && code == SELECTDOWN && row < 0) done = 1;
            else if (cls == IDCMP_INACTIVEWINDOW) done = 1;
            else if (cls == IDCMP_RAWKEY) {
                if (code == 0x45) done = 1;
                else if ((code == 0x44 || code == 0x43) && hot >= 0) { pick = hot; done = 1; }
                else if (code == 0x4D) hot = hot + 1 >= COG_SECTIONS ? 0 : hot + 1;
                else if (code == 0x4C) hot = hot <= 0 ? COG_SECTIONS - 1 : hot - 1;
            }
        }
    }
    CloseWindow(w);
    FreeScreenDrawInfo(s, d);
    return pick;
}

static void os_events(void)
{
    int i;
    for (i = 0; i < OS_MAX; i++) {
        struct IntuiMessage *m;
        while (os_cog[i].w && (m = (struct IntuiMessage *)GetMsg(os_cog[i].w->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code;
            ReplyMsg((struct Message *)m);
            if (cls == IDCMP_REFRESHWINDOW) { BeginRefresh(os_cog[i].w); EndRefresh(os_cog[i].w, TRUE); os_draw(i); }
            else if (cls == IDCMP_MOUSEBUTTONS && code == SELECTDOWN && prefs_app()) {
                int pick = os_menu(os_cog[i].s, os_cog[i].w->LeftEdge + os_cog[i].gw, os_cog[i].bh);
                if (pick >= 0) prefs_start(os_cog[i].name, cog_items[pick].section);
            }
        }
    }
}

static ULONG os_sigs(void)
{
    ULONG m = 0;
    int i;
    for (i = 0; i < OS_MAX; i++) if (os_cog[i].w) m |= 1UL << os_cog[i].w->UserPort->mp_SigBit;
    return m;
}

/* 0.6.4: a double-click on the date and time opens GFOpenDateTime (date, time, the clock and the region in one
 * editor); until it is installed, the OS's Time editor (the user, 10 Oct 2026) */
static void clock_click(ULONG secs, ULONG micros)
{
    static ULONG last_s, last_m;
    if (last_s && DoubleClick(last_s, last_m, secs, micros)) {
        static const char *const editors[] = { "SYS:Prefs/GFOpenDateTime", "SYS:Prefs/Time", "SYS:Prefs/Classic/Time" };
        int k;
        last_s = 0;
        for (k = 0; k < 3; k++) {
            BPTR l = Lock((STRPTR)editors[k], ACCESS_READ);
            if (l) { UnLock(l); cog_start(editors[k]); break; }
        }
    } else { last_s = secs; last_m = micros; }
}

static void events(struct Window *w)
{
    struct IntuiMessage *m;
    if (!w) return;
    while ((m = (struct IntuiMessage *)GetMsg(w->UserPort))) {
        ULONG cls = m->Class, secs = m->Seconds, micros = m->Micros;
        UWORD code = m->Code;
        WORD my = m->MouseY;
        ReplyMsg((struct Message *)m);
        if (w == row_w) {                      /* 0.8: the row: the item under the pointer */
            if (cls == IDCMP_REFRESHWINDOW) { BeginRefresh(w); row_show(); EndRefresh(w, TRUE); continue; }
            if (cls == IDCMP_ACTIVEWINDOW) { if (last_other) ActivateWindow(last_other); continue; }
            if (cls == IDCMP_MOUSEBUTTONS && (code == SELECTDOWN || code == MENUDOWN)) {
                struct Window *iw = row_item();
                if (!iw) continue;
                if (iw == cog_w || iw == cx_w) {   /* 0.9: Prefs, Power and the commodities: their menus */
                    int o = owner_at_pointer();
                    if (menu_w) menu_close(1);
                    else if (cog_skip && cog_skip == o) cog_skip = 0;
                    else if (o) owner_click(o, code == MENUDOWN);
                    return;
                }
                if (code == MENUDOWN) {        /* 0.11: on a picture item: its program's broker's menu */
                    int o = owner_at_pointer();
                    if (menu_w) menu_close(1);
                    else if (o >= OW_TS && o < OW_TS + TS_ICONS) owner_click(o, 1);
                    return;
                }
                if (code != SELECTDOWN) continue;
                if (iw == clock_w) { clock_click(secs, micros); return; }
                if (iw == net_w) { if (pop_w) pop_close(); else pop_open(); return; }
                {
                    int i;
                    for (i = 0; i < TS_ICONS; i++)
                        if (ts[i] && ts[i]->win == iw) {
                            help_close();
                            if (!ts_tell(ts[i], TSE_CLICK)) ts_drop(i);
                            return;
                        }
                }
            }
            continue;
        }
        if (cls == IDCMP_REFRESHWINDOW) {
            int i;
            BeginRefresh(w);
            for (i = 0; i < TS_ICONS; i++) if (ts[i] && ts[i]->win == w) break;
            if (i < TS_ICONS) ts_draw(ts[i], 1);
            else if (w == logo_w) draw_logo(); else if (w == net_w) draw_net(1); else if (w == pop_w) draw_pop();
            else if (w == cog_w) draw_cog(1); else draw_clock(1);
            EndRefresh(w, TRUE);
        }
        else if (cls == IDCMP_MOUSEBUTTONS && code == SELECTDOWN) {
            if (w == net_w) { if (pop_w) pop_close(); else pop_open(); return; }
            if (w == pop_w) { pop_click(my); return; }
            {   /* a program's icon: tell it; drop the icon when the program is gone */
                int i;
                for (i = 0; i < TS_ICONS; i++)
                    if (ts[i] && ts[i]->win == w) {
                        help_close();
                        if (!ts_tell(ts[i], TSE_CLICK)) ts_drop(i);
                        return;
                    }
            }
        }
        /* a click on ours: the window before stays the active one (the panel keeps it while open) */
        else if (cls == IDCMP_ACTIVEWINDOW && last_other && w != pop_w) ActivateWindow(last_other);
    }
}

/* every item of the row where the tray says now (0.9: also at once when the systray grew or shrank) */
static void place_all(void)
{
    int i;
    if (!scr) return;
    if (net_w) {
        WORD px = nx, py = ny, pw = nw, ph = nh;
        net_place();
        if (nx != px || ny != py || nw != pw || nh != ph) {
            slot_move(net_w, nx, ny, nw, nh);
            tray("Network", nw, NET_ORDER);
            net_drawn = -100;
        }
    }
    if (clock_w) {
        WORD ox = cx, oy = cy, ow = cw, oh = ch;
        place();
        if (cx != ox || cy != oy || cw != ow || ch != oh) { slot_move(clock_w, cx, cy, cw, ch); tray("Clock", cw, CLOCK_ORDER); shown[0] = 0; }
    }
    for (i = 0; i < TS_ICONS; i++) {
        struct ts_icon *t = ts[i];
        WORD px, py, pw, ph;
        if (!t || !t->win) continue;
        px = t->x; py = t->y; pw = t->bw; ph = t->bh;
        ts_place(t);
        if (t->x != px || t->y != py || t->bw != pw || t->bh != ph) {
            if (help_for == i) help_close();
            slot_move(t->win, t->x, t->y, t->bw, t->bh);
            tray(t->name, t->bw, t->order);
            if (t->version >= 2) ts_tell(t, TSE_PLACE);
            t->drawn = -1;
        }
    }
    if (menu_w) { rb_publish(); rb_scr = scr; return; }   /* the menus hang from Prefs, Power and the systray: they stay */
    if (cog_w) {
        WORD px = gx, py = gy, pw = gw, ph = gh;
        cog_place();
        if (gx != px || gy != py || gw != pw || gh != ph) {
            slot_move(cog_w, gx, gy, gw, gh);
            tray("Cog", gw, COG_ORDER);
            cog_drawn = -1;
        }
    }
    if (cx_w) {
        WORD px = xx, py = xy, pw = xw, ph = xh;
        cx_place();
        if (xx != px || xy != py || xw != pw || xh != ph) {
            slot_move(cx_w, xx, xy, xw, xh);
            tray("Commodities", xw, CX_ORDER);
            cx_drawn = -1;
        }
    }
    rb_publish();                              /* 0.9.2: the cog's and the systray's places for the menu button */
    rb_scr = scr;
}

/* 1 when the bar's title has room for the logo: Workbench's, which GFOpenTitle's
 * title format starts with spaces (another program's starts at the left edge) */
static int wb_title_shown(void)
{
    struct Window *a = IntuitionBase->ActiveWindow;
    const char *t;
    if (!a || a->WScreen != scr || bar_ours(a)) a = last_other;
    t = (const char *)(a && a->WScreen == scr && a->ScreenTitle ? a->ScreenTitle : scr->Title);
    return t && !strncmp(t, LOGO_PAD, strlen(LOGO_PAD));
}

/* the logo only beside Workbench's title: another program's title starts at the bar's left edge */
static void logo_follow(void)
{
    int want = prefs.logo && !icon_in_bar && wb_title_shown();
    if (want && !logo_w) {
        bar_sample_now();
        if ((logo_w = see_new(bar_window(0, 0, logo_window_w(), scr->BarHeight)))) draw_logo();
    } else if (!want && logo_w) {
        CloseWindow(logo_w);
        logo_w = NULL;
    }
}

static int is_window(struct Window *w)
{
    struct Window *o;
    if (!w || !scr) return 0;
    for (o = scr->FirstWindow; o; o = o->NextWindow) if (o == w) return 1;
    return 0;
}

/* the windows that wake the task */
static ULONG window_sigs(void)
{
    struct Window *const ws[5] = { logo_w, row_w, pop_w, help_w, menu_w };
    ULONG m = 0;
    int i;
    for (i = 0; i < 5; i++) if (ws[i] && ws[i]->UserPort) m |= 1UL << ws[i]->UserPort->mp_SigBit;
    return m | os_sigs();
}

/* 0.7.9 (4 November 2026): a second public port under the name from before the GF rename. The ROM's own
 * title-bar items (OpenKick's OpenBar, amicradle-guest common/openbar) hide while a title program runs, and
 * the OpenKick ROMs up to full32 look for it only as "OpenTitle": with GFOpenTitle's port alone, OpenBar drew
 * its clock, network, speaker and cog a second time over the bar, in a white box (Instance-32 after GFOpenUp
 * 0.8.15, Dale, 4 November 2026, 20:50). Nothing is sent to it; it is only there to be found. */
#define TB_OLD_PORT "OpenTitle"
static struct MsgPort *old_port;

static struct MsgPort *old_name_port(const char *name)
{
    struct MsgPort *p, *there;
    Forbid();
    there = FindPort((STRPTR)name);
    Permit();
    if (there || !(p = CreateMsgPort())) return NULL;  /* an OpenTitle from before the rename runs: it's its name */
    p->mp_Node.ln_Name = (char *)name;
    p->mp_Node.ln_Pri = -128;                  /* found by name only, after any other port of that name */
    AddPort(p);
    return p;
}

static void old_name_port_end(struct MsgPort *p)
{
    struct Message *m;
    if (!p) return;
    RemPort(p);
    while ((m = GetMsg(p))) ReplyMsg(m);
    DeleteMsgPort(p);
}

int main(void)
{
    struct MsgPort *port, *tport = NULL;
    struct timerequest *tr = NULL;
    int tick = 0, quit = 0, timer = 0;
    ULONG waited = 0;                          /* ms since the last half-second's work */
    (void)version;
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 39);
    LayersBase = OpenLibrary((STRPTR)"layers.library", 39);
    UtilityBase = OpenLibrary((STRPTR)"utility.library", 39);
    CyberGfxBase = OpenLibrary((STRPTR)"cybergraphics.library", 41);   /* the bar's own colours; without it the bar's pen */
    aat_open();                                         /* 0.8.1: the bar's text through GFOpenLayout 19, else Text() */
    CxBase = OpenLibrary((STRPTR)"commodities.library", 37);   /* 0.9: the systray; none without it */
    if (!IntuitionBase || !GfxBase || !LayersBase || !UtilityBase) goto out;
    wbc_start();
    Forbid();
    port = FindPort((STRPTR)TB_PORT);
    Permit();
    if (port) { PutStr((STRPTR)"GFOpenTitle is already running\n"); goto out; }
    if (!(port = CreateMsgPort())) goto out;
    port->mp_Node.ln_Name = (char *)TB_PORT;
    port->mp_Node.ln_Pri = 0;
    AddPort(port);
    old_port = old_name_port(TB_OLD_PORT);     /* 0.7.9: the ROM's OpenBar steps aside for it */
    ts_reply = CreateMsgPort();
    if ((tport = CreateMsgPort()) && (tr = (struct timerequest *)CreateIORequest(tport, sizeof *tr)))
        timer = !OpenDevice((STRPTR)TIMERNAME, UNIT_VBLANK, (struct IORequest *)tr, 0);

    read_prefs();
    watch_start();
    rb_start();                                /* 0.9.1 */
    if (!open_all()) quit = 1;
    apply_title();                             /* 0.7.8: after open_all, which finds out whether Intuition draws the logo */
    sb_start();                                /* 0.10: gfopensysbar.library's items, after the row is open */
    while (!quit) {
        ULONG sig, ms;
        struct Window *act;
        /* a tenth of a second while the pointer is on the bar or the list is open (the cog lights up), else half */
        ms = menu_w || cog_drawn > 0 || hover_for >= 0 || (scr && IntuitionBase->FirstScreen == scr && scr->MouseY <= scr->BarHeight) ? 100 : 500;
        if (see_live) {                        /* 0.7.5: sooner only while the see-through bar fades, or for its grace */
            int n = tbf_next(&see_fade);
            if (n > 0 && (ULONG)n < ms) ms = n < 20 ? 20 : n;
        }
        if (timer) {
            tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 0; tr->tr_time.tv_micro = ms * 1000;
            SendIO((struct IORequest *)tr);
            sig = Wait(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | (1UL << tport->mp_SigBit) | (1UL << port->mp_SigBit)
                       | (ts_reply ? 1UL << ts_reply->mp_SigBit : 0) | window_sigs() | wbc_sigmask() | watch_mask() | rb_mask()
                       | sb_mask());
            /* Only a request that came back by itself counts as time waited. An
             * aborted one comes back too, and its reply sets the timer port's
             * signal after Wait() has returned; WaitIO() takes the reply but leaves
             * the signal. Left set, the next Wait() returned at once, its request
             * was aborted again, and GFOpenTitle went round without stopping: any
             * message woke it into that, a taskspace icon's first of all, and no
             * task below priority 0 ran again (GFOpenFTP's transfers, 8 Oct 2026). */
            {
                int done = CheckIO((struct IORequest *)tr) != NULL;
                if (!done) AbortIO((struct IORequest *)tr);
                WaitIO((struct IORequest *)tr);
                SetSignal(0, 1UL << tport->mp_SigBit);
                if (done) waited += ms;
            }
        } else {
            Delay(TICKS_PER_SECOND / 2);
            sig = SetSignal(0, SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | watch_mask());
            waited += 500;
        }
        if (sig & SIGBREAKF_CTRL_C) break;
        if (sig & watch_mask()) watch_changed();
        /* Workbench is shutting down (Font prefs' Use, a screen mode): no window, no lock on its screen until it is back */
        switch (wbc_take()) {
        case WBC_CLOSE: close_all(); wb_down = 1; wbc_closed(); break;
        /* 0.6.14: only when the windows are down; a refused CloseWorkBench and the OpenWorkBench after it say
         * "open" twice, and the second opened every bar window again over the first */
        case WBC_OPEN:  if (wb_down) wb_down = 2; break;
        }
        if (wb_down == 2 && open_all()) wb_down = 0;
        if (wb_down) { port_events(port); ts_replies(); continue; }
        if (sig & SIGBREAKF_CTRL_F) {
            close_all();
            read_prefs();
            if (!open_all()) break;
            apply_title();                     /* 0.7.8: after open_all, as at the start */
        }
        port_events(port);
        ts_replies();
        if (sig & sb_mask()) sb_sync();       /* 0.10: the library's items changed */
        menu_events();
        rb_events(sig);                        /* 0.9.1: the menu button on the cog or the systray */
        events(logo_w);
        events(row_w);                         /* 0.8: the cog, the network and the programs' icons */
        os_events();                           /* 0.7: the cogs on the other public screens */
        if (pop_w) events(pop_w);
        {
            int i;
            (void)i;
        }
        if (scr) ts_hover();
        if (cog_skip && owner_at_pointer() != cog_skip) cog_skip = 0;
        draw_cog(0);
        see_tick();                            /* 0.7.5: the see-through bar's fade */
        act = IntuitionBase->ActiveWindow;
        if (act && !bar_ours(act)) last_other = act;
        if (last_other && !is_window(last_other)) last_other = NULL;
        if (waited < 500) continue;
        waited = 0;                            /* every half second, as before */
        if (pop_w && ++pop_ticks > 20) pop_close();     /* the panel goes after 10 s */
        /* 0.6.11: the bar changed height (GFOpenLook makes it taller as it starts): every window made again */
        if (scr && scr->BarHeight != made_bh) { close_all(); if (!open_all()) break; apply_title(); }   /* 0.6.12: the room for the logo too */
        logo_follow();
        draw_clock(0);
        if (tick % 2 == 0) os_poll(1);        /* every second: the other public screens' cogs (0.7) */
        if (++tick % 4 == 0) {                 /* every 2 s: the tray and GFOpenMenus' bar may have moved */
            int edge = read_bar_edge();
            if (edge != bar_edge && (clock_w || net_w || cog_w || cx_w || ts_count())) { close_all(); if (!open_all()) break; }
            else {
                if (net_w && net_elsewhere()) {   /* 0.8: GFOpenNetwork started: its icons, not ours */
                    if (pop_w) pop_close();
                    slot_close(net_w); net_w = NULL; tray("Network", 0, NET_ORDER);
                } else if (!net_w && prefs.network && !net_elsewhere()) net_open();   /* it ended: ours again */
                if (net_w && tick % 20 == 0) net_poll();          /* every 10 s */
                if (!menu_w) cx_sync(1);          /* 0.9: the commodities that came and went */
                place_all();
                if (net_w) draw_net(0);
            }
            if (scr && bar_sample_now()) {        /* the theme changed the bar: paint again */
                int i;
                draw_logo();
                shown[0] = 0; draw_clock(1);
                row_redraw();                     /* 0.8: the row's background, the cog, the network, the icons */
                (void)i;
            }
            if (scr) see_check(0);                /* 0.7.5: something behind the bar to see (GFOpenBench may have come or gone) */
        }
    }
    rb_stop();                                 /* 0.9.1: before the windows it looks at go */
    os_poll(0);                                /* the other screens' cogs closed */
    RemPort(port);                             /* no new icons from here on */
    old_name_port_end(old_port);
    port_events(port);
    ts_end();
    sb_stop();                                 /* 0.10 */
    close_all();
    DeleteMsgPort(port);
    if (ts_reply && !ts_out) DeleteMsgPort(ts_reply);   /* a program that never replied keeps it: better a leak than a crash */
out:
    watch_stop();
    if (timer) CloseDevice((struct IORequest *)tr);
    if (tr) DeleteIORequest((struct IORequest *)tr);
    if (tport) DeleteMsgPort(tport);
    wbc_stop();
    aat_close();
    if (WorkbenchBase) CloseLibrary(WorkbenchBase);
    if (SocketBase) CloseLibrary(SocketBase);
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (CxBase) CloseLibrary(CxBase);
    if (UtilityBase) CloseLibrary(UtilityBase);
    if (LayersBase) CloseLibrary(LayersBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
