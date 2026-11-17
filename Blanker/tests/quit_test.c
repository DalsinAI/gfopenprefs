/* GFOpenBlanker's quit while its show is up (bl_quit.h, 1.8). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "bl_quit.h"

static int fails;
#define CHECK(what, ok) do { if (ok) printf("ok   %s\n", what); else { printf("FAIL %s\n", what); fails++; } } while (0)

int main(void)
{
    int on = 1, quit = 0;
    CHECK("the show goes on while nothing has happened", bl_show_goes_on(0, 0));
    CHECK("a key or the mouse ends the show", !bl_show_goes_on(1, 0));
    CHECK("a quit ends the show with no key or mouse", !bl_show_goes_on(0, 1));
    CHECK("Exchange's Kill asks it to quit and leaves the broker alone", bl_cx_command(CXCMD_KILL, &on, &quit) == -1 && quit == 1 && on == 1);
    CHECK("... so the show stops at its next frame", !bl_show_goes_on(0, quit));
    on = 1; quit = 0;
    CHECK("Disable turns it and its broker off, and doesn't quit", bl_cx_command(CXCMD_DISABLE, &on, &quit) == 0 && on == 0 && quit == 0);
    CHECK("Enable turns it and its broker on", bl_cx_command(CXCMD_ENABLE, &on, &quit) == 1 && on == 1 && quit == 0);
    CHECK("started again (Unique): nothing changes", bl_cx_command(CXCMD_UNIQUE, &on, &quit) == -1 && on == 1 && quit == 0);
    CHECK("an unknown command: nothing changes", bl_cx_command(9999, &on, &quit) == -1 && on == 1 && quit == 0);
    printf(fails ? "FAIL\n" : "PASS\n");
    return fails != 0;
}
