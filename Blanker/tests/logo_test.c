/* Host tests for the show's last card: the logo (bl_logo.h) and the credit
 * line (bl_wordmark.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "bl_wordmark.h"
#include "bl_logo.h"

static int fails, oks;
#define CHECK(c, what) do { if (c) { oks++; printf("ok   %s\n", what); } else { fails++; printf("FAIL %s\n", what); } } while (0)

int main(void)
{
    long sum = 0, inked = 0;
    int i, bad = 0, used[BL_LOGO_COLOURS], all = 1;
    memset(used, 0, sizeof used);
    for (i = 0; i < BL_LOGO_RUNS; i++) {
        int n = bl_logo_rle[i * 2], c = bl_logo_rle[i * 2 + 1];
        if (!n || c >= BL_LOGO_COLOURS) bad++; else used[c] = 1;
        sum += n;
        if (c) inked += n;
    }
    for (i = 1; i < BL_LOGO_COLOURS; i++) if (!used[i]) all = 0;
    CHECK(strcmp(BL_CREDIT, "The AmiCradle Team") == 0, "the credit line is The AmiCradle Team");
    CHECK(!bad, "every run has a length and a colour the palette holds");
    CHECK(sum == (long)BL_LOGO_W * BL_LOGO_H_PX, "the runs fill the logo's rectangle exactly");
    CHECK(inked > sum / 4 && inked < sum, "the logo is a picture on the background, not a full or an empty box");
    CHECK(used[0] && all, "the background and every colour are used");
    CHECK(BL_LOGO_COLOURS <= 32 && BL_LOGO_W <= 320 && BL_LOGO_H_PX <= 200, "it fits a 320 x 200 screen and 32 pens");
    CHECK(bl_logo_scale(200) == 1 && bl_logo_scale(480) == 1 && bl_logo_scale(768) == 1, "one screen pixel a logo pixel up to 959 lines");
    CHECK(bl_logo_scale(1080) == 2 && bl_logo_scale(1200) == 2 && bl_logo_scale(2160) == 4, "2 at 1080 and 1200 lines, 4 at 2160");
    CHECK(bl_logo_left(1920, 2) == (1920 - BL_LOGO_W * 2) / 2 && bl_logo_left(320, 1) >= 0, "it is centred");
    printf("%d checks\n%s\n", oks + fails, fails ? "FAIL" : "PASS");
    return fails != 0;
}
