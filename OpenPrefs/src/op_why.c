/* op_why.c: what GFOpenPrefs says when a page can't start (opframe.c lt_page_say), in plain C with no Amiga
 * headers, so the host test (OpenPrefs/tests/why_test.c) builds it with the PC's compiler.
 *
 * Instance-40, 4 November 2026: its disk had no iffparse.library, and Screen Mode, Pointer, Fonts, Colours,
 * Backdrops, Workbench, File Requesters, IControl and Printing each ended GFOpenPrefs with no word (return code
 * 20). Now the library and the version the page asked for are named, and the window stays.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "op_why.h"

/* A library a page couldn't open, added to list ("iffparse.library version 39, diskfont.library version 36"),
 * each one once; one that doesn't fit is left out */
void op_note_lib(char *list, size_t size, const char *name, unsigned long version)
{
    char one[72];
    size_t n;
    if (!list || !size || !name || !*name) return;
    if (version) snprintf(one, sizeof one, "%s version %lu", name, version);
    else snprintf(one, sizeof one, "%s", name);
    if (strstr(list, one)) return;
    n = strlen(list);
    if (n + strlen(one) + (n ? 2 : 0) + 1 > size) return;
    if (n) strcat(list, ", ");
    strcat(list, one);
}

/* The requester's text for page, which ended with rc before its window, having failed to open missing ("" or
 * NULL: none) */
void op_why(char *text, size_t size, const char *page, int rc, const char *missing)
{
    if (!text || !size) return;
    if (!page) page = "That section";
    if (missing && *missing)
        snprintf(text, size, "%s can't open: it needs %s,\nwhich isn't installed (LIBS:).\n\n"
                 "On the open ROM, the OpenKick boot disk's\nInstall-OpenKick puts it there.", page, missing);
    else
        snprintf(text, size, "%s couldn't open (return code %d).", page, rc);
}
