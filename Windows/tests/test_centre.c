/* GFOpenWindows 0.12: centring and the snapshot gadget by default (src/centre_prefs.h), on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "../src/centre_prefs.h"

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); } } while (0)

static void one(int ver, int cl, int gl, int want_c, int want_g, const char *what)
{
    int c = -9, g = -9;
    centre_prefs_resolve(ver, cl, gl, &c, &g);
    CHECK(c == want_c && g == want_g, "%s: centre %d gadget %d (want %d %d)", what, c, g, want_c, want_g);
}

int main(void)
{
    one(0, -1, -1, 1, -1, "no file or no lines: centring on, the gadget follows (on)");
    one(0, 0, -1, 1, -1, "0.8.14's editor wrote centre off: never chosen, on");
    one(0, 0, 0, 1, -1, "0.8.14's centre off and snapshot.gadget off: never chosen, on");
    one(0, 1, -1, 1, -1, "0.8.14's centre on: on");
    one(1, 0, 0, 1, -1, "an unknown older mark: never chosen");
    one(2, 0, -1, 0, -1, "0.12's editor: centre off chosen, the gadget follows (off)");
    one(2, 1, 0, 1, 0, "0.12's editor: centre on, gadget off chosen");
    one(2, 0, 1, 0, 1, "0.12's editor: centre off, gadget on chosen");
    one(2, -1, -1, 1, -1, "a mark with no lines: the defaults");
    one(3, 0, -1, 0, -1, "a later mark is a choice too");
    printf("test_centre: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
