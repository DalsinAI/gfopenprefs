/* Host checks for Common/aatext.h's own arithmetic: which text goes to OLUI_AmigaText (plain ASCII), the pen's
 * colour as 0xRRGGBB from GetRGB32's guns, and the baseline that centres it (the same as Text()'s before).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#define AAT_HOST
#include "aatext.h"

static int fails;
#define CHECK(c, what) do { if (c) printf("ok   %s\n", what); else { printf("FAIL %s\n", what); fails++; } } while (0)

int main(void)
{
    static const unsigned long white[3] = { 0xFFFFFFFFUL, 0xFFFFFFFFUL, 0xFFFFFFFFUL };
    static const unsigned long blue[3] = { 0x11223344UL, 0x55667788UL, 0x99AABBCCUL };
    CHECK(aat_ascii("Thu 12 Nov  15:50", 17), "the clock is ASCII");
    CHECK(!aat_ascii("Caf\xe9", 4), "Latin-1 isn't: Text() draws it");
    CHECK(aat_ascii("Caf\xe9", 3), "only the n bytes count");
    CHECK(aat_ascii("", 0), "nothing is ASCII");
    CHECK(aat_argb(white) == 0xFFFFFFUL, "white");
    CHECK(aat_argb(blue) == 0x1155 * 0x100UL + 0x99, "each gun's top byte, red high");
    /* Text()'s old baseline: top + (h - TxHeight) / 2 + TxBaseline */
    CHECK(aat_baseline(0, 20, 7, 2) == (20 - 9) / 2 + 7, "Topaz 8 in a 20-row bar");
    CHECK(aat_baseline(3, 13, 10, 3) == 3 + 10, "a box the text's own height: the baseline at top + ascent");
    CHECK(aat_baseline(0, 11, 11, 3) == (11 - 14) / 2 + 11, "taller than the box: centred, as Text()");
    if (fails) return 1;
    printf("PASS\n");
    return 0;
}
