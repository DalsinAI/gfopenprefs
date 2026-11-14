/* GFOpenWindows' Amiga+Tab rule: which windows the switcher cycles, plain C so the host tests read the same rule
 * (and WBGapTest's W2 row, gfopenbench tests/amiga, counts the same way).
 *
 * A window is offered when a hand could arrange it: not a backdrop, not tiny, not Workbench's own root window (a
 * Workbench window with no close gadget), and it has at least one system gadget (drag bar, depth, close or size).
 * 0.12.3 and before offered every window that wasn't a backdrop or tiny, so desktop furniture, the borderless
 * screen-bar tray buttons (25x23) and panels such as GFOpenMeters, came to the front on Amiga+Tab and couldn't be
 * made the active window: Amiga+Tab from two windows showed a tray button over them (WBGapTest W2, 14 Nov 2026).
 * The flag values are Intuition's (intuition/intuition.h), copied so the host build needs no NDK.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SWITCH_RULE_H
#define SWITCH_RULE_H

#define SWR_SIZEGADGET   0x00000001UL
#define SWR_DRAGBAR      0x00000002UL
#define SWR_DEPTHGADGET  0x00000004UL
#define SWR_CLOSEGADGET  0x00000008UL
#define SWR_BACKDROP     0x00000100UL
#define SWR_WBENCHWINDOW 0x02000000UL
#define SWR_MIN          8      /* narrower or lower than this: not a window a hand picks */

/* flags: the window's Flags; 1 when Amiga+Tab offers it */
static inline int switch_rule_offers(unsigned long flags, int width, int height)
{
    if ((flags & SWR_BACKDROP) || width < SWR_MIN || height < SWR_MIN) return 0;
    if ((flags & SWR_WBENCHWINDOW) && !(flags & SWR_CLOSEGADGET)) return 0;
    if (!(flags & (SWR_DRAGBAR | SWR_DEPTHGADGET | SWR_CLOSEGADGET | SWR_SIZEGADGET))) return 0;
    return 1;
}

#endif
