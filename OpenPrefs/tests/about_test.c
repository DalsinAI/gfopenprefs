/* GFOpenPrefs' About (op_about.h): the words olabout shows and where the button stands under the list.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "op_about.h"

static int fails, n;
static void check(const char *what, int ok) { n++; if (!ok) { fails++; printf("FAIL %s\n", what); } }

int main(void)
{
    check("the name is GFOpenPrefs", !strcmp(OP_ABOUT_NAME, "GFOpenPrefs"));
    check("the version is GFOpenPrefs' own", !strcmp(OP_ABOUT_VERSION, OP_VERSION));
    check("the version is 3.1 or later", strcmp(OP_VERSION, "3.1") >= 0);
    check("the text names the Team's licence", strstr(OP_ABOUT_TEXT, "AmiCradle Team") != NULL);
    check("no AmiCradle logo wording (the GFOS mark is drawn, not named)", strstr(OP_ABOUT_TEXT, "logo") == NULL);
    check("the text has no empty line", strstr(OP_ABOUT_TEXT, "\n\n") == NULL);
    check("the button says About", !strcmp(OP_ABOUT_LABEL, "About"));
    /* a 640 x 512 screen with an 8 px font: the list from 20, 25 rows of 13 and 6 gaps of 4 */
    {
        int ly = 20, lh = 25 * 13 + 6 * 4, rowh = 15, top = 11;
        int y = op_about_y(ly, lh);
        check("the button stands below the list's recessed box", y > ly + lh + 2);
        check("with the gap", y == ly + lh + 2 + OP_ABOUT_GAP);
        check("the window is tall enough for it", top + op_about_min_h(ly, lh, rowh, top) >= y + rowh + 2);
        check("and taller than for the list alone", op_about_min_h(ly, lh, rowh, top) > lh + 8 + (ly - top));
    }
    printf("%s about_test: %d checks\n", fails ? "FAIL" : "ok  ", n);
    return fails != 0;
}
