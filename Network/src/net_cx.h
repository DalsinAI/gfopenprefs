/* GFOpenNetwork 0.2: what each of Exchange's commands does (the system bar's Commodities menu sends the same), plain
 * C so the host tests (tests/test_np.c) run it.
 *   Disable  the broker off and both icons off the bar (the panel closed): the tray is where commodities live, so a
 *            disabled one isn't in it. Enable puts them back.
 *   Hide     the panel closed; Show opens it under the LAN icon. The icons stay: they are the commodity, not its
 *            interface. A disabled one opens nothing.
 *   Kill     ends it. Started again while running (NBU_NOTIFY): as Show, after Enable.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef NET_CX_H
#define NET_CX_H

#ifndef CXCMD_DISABLE                          /* libraries/commodities.h's, for the host */
#define CXCMD_DISABLE   15
#define CXCMD_ENABLE    17
#define CXCMD_APPEAR    19
#define CXCMD_DISAPPEAR 21
#define CXCMD_KILL      23
#define CXCMD_UNIQUE    25
#endif

#define NCX_BROKER_OFF  0x01
#define NCX_BROKER_ON   0x02
#define NCX_ICONS_OFF   0x04
#define NCX_ICONS_ON    0x08
#define NCX_PANEL_CLOSE 0x10
#define NCX_PANEL_OPEN  0x20
#define NCX_QUIT        0x40

#define NCX_PRI_DEFAULT 0

static inline int ncx_do(long id, int disabled)
{
    switch (id) {
    case CXCMD_DISABLE:   return NCX_BROKER_OFF | NCX_ICONS_OFF | NCX_PANEL_CLOSE;
    case CXCMD_ENABLE:    return NCX_BROKER_ON | NCX_ICONS_ON;
    case CXCMD_APPEAR:    return disabled ? 0 : NCX_PANEL_OPEN;
    case CXCMD_DISAPPEAR: return NCX_PANEL_CLOSE;
    case CXCMD_KILL:      return NCX_QUIT;
    case CXCMD_UNIQUE:    return NCX_BROKER_ON | NCX_ICONS_ON | NCX_PANEL_OPEN;
    }
    return 0;
}

#endif
