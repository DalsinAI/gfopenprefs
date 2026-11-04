/* WheelTest: a window with the scrollers GFOpenWindows' wheel (0.11, src/wheel.h) moves for a program that
 * doesn't know the wheel, and what the program hears of them:
 *   - a plain Intuition prop in the right border (GTYP_PROPGADGET, FREEVERT, GACT_RELVERIFY | GACT_FOLLOWMOUSE),
 *     as an OS 3.x program of the old kind makes it;
 *   - a BOOPSI propgclass scroller inside (PGA_Top/Total/Visible, ICA_TARGET ICTARGET_IDCMP), whose moves come as
 *     IDCMP_IDCMPUPDATE.
 * It prints each message (class, the gadget, the pot or top) and the wheel's raw keys, for SECONDS (default 20).
 *   WheelTest [SECONDS n] [RAW]       RAW: it scrolls by the wheel's keys itself (a program that knows the wheel)
 * Built by Windows/build.sh. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <stdio.h>
#include <string.h>

struct IntuitionBase *IntuitionBase;

int main(void)
{
    LONG args[2] = { 0, 0 }, secs = 20;
    struct RDArgs *rd = ReadArgs((STRPTR)"SECONDS/N,RAW/S", args, NULL);
    struct Window *w;
    struct PropInfo pi;
    struct Image knob;
    struct Gadget prop;
    Object *bp = NULL;
    ULONG start, now, mic;
    int raw;
    if (!rd) { PrintFault(IoErr(), (STRPTR)"WheelTest"); return 20; }
    if (args[0]) secs = *(LONG *)args[0];
    raw = args[1] != 0;
    FreeArgs(rd);
    if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39))) return 20;
    memset(&pi, 0, sizeof pi);
    memset(&prop, 0, sizeof prop);
    memset(&knob, 0, sizeof knob);
    pi.Flags = AUTOKNOB | FREEVERT | PROPNEWLOOK | PROPBORDERLESS;
    pi.VertBody = MAXBODY / 10;                 /* a hundred lines, ten shown */
    pi.HorizBody = MAXBODY;
    prop.LeftEdge = -13; prop.TopEdge = 20; prop.Width = 10; prop.Height = -40;
    prop.Flags = GFLG_RELRIGHT | GFLG_RELHEIGHT | GFLG_GADGHCOMP;
    prop.Activation = GACT_RELVERIFY | GACT_FOLLOWMOUSE | GACT_IMMEDIATE | GACT_RIGHTBORDER;
    prop.GadgetType = GTYP_PROPGADGET;
    prop.GadgetRender = &knob;
    prop.SpecialInfo = &pi;
    prop.GadgetID = 1;
    w = OpenWindowTags(NULL, WA_Title, (ULONG)(raw ? "WheelTest RAW" : "WheelTest"), WA_Left, 400, WA_Top, 300, WA_Width, 420,
                       WA_Height, 300, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_SizeGadget, TRUE,
                       WA_Activate, TRUE, WA_Gadgets, (ULONG)&prop,
                       WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MOUSEMOVE | IDCMP_IDCMPUPDATE |
                                 IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS, TAG_DONE);
    if (!w) { CloseLibrary((struct Library *)IntuitionBase); return 20; }
    bp = NewObject(NULL, (STRPTR)"propgclass", GA_Left, 40, GA_Top, 60, GA_Width, 16, GA_Height, 180, GA_ID, 2,
                   GA_RelVerify, TRUE, PGA_Freedom, FREEVERT, PGA_NewLook, TRUE, PGA_Total, 200, PGA_Visible, 20, PGA_Top, 0,
                   ICA_TARGET, ICTARGET_IDCMP, TAG_DONE);
    if (bp) { AddGList(w, (struct Gadget *)bp, -1, 1, NULL); RefreshGList((struct Gadget *)bp, w, NULL, 1); }
    printf("WheelTest: a plain prop (right border) and a propgclass one (inside, at 40,60)%s\n", raw ? "; RAW: the wheel's keys scroll it" : "");
    CurrentTime(&start, &mic);
    for (;;) {
        struct IntuiMessage *m;
        int quit = 0;
        Delay(5);
        while ((m = (struct IntuiMessage *)GetMsg(w->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code, qual = m->Qualifier;
            APTR ia = m->IAddress;
            ReplyMsg((struct Message *)m);
            if (cls == IDCMP_CLOSEWINDOW) quit = 1;
            else if (cls == IDCMP_GADGETUP || cls == IDCMP_GADGETDOWN || (cls == IDCMP_MOUSEMOVE && ia == &prop))
                printf("%s gadget %d pot %u\n", cls == IDCMP_GADGETUP ? "GADGETUP" : cls == IDCMP_GADGETDOWN ? "GADGETDOWN" : "MOUSEMOVE",
                       ia == &prop ? 1 : 2, (unsigned)pi.VertPot);
            else if (cls == IDCMP_IDCMPUPDATE) {
                ULONG top = 0;
                GetAttr(PGA_Top, bp, &top);
                printf("IDCMPUPDATE gadget 2 top %lu\n", (unsigned long)top);
            } else if (cls == IDCMP_RAWKEY && (code & 0x7F) >= 0x7A && (code & 0x7F) <= 0x7D) {
                printf("RAWKEY $%02x qual $%04x\n", code, qual);
                if (raw && !(code & 0x80)) {        /* knows the wheel: three lines a step */
                    LONG p = (LONG)pi.VertPot + (code == 0x7B ? 3 : -3) * (MAXPOT / 90);
                    if (p < 0) p = 0;
                    if (p > MAXPOT) p = MAXPOT;
                    NewModifyProp(&prop, w, NULL, pi.Flags, 0, (UWORD)p, MAXBODY, pi.VertBody, 1);
                }
            } else if (cls == IDCMP_MOUSEBUTTONS)
                printf("MOUSEBUTTONS code $%02x qual $%04x\n", code, qual);
        }
        fflush(stdout);
        CurrentTime(&now, &mic);
        if (quit || (LONG)(now - start) >= secs) break;
    }
    if (bp) { RemoveGList(w, (struct Gadget *)bp, 1); DisposeObject(bp); }
    CloseWindow(w);
    printf("WheelTest: done\n");
    CloseLibrary((struct Library *)IntuitionBase);
    return 0;
}
