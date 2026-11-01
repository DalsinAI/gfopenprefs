/* GFOpenPrefs APPLY: the one boot call in place of User-Startup's
 *     SYS:Prefs/GFOpenSound USE
 *     SYS:Prefs/GFOpenGamepads USE        (GFOpenUp's GFOpenInput part)
 *     GFOpenSystem APPLY                  (System/DESIGN.md)
 * (Dale, 17 October 2026, 08:24). Each is that editor's own Shell path, run with the same arguments
 * (lt_args): Sound's levels to the ACAHI board and its files, Gamepads' settings to gfopeninput.library (only
 * when gfopeninput.library opens, as its line was only there with GFOpenInput), and the stack caps to
 * AmiCradle's runtime; GFOpenSystem's "no runtime here" line is not printed at boot (its return code 5 is
 * kept out of APPLY's). On an open ROM whose prefs task applies them (prefs.resource 47.101: intuition's
 * prefs_boot.c reads the same three files at the first pass, earlier and with no program) the line goes
 * (GFOpenUp, salvor/Design-No-ROM-Volume-20261024.md, section 9.3). It stays for OS 3.2.3's own ROM, as
 * SYS:Preferences/GFOpenPrefs APPLY, by its full path (the small body, C:GFOpenPrefs, is no longer built).
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "openprefs.h"

extern const char *lt_args;
char *op_argv[] = { (char *)"GFOpenPrefs", NULL };

int op_apply(void)
{
    int rc, r;
    struct Library *oi;
    BPTR nil, was;
    lt_args = "USE\n";
    rc = sound_main(1, op_argv);
    if ((oi = OpenLibrary((STRPTR)"gfopeninput.library", 0))) {
        CloseLibrary(oi);
        lt_args = "USE\n";
        if ((r = gamepads_main(1, op_argv)) > rc) rc = r;
    }
    lt_args = "APPLY\n";
    if ((nil = Open((STRPTR)"NIL:", MODE_NEWFILE))) {
        was = SelectOutput(nil);
        r = system_main(1, op_argv);
        SelectOutput(was);
        Close(nil);
        if (r > RETURN_WARN && r > rc) rc = r;
    }
    lt_args = NULL;
    return rc;
}
