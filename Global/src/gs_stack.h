/* gs_stack.h: Global Settings' default stack size in ENV:GFOpenPrefs/Stack, the file GFOpenDOS reads for its floor
 * under every stack (amicradle-guest libraries/opendos, stack_prefs_parse). Plain C, so the PC's compiler tests it
 * (Global/tests/stack_test.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef GS_STACK_H
#define GS_STACK_H

#define GS_STACK_DEFAULT 8192UL             /* GFOpenDOS's "minimum" with no file or line */
#define GS_STACK_MAX (1UL << 20)            /* GFOpenDOS takes "minimum" as 1 MB at most */

/* "minimum" and "auto" out of the file's text, as GFOpenDOS reads them (a missing line its default) */
void gs_parse(const char *text, unsigned long *minimum, int *autosize);
/* the file again in out (size bytes): every line of text but "minimum" (and, for 0, "auto") as it was, then
 * "minimum N" and, for 0 (off), "auto off"; its length */
long gs_text(const char *text, char *out, long size, unsigned long minimum);
/* a box's value as it is written: 1 MB at most, a multiple of 4 */
unsigned long gs_round(unsigned long n);

#endif
