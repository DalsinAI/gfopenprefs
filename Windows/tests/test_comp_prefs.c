/* The Windows editor's composited-mode lines and words (src/comp_prefs.h) on the host.
 *   Windows/tests/run.sh
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "../src/comp_prefs.h"

static int fails, checks;
#define CHECK(c) do { checks++; if (!(c)) { fails++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

int main(void)
{
    CHECK(comp_prefs_mode("composited") == 1);
    CHECK(comp_prefs_mode("composited\n") == 1);
    CHECK(comp_prefs_mode("direct") == 0);
    CHECK(comp_prefs_mode("") == 0);
    CHECK(comp_prefs_mode("compose") == 0);
    CHECK(!strcmp(comp_prefs_mode_name(0), "direct") && !strcmp(comp_prefs_mode_name(1), "composited"));
    /* a line written is read back the same */
    CHECK(comp_prefs_mode(comp_prefs_mode_name(1)) == 1 && comp_prefs_mode(comp_prefs_mode_name(0)) == 0);
    CHECK(comp_prefs_why(0) == 0 && comp_prefs_why(10) == 0);
    CHECK(!strcmp(comp_prefs_why(1), "not asked"));
    CHECK(!strcmp(comp_prefs_why(2), "switched off"));
    CHECK(!strcmp(comp_prefs_why(4), "8-bit"));
    CHECK(!strcmp(comp_prefs_why(8), "no memory"));
    CHECK(!strcmp(comp_prefs_why(9), "screen direct mode"));
    if (fails) { printf("test_comp_prefs: %d of %d failed\n", fails, checks); return 1; }
    printf("test_comp_prefs: %d checks, ok\n", checks);
    return 0;
}
