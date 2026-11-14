/* Host tests for GFOpenSpeaker 0.3's commodity options and pictures (src/sp_cx.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "sp_cx.h"
#include <stdio.h>
#include <string.h>

static int fails, checks;
#define CHECK(c, what) do { checks++; if (c) printf("ok   %s\n", what); else { fails++; printf("FAIL %s (line %d)\n", what, __LINE__); } } while (0)

static unsigned char pic[SPX_STATES][SPX_W * SPX_H];
static int at(int s, int x, int y) { return x >= 0 && y >= 0 && x < SPX_W && y < SPX_H && pic[s][y * SPX_W + x]; }

/* the lit pixels right of the cone (x > 7, the waves' room) form exactly n pieces, each 8-connected */
static int pieces(int s)
{
    unsigned char seen[SPX_W * SPX_H] = { 0 };
    int n = 0, x, y;
    for (y = 0; y < SPX_H; y++)
        for (x = 8; x < SPX_W; x++)
            if (at(s, x, y) && !seen[y * SPX_W + x]) {
                int stack[SPX_W * SPX_H], sp = 0;
                n++;
                stack[sp++] = y * SPX_W + x; seen[y * SPX_W + x] = 1;
                while (sp) {
                    int p = stack[--sp], px = p % SPX_W, py = p / SPX_W, dx, dy;
                    for (dy = -1; dy <= 1; dy++) for (dx = -1; dx <= 1; dx++) {
                        int qx = px + dx, qy = py + dy;
                        if (qx >= 8 && at(s, qx, qy) && !seen[qy * SPX_W + qx]) { seen[qy * SPX_W + qx] = 1; stack[sp++] = qy * SPX_W + qx; }
                    }
                }
            }
    return n;
}

static int symmetric(int s)
{
    int x, y;
    for (y = 0; y < SPX_H; y++) for (x = 0; x < SPX_W; x++) if (at(s, x, y) != at(s, x, SPX_H - 1 - y)) return 0;
    return 1;
}

int main(void)
{
    spx_opts o;
    int s, x, y, ok;
    /* options */
    spx_defaults(&o);
    CHECK(o.pri == 10 && !o.popup && !o.mute[0], "defaults: priority 10, no popup, no hotkey");
    CHECK(spx_word(&o, "CX_PRIORITY=5") && o.pri == 5, "CX_PRIORITY=5");
    CHECK(spx_word(&o, "cx_priority = -20") && o.pri == -20, "lower case and spaces, a negative priority");
    CHECK(spx_word(&o, "CX_PRIORITY=999") && o.pri == 127, "a priority past 127 is 127");
    spx_word(&o, "CX_PRIORITY=-999");
    CHECK(o.pri == -128, "and below -128 is -128");
    spx_word(&o, "CX_PRIORITY=7");
    CHECK(spx_word(&o, "CX_PRIORITY=x") && o.pri == 7, "a priority that isn't a number leaves it as it was");
    CHECK(spx_word(&o, "CX_POPUP=YES") && o.popup, "CX_POPUP=YES");
    CHECK(spx_word(&o, "CX_POPUP=NO") && !o.popup, "CX_POPUP=NO");
    CHECK(spx_word(&o, "CX_POPUP") && o.popup, "CX_POPUP alone is yes");
    spx_word(&o, "CX_POPUP=off");
    CHECK(!o.popup, "CX_POPUP=off is no");
    CHECK(spx_word(&o, "MUTE=ctrl alt m") && !strcmp(o.mute, "ctrl alt m"), "MUTE takes a commodity hotkey");
    CHECK(spx_word(&o, "MUTE=NONE") && !o.mute[0], "MUTE=NONE takes it away");
    CHECK(!spx_word(&o, "CX_PRIORITYX=3") && !spx_word(&o, "DONOTWAIT") && !spx_word(&o, NULL) && o.pri == 7,
          "other words aren't ours");
    /* the level's picture */
    CHECK(spx_state(1, 80) == SPX_MUTED && spx_state(0, 0) == SPX_OFF, "muted, and off at 0%");
    CHECK(spx_state(0, 1) == SPX_WAVES1 && spx_state(0, 33) == SPX_WAVES1 && spx_state(0, 34) == SPX_WAVES2 &&
          spx_state(0, 66) == SPX_WAVES2 && spx_state(0, 67) == SPX_WAVES3 && spx_state(0, 100) == SPX_WAVES3,
          "one, two and three waves for the volume's thirds");
    /* the pictures */
    for (s = 0; s < SPX_STATES; s++) spx_pic(pic[s], s);
    for (ok = 1, s = 0; s < SPX_STATES; s++) ok &= symmetric(s);
    CHECK(ok, "every picture is the same above and below its middle row");
    for (ok = 1, s = 0; s < SPX_STATES; s++) for (y = 0; y < SPX_H; y++) ok &= at(s, 6, y);
    CHECK(ok, "the cone's mouth is the full 11 rows in every picture (as tall as the network icons)");
    for (ok = 1, s = 0; s < SPX_STATES; s++) for (y = 0; y < SPX_H; y++) for (x = 0; x < SPX_W; x++)
        ok &= pic[s][y * SPX_W + x] <= 1;
    CHECK(ok, "only pixel 1, the bar's text colour, as the other tray icons");
    CHECK(pieces(SPX_OFF) == 0, "off: the speaker alone");
    CHECK(pieces(SPX_WAVES1) == 1 && pieces(SPX_WAVES2) == 2 && pieces(SPX_WAVES3) == 3, "one, two and three waves, each unbroken");
    for (ok = 1, s = SPX_WAVES1; s < SPX_WAVES3; s++) for (y = 0; y < SPX_H; y++) for (x = 0; x < SPX_W; x++)
        ok &= !at(s, x, y) || at(s + 1, x, y);
    CHECK(ok, "each louder picture keeps the waves of the one before");
    CHECK(pieces(SPX_MUTED) == 1 && at(SPX_MUTED, 9, 2) && at(SPX_MUTED, 15, 8) && at(SPX_MUTED, 15, 2) && at(SPX_MUTED, 9, 8),
          "muted: the cross, corner to corner");
    {
        int top = SPX_H, bot = -1;
        for (y = 0; y < SPX_H; y++) for (x = 8; x < SPX_W; x++) if (at(SPX_WAVES3, x, y)) { if (y < top) top = y; if (y > bot) bot = y; }
        CHECK(top >= 0 && bot <= SPX_H - 1 && bot - top >= 8, "the outer wave reaches near the cone's height");
    }
    for (s = 0; s < SPX_STATES; s++) {                 /* the pictures, to look at */
        printf("     state %d\n", s);
        for (y = 0; y < SPX_H; y++) { printf("     "); for (x = 0; x < SPX_W; x++) putchar(at(s, x, y) ? '#' : '.'); putchar('\n'); }
    }
    printf("%d checks, %d failed\n%s\n", checks, fails, fails ? "FAIL" : "PASS");
    return fails != 0;
}
