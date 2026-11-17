/* The bar logo's size, apart from barlogo.h's Amiga calls so the host tests read it (TitleBar/tests/test_logo.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef BARLOGO_SIZE_H
#define BARLOGO_SIZE_H

#define BARLOGO_PIC_W 58                /* the logo's picture (GFOpenTitle's logo_big.h: the Deep cradle, 14 Nov 2026) */
#define BARLOGO_PIC_H 44
#define BARLOGO_GAP 6                   /* between the logo's window and the title's first letter */
#define BARLOGO_MIN_SPACES 9
#define BARLOGO_MAX_SPACES 40

/* The logo's height for a bar of bar_h pixels, and its width in *w: a tenth smaller than the bar allows (Dale, 17
 * November 2026, of the Deep cradle on the Pi 5's bar: "seems perhaps 10% too big"; it was the bar less 2). */
static inline short barlogo_size(short bar_h, short *w)
{
    short h = (short)((bar_h - 2) * 9 / 10);
    if (h > BARLOGO_PIC_H) h = BARLOGO_PIC_H;
    if (h < 8) h = 8;
    *w = (short)((h * BARLOGO_PIC_W + BARLOGO_PIC_H / 2) / BARLOGO_PIC_H);
    return h;
}

#endif
