/* Host tests for Blanker/src/bl_wordmark.h: the wordmark is GFOS, the credit
 * line is the team's, and the parts centre.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "bl_wordmark.h"

static int fails, oks;
#define CHECK(c, what) do { if (c) { oks++; printf("ok   %s\n", what); } else { fails++; printf("FAIL %s\n", what); } } while (0)

static int has_ci(const char *s, const char *w)
{
    size_t n = strlen(w), i, k;
    for (i = 0; s[i]; i++) {
        for (k = 0; k < n && s[i + k] && tolower((unsigned char)s[i + k]) == w[k]; k++) ;
        if (k == n) return 1;
    }
    return 0;
}

int main(void)
{
    char all[64] = "";
    int i, w[4] = { 40, 60 }, lens_ok = 1;
    for (i = 0; i < BL_WORDMARK_PARTS; i++) {
        strcat(all, bl_wordmark[i].text);
        if ((int)strlen(bl_wordmark[i].text) != bl_wordmark[i].len) lens_ok = 0;
    }
    CHECK(strcmp(all, "GFOS") == 0, "the wordmark reads GFOS");
    CHECK(lens_ok, "each part's length is its text's");
    CHECK(!has_ci(all, "chrome") && !has_ci(all, "amiga"), "the wordmark holds no part of the old name");
    CHECK(strcmp(BL_CREDIT, "The AmiCradle Team") == 0, "the credit line is The AmiCradle Team");
    CHECK(!has_ci(BL_CREDIT, "chrome"), "the credit line holds no part of the old name");
    CHECK(bl_wordmark_left(640, w, 2) == 270, "two parts 40 + 60 wide centre at 270 on 640");
    CHECK(bl_wordmark_left(1920, w, 1) == 940, "one part 40 wide centres at 940 on 1920");
    CHECK(bl_wordmark_left(30, w, 2) == -35, "wider than the screen: left of 0, as Text() clips");
    printf("%d checks\n%s\n", oks + fails, fails ? "FAIL" : "PASS");
    return fails != 0;
}
