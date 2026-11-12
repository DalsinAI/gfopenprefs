/* GFOpenGlobal: Global Settings, a page of GFOpenPrefs for settings the whole system follows (Dale, 12 November
 * 2026, 15:46: "add a preferences setting called global settings, first one in = Default stack size which
 * overrides what was set if it's below the number in the box").
 *
 *   GFOpenGlobal [FROM file] [STACK bytes] [USE] [SAVE]
 *
 *   - Default stack size: a program started with a smaller stack gets this one; a bigger one is kept. 0 is off.
 *
 * The rule itself is in GFOpenDOS (amicradle-guest libraries/opendos, "a floor under every stack", since 3 November
 * 2026): every stack dos makes, so CreateProc, CreateNewProc and SystemTagList (NP_StackSize), RunCommand, the
 * Shell's commands (Stack and the CLI's default), Run, Execute, and Workbench's and WBStartup's programs, which
 * start through CreateNewProc, is at least the floor. It reads ENV:GFOpenPrefs/Stack, one setting a line, at most
 * once a second: "minimum <bytes>" is this box; "auto on" also raises the floor with the memory (16 KB above
 * 16 MB of RAM up to 128 KB above 128 MB); "add <bytes>" is added to every stack. This page writes "minimum" and
 * keeps every other line; 0 writes "minimum 0" and "auto off", so no stack is raised at all.
 *
 * Use writes ENV: (dos takes it within a second); Save writes ENVARC: as well; Test writes ENV: and Cancel then
 * puts back what ENV: had. More global settings go below this one, each a row.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
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
#include "gs_stack.h"
#ifdef LT_ONE
#include "looktabs.h"                       /* a page of GFOpenPrefs (LT_OP): its frame and sections */
#define LT_REFRESH(w) lt_draw((w), -1)
#else
#define LT_REFRESH(w) ((void)0)
#define LT_DIRTY 1
#define LT_DX 0
#define lt_note_to(f) ((void)0)
#endif

#ifndef IN_OPENBENCH
static const char version[] __attribute__((used)) = "$VER: GFOpenGlobal 0.1 (12.11.2026) GFOpenPrefs, MIT, Copyright (c) 2026 AmiCradle Team";
#endif

#define STACK_ENV "ENV:GFOpenPrefs/Stack"
#define STACK_ENVARC "ENVARC:GFOpenPrefs/Stack"
#define FILE_MAX 1024

/* ---- the settings file ---------------------------------------------------------------- */

static char file_text[FILE_MAX + 1];        /* ENV:'s file as read: its other lines are kept */
static char test_text[FILE_MAX + 1];        /* ENV:'s file before Test, for Cancel */
static LONG test_len;                       /* -1: there was no file */
static int tested;
static unsigned long stack_min;             /* "minimum" */
static int stack_auto;                      /* "auto" */

static LONG read_text(const char *path, char *buf, LONG size)
{
    BPTR fh = Open((STRPTR)path, MODE_OLDFILE);
    LONG n;
    buf[0] = 0;
    if (!fh) return -1;
    n = Read(fh, buf, size - 1);
    Close(fh);
    if (n < 0) n = 0;
    buf[n] = 0;
    return n;
}

static int write_text(const char *path, const char *text, LONG len)
{
    BPTR fh = Open((STRPTR)path, MODE_NEWFILE), lock;
    LONG got;
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

/* what "auto on" raises the floor to on this machine: GFOpenDOS's ram_size (memory with something free) */
static ULONG auto_floor(void)
{
    struct MemHeader *mh;
    ULONG ram = 0;
    Forbid();
    for (mh = (struct MemHeader *)SysBase->MemList.lh_Head; mh->mh_Node.ln_Succ; mh = (struct MemHeader *)mh->mh_Node.ln_Succ)
        if (mh->mh_Free) ram += (ULONG)mh->mh_Upper - (ULONG)mh->mh_Lower;
    Permit();
    return ram > (128UL << 20) ? 131072 : ram > (64UL << 20) ? 65536 : ram > (32UL << 20) ? 32768 : ram > (16UL << 20) ? 16384 : 0;
}

/* Use (0), Save (1) or Test (2) with minimum. 1 done. */
static int put_stack(ULONG minimum, int how, void (*say)(const char *))
{
    char out[FILE_MAX + 64];
    LONG n;
    minimum = gs_round(minimum);
    if (how == 2 && !tested) { test_len = read_text(STACK_ENV, test_text, sizeof test_text); tested = 1; }
    n = gs_text(file_text, out, sizeof out, minimum);
    if (how == 1 && !write_text(STACK_ENVARC, out, n)) { if (say) say("Couldn't write ENVARC:GFOpenPrefs/Stack."); return 0; }
    if (!write_text(STACK_ENV, out, n)) { if (say) say("Couldn't write ENV:GFOpenPrefs/Stack."); return 0; }
    return 1;
}

/* Cancel after Test: ENV: as it was */
static void untest(void)
{
    if (!tested) return;
    if (test_len < 0) DeleteFile((STRPTR)STACK_ENV);
    else write_text(STACK_ENV, test_text, test_len);
    tested = 0;
}

/* ---- the window ------------------------------------------------------------------ */

enum { G_STACK = 1, G_STACKNOTE, G_STATUS, G_SAVE, G_USE, G_TEST, G_CANCEL, G_COUNT };

static struct Window *win;
static struct Gadget *gad[G_COUNT];
static char status_text[80], note_text[80];

static void status(const char *s)
{
    strncpy(status_text, s, sizeof status_text - 1);
    if (win && gad[G_STATUS]) GT_SetGadgetAttrs(gad[G_STATUS], win, NULL, GTTX_Text, (ULONG)status_text, TAG_DONE);
}

static ULONG box(void)
{
    LONG v = 0;
    if (gad[G_STACK]) GT_GetGadgetAttrs(gad[G_STACK], win, NULL, GTIN_Number, (ULONG)&v, TAG_DONE);
    return v < 0 ? 0 : (ULONG)v;
}

/* the line under the box: what a program gets now */
static void note(ULONG minimum)
{
    ULONG a = stack_auto && minimum ? auto_floor() : 0;
    if (!minimum) snprintf(note_text, sizeof note_text, "Off: programs get the stack they ask for.");
    else if (a > minimum) snprintf(note_text, sizeof note_text, "At least %lu bytes (raised with this machine's memory).", (unsigned long)a);
    else snprintf(note_text, sizeof note_text, "Programs get at least %lu bytes.", gs_round(minimum));
    if (win && gad[G_STACKNOTE]) GT_SetGadgetAttrs(gad[G_STACKNOTE], win, NULL, GTTX_Text, (ULONG)note_text, TAG_DONE);
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
    int fh, lh, top, row, quit = 0, rc = RETURN_OK, W, X, lw, nw, i;
    if (!vi) { if (scr) UnlockPubScreen(NULL, scr); return RETURN_FAIL; }
    memset(gad, 0, sizeof gad);
    fh = scr->Font->ta_YSize;
    lh = fh + 6;
    top = scr->WBorTop + fh + 1 + 8;
    lw = TextLength(&scr->RastPort, (STRPTR)"Default stack size (bytes)", 26) + 16;   /* the labels' column */
    X = 12 + lw;
    nw = TextLength(&scr->RastPort, (STRPTR)"00000000", 8) + 16;             /* a number */
    W = 12 + TextLength(&scr->RastPort, (STRPTR)"At least 131072 bytes (raised with this machine's memory).", 58) + 24;
    if (W < X + nw + 80) W = X + nw + 80;
    snprintf(status_text, sizeof status_text, "%s", "Change what you like, then Use or Save.");
    note_text[0] = 0;

    g = CreateContext(&glist);
    memset(&ng, 0, sizeof ng);
    ng.ng_TextAttr = scr->Font;
    ng.ng_VisualInfo = vi;
#define G(kind, id, x, y, w, h, text, flags, ...) \
    (ng.ng_LeftEdge = (x), ng.ng_TopEdge = (y), ng.ng_Width = (w), ng.ng_Height = (h), ng.ng_GadgetText = (STRPTR)(text), \
     ng.ng_Flags = (flags), ng.ng_GadgetID = (id), g = gad[id] = CreateGadget(kind, g, &ng, __VA_ARGS__, TAG_DONE))
#ifdef LT_ONE
    lt_make(NULL, NULL, 10, top, W - 24, lh, LT_GLOBAL);     /* the frame's sections (GFOpenPrefs) */
    top += lt_space(scr->Font, W - 24, lh);
#endif
    row = top;
    /* each global setting a row, the next below this one */
    G(INTEGER_KIND, G_STACK, X, row, nw, lh, "Default stack size (bytes)", PLACETEXT_LEFT, GTIN_Number, stack_min, GTIN_MaxChars, 7);
    row += lh + 2;
    G(TEXT_KIND, G_STACKNOTE, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)note_text);
    row += lh + 10;
    G(TEXT_KIND, G_STATUS, 12, row, W - 24, lh, NULL, 0, GTTX_Text, (ULONG)status_text, GTTX_Border, TRUE);
    row += lh + 6;
    {
        static const char *const names[4] = { "Save", "Use", "Test", "Cancel" };
        static const int ids[4] = { G_SAVE, G_USE, G_TEST, G_CANCEL };
        int bw = (W - 24 - 3 * 10) / 4;
        for (i = 0; i < 4; i++) G(BUTTON_KIND, ids[i], 12 + i * (bw + 10), row, bw, lh, names[i], 0, GA_Disabled, FALSE);
        row += lh + 8;
    }
    if (!g) { rc = RETURN_FAIL; goto out; }
#ifdef LT_ONE
    win = lt_open(scr, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, scr->BarHeight + 20, W, row - scr->WBorTop - fh - 1, glist, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | INTEGERIDCMP | TEXTIDCMP, "GFOpenGlobal 0.1");
#else
    win = OpenWindowTags(NULL, WA_Title, (ULONG)"Global Settings", WA_ScreenTitle, (ULONG)"GFOpenGlobal 0.1", WA_PubScreen, (ULONG)scr,
                         WA_Left, (scr->Width - W) / 2 > 0 ? (scr->Width - W) / 2 : 0, WA_Top, scr->BarHeight + 20,
                         WA_InnerWidth, W, WA_InnerHeight, row - scr->WBorTop - fh - 1,
                         WA_Gadgets, (ULONG)glist, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_Activate, TRUE,
                         WA_SmartRefresh, TRUE, WA_NewLookMenus, TRUE,
                         WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MENUPICK | BUTTONIDCMP | INTEGERIDCMP | TEXTIDCMP,
                         TAG_DONE);
#endif
    if (!win) { rc = RETURN_FAIL; goto out; }
    if ((menu = CreateMenus(menus, TAG_DONE)) && LayoutMenus(menu, vi, GTMN_NewLookMenus, TRUE, TAG_DONE)) SetMenuStrip(win, menu);
    note(stack_min);
    GT_RefreshWindow(win, NULL);
    lt_note_to(status);

    while (!quit) {
        struct IntuiMessage *m;
#ifdef IN_OPENBENCH
        ULONG got = Wait((1UL << win->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);
        if (got & SIGBREAKF_CTRL_C) { untest(); quit = 1; }
        if (got & SIGBREAKF_CTRL_F) { ScreenToFront(win->WScreen); WindowToFront(win); ActivateWindow(win); }
#else
        WaitPort(win->UserPort);
#endif
        while (!quit && (m = GT_GetIMsg(win->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code;
            struct Gadget *gg = (struct Gadget *)m->IAddress;
            GT_ReplyIMsg(m);
            if (cls == IDCMP_CLOSEWINDOW) { untest(); quit = 1; }
            else if (cls == IDCMP_REFRESHWINDOW) { GT_BeginRefresh(win); LT_REFRESH(win); GT_EndRefresh(win, TRUE); }
            else if (cls == IDCMP_MENUPICK) {
                while (code != MENUNULL && !quit) {
                    struct MenuItem *it = ItemAddress(menu, code);
                    if (!it) break;
                    switch ((ULONG)GTMENUITEM_USERDATA(it)) {
                    case 1: if (put_stack(box(), 1, status)) quit = 1; break;
                    case 2: if (put_stack(box(), 0, status)) quit = 1; break;
                    case 3: untest(); quit = 1; break;
                    }
                    code = it->NextSelect;
                }
            }
            else if (cls == IDCMP_GADGETUP) {
                switch (gg->GadgetID) {
                case G_STACK: note(box()); break;
#ifdef LT_ONE
                default: {                          /* another section: the page's changes kept, as Use, when it has any */
                    int to = lt_hit(gg, code);
                    if (to >= 0) { if (LT_DIRTY) put_stack(box(), 0, status); lt_go(win, to); quit = 1; }
                    break;
                }
#endif
                case G_SAVE: if (put_stack(box(), 1, status)) { tested = 0; quit = 1; } break;
                case G_USE: if (put_stack(box(), 0, status)) { tested = 0; quit = 1; } break;
                case G_TEST:
                    note(box());
                    if (put_stack(box(), 2, status)) status("In use until you Cancel: start a program to try it.");
                    break;
                case G_CANCEL: untest(); quit = 1; break;
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
    struct RDArgs *rd = argc > 0 ? ReadArgs((STRPTR)"FROM,STACK/N,USE/S,SAVE/S", args, NULL) : NULL;
    int rc;
    (void)argv;
#if defined(IN_OPENBENCH) || defined(LT_ONE)
    win = NULL;                             /* its statics outlive a run inside the frame */
    tested = 0;
#else
    (void)version;
#endif
    if (!rd && argc > 0) { PrintFault(IoErr(), (STRPTR)"GFOpenGlobal"); return RETURN_FAIL; }
    if (read_text(args[0] ? (const char *)args[0] : STACK_ENV, file_text, sizeof file_text) < 0 && !args[0])
        read_text(STACK_ENVARC, file_text, sizeof file_text);
    gs_parse(file_text, &stack_min, &stack_auto);
    if (args[1]) stack_min = gs_round(*(ULONG *)args[1]);
    if (args[2] || args[3]) rc = put_stack(stack_min, args[3] ? 1 : 0, NULL) ? RETURN_OK : RETURN_ERROR;
    else if (args[1]) {                     /* STACK alone: say what it would be */
        printf("Default stack size %lu bytes\n", (unsigned long)stack_min);
        rc = RETURN_OK;
    } else rc = gui();
    if (rd) FreeArgs(rd);
    return rc;
}
