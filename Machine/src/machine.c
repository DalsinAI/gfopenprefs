/* GFOpenMachine: Machine, a page of GFOpenPrefs for the machine's global settings (gfosdesign
 * gfos/machines/machine-settings, section 5; Dale, 21 November 2026: "build the machine section of openprefs").
 *
 *   GFOpenMachine [FPU Double|Extended] [PACE Hyper|A1200|<MHz>] [CHIPPLANE On|Off] [REFRESH Auto|50|60]
 *                 [USE] [SAVE] [APPLY]
 *
 *   - FPU precision: Double (faster) or Extended (the exact 68040). It changes from the next restart, because the
 *     FPU is never switched under running programs.
 *   - CPU pace: as fast as it can, or an A1200's speed (or the MHz set elsewhere, kept as it is).
 *   - Chipset display plane: as the card is set, on or off (the display hardware scales the chipset's screen).
 *   - HDMI refresh: Auto, 50 Hz or 60 Hz (the monitor blanks briefly while it changes).
 *   - Read only under them: what is in force now, and how many running programs have their own settings (from
 *     their icons' AC. tooltypes, which win over these while they run).
 *
 * The settings are the board's, through accontrol.device 1.8's ACCMD_MACHINE (ACBridge capability bit 22): GET
 * reads them, SET writes the global ones. Use sets them now. Save sets them and writes ENVARC:GFOpenPrefs/Machine
 * and ENV:, the words one line ("FPU=EXTENDED PACE=HYPER"), which GFOpenPrefs APPLY (machine_apply) sends at boot.
 * A machine without the command (a real Amiga, an older board) says so and has nothing to choose.
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
#include "mc_words.h"
#ifdef LT_ONE
#include "looktabs.h"                       /* a page of GFOpenPrefs (LT_OP): its frame and sections */
#define LT_REFRESH(w) lt_draw((w), -1)
#else
#define LT_REFRESH(w) ((void)0)
#define LT_DIRTY 1
#define lt_note_to(f) ((void)0)
#endif

#ifndef IN_OPENBENCH
static const char version[] __attribute__((used)) = "$VER: GFOpenMachine 0.1 (21.11.2026) GFOpenPrefs, MIT, Copyright (c) 2026 AmiCradle Team";
#endif

#define MACHINE_ENV "ENV:GFOpenPrefs/Machine"
#define MACHINE_ENVARC "ENVARC:GFOpenPrefs/Machine"

/* accontrol.device 1.8 (amicradle-guest devices/accontrol.device/include/accontrol.h) */
#define ACCMD_MACHINE (CMD_NONSTD + 17)
#define ACC_MACHINE_GET 3
#define ACC_MACHINE_SET 4
#define ACCERR_NOMACHINE 51

/* ---- the machine ---------------------------------------------------------------------- */

static char answer[1024];

/* ACCMD_MACHINE: op GET (words NULL) or SET (words the global ones). answer gets GET's text, or SET's refused words.
 * 0 when the machine answered; ACCERR_NOMACHINE without the command; -1 without accontrol.device. */
static int mc_call(ULONG op, const char *words)
{
    struct MsgPort *port = CreateMsgPort();
    struct IOStdReq *io = port ? (struct IOStdReq *)CreateIORequest(port, sizeof *io) : NULL;
    int err = -1;
    answer[0] = 0;
    if (words) { strncpy(answer, words, sizeof answer - 1); answer[sizeof answer - 1] = 0; }
    if (io && !OpenDevice((STRPTR)"accontrol.device", 0, (struct IORequest *)io, 0)) {
        io->io_Command = ACCMD_MACHINE;
        io->io_Offset = op;
        io->io_Data = answer;
        io->io_Length = sizeof answer;
        err = DoIO((struct IORequest *)io);
        if (!err && op == ACC_MACHINE_GET) answer[io->io_Actual < sizeof answer ? io->io_Actual : sizeof answer - 1] = 0;
        CloseDevice((struct IORequest *)io);
    }
    if (io) DeleteIORequest((struct IORequest *)io);
    if (port) DeleteMsgPort(port);
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

/* the words ENV: (else ENVARC:) holds, into s; 0 when there are none */
static int saved_words(mc_set *s)
{
    char buf[160];
    BPTR fh = Open((STRPTR)MACHINE_ENV, MODE_OLDFILE);
    LONG n;
    if (!fh) fh = Open((STRPTR)MACHINE_ENVARC, MODE_OLDFILE);
    if (!fh) return 0;
    n = Read(fh, buf, sizeof buf - 1);
    Close(fh);
    buf[n > 0 ? n : 0] = 0;
    mc_clear(s);
    return mc_parse(buf, s);
}

/* the global words set, and with save the file written: 1 done; say gets why not */
static int put_words(const mc_set *s, int save, void (*say)(const char *))
{
    char words[160], line[168];
    int err;
    mc_format(s, words, sizeof words);
    err = mc_call(ACC_MACHINE_SET, words);
    if (err == -1 || err == ACCERR_NOMACHINE) { if (say) say("No machine settings on this machine."); return 0; }
    if (err) { if (say) say("The machine didn't answer."); return 0; }
    if (answer[0]) {
        snprintf(line, sizeof line, "Not taken: %s", answer);
        if (say) say(line);
        return 0;
    }
    if (save) {
        snprintf(line, sizeof line, "%s\n", words);
        if (!write_text(MACHINE_ENVARC, line) || !write_text(MACHINE_ENV, line)) {
            if (say) say("The settings are set, but GFOpenPrefs/Machine couldn't be written.");
            return 0;
        }
    }
    return 1;
}

/* GFOpenPrefs APPLY at boot: the saved words, when there are any and the machine takes them; silent */
int machine_apply(void)
{
    mc_set s;
    if (!saved_words(&s)) return RETURN_OK;
    return put_words(&s, 0, NULL) ? RETURN_OK : RETURN_WARN;
}

/* ---- the window ------------------------------------------------------------------ */

enum { G_FPU = 1, G_PACE, G_PLANE, G_REFRESH, G_NOW, G_PROGRAMS, G_STATUS, G_SAVE, G_USE, G_CANCEL, G_COUNT };

static struct Window *win;
static struct Gadget *gad[G_COUNT];
static char status_text[200], now_text[160], programs_text[120], mhz_label[24];
static mc_set global, in_force, chosen;
static int have;                            /* 1 the machine answered, 0 no machine settings here */

static const char *const fpu_labels[] = { "Double: faster", "Extended: the exact 68040", NULL };
static const char *pace_labels[] = { "As fast as it can", "An A1200's speed", NULL, NULL };
static const char *const plane_labels[] = { "As the card is set", "On", "Off", NULL };
static const char *const refresh_labels[] = { "Auto", "50 Hz", "60 Hz", NULL };

static void status(const char *s)
{
    strncpy(status_text, s, sizeof status_text - 1);
    status_text[sizeof status_text - 1] = 0;
    if (win && gad[G_STATUS]) GT_SetGadgetAttrs(gad[G_STATUS], win, NULL, GTTX_Text, (ULONG)status_text, TAG_DONE);
}

static void read_machine(void)
{
    int pushes;
    char words[160];
    mc_clear(&global); mc_clear(&in_force);
    have = !mc_call(ACC_MACHINE_GET, NULL);
    pushes = have ? mc_read_get(answer, &in_force, &global) : 0;
    chosen = global;
    if (global.fpu_next != MC_NONE) chosen.fpu = global.fpu_next;   /* what the next restart brings */
    if (chosen.fpu == MC_NONE) chosen.fpu = MC_FPU_DOUBLE;
    if (chosen.pace == MC_NONE) chosen.pace = MC_PACE_HYPER;
    mc_format(&in_force, words, sizeof words);
    snprintf(now_text, sizeof now_text, "In force now: %s", have ? (words[0] ? words : "the machine's own") : "-");
    if (!have) programs_text[0] = 0;
    else if (!pushes) snprintf(programs_text, sizeof programs_text, "No running program has its own settings.");
    else snprintf(programs_text, sizeof programs_text, "%d running program%s %s its own, from %s icon, until it quits.",
                  pushes, pushes == 1 ? "" : "s", pushes == 1 ? "has" : "have", pushes == 1 ? "its" : "their");
    pace_labels[2] = NULL;
    if (global.pace == MC_PACE_MHZ) {       /* a pace set elsewhere (ACPace, Cradle) stays choosable */
        snprintf(mhz_label, sizeof mhz_label, "%u MHz", global.mhz);
        pace_labels[2] = mhz_label;
    }
}

static int pace_index(void) { return chosen.pace == MC_PACE_A1200 ? 1 : chosen.pace == MC_PACE_MHZ ? 2 : 0; }
static int plane_index(void) { return chosen.chipplane == 1 ? 1 : chosen.chipplane == 0 ? 2 : 0; }
static int refresh_index(void) { return chosen.refresh == 50 ? 1 : chosen.refresh == 60 ? 2 : 0; }

/* what Use and Save send: every word the page shows, the plane and refresh only when one is chosen */
static mc_set to_send(void)
{
    mc_set s = chosen;
    s.fpu_next = MC_NONE;
    return s;
}

static int use_or_save(int save)
{
    mc_set s = to_send();
    int fpu_later = s.fpu != MC_NONE && s.fpu != in_force.fpu;
    if (!put_words(&s, save, status)) return 0;
    if (fpu_later) status("Set. The FPU precision changes at the next restart.");
    return 1;
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
    int fh, lh, top, row, quit = 0, rc = RETURN_OK, W, X, lw, cw, i, off, fpu_was;
    if (!vi) { if (scr) UnlockPubScreen(NULL, scr); return RETURN_FAIL; }
    memset(gad, 0, sizeof gad);
    read_machine();
    off = !have;
    fpu_was = chosen.fpu;
    fh = scr->Font->ta_YSize;
    lh = fh + 6;
    top = scr->WBorTop + fh + 1 + 8;
    lw = TextLength(&scr->RastPort, (STRPTR)"Chipset display plane", 21) + 16;   /* the labels' column */
    X = 12 + lw;
    cw = TextLength(&scr->RastPort, (STRPTR)"Extended: the exact 68040", 25) + 48;   /* the cycles */
    W = X + cw + 12;
    i = 12 + TextLength(&scr->RastPort, (STRPTR)"2 running programs have their own, from their icons, until it quits.", 68) + 24;
    if (W < i) W = i;
    snprintf(status_text, sizeof status_text, "%s", off ? "No machine settings on this machine."
             : global.fpu_next != MC_NONE ? "The FPU precision chosen changes at the next restart."
             : "Programs' icons can set their own with AC. tooltypes (AC.FPU=EXTENDED).");

    g = CreateContext(&glist);
    memset(&ng, 0, sizeof ng);
    ng.ng_TextAttr = scr->Font;
    ng.ng_VisualInfo = vi;
#define G(kind, id, x, y, w, h, text, flags, ...) \
    (ng.ng_LeftEdge = (x), ng.ng_TopEdge = (y), ng.ng_Width = (w), ng.ng_Height = (h), ng.ng_GadgetText = (STRPTR)(text), \
     ng.ng_Flags = (flags), ng.ng_GadgetID = (id), g = gad[id] = CreateGadget(kind, g, &ng, __VA_ARGS__, TAG_DONE))
#ifdef LT_ONE
    lt_make(NULL, NULL, 10, top, W - 24, lh, LT_MACHINE);    /* the frame's sections (GFOpenPrefs) */
    top += lt_space(scr->Font, W - 24, lh);
#endif
    row = top;
    G(CYCLE_KIND, G_FPU, X, row, cw, lh, "FPU precision", PLACETEXT_LEFT, GTCY_Labels, (ULONG)fpu_labels,
      GTCY_Active, chosen.fpu == MC_FPU_EXTENDED, GA_Disabled, off);
    row += lh + 2;
    G(CYCLE_KIND, G_PACE, X, row, cw, lh, "CPU pace", PLACETEXT_LEFT, GTCY_Labels, (ULONG)pace_labels,
      GTCY_Active, pace_index(), GA_Disabled, off);
    row += lh + 2;
    G(CYCLE_KIND, G_PLANE, X, row, cw, lh, "Chipset display plane", PLACETEXT_LEFT, GTCY_Labels, (ULONG)plane_labels,
      GTCY_Active, plane_index(), GA_Disabled, off);
    row += lh + 2;
    G(CYCLE_KIND, G_REFRESH, X, row, cw, lh, "HDMI refresh", PLACETEXT_LEFT, GTCY_Labels, (ULONG)refresh_labels,
      GTCY_Active, refresh_index(), GA_Disabled, off);
    row += lh + 8;
    G(TEXT_KIND, G_NOW, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)now_text);
    row += lh + 2;
    G(TEXT_KIND, G_PROGRAMS, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)programs_text);
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
    win = lt_open(scr, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, scr->BarHeight + 20, W, row - scr->WBorTop - fh - 1, glist, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | CYCLEIDCMP | TEXTIDCMP, "GFOpenMachine 0.1");
#else
    win = OpenWindowTags(NULL, WA_Title, (ULONG)"Machine", WA_ScreenTitle, (ULONG)"GFOpenMachine 0.1", WA_PubScreen, (ULONG)scr,
                         WA_Left, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, WA_Top, scr->BarHeight + 20,
                         WA_InnerWidth, W, WA_InnerHeight, row - scr->WBorTop - fh - 1,
                         WA_Gadgets, (ULONG)glist, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_Activate, TRUE,
                         WA_SmartRefresh, TRUE, WA_NewLookMenus, TRUE,
                         WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | CYCLEIDCMP | TEXTIDCMP,
                         TAG_DONE);
#endif
    if (!win) { rc = RETURN_FAIL; goto out; }
    if ((menu = CreateMenus(menus, TAG_DONE)) && LayoutMenus(menu, vi, GTMN_NewLookMenus, TRUE, TAG_DONE)) SetMenuStrip(win, menu);
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
                    case 1: if (off || use_or_save(1)) quit = 1; break;
                    case 2: if (off || use_or_save(0)) quit = 1; break;
                    case 3: quit = 1; break;
                    }
                    code = it->NextSelect;
                }
            }
            else if (cls == IDCMP_GADGETUP) {
                switch (gg->GadgetID) {
                case G_FPU:
                    chosen.fpu = code ? MC_FPU_EXTENDED : MC_FPU_DOUBLE;
                    if (chosen.fpu != fpu_was) status("The FPU precision changes at the next restart.");
                    break;
                case G_PACE: chosen.pace = code == 1 ? MC_PACE_A1200 : code == 2 ? MC_PACE_MHZ : MC_PACE_HYPER; break;
                case G_PLANE: chosen.chipplane = code == 1 ? 1 : code == 2 ? 0 : MC_NONE; break;
                case G_REFRESH: chosen.refresh = code == 1 ? 50 : code == 2 ? 60 : MC_REFRESH_AUTO; break;
#ifdef LT_ONE
                default: {                          /* another section: nothing is set by leaving */
                    int to = lt_hit(gg, code);
                    if (to >= 0) { lt_go(win, to); quit = 1; }
                    break;
                }
#endif
                case G_SAVE: if (use_or_save(1)) quit = 1; break;
                case G_USE: if (use_or_save(0)) quit = 1; break;
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
    LONG args[7] = { 0, 0, 0, 0, 0, 0, 0 };
    struct RDArgs *rd = argc > 0 ? ReadArgs((STRPTR)"FPU/K,PACE/K,CHIPPLANE/K,REFRESH/K,USE/S,SAVE/S,APPLY/S", args, NULL) : NULL;
    int rc, i, given = 0;
    mc_set s;
    char words[160];
    (void)argv;
#if defined(IN_OPENBENCH) || defined(LT_ONE)
    win = NULL;                             /* its statics outlive a run inside the frame */
#else
    (void)version;
#endif
    if (!rd && argc > 0) { PrintFault(IoErr(), (STRPTR)"GFOpenMachine"); return RETURN_FAIL; }
    mc_clear(&s);
    words[0] = 0;
    for (i = 0; i < 4; i++)                 /* the arguments as words: FPU Extended is FPU=Extended */
        if (args[i]) {
            static const char *const key[4] = { "FPU", "PACE", "CHIPPLANE", "REFRESH" };
            int n = (int)strlen(words);
            snprintf(words + n, sizeof words - n, "%s%s=%s", n ? " " : "", key[i], (const char *)args[i]);
            given++;
        }
    if (given && mc_parse(words, &s) != given) {
        printf("GFOpenMachine: FPU is Double or Extended, PACE Hyper, A1200 or 1-200, CHIPPLANE On or Off, REFRESH Auto, 50 or 60\n");
        FreeArgs(rd);
        return RETURN_ERROR;
    }
    if (args[6]) rc = machine_apply();
    else if (given || args[4] || args[5]) {
        if (!given) {                       /* USE or SAVE alone: the global words now */
            mc_set now;
            mc_clear(&now);
            if (mc_call(ACC_MACHINE_GET, NULL)) { printf("GFOpenMachine: no machine settings here\n"); FreeArgs(rd); return RETURN_WARN; }
            mc_read_get(answer, &now, &s);
        }
        rc = put_words(&s, args[5] != 0, NULL) ? RETURN_OK : RETURN_ERROR;
    } else rc = gui();
    if (rd) FreeArgs(rd);
    return rc;
}
