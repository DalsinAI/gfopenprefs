/* stack_test.c: Global Settings' default stack size in ENV:GFOpenPrefs/Stack, with the PC's compiler, and the
 * floor GFOpenDOS makes of it (its stack_size, copied here): a smaller stack is raised, a bigger one kept.
 *   cc -I Global/src -o stack_test Global/tests/stack_test.c Global/src/gs_stack.c && ./stack_test
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "gs_stack.h"

static int fails;
static void want(int ok, const char *what)
{
    if (!ok) { printf("FAIL %s\n", what); fails++; }
}

/* GFOpenDOS's stack_size (libraries/opendos/opendos_lib.c), without the runtime's scale: the floor, then the stack */
static unsigned long dos_stack(const char *file, unsigned long ram, unsigned long stack)
{
    unsigned long minimum, floor, r;
    int autosize;
    gs_parse(file, &minimum, &autosize);
    r = autosize ? ram : 0;
    floor = r > (128UL << 20) ? 131072 : r > (64UL << 20) ? 65536 : r > (32UL << 20) ? 32768 : r > (16UL << 20) ? 16384 : 0;
    if (floor < minimum) floor = minimum;
    return stack < floor ? floor : stack;
}

int main(void)
{
    char out[256];
    unsigned long m;
    int a;
    long n;

    gs_parse("", &m, &a);
    want(m == 8192 && a == 1, "no file: minimum 8192, auto on");
    gs_parse("minimum 65536\nauto off\n", &m, &a);
    want(m == 65536 && a == 0, "minimum and auto read");
    gs_parse("  MINIMUM\t32768\r\nAuto No\n", &m, &a);
    want(m == 32768 && a == 0, "any case, tabs, CR");
    gs_parse("minimum 99999999\n", &m, &a);
    want(m == GS_STACK_MAX, "1 MB at most");
    gs_parse("minimum 12k\n", &m, &a);
    want(m == 8192, "not a number: the default, as GFOpenDOS");
    gs_parse("minimumx 4\nauto maybe\n", &m, &a);
    want(m == 8192 && a == 1, "other words left alone");

    n = gs_text("auto off\nadd 1024\nminimum 4096\n# note\n", out, sizeof out, 65536);
    want(!strcmp(out, "auto off\nadd 1024\n# note\nminimum 65536\n") && n == (long)strlen(out), "other lines kept, minimum replaced");
    gs_text("", out, sizeof out, 16384);
    want(!strcmp(out, "minimum 16384\n"), "a new file");
    gs_text("auto on\nadd 8\nminimum 8192\n", out, sizeof out, 0);
    want(!strcmp(out, "add 8\nminimum 0\nauto off\n"), "0: off, auto off too");
    gs_text("auto on\nminimum 8192\n", out, 12, 16384);
    want(strlen(out) < 12, "a small buffer is not overrun");
    want(gs_round(4097) == 4100 && gs_round(5UL << 20) == GS_STACK_MAX, "rounded to 4, 1 MB at most");

    /* the rule: below the box raised to it, above it kept; auto raises it with the memory; 0 is off */
    want(dos_stack("minimum 65536\nauto off\n", 0, 4096) == 65536, "Stack 4096 under a 65536 box: 65536");
    want(dos_stack("minimum 65536\nauto off\n", 0, 200000) == 200000, "a bigger stack kept");
    want(dos_stack("minimum 16384\n", 256UL << 20, 4096) == 131072, "auto on, 256 MB: 128 KB");
    want(dos_stack("minimum 0\nauto off\n", 256UL << 20, 4096) == 4096, "0: off, the stack asked for");
    gs_text("auto on\n", out, sizeof out, 0);
    want(dos_stack(out, 256UL << 20, 4096) == 4096, "the page's 0 turns GFOpenDOS's floor off");
    gs_text("", out, sizeof out, 32768);
    want(dos_stack(out, 8UL << 20, 4096) == 32768, "the page's box is GFOpenDOS's floor");

    if (!fails) printf("ok   Global Settings' stack size: the file, and below raised, above kept\n");
    return fails != 0;
}
