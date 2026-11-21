/* mc_words.h: the Machine page's words (gfosdesign gfos/machines/machine-settings, section 1): the global settings as
 * the board keeps them, read from ACCMD_MACHINE's GET and written for its SET. Plain C, tested on the host
 * (Machine/tests/mc_test.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef MC_WORDS_H
#define MC_WORDS_H

#define MC_NONE (-1)            /* not given: the machine's own */

enum { MC_FPU_DOUBLE, MC_FPU_EXTENDED };
enum { MC_PACE_HYPER, MC_PACE_A1200, MC_PACE_MHZ };
enum { MC_REFRESH_AUTO = 0 };   /* else 50 or 60 */

typedef struct {
    int fpu;                    /* MC_FPU_*, MC_NONE */
    int pace;                   /* MC_PACE_*, MC_NONE */
    unsigned mhz;               /* with MC_PACE_MHZ, 1 to 200 */
    int chipplane;              /* 1 ON, 0 OFF, MC_NONE */
    int refresh;                /* MC_REFRESH_AUTO, 50, 60, MC_NONE */
    int fpu_next;               /* GET's "FPU=... at the next reset": MC_FPU_*, MC_NONE */
} mc_set;

void mc_clear(mc_set *s);

/* The words on one line ("FPU=DOUBLE PACE=HYPER CHIPPLANE=ON"), case-blind, into s over what it holds; a word it
 * doesn't know is passed over. "FPU=X at the next reset" sets fpu_next. How many were taken. */
int mc_parse(const char *line, mc_set *s);

/* GET's answer: the line in force (its first), the global words (the line "global ..."), and how many programs
 * have pushed their own (the "push ..." lines). Each of now and global may be NULL. */
int mc_read_get(const char *answer, mc_set *now, mc_set *global);

/* s as words for SET, only those given, separated by spaces; the length */
int mc_format(const mc_set *s, char *out, int cap);

#endif
