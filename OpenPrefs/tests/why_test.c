/* why_test.c: GFOpenPrefs names what a page that can't start needs (op_why.c), with the PC's compiler.
 * Instance-40, 4 November 2026: nine pages ended GFOpenPrefs with no word when iffparse.library wasn't there.
 *   cc -I OpenPrefs/src -o why_test OpenPrefs/tests/why_test.c OpenPrefs/src/op_why.c && ./why_test
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "op_why.h"

static int fails;
static void want(int ok, const char *what, const char *got)
{
    if (!ok) { printf("FAIL %s: \"%s\"\n", what, got); fails++; }
}

int main(void)
{
    char list[200] = "", text[400], small[24] = "";

    op_note_lib(list, sizeof list, "iffparse.library", 39);
    want(!strcmp(list, "iffparse.library version 39"), "one library", list);
    op_note_lib(list, sizeof list, "iffparse.library", 39);         /* pf_load, then pf_open_lib again */
    want(!strcmp(list, "iffparse.library version 39"), "the same library once", list);
    op_note_lib(list, sizeof list, "asl.library", 38);
    want(!strcmp(list, "iffparse.library version 39, asl.library version 38"), "two libraries", list);
    op_note_lib(list, sizeof list, "gfopeninput.library", 0);
    want(strstr(list, ", gfopeninput.library") && !strstr(list, "gfopeninput.library version"), "any version", list);
    op_note_lib(list, sizeof list, NULL, 39);
    op_note_lib(list, sizeof list, "", 39);
    want(!strstr(list, ", ,") && list[strlen(list) - 1] != ' ', "no name, nothing added", list);

    op_note_lib(small, sizeof small, "iffparse.library", 39);       /* 27 characters: doesn't fit, left out */
    want(small[0] == 0, "one that doesn't fit is left out", small);
    op_note_lib(small, sizeof small, "z.library", 1);
    want(!strcmp(small, "z.library version 1"), "one that fits", small);

    op_why(text, sizeof text, "Screen Mode", 20, "iffparse.library version 39");
    want(!strncmp(text, "Screen Mode can't open: it needs iffparse.library version 39,", 61), "the page and the library", text);
    want(strstr(text, "LIBS:") != NULL, "where it goes", text);
    want(strstr(text, "Install-OpenKick") != NULL, "what puts it there", text);
    want(strchr(text, '%') == NULL, "no format left in it", text);

    op_why(text, sizeof text, "Printing", 20, "");
    want(!strcmp(text, "Printing couldn't open (return code 20)."), "no library: the return code", text);
    op_why(text, sizeof text, NULL, 10, NULL);
    want(!strcmp(text, "That section couldn't open (return code 10)."), "no page name", text);
    op_why(text, 16, "Screen Mode", 20, "iffparse.library version 39");
    want(strlen(text) == 15, "cut to the buffer", text);

    if (!fails) printf("ok   a page that can't start is named with the library and version it needs\n");
    return fails != 0;
}
