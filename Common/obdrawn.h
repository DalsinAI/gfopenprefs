/* obdrawn.h: "the desktop is drawn" (GFOpenBench 0.1.71). Dale saw GFOpenMeters and the system tray tear and draw
 * again the old way on the Pi 5 while GFOpenBench was starting (17 November 2026): GFOpenMeters copied the desktop
 * behind its panel while GFOpenBench was still drawing it (the gradient, then the picture from its own process), so
 * it closed and opened its panel again at 3 and 6 seconds; GFOpenTitle went see-through and painted its tray again
 * as GFOpenBench's strip came, then again as the bar's colours changed under it.
 *
 * So GFOpenBench keeps a public port, OB_DRAWN_PORT, from its start to its end, saying whether its desktop is drawn:
 * the backdrop window, the strip under the bar, the icons, and the picture once it is decoded (or none). It says
 * "not drawn" while its desktop is down for a Workbench reset and while it comes up again. A program drawn over the
 * desktop waits while it is not drawn, then draws once. No port (an older GFOpenBench, or Workbench): no wait, as
 * before. Never more than OB_DRAWN_CAP_MS at a time: a picture that never comes holds nobody.
 *
 * The port takes no messages; it is only looked at, under Forbid. The same file is gfopenbench desktop/ob_drawn.h
 * (GFOpenBench and GFOpenMeters include that one): keep the two the same.
 *
 * The wait itself (ob_hold_step) is plain C, for the host tests (gfopenbench tests/host/test_ob_drawn.c, and
 * TitleBar/tests/test_tb.c here); the port is Amiga only (not with OB_DRAWN_HOST).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_DRAWN_H
#define OB_DRAWN_H

#define OB_DRAWN_PORT "GFOPENBENCH.DESKTOP"
#define OB_DRAWN_MAGIC 0x4F424431UL               /* 'OBD1' */
#define OB_DRAWN_CAP_MS 10000U                     /* the longest wait: the picture's process is hurried at 4 s */

/* what a program over the desktop does at this look */
#define OBH_GO 0                                   /* draw as it comes (nothing to wait for) */
#define OBH_HOLD 1                                 /* GFOpenBench is still drawing: not yet */
#define OBH_RELEASE 2                              /* the wait is over (drawn, gone, or waited too long): draw now, once */

typedef struct {
    int held;                                      /* waiting now */
    int spent;                                     /* waited the most this time: no wait again until it is drawn */
    unsigned int since;                            /* when the wait began, in ms (32 bits, wrapping) */
} ob_hold;

/* One look. bench: GFOpenBench's port is there; drawn: it says the desktop is drawn; now: a clock in ms (it may wrap) */
static inline int ob_hold_step(ob_hold *h, int bench, int drawn, unsigned int now)
{
    if (!bench || drawn) {
        h->spent = 0;
        if (h->held) { h->held = 0; return OBH_RELEASE; }
        return OBH_GO;
    }
    if (h->spent) return OBH_GO;
    if (!h->held) { h->held = 1; h->since = now; return OBH_HOLD; }
    if (now - h->since >= OB_DRAWN_CAP_MS) { h->held = 0; h->spent = 1; return OBH_RELEASE; }
    return OBH_HOLD;
}

#ifndef OB_DRAWN_HOST
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <proto/exec.h>

struct ob_drawn_port {
    struct MsgPort mp;                             /* named OB_DRAWN_PORT; no messages */
    ULONG magic;
    volatile LONG drawn;
};

/* What GFOpenBench says now: *bench 1 when its port is there, *drawn 1 when its desktop is drawn. */
static __attribute__((unused)) void ob_drawn_look(int *bench, int *drawn)
{
    struct ob_drawn_port *p;
    Forbid();
    p = (struct ob_drawn_port *)FindPort((CONST_STRPTR)OB_DRAWN_PORT);
    *bench = p && p->magic == OB_DRAWN_MAGIC;
    *drawn = *bench && p->drawn;
    Permit();
}
#endif

#endif
