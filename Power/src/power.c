/* GFOpenPower: Power, a page of GFOpenPrefs for the host's power level (CM4 item 9; Team 5's note of 12 November 2026,
 * 20:45, section 3).
 *
 *   GFOpenPower [LEVEL Quiet|Balanced|Full] [USE] [SAVE] [APPLY]
 *
 *   - Power level: Quiet (a dynamic governor, the clock capped: 1.3 GHz on the CM4), Balanced (a dynamic governor, the
 *     full clock) or Full (the performance governor).
 *   - Under it, read only: the host's temperature, clock, fan, throttling and under-voltage, as far as it measures
 *     them. The full detail is GFOpenMonitor's.
 *
 * The level is the host's, asked through accontrol.device 1.6's ACCMD_POWERLEVEL (ACBridge capability bit 20): io_Offset
 * 0 reads, 1 to 3 set; io_Data comes back with the host's sensors as JSON (pw_info.c reads it). A host that can't set
 * its level (powerLevelCanSet false: the root helper isn't installed) greys the choice; a machine without the
 * command (a real 68k, an older board) says so and has nothing to choose.
 *
 * Use sets the level now. Save sets it and writes ENVARC:GFOpenPrefs/Power and ENV:, the level's name, which
 * GFOpenPrefs APPLY (power_apply) sends at boot. The host keeps its last level across restarts by itself, so the
 * file only matters when the Amiga's choice should win. Cancel changes nothing.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pw_info.h"
#ifdef LT_ONE
#include "looktabs.h"                       /* a page of GFOpenPrefs (LT_OP): its frame and sections */
#define LT_REFRESH(w) lt_draw((w), -1)
#else
#define LT_REFRESH(w) ((void)0)
#define LT_DIRTY 1
#define lt_note_to(f) ((void)0)
#endif

#ifndef IN_OPENBENCH
static const char version[] __attribute__((used)) = "$VER: GFOpenPower 0.1 (12.11.2026) GFOpenPrefs, MIT, Copyright (c) 2026 AmiCradle Team";
#endif

#define POWER_ENV "ENV:GFOpenPrefs/Power"
#define POWER_ENVARC "ENVARC:GFOpenPrefs/Power"

/* accontrol.device 1.6 (amicradle-guest devices/accontrol.device/include/accontrol.h) */
#define ACCMD_POWERLEVEL (CMD_NONSTD + 15)
#define ACCERR_NOPOWERLEVEL 49
#define PW_DONE 0                           /* the host's answer in io_Offset: done */
#define PW_NOT_A_LEVEL 3
#define PW_CANT_SET 5

/* ---- the host --------------------------------------------------------------------------- */

static char json[1024];

/* ACCMD_POWERLEVEL with offset (0 reads, PW_QUIET..PW_FULL set): 0 when the host answered, *answer its io_Offset;
 * ACCERR_NOPOWERLEVEL without the command; -1 without accontrol.device */
static int pw_call(ULONG offset, pw_info *p, ULONG *answer)
{
    struct MsgPort *port = CreateMsgPort();
    struct IOStdReq *io = port ? (struct IOStdReq *)CreateIORequest(port, sizeof *io) : NULL;
    int err = -1;
    json[0] = 0;
    if (io && !OpenDevice((STRPTR)"accontrol.device", 0, (struct IORequest *)io, 0)) {
        io->io_Command = ACCMD_POWERLEVEL;
        io->io_Offset = offset;
        io->io_Data = json;
        io->io_Length = sizeof json;
        err = DoIO((struct IORequest *)io);
        if (!err) {
            if (answer) *answer = io->io_Offset;
            json[io->io_Actual < sizeof json ? io->io_Actual : sizeof json - 1] = 0;
        }
        CloseDevice((struct IORequest *)io);
    }
    if (io) DeleteIORequest((struct IORequest *)io);
    if (port) DeleteMsgPort(port);
    if (p) pw_parse(json, p);
    return err;
}

static int write_text(const char *path, const char *text)
{
    BPTR fh = Open((STRPTR)path, MODE_NEWFILE), lock;
    LONG len = strlen(text), got;
    if (!fh) {                              /* no GFOpenPrefs drawer there yet */
        char dir[32];
        strncpy(dir, path, sizeof dir - 1);
        dir[sizeof dir - 1] = 0;
        *(char *)PathPart((STRPTR)dir) = 0;
        if ((lock = CreateDir((STRPTR)dir))) UnLock(lock);
        if (!(fh = Open((STRPTR)path, MODE_NEWFILE))) return 0;
    }
    got = Write(fh, (APTR)text, len);
    Close(fh);
    return got == len;
}

/* the level ENV: (else ENVARC:) names; -1 for none */
static int saved_level(void)
{
    char buf[32];
    BPTR fh = Open((STRPTR)POWER_ENV, MODE_OLDFILE);
    LONG n;
    if (!fh) fh = Open((STRPTR)POWER_ENVARC, MODE_OLDFILE);
    if (!fh) return -1;
    n = Read(fh, buf, sizeof buf - 1);
    Close(fh);
    buf[n > 0 ? n : 0] = 0;
    n = pw_level_named(buf);
    return n >= PW_QUIET ? n : -1;
}

/* the level set, and with save the file written: 1 done; say gets why not */
static int put_level(int level, int save, void (*say)(const char *))
{
    ULONG answer = PW_DONE;
    char line[16];
    int err = pw_call((ULONG)level, NULL, &answer);
    if (err == -1 || err == ACCERR_NOPOWERLEVEL) { if (say) say("No power control on this machine."); return 0; }
    if (err) { if (say) say("The host didn't answer."); return 0; }
    if (answer == PW_CANT_SET) { if (say) say("This host can't change its power level."); return 0; }
    if (answer != PW_DONE) { if (say) say("The host didn't take that level."); return 0; }
    if (save) {
        snprintf(line, sizeof line, "%s\n", pw_level_name(level));
        if (!write_text(POWER_ENVARC, line) || !write_text(POWER_ENV, line)) {
            if (say) say("The level is set, but GFOpenPrefs/Power couldn't be written.");
            return 0;
        }
    }
    return 1;
}

/* GFOpenPrefs APPLY at boot: the saved level, when there is one and the host can take it; silent */
int power_apply(void)
{
    int level = saved_level();
    pw_info p;
    if (level < 0) return RETURN_OK;
    if (pw_call(0, &p, NULL) || !p.can_set || p.level == level) return RETURN_OK;
    return put_level(level, 0, NULL) ? RETURN_OK : RETURN_WARN;
}

/* ---- the window ------------------------------------------------------------------ */

enum { G_LEVEL = 1, G_NOTE, G_SENSORS, G_STATUS, G_SAVE, G_USE, G_CANCEL, G_COUNT };

static struct Window *win;
static struct Gadget *gad[G_COUNT];
static char status_text[96], note_text[96], sensors_text[128];
static pw_info info;
static int have;                            /* 1 the host answered, 0 no power control here */
static int chosen;                          /* PW_QUIET..PW_FULL, as the cycle shows it */

static const char *const level_labels[] = { "Quiet", "Balanced", "Full", NULL };

static void status(const char *s)
{
    strncpy(status_text, s, sizeof status_text - 1);
    status_text[sizeof status_text - 1] = 0;
    if (win && gad[G_STATUS]) GT_SetGadgetAttrs(gad[G_STATUS], win, NULL, GTTX_Text, (ULONG)status_text, TAG_DONE);
}

/* what the chosen level does, under the choice */
static void note(void)
{
    static const char *const what[4] = {
        "",
        "Quiet: the clock is held back, for less heat and noise.",
        "Balanced: the full clock when there is work, slower when idle.",
        "Full: the full clock all the time.",
    };
    if (!have) snprintf(note_text, sizeof note_text, "No power control on this machine.");
    else if (!info.can_set) snprintf(note_text, sizeof note_text, "This host can't change its power level.");
    else snprintf(note_text, sizeof note_text, "%s", what[chosen]);
    if (win && gad[G_NOTE]) GT_SetGadgetAttrs(gad[G_NOTE], win, NULL, GTTX_Text, (ULONG)note_text, TAG_DONE);
}

static void read_host(void)
{
    int err = pw_call(0, &info, NULL);
    have = !err;
    chosen = have && info.level >= PW_QUIET ? info.level : PW_BALANCED;
    sensors_text[0] = 0;
    if (have) pw_sensors_text(&info, sensors_text, sizeof sensors_text);
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
    int fh, lh, top, row, quit = 0, rc = RETURN_OK, W, X, lw, cw, i, off;
    if (!vi) { if (scr) UnlockPubScreen(NULL, scr); return RETURN_FAIL; }
    memset(gad, 0, sizeof gad);
    read_host();
    off = !have || !info.can_set;
    fh = scr->Font->ta_YSize;
    lh = fh + 6;
    top = scr->WBorTop + fh + 1 + 8;
    lw = TextLength(&scr->RastPort, (STRPTR)"Power level", 11) + 16;      /* the labels' column */
    X = 12 + lw;
    cw = TextLength(&scr->RastPort, (STRPTR)"Balanced", 8) + 48;           /* the cycle */
    W = 12 + TextLength(&scr->RastPort, (STRPTR)"Balanced: the full clock when there is work, slower when idle.", 62) + 24;
    if (W < X + cw + 80) W = X + cw + 80;
    if (!have) snprintf(status_text, sizeof status_text, "%s", "No power control on this machine.");
    else if (info.level == PW_CUSTOM) snprintf(status_text, sizeof status_text, "%s", "The host's level was set by hand (Custom).");
    else snprintf(status_text, sizeof status_text, "%s", off ? "The host's level, as it is now." : "Choose a level, then Use or Save.");
    note_text[0] = 0;

    g = CreateContext(&glist);
    memset(&ng, 0, sizeof ng);
    ng.ng_TextAttr = scr->Font;
    ng.ng_VisualInfo = vi;
#define G(kind, id, x, y, w, h, text, flags, ...) \
    (ng.ng_LeftEdge = (x), ng.ng_TopEdge = (y), ng.ng_Width = (w), ng.ng_Height = (h), ng.ng_GadgetText = (STRPTR)(text), \
     ng.ng_Flags = (flags), ng.ng_GadgetID = (id), g = gad[id] = CreateGadget(kind, g, &ng, __VA_ARGS__, TAG_DONE))
#ifdef LT_ONE
    lt_make(NULL, NULL, 10, top, W - 24, lh, LT_POWER);      /* the frame's sections (GFOpenPrefs) */
    top += lt_space(scr->Font, W - 24, lh);
#endif
    row = top;
    G(CYCLE_KIND, G_LEVEL, X, row, cw, lh, "Power level", PLACETEXT_LEFT, GTCY_Labels, (ULONG)level_labels,
      GTCY_Active, chosen - PW_QUIET, GA_Disabled, off);
    row += lh + 2;
    G(TEXT_KIND, G_NOTE, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)note_text);
    row += lh + 2;
    G(TEXT_KIND, G_SENSORS, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)sensors_text);
    row += lh + 10;
    G(TEXT_KIND, G_STATUS, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)status_text, GTTX_Border, TRUE);
    row += lh + 6;
    {
        static const char *const names[3] = { "Save", "Use", "Cancel" };
        static const int ids[3] = { G_SAVE, G_USE, G_CANCEL };
        int bw = (W - 24 - 2 * 10) / 3;
        for (i = 0; i < 3; i++) G(BUTTON_KIND, ids[i], 12 + i * (bw + 10), row, bw, lh, names[i], 0, GA_Disabled, off && ids[i] != G_CANCEL);
        row += lh + 8;
    }
    if (!g) { rc = RETURN_FAIL; goto out; }
#ifdef LT_ONE
    win = lt_open(scr, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, scr->BarHeight + 20, W, row - scr->WBorTop - fh - 1, glist, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | CYCLEIDCMP | TEXTIDCMP, "GFOpenPower 0.1");
#else
    win = OpenWindowTags(NULL, WA_Title, (ULONG)"Power", WA_ScreenTitle, (ULONG)"GFOpenPower 0.1", WA_PubScreen, (ULONG)scr,
                         WA_Left, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, WA_Top, scr->BarHeight + 20,
                         WA_InnerWidth, W, WA_InnerHeight, row - scr->WBorTop - fh - 1,
                         WA_Gadgets, (ULONG)glist, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_Activate, TRUE,
                         WA_SmartRefresh, TRUE, WA_NewLookMenus, TRUE,
                         WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | CYCLEIDCMP | TEXTIDCMP,
                         TAG_DONE);
#endif
    if (!win) { rc = RETURN_FAIL; goto out; }
    if ((menu = CreateMenus(menus, TAG_DONE)) && LayoutMenus(menu, vi, GTMN_NewLookMenus, TRUE, TAG_DONE)) SetMenuStrip(win, menu);
    note();
    GT_RefreshWindow(win, NULL);
    lt_note_to(status);

    while (!quit) {
        struct IntuiMessage *m;
#ifdef IN_OPENBENCH
        ULONG got = Wait((1UL << win->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);
        if (got & SIGBREAKF_CTRL_C) quit = 1;
        if (got & SIGBREAKF_CTRL_F) { ScreenToFront(win->WScreen); WindowToFront(win); ActivateWindow(win); }
#else
        WaitPort(win->UserPort);
#endif
        while (!quit && (m = GT_GetIMsg(win->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code;
            struct Gadget *gg = (struct Gadget *)m->IAddress;
            GT_ReplyIMsg(m);
            if (cls == IDCMP_CLOSEWINDOW) quit = 1;
            else if (cls == IDCMP_REFRESHWINDOW) { GT_BeginRefresh(win); LT_REFRESH(win); GT_EndRefresh(win, TRUE); }
            else if (cls == IDCMP_MENUPICK) {
                while (code != MENUNULL && !quit) {
                    struct MenuItem *it = ItemAddress(menu, code);
                    if (!it) break;
                    switch ((ULONG)GTMENUITEM_USERDATA(it)) {
                    case 1: if (off || put_level(chosen, 1, status)) quit = 1; break;
                    case 2: if (off || put_level(chosen, 0, status)) quit = 1; break;
                    case 3: quit = 1; break;
                    }
                    code = it->NextSelect;
                }
            }
            else if (cls == IDCMP_GADGETUP) {
                switch (gg->GadgetID) {
                case G_LEVEL: chosen = PW_QUIET + (code % 3); note(); break;
#ifdef LT_ONE
                default: {                          /* another section: nothing is set by leaving */
                    int to = lt_hit(gg, code);
                    if (to >= 0) { lt_go(win, to); quit = 1; }
                    break;
                }
#endif
                case G_SAVE: if (put_level(chosen, 1, status)) quit = 1; break;
                case G_USE: if (put_level(chosen, 0, status)) quit = 1; break;
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
    LONG args[4] = { 0, 0, 0, 0 };
    struct RDArgs *rd = argc > 0 ? ReadArgs((STRPTR)"LEVEL/K,USE/S,SAVE/S,APPLY/S", args, NULL) : NULL;
    int rc, level = -1;
    (void)argv;
#if defined(IN_OPENBENCH) || defined(LT_ONE)
    win = NULL;                             /* its statics outlive a run inside the frame */
#else
    (void)version;
#endif
    if (!rd && argc > 0) { PrintFault(IoErr(), (STRPTR)"GFOpenPower"); return RETURN_FAIL; }
    if (args[0] && (level = pw_level_named((const char *)args[0])) < PW_QUIET) {
        printf("GFOpenPower: LEVEL is Quiet, Balanced or Full\n");
        FreeArgs(rd);
        return RETURN_ERROR;
    }
    if (args[3]) rc = power_apply();
    else if (args[1] || args[2]) {
        if (level < 0) {                    /* USE or SAVE alone: the host's level now */
            if (pw_call(0, &info, NULL) || info.level < PW_QUIET) { printf("GFOpenPower: no level to keep\n"); FreeArgs(rd); return RETURN_WARN; }
            level = info.level;
        }
        rc = put_level(level, args[2] != 0, NULL) ? RETURN_OK : RETURN_ERROR;
    } else if (args[0]) {                   /* LEVEL alone: set it now, as Use */
        rc = put_level(level, 0, NULL) ? RETURN_OK : RETURN_ERROR;
    } else rc = gui();
    if (rd) FreeArgs(rd);
    return rc;
}
