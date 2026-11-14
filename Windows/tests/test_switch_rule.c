/* GFOpenWindows' Amiga+Tab rule (src/switch_rule.h) on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "../src/switch_rule.h"

static int checks, fails;
static void check(int got, int want, const char *what)
{
    checks++;
    if (got != want) { fails++; printf("FAIL %s: got %d, want %d\n", what, got, want); }
}

int main(void)
{
    /* a plain application window: drag bar, depth, close */
    check(switch_rule_offers(SWR_DRAGBAR | SWR_DEPTHGADGET | SWR_CLOSEGADGET, 220, 90), 1, "app window");
    /* each system gadget alone is enough */
    check(switch_rule_offers(SWR_DRAGBAR, 220, 90), 1, "drag bar only");
    check(switch_rule_offers(SWR_DEPTHGADGET, 220, 90), 1, "depth only");
    check(switch_rule_offers(SWR_CLOSEGADGET, 220, 90), 1, "close only");
    check(switch_rule_offers(SWR_SIZEGADGET, 220, 90), 1, "size only");
    /* the lab's furniture, flags as WBGapTest W2 read them: borderless + RMB trap, no system gadget */
    check(switch_rule_offers(0x10800UL, 25, 23), 0, "tray button 25x23");
    check(switch_rule_offers(0x10800UL, 113, 23), 0, "tray clock 113x23");
    check(switch_rule_offers(0x00840UL, 200, 120), 0, "borderless panel (GFOpenMeters)");
    /* backdrops, tiny ones, Workbench's root */
    check(switch_rule_offers(SWR_BACKDROP | SWR_DRAGBAR, 640, 480), 0, "backdrop");
    check(switch_rule_offers(SWR_DRAGBAR | SWR_CLOSEGADGET, 7, 90), 0, "too narrow");
    check(switch_rule_offers(SWR_DRAGBAR | SWR_CLOSEGADGET, 90, 7), 0, "too low");
    check(switch_rule_offers(SWR_DRAGBAR | SWR_CLOSEGADGET, 8, 8), 1, "8x8 is enough");
    check(switch_rule_offers(SWR_WBENCHWINDOW | SWR_DRAGBAR | SWR_DEPTHGADGET, 640, 480), 0, "Workbench root");
    check(switch_rule_offers(SWR_WBENCHWINDOW | SWR_DRAGBAR | SWR_CLOSEGADGET, 300, 200), 1, "Workbench drawer");
    printf("test_switch_rule: %d checks, %d failures\n", checks, fails);
    return fails ? 1 : 0;
}
