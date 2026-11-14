/* GFOpenBlanker's wordmark: what the first and last cards draw, and the credit
 * line under the last. 1.5 (14 November 2026): the plain wordmark "GFOS" in
 * text, until Dale picks a GFOS logo; the logo goes in by changing
 * BL_WORDMARK_PARTS (or by drawing an image where bl_wordmark_part is drawn),
 * nothing else. Each part is drawn in its own pen, side by side, centred.
 * Nothing here may spell the old name, split or joined (Blanker/tests/run.sh
 * greps the source and the built binary for it).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef BL_WORDMARK_H
#define BL_WORDMARK_H

enum { BLW_RED, BLW_INK };
typedef struct { const char *text; int len; int pen; } bl_wordmark_part;

static const bl_wordmark_part bl_wordmark[] = {
    { "GFOS", 4, BLW_INK },
};
#define BL_WORDMARK_PARTS (int)(sizeof bl_wordmark / sizeof bl_wordmark[0])
#define BL_CREDIT "The AmiCradle Team"

/* The left edge that centres parts of these widths on a screen W wide. */
static int bl_wordmark_left(int W, const int *widths, int n)
{
    int i, sum = 0;
    for (i = 0; i < n; i++) sum += widths[i];
    return (W - sum) / 2;
}

#endif
