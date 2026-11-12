/* GFOpenNetwork 0.1: the network and Wi-Fi icons in the system bar, as a commodity (Dale, 12 November 2026, 01:00:
 * "should we make the sound and wifi and network commodities ? might be a good idea"; TitleBar/docs/system-bar.md).
 *
 * It was part of GFOpenTitle (0.7.10 and before drew the icons itself). Now it is its own commodity, so it shows in
 * Exchange (hide, show, quit) and in the bar like every other commodity, and another program can take its place.
 * It draws nothing on the bar itself: its icons are items in GFOpenTitle's row (taskspace version 2,
 * TitleBar/include/taskspace.h), which draws them in the bar's colours:
 *   LAN    filled when the machine has an address, hollow when not, crossed with no bsdsocket.library;
 *   Wi-Fi  only when gfopensocketwifi.device is there: a dot and one to three arcs by the signal, crossed when not
 *          connected.
 * The help under each says it in words. A click on either opens a panel under it with what the network is and a
 * button for the network settings (SYS:Tools/Commodities/GFOpenSocketControl), as GFOpenTitle's did; the panel
 * goes after 10 seconds, with a click on it, or with another click on the icon.
 * Every 5 seconds it looks again. While GFOpenTitle isn't running it waits (checked every 5 seconds) and the ROM's
 * GFOpenBar shows the network as before.
 *
 * 0.1.1 (12 November 2026): the panel's text anti-aliased through GFOpenLayout 19, as GFOpenTitle 0.8.1's
 *   (Common/aatext.h); Text() where it can't.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/io.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/commodities.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/commodities.h>
#include <proto/bsdsocket.h>

#include <string.h>
#include <stdio.h>

#include "taskspace.h"
#include "popuptheme.h"
#include "net_pics.h"
#include "aatext.h"

const char version[] __attribute__((used)) = "$VER: GFOpenNetwork 0.1.1 (12.11.2026) GFOpenPrefs, AmiCradle Team";

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
struct Library *CxBase, *SocketBase, *CyberGfxBase;

#define NET_TOOL    "SYS:Tools/Commodities/GFOpenSocketControl"
#define WIFI_DEVICE "gfopensocketwifi.device"    /* GFOpenSocket's: the PC's (or the card's) Wi-Fi */
#define WIFI_STATUS 2
#define EVENTS_PORT "GFOpenNetwork.bar"          /* GFOpenTitle's events for our icons */
#define LAN_NAME    "LAN"
#define WIFI_NAME   "WiFi"
#define LAN_ORDER   6                            /* where GFOpenTitle's network icons were (order 5): */
#define WIFI_ORDER  5                            /* the Wi-Fi on the right, the LAN left of it */
#define POP_W       230

struct wifi_row {                              /* gfopensocketwifi.device's record (acnetwork.h), 64 bytes */
    char ssid[33];
    UBYTE bssid[6], signal, security, flags, reserved0[2];
    ULONG channel, rate_mbps;
    UBYTE reserved1[12];
};
struct wifi_call { ULONG command; const char *ssid; struct wifi_row *networks; ULONG capacity; LONG result; ULONG error; };

static int lan_state;                          /* 0 no bsdsocket.library, 1 no address, 2 connected */
static int wifi_state = -1, wifi_signal;       /* -1 no Wi-Fi to show, 0 not connected, 1 connected */
static char lan_addr[20], wifi_ssid[34];

static struct MsgPort *events;
static int lan_in, wifi_in;                    /* our icons are in GFOpenTitle's row */
static int lan_shown = -1, wifi_shown = -1;
static WORD box[2][4];                         /* LAN, Wi-Fi: left, top, width, height on the screen (TSE_PLACE) */
static int hidden;                             /* Exchange's Hide */

static struct Screen *pscr;
static struct DrawInfo *pdri;
static struct Window *pop;
static int pop_for, pop_ticks;

/* ---- what the network is (as GFOpenTitle 0.7.10) ---- */

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

static void lan_line(char *t, int size)
{
    if (lan_state == 0) strlcpy(t, "Network: GFOpenSocket isn't running", size);
    else if (lan_state == 1) strlcpy(t, "Network: not connected", size);
    else snprintf(t, size, "Network: connected, %s", lan_addr);
}

static void wifi_line(char *t, int size)
{
    if (wifi_state < 0) strlcpy(t, "Wi-Fi: none", size);
    else if (!wifi_state) strlcpy(t, "Wi-Fi: not connected", size);
    else snprintf(t, size, "Wi-Fi: %s, signal %d%%", wifi_ssid, wifi_signal);
}

/* ---- our icons in GFOpenTitle's row ---- */

/* one message to GFOpenTitle and its reply: the result, or -1 when GFOpenTitle isn't there */
static LONG ts_send(int wifi, UWORD command)
{
    static UBYTE pics[4 * NP_W * NP_H];
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
    strcpy(m.tsm_Name, wifi ? WIFI_NAME : LAN_NAME);
    m.tsm_Order = wifi ? WIFI_ORDER : LAN_ORDER;
    m.tsm_Width = NP_W; m.tsm_Height = NP_H;
    if (wifi) {
        m.tsm_States = 4;
        for (i = 0; i < 4; i++) np_wifi(pics + i * NP_W * NP_H, i);
        m.tsm_State = np_arcs(wifi_state > 0, wifi_signal);
        wifi_line(m.tsm_Help, sizeof m.tsm_Help);
    } else {
        m.tsm_States = 3;
        for (i = 0; i < 3; i++) np_lan(pics + i * NP_W * NP_H, i);
        m.tsm_State = lan_state;
        lan_line(m.tsm_Help, sizeof m.tsm_Help);
    }
    m.tsm_Pixels = pics;
    strcpy(m.tsm_Port, EVENTS_PORT);
    Forbid();
    if ((p = FindPort((STRPTR)TASKSPACE_PORT))) PutMsg(p, &m.tsm_Message);
    Permit();
    if (p) { WaitPort(reply); GetMsg(reply); }
    DeleteMsgPort(reply);
    if (!p) return -1;
    if (m.tsm_Result == TSR_OK && command != TSC_REMOVE) { if (wifi) wifi_shown = m.tsm_State; else lan_shown = m.tsm_State; }
    return m.tsm_Result;
}

static void icons_out(void)
{
    if (lan_in) { ts_send(0, TSC_REMOVE); lan_in = 0; }
    if (wifi_in) { ts_send(1, TSC_REMOVE); wifi_in = 0; }
    lan_shown = wifi_shown = -1;
}

/* what the icons show now: added, changed or taken away */
static void icons_update(void)
{
    if (hidden) { icons_out(); return; }
    if (!lan_in) lan_in = ts_send(0, TSC_ADD) == TSR_OK;
    else ts_send(0, TSC_SET);              /* the picture or the help (the address) may have changed */
    if (wifi_state >= 0) {
        if (!wifi_in) wifi_in = ts_send(1, TSC_ADD) == TSR_OK;
        else ts_send(1, TSC_SET);
    } else if (wifi_in) { ts_send(1, TSC_REMOVE); wifi_in = 0; wifi_shown = -1; }
}

/* ---- the panel under an icon ---- */

static void pop_close(void)
{
    if (pop) { CloseWindow(pop); pop = NULL; }
    pt_free();
    if (pdri) { FreeScreenDrawInfo(pscr, pdri); pdri = NULL; }
    if (pscr) { UnlockPubScreen(NULL, pscr); pscr = NULL; }
}

static void pop_draw(void)
{
    struct RastPort *rp = pop->RPort;
    char l[3][64];
    WORD fh = rp->TxHeight, y, i;
    lan_line(l[0], 64);
    wifi_line(l[1], 64);
    strcpy(l[2], "Network settings...");
    if (pt_ok) pt_panel(rp, pop->Width, pop->Height);
    else {
        SetAPen(rp, pdri->dri_Pens[BACKGROUNDPEN]);
        RectFill(rp, 0, 0, pop->Width - 1, pop->Height - 1);
        SetAPen(rp, pdri->dri_Pens[SHADOWPEN]);
        Move(rp, 0, 0); Draw(rp, pop->Width - 1, 0); Draw(rp, pop->Width - 1, pop->Height - 1); Draw(rp, 0, pop->Height - 1); Draw(rp, 0, 0);
    }
    SetAPen(rp, pt_ok ? pt_text_pen() : pdri->dri_Pens[TEXTPEN]);
    SetDrMd(rp, JAM1);
    for (i = 0, y = 6; i < 2; i++, y += fh + 4) aat_box(rp, pscr, 8, y, fh, l[i], strlen(l[i]));
    if (pt_ok) {
        SetAPen(rp, pt_hot_pen());
        RectFill(rp, 8, y, pop->Width - 9, y + fh + 5);
        SetAPen(rp, pt_hot_text_pen());
    } else {
        SetAPen(rp, pdri->dri_Pens[SHINEPEN]);
        Move(rp, 8, y + fh + 5); Draw(rp, 8, y); Draw(rp, pop->Width - 9, y);
        SetAPen(rp, pdri->dri_Pens[SHADOWPEN]);
        Draw(rp, pop->Width - 9, y + fh + 5); Draw(rp, 8, y + fh + 5);
        SetAPen(rp, pdri->dri_Pens[TEXTPEN]);
    }
    aat_box(rp, pscr, (pop->Width - aat_length(rp, l[2], strlen(l[2]))) / 2, y + 3, fh, l[2], strlen(l[2]));
}

static void pop_open(int which)
{
    WORD fh, h, x, y, pw = POP_W;
    WORD *b = box[which];
    if (!(pscr = LockPubScreen((STRPTR)"Workbench"))) return;
    if (!(pdri = GetScreenDrawInfo(pscr))) { pop_close(); return; }
    pt_load(pscr);
    fh = pdri->dri_Font->tf_YSize;
    h = 3 * (fh + 4) + 14;
    {   /* 0.1.1: as wide as its longest line (outline text is wider than Text()'s), POP_W at least */
        struct RastPort trp = pscr->RastPort;
        char l[2][64];
        WORD tw;
        int i;
        SetFont(&trp, pdri->dri_Font);
        lan_line(l[0], 64);
        wifi_line(l[1], 64);
        for (i = 0; i < 2; i++)
            if ((tw = aat_length(&trp, l[i], strlen(l[i])) + 16) > pw) pw = tw;
    }
    x = b[0] + b[2] - pw;
    if (x < 0) x = 0;
    y = b[1] + b[3] + 1;
    if (y + h > pscr->Height) y = b[1] - h;
    if (!(pop = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)pscr, WA_Left, x, WA_Top, y, WA_Width, pw, WA_Height, h,
                               WA_Borderless, TRUE, WA_Activate, FALSE, WA_RMBTrap, TRUE, WA_SmartRefresh, TRUE,   /* the active window stays */
                               WA_IDCMP, IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS, TAG_DONE))) { pop_close(); return; }
    SetFont(pop->RPort, pdri->dri_Font);
    pop_for = which;
    pop_ticks = 0;
    pop_draw();
}

static void settings(void)
{
    BPTR in = Open((STRPTR)"NIL:", MODE_OLDFILE), out = Open((STRPTR)"NIL:", MODE_NEWFILE);
    /* asynchronous: both handles are the new process's, closed when it ends */
    if (!in || !out || SystemTags((STRPTR)NET_TOOL, SYS_Input, in, SYS_Output, out, SYS_Asynch, TRUE, SYS_UserShell, TRUE,
                                  NP_StackSize, 16384, TAG_DONE) == -1) {
        if (in) Close(in);
        if (out) Close(out);
    }
}

static void pop_events(void)
{
    struct IntuiMessage *m;
    int close = 0, go = 0;
    while (pop && (m = (struct IntuiMessage *)GetMsg(pop->UserPort))) {
        ULONG cls = m->Class;
        UWORD code = m->Code;
        WORD my = m->MouseY;
        ReplyMsg((struct Message *)m);
        if (cls == IDCMP_REFRESHWINDOW) { BeginRefresh(pop); pop_draw(); EndRefresh(pop, TRUE); }
        else if (cls == IDCMP_MOUSEBUTTONS && code == SELECTDOWN) {
            WORD fh = pop->RPort->TxHeight, by = 6 + 2 * (fh + 4);
            go = my >= by && my <= by + fh + 5;
            close = 1;
        }
    }
    if (close) pop_close();
    if (go) settings();
}

/* GFOpenTitle's events: where it put our icons, a click on one, or that it is ending */
static void bar_events(void)
{
    struct TaskspaceEvent *e;
    while ((e = (struct TaskspaceEvent *)GetMsg(events))) {
        if (e->tse_Magic == TASKSPACE_MAGIC) {
            int which = !strcmp(e->tse_Name, WIFI_NAME) ? 1 : 0;
            if (e->tse_Kind == TSE_PLACE || e->tse_Kind == TSE_CLICK) {
                box[which][0] = e->tse_Left; box[which][1] = e->tse_Top; box[which][2] = e->tse_Width; box[which][3] = e->tse_Height;
            }
            if (e->tse_Kind == TSE_CLICK) {
                int again = pop && pop_for == which;
                if (pop) pop_close();
                if (!again) { net_poll(); pop_open(which); }
            }
            if (e->tse_Kind == TSE_GONE) {           /* GFOpenTitle is ending: back when it is */
                if (which) wifi_in = 0, wifi_shown = -1; else lan_in = 0, lan_shown = -1;
                if (pop) pop_close();
            }
        }
        ReplyMsg(&e->tse_Message);
    }
}

static struct NewBroker nb = {
    NB_VERSION, (STRPTR)"GFOpenNetwork", (STRPTR)"GFOpenNetwork 0.1", (STRPTR)"The network and Wi-Fi in the system bar",
    NBU_UNIQUE | NBU_NOTIFY, COF_SHOW_HIDE, 0, NULL, 0
};

int main(void)
{
    struct MsgPort *cxport = NULL, *tport = NULL;
    struct timerequest *tr = NULL;
    CxObj *broker = NULL;
    int quit = 0, timer = 0, ticks = 0;
    IntuitionBase = (struct IntuitionBase *)OpenLibrary((STRPTR)"intuition.library", 39);
    GfxBase = (struct GfxBase *)OpenLibrary((STRPTR)"graphics.library", 39);
    CxBase = OpenLibrary((STRPTR)"commodities.library", 37);
    CyberGfxBase = OpenLibrary((STRPTR)"cybergraphics.library", 41);   /* the panel's exact colours */
    aat_open();                                         /* the panel's text through GFOpenLayout 19, else Text() */
    if (!IntuitionBase || !GfxBase || !CxBase) goto out;
    if (!(cxport = CreateMsgPort())) goto out;
    nb.nb_Port = cxport;
    if (!(broker = CxBroker(&nb, NULL))) goto out;            /* one GFOpenNetwork: a second one just ends */
    if (!(events = CreateMsgPort())) goto out;
    events->mp_Node.ln_Name = (char *)EVENTS_PORT;
    AddPort(events);
    if ((tport = CreateMsgPort()) && (tr = (struct timerequest *)CreateIORequest(tport, sizeof *tr)))
        timer = !OpenDevice((STRPTR)TIMERNAME, UNIT_VBLANK, (struct IORequest *)tr, 0);
    if (!timer) goto out;
    ActivateCxObj(broker, 1);
    net_poll();
    icons_update();
    tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 1; tr->tr_time.tv_micro = 0;
    SendIO((struct IORequest *)tr);
    while (!quit) {
        ULONG got = Wait((1UL << cxport->mp_SigBit) | (1UL << tport->mp_SigBit) | (1UL << events->mp_SigBit) | SIGBREAKF_CTRL_C |
                         (pop ? 1UL << pop->UserPort->mp_SigBit : 0));
        CxMsg *cm;
        if (got & SIGBREAKF_CTRL_C) quit = 1;
        while ((cm = (CxMsg *)GetMsg(cxport))) {
            ULONG type = CxMsgType(cm), id = CxMsgID(cm);
            ReplyMsg((struct Message *)cm);
            if (type != CXM_COMMAND) continue;
            switch (id) {
            case CXCMD_KILL: quit = 1; break;
            case CXCMD_DISABLE: ActivateCxObj(broker, 0); break;
            case CXCMD_ENABLE: ActivateCxObj(broker, 1); break;
            case CXCMD_DISAPPEAR: hidden = 1; if (pop) pop_close(); icons_update(); break;
            case CXCMD_APPEAR: hidden = 0; net_poll(); icons_update(); break;
            case CXCMD_UNIQUE: hidden = 0; net_poll(); icons_update(); break;   /* started again: show */
            }
        }
        bar_events();
        if (pop) pop_events();
        if (CheckIO((struct IORequest *)tr)) {
            WaitIO((struct IORequest *)tr);
            ticks++;
            if (pop && ++pop_ticks >= 10) pop_close();       /* the panel goes after 10 s */
            if (ticks % 5 == 0) { net_poll(); icons_update(); if (pop) pop_draw(); }   /* every 5 s; GFOpenTitle back too */
            tr->tr_node.io_Command = TR_ADDREQUEST; tr->tr_time.tv_secs = 1; tr->tr_time.tv_micro = 0;
            SendIO((struct IORequest *)tr);
        }
    }
out:
    if (pop) pop_close();
    icons_out();
    if (broker) DeleteCxObjAll(broker);
    if (timer) { if (!CheckIO((struct IORequest *)tr)) AbortIO((struct IORequest *)tr); WaitIO((struct IORequest *)tr); CloseDevice((struct IORequest *)tr); }
    if (tr) DeleteIORequest((struct IORequest *)tr);
    if (tport) DeleteMsgPort(tport);
    if (events) {
        struct Message *m;
        RemPort(events);
        while ((m = GetMsg(events))) ReplyMsg(m);
        DeleteMsgPort(events);
    }
    if (cxport) {
        struct Message *m;
        while ((m = GetMsg(cxport))) ReplyMsg(m);
        DeleteMsgPort(cxport);
    }
    aat_close();
    if (SocketBase) CloseLibrary(SocketBase);
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (CxBase) CloseLibrary(CxBase);
    if (GfxBase) CloseLibrary((struct Library *)GfxBase);
    if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
    return RETURN_OK;
}
