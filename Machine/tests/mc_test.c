/* mc_test: the Machine page's words. GET's answers as gfosmotherboards' ms_report writes them, read back into the
 * page's choices, and the choices written as SET's words.
 *   cc -std=c99 -Wall -Werror -IMachine/src -o mc_test Machine/tests/mc_test.c Machine/src/mc_words.c
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "mc_words.h"

static int checks, fails;
#define CHECK(what, c) do { checks++; if (!(c)) { fails++; printf("FAIL %s\n", what); } } while (0)

int main(void)
{
    mc_set now, g, s;
    char out[128], tiny[12];
    int pushes;

    /* a machine with one program's push, and SET's FPU waiting for the reset */
    static const char get1[] = "FPU=EXTENDED PACE=HYPER CHIPPLANE=OFF\nglobal FPU=DOUBLE PACE=HYPER REFRESH=50 FPU=EXTENDED at the next reset\n"
                               "push 3 0007a3c0 FPU=EXTENDED CHIPPLANE=OFF\n";
    mc_clear(&now); mc_clear(&g);
    pushes = mc_read_get(get1, &now, &g);
    CHECK("one program has its own", pushes == 1);
    CHECK("in force: the push's words", now.fpu == MC_FPU_EXTENDED && now.pace == MC_PACE_HYPER && now.chipplane == 0);
    CHECK("global: double, hyper, 50 Hz, the plane not given", g.fpu == MC_FPU_DOUBLE && g.pace == MC_PACE_HYPER
          && g.refresh == 50 && g.chipplane == MC_NONE);
    CHECK("SET's FPU at the next reset", g.fpu_next == MC_FPU_EXTENDED);

    /* an older board's GET: FPU and PACE only, no pushes */
    mc_clear(&now); mc_clear(&g);
    CHECK("no pushes", mc_read_get("FPU=DOUBLE PACE=A1200\nglobal FPU=DOUBLE PACE=A1200\n", &now, &g) == 0);
    CHECK("A1200 pace", g.pace == MC_PACE_A1200 && g.refresh == MC_NONE);

    /* the words, case aside, a pace in MHz, and words not known passed over */
    mc_clear(&s);
    CHECK("four taken", mc_parse("fpu=extended Pace=50 chipplane=yes refresh=auto CACHES=OFF PACE=500", &s) == 4);
    CHECK("50 MHz, ON, AUTO", s.pace == MC_PACE_MHZ && s.mhz == 50 && s.chipplane == 1 && s.refresh == MC_REFRESH_AUTO);

    /* SET's words: only those given */
    mc_format(&s, out, sizeof out);
    CHECK("SET's words", !strcmp(out, "FPU=EXTENDED PACE=50 CHIPPLANE=ON REFRESH=AUTO"));
    mc_clear(&s);
    s.refresh = 60;
    mc_format(&s, out, sizeof out);
    CHECK("one word", !strcmp(out, "REFRESH=60"));
    mc_clear(&s);
    CHECK("none: an empty line", mc_format(&s, out, sizeof out) == 0 && !out[0]);
    s.fpu = MC_FPU_DOUBLE; s.pace = MC_PACE_HYPER;
    mc_format(&s, tiny, sizeof tiny);
    CHECK("cut to its buffer, NUL-ended", strlen(tiny) == sizeof tiny - 1);

    printf("%d/%d Machine page checks pass\n", checks - fails, checks);
    return fails != 0;
}
