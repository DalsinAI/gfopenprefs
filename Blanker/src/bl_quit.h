/* GFOpenBlanker's quit, also while its show is up (1.8): what a commodity
 * command or a break asks, kept apart from the Amiga calls so the host can
 * test it (tests/quit_test.c). Before 1.8 the show looked only for a key or
 * the mouse, so a Break (CTRL_C) or Exchange's Kill waited until someone
 * moved the mouse, and a new GFOpenBlanker could not take over from one
 * that was showing.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef BL_QUIT_H
#define BL_QUIT_H

#ifndef CXCMD_DISABLE                 /* libraries/commodities.h's, for the host */
#define CXCMD_DISABLE 15
#define CXCMD_ENABLE  17
#define CXCMD_KILL    23
#define CXCMD_UNIQUE  25
#endif

/* What a commodity command does: *on and *quit change; the answer is the
 * broker's new state (1 on, 0 off) or -1 to leave it as it is. */
static int bl_cx_command(unsigned long id, int *on, int *quit)
{
    switch (id) {
    case CXCMD_DISABLE: *on = 0; return 0;
    case CXCMD_ENABLE: *on = 1; return 1;
    case CXCMD_KILL: *quit = 1; return -1;
    default: return -1;               /* CXCMD_UNIQUE: started again, nothing (NOW asks by signal) */
    }
}

/* The show goes on to the next frame while no key or mouse has woken it
 * and no quit has been asked for. */
static int bl_show_goes_on(int woken, int quit)
{
    return !woken && !quit;
}

#endif
