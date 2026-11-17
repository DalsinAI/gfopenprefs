/* The bar logo's size (Common/barlogo_size.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "barlogo_size.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

int main(void)
{
    short w, h, bar;
    h = barlogo_size(22, &w);                       /* the Pi 5's bar: 20 high before, 18 now */
    CHECK(h == 18 && w == 24, "a 22 pixel bar: %d x %d", w, h);
    for (bar = 0; bar <= 120; bar++) {
        h = barlogo_size(bar, &w);
        CHECK(h >= 8 && h <= BARLOGO_PIC_H, "bar %d: height %d", bar, h);
        CHECK(bar < 12 || h <= bar - 2, "bar %d: %d does not fit", bar, h);
        CHECK(bar < 12 || bar > 50 || (h * 10 <= (bar - 2) * 9 && (h + 1) * 10 > (bar - 2) * 9), "bar %d: %d is not a tenth smaller", bar, h);
        CHECK(w * BARLOGO_PIC_H >= h * BARLOGO_PIC_W - BARLOGO_PIC_H && w * BARLOGO_PIC_H <= h * BARLOGO_PIC_W + BARLOGO_PIC_H,
              "bar %d: %d x %d loses the picture's shape", bar, w, h);
    }
    printf(fails ? "logo size: %d FAILED\n" : "logo size: ok\n", fails);
    return fails != 0;
}
