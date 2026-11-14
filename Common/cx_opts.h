/* The tool types every GFOpenPrefs commodity reads (Exchange's own words): CX_PRIORITY (-128 to 127) and CX_POPUP
 * (YES opens its interface at the start), from the icon's tool types or the Shell's KEY=VALUE words. Plain C, so
 * the host tests (Common/tests/cx_opts_test.c) run it. GFOpenNetwork 0.2 first; GFOpenSpeaker's sp_cx.h has the same
 * rules and moves onto this once gfopenprefs #202 is in.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef CX_OPTS_H
#define CX_OPTS_H

typedef struct cxo_opts {
    int pri;                                   /* CX_PRIORITY */
    int popup;                                 /* CX_POPUP */
} cxo_opts;

static inline int cxo_upper(int c) { return c >= 'a' && c <= 'z' ? c - 32 : c; }

/* the word is name (letter case aside), then '=', spaces or its end; *value: what follows */
static inline int cxo_key(const char *s, const char *name, const char **value)
{
    while (*name && cxo_upper((unsigned char)*s) == *name) s++, name++;
    if (*name) return 0;
    while (*s == ' ') s++;
    if (*s == '=') { s++; while (*s == ' ') s++; *value = s; return 1; }
    if (!*s) { *value = s; return 1; }
    return 0;
}

static inline void cxo_defaults(cxo_opts *o, int pri) { o->pri = pri; o->popup = 0; }

/* yes: YES, Y..., ON, or nothing after the name */
static inline int cxo_yes(const char *v)
{
    return !*v || cxo_upper((unsigned char)v[0]) == 'Y' || (cxo_upper((unsigned char)v[0]) == 'O' && cxo_upper((unsigned char)v[1]) == 'N');
}

/* one tool type or KEY=VALUE word; 1 when it was CX_PRIORITY or CX_POPUP. A priority that isn't a number leaves it
 * as it was; one past -128..127 is held to it. */
static inline int cxo_word(cxo_opts *o, const char *w)
{
    const char *v;
    if (!w) return 0;
    if (cxo_key(w, "CX_PRIORITY", &v)) {
        long n = 0;
        int neg = *v == '-', any = 0;
        if (*v == '-' || *v == '+') v++;
        for (; *v >= '0' && *v <= '9' && n < 1000; v++) n = n * 10 + (*v - '0'), any = 1;
        if (any) { if (neg) n = -n; o->pri = n < -128 ? -128 : n > 127 ? 127 : (int)n; }
        return 1;
    }
    if (cxo_key(w, "CX_POPUP", &v)) { o->popup = cxo_yes(v); return 1; }
    return 0;
}

#endif
