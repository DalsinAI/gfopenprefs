/* GFOpenTitle 0.12.5: the power menu's rows, what each asks first, and what the host's answer says, kept apart so the
 * host tests check them (tests/test_tb.c).
 *
 * Reboot is the Amiga's own (ColdReboot), asked nothing. Restart the machine..., Restart into AKS... and Power off...
 * ask first, then go to the host through accontrol.device's ACCMD_POWER: io_Offset 1 restart, 2 power off, 3 restart
 * into the Amiga Kickstart Screen (accontrol.device 1.10; AC-125, Dale, 23 November 2026: "add a feature to the power
 * menu to reboot into AKS"). On the appliance the machine leaves a one-shot flag and the boot screen opens the Amiga
 * Kickstart Screen at the next start, in place of its F2/Del/F12 countdown; the start after that is as set. A PC has
 * no boot screen: there it is a plain restart.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef TB_POWER_H
#define TB_POWER_H

#define PW_REBOOT  1                           /* warm: the Amiga's own ColdReboot */
#define PW_RESTART 2                           /* cold: the host starts the machine again (accontrol.device) */
#define PW_OFF     3                           /* the card shuts down; on a PC the instance closes */
#define PW_AKS     4                           /* 0.12.5: the host starts the machine again into the Amiga Kickstart Screen */
#define TB_POWER_ROWS 4                        /* the rows under the power menu's line, in this order: */
#define TB_POWER_ORDER { PW_REBOOT, PW_RESTART, PW_AKS, PW_OFF }

#define TB_HOST_RESTART 1                      /* ACCMD_POWER's io_Offset (amicradle-guest accontrol.h ACC_POWER_*) */
#define TB_HOST_OFF     2
#define TB_HOST_AKS     3                      /* accontrol.device 1.10 */

#define TB_ERR_NOPOWER  48                     /* ACCERR_NOPOWER: no restart and power off on this board */
#define TB_ERR_OPENFAIL (-1)                   /* exec's IOERR_OPENFAIL: no accontrol.device */
#define TB_ERR_BADADDR  (-5)                   /* exec's IOERR_BADADDRESS: an accontrol.device before 1.10 takes no 3 */

#define TB_PW_REBOOT_LABEL  "Reboot"          /* the rows' words, for cog_items and the tests */
#define TB_PW_RESTART_LABEL "Restart the machine..."
#define TB_PW_AKS_LABEL     "Restart into AKS..."
#define TB_PW_OFF_LABEL     "Power off..."

struct tb_power_ask { const char *label, *title, *text, *buttons; unsigned long host; };

/* a power row's words and the host's offset; host 0 for Reboot (nothing asked, nothing sent); NULL for no such row */
static __attribute__((unused)) const struct tb_power_ask *tb_power_ask(int what)
{
    static const struct tb_power_ask rows[] = {
        { TB_PW_REBOOT_LABEL, 0, 0, 0, 0 },
        { TB_PW_RESTART_LABEL, "Restart the machine", "Restart the machine from cold?\nWork not saved in open programs is lost.",
          "Restart|Cancel", TB_HOST_RESTART },
        { TB_PW_OFF_LABEL, "Power off", "Power the machine off?\nWork not saved in open programs is lost.", "Power off|Cancel", TB_HOST_OFF },
        { TB_PW_AKS_LABEL, "Restart into AKS",
          "Restart the machine into the Amiga Kickstart Screen?\nIt opens at the next start, in place of the countdown.\n"
          "Work not saved in open programs is lost.\n(On a PC the machine restarts as usual.)", "Restart|Cancel", TB_HOST_AKS },
    };
    return what >= PW_REBOOT && what <= PW_AKS ? &rows[what - 1] : 0;
}

/* what the bar says after the host answered err (DoIO's io_Error) for offset host: NULL when it was taken */
static __attribute__((unused)) const char *tb_power_said(unsigned long host, int err)
{
    if (!err) return 0;
    if (host == TB_HOST_AKS) {
        if (err == TB_ERR_BADADDR) return "Restarting into the Amiga Kickstart Screen needs accontrol.device 1.10 or newer.";
        if (err == TB_ERR_NOPOWER || err == TB_ERR_OPENFAIL) return "This machine can't be restarted into the Amiga Kickstart Screen from here.";
        return "The host didn't restart the machine into the Amiga Kickstart Screen.";
    }
    if (err == TB_ERR_NOPOWER || err == TB_ERR_OPENFAIL)
        return host == TB_HOST_RESTART ? "This machine can't be restarted from here." : "This machine can't be powered off from here.";
    return host == TB_HOST_RESTART ? "The host didn't restart the machine." : "The host didn't power the machine off.";
}

#endif
