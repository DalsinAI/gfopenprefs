/* gs_stack.c: Global Settings' default stack size in ENV:GFOpenPrefs/Stack (gs_stack.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "gs_stack.h"

/* the line's first word is w, any case, then a space or its end; *v is the rest's first word */
static int line_is(const char *s, const char *w, const char **v)
{
    while (*s == ' ' || *s == '\t') s++;
    while (*w && (*s | 0x20) == *w) s++, w++;
    if (*w || (*s != ' ' && *s != '\t' && *s != '\n' && *s != '\r' && *s)) return 0;
    while (*s == ' ' || *s == '\t') s++;
    *v = s;
    return 1;
}

/* the word at v is w, any case */
static int word_is(const char *v, const char *w)
{
    while (*w && (*v | 0x20) == *w) v++, w++;
    return !*w && (*v == ' ' || *v == '\t' || *v == '\n' || *v == '\r' || !*v);
}

void gs_parse(const char *s, unsigned long *minimum, int *autosize)
{
    *minimum = GS_STACK_DEFAULT;
    *autosize = 1;
    while (*s) {
        const char *v;
        if (line_is(s, "minimum", &v) && *v >= '0' && *v <= '9') {
            unsigned long n = 0;
            const char *p = v;
            while (*p >= '0' && *p <= '9') { if (n < GS_STACK_MAX) n = n * 10 + (unsigned long)(*p - '0'); p++; }
            if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || !*p)    /* as GFOpenDOS: digits only */
                *minimum = n > GS_STACK_MAX ? GS_STACK_MAX : n;
        } else if (line_is(s, "auto", &v)) {
            if (word_is(v, "on") || word_is(v, "yes") || word_is(v, "1")) *autosize = 1;
            else if (word_is(v, "off") || word_is(v, "no") || word_is(v, "0")) *autosize = 0;
        }
        while (*s && *s != '\n') s++;
        if (*s) s++;
    }
}

long gs_text(const char *s, char *out, long size, unsigned long minimum)
{
    long o = 0;
    if (size < 1) return 0;
    out[0] = 0;
    while (*s) {
        const char *e = s, *v;
        while (*e && *e != '\n') e++;
        if (!line_is(s, "minimum", &v) && !(minimum == 0 && line_is(s, "auto", &v)) && e > s && o + (e - s) + 1 < size) {
            memcpy(out + o, s, e - s);
            o += e - s;
            out[o++] = '\n';
            out[o] = 0;
        }
        s = *e ? e + 1 : e;
    }
    if (o < size) o += snprintf(out + o, size - o, "minimum %lu\n", minimum);
    if (minimum == 0 && o < size) o += snprintf(out + o, size - o, "auto off\n");
    return o < size ? o : size - 1;
}

unsigned long gs_round(unsigned long n)
{
    if (n > GS_STACK_MAX) n = GS_STACK_MAX;
    return (n + 3) & ~3UL;
}
