/* GFOpenTitle 0.11 (Dale, 14 November 2026, 03:44: "make sure the new gfopensystray is that CX Exchange replacement"):
 * whose menu the menu button opens, which broker a picture item's program is, and which brokers the systray's strip
 * shows. Plain C, host-tested (tests/test_tb.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef TB_OWNER_H
#define TB_OWNER_H

#define TBO_TS 50                              /* a picture item's menu: TBO_TS + its slot (fewer than 50 slots) */
#define TBO_CX 100                             /* a strip letter's: TBO_CX + its place in the strip */

/* cog: OW_PREFS or OW_POWER under the pointer, else 0; cx: the strip's letter under it, else -1; ts: the picture
 * item's slot under it, else -1. 0: nothing's menu. */
static inline int tbo_owner(int cog, int cx, int ts)
{
    if (cog) return cog;
    if (cx >= 0) return TBO_CX + cx;
    if (ts >= 0) return TBO_TS + ts;
    return 0;
}

/* the broker whose program (task) has the picture item: its place in tasks[n], or -1 (no broker: no menu) */
static inline int tbo_broker_of(const void *const *tasks, int n, const void *task)
{
    int i;
    if (!task) return -1;
    for (i = 0; i < n; i++) if (tasks[i] == task) return i;
    return -1;
}

/* a broker gets a letter in the strip when it isn't GFOpenTitle's own, is active (a disabled one is off the bar,
 * as a disabled GFOpenSpeaker or GFOpenNetwork takes its icons away; it stays in Commodities... to enable again),
 * and has no picture item of its own */
static inline int tbo_in_strip(int ours, int active, int has_icons)
{
    return !ours && active && !has_icons;
}

#endif
