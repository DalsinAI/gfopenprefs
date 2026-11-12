/* Host test for GFOpenNetwork's pictures (src/net_pics.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "net_pics.h"

static int fails, checks;
#define CHECK(c, msg) do { checks++; if (!(c)) { fails++; printf("FAIL %s\n", msg); } } while (0)

static int count(const unsigned char *p) { int i, n = 0; for (i = 0; i < NP_W * NP_H; i++) n += p[i]; return n; }

int main(void)
{
    unsigned char a[NP_W * NP_H], b[NP_W * NP_H], c[NP_W * NP_H];
    int i;
    np_lan(a, 0); np_lan(b, 1); np_lan(c, 2);
    CHECK(count(c) > count(b), "connected LAN is filled: more pixels than hollow");
    CHECK(a[0] && a[NP_W - 1] && a[10 * NP_W] && a[10 * NP_W + 15], "no network: crossed corner to corner");
    CHECK(!b[0 * NP_W + 0] && b[0 * NP_W + 5], "not connected: hollow box, no cross");
    CHECK(c[2 * NP_W + 7] && !b[2 * NP_W + 7], "connected: the top box filled inside");
    for (i = 0; i <= 3; i++) {
        np_wifi(a, i);
        CHECK(a[10 * NP_W + 8] && a[9 * NP_W + 7], "Wi-Fi: the dot");
    }
    np_wifi(a, 1); np_wifi(b, 2); np_wifi(c, 3);
    CHECK(count(a) < count(b) && count(b) < count(c), "Wi-Fi: more arcs, more pixels");
    CHECK(c[0 * NP_W + 8], "three arcs reach the top row");
    np_wifi(a, 0);
    CHECK(a[2 * NP_W + 10] && a[7 * NP_W + 15], "not connected: the cross");
    CHECK(np_arcs(0, 90) == 0 && np_arcs(1, 90) == 3 && np_arcs(1, 50) == 2 && np_arcs(1, 10) == 1, "arcs by signal");
    printf("%s   %d checks (LAN and Wi-Fi pictures)\n", fails ? "FAIL" : "ok", checks);
    return fails != 0;
}
