/* mc_words.c: the Machine page's words (mc_words.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "mc_words.h"

void mc_clear(mc_set *s)
{
    s->fpu = s->pace = s->chipplane = s->refresh = s->fpu_next = MC_NONE;
    s->mhz = 0;
}

static int up(int c) { return c >= 'a' && c <= 'z' ? c - 32 : c; }

/* the n characters at a are b, case aside */
static int is(const char *a, int n, const char *b)
{
    int i;
    for (i = 0; i < n; i++) if (!b[i] || up((unsigned char)a[i]) != up((unsigned char)b[i])) return 0;
    return !b[i];
}

static int onoff(const char *v, int n)
{
    if (is(v, n, "ON") || is(v, n, "YES") || is(v, n, "TRUE") || is(v, n, "1")) return 1;
    if (is(v, n, "OFF") || is(v, n, "NO") || is(v, n, "FALSE") || is(v, n, "0")) return 0;
    return MC_NONE;
}

int mc_parse(const char *t, mc_set *s)
{
    int took = 0;
    while (t && *t && *t != '\n') {
        const char *a, *eq, *v;
        int kn, vn;
        while (*t == ' ' || *t == '\t') t++;
        a = t;
        while (*t && *t != ' ' && *t != '\t' && *t != '\n') t++;
        if (t == a) break;
        for (eq = a; eq < t && *eq != '='; eq++) {}
        if (eq == t) continue;
        kn = (int)(eq - a); v = eq + 1; vn = (int)(t - v);
        if (is(a, kn, "FPU")) {
            int f = is(v, vn, "DOUBLE") ? MC_FPU_DOUBLE : is(v, vn, "EXTENDED") ? MC_FPU_EXTENDED : MC_NONE;
            if (f == MC_NONE) continue;
            if (!strncmp(t, " at the next reset", 18)) { s->fpu_next = f; t += 18; }
            else s->fpu = f;
            took++;
        } else if (is(a, kn, "PACE")) {
            if (is(v, vn, "HYPER")) { s->pace = MC_PACE_HYPER; took++; }
            else if (is(v, vn, "A1200")) { s->pace = MC_PACE_A1200; took++; }
            else {
                unsigned m = 0;
                int i;
                for (i = 0; i < vn && v[i] >= '0' && v[i] <= '9' && m < 1000; i++) m = m * 10 + (unsigned)(v[i] - '0');
                if (i == vn && m >= 1 && m <= 200) { s->pace = MC_PACE_MHZ; s->mhz = m; took++; }
            }
        } else if (is(a, kn, "CHIPPLANE")) {
            int o = onoff(v, vn);
            if (o != MC_NONE) { s->chipplane = o; took++; }
        } else if (is(a, kn, "REFRESH")) {
            if (is(v, vn, "AUTO")) { s->refresh = MC_REFRESH_AUTO; took++; }
            else if (is(v, vn, "50")) { s->refresh = 50; took++; }
            else if (is(v, vn, "60")) { s->refresh = 60; took++; }
        }
    }
    return took;
}

int mc_read_get(const char *answer, mc_set *now, mc_set *global)
{
    const char *p = answer;
    int line = 0, pushes = 0;
    while (p && *p) {
        const char *e = strchr(p, '\n');
        if (line == 0 && now) mc_parse(p, now);
        else if (!strncmp(p, "global ", 7) && global) mc_parse(p + 7, global);
        else if (!strncmp(p, "push ", 5)) pushes++;
        line++;
        if (!e) break;
        p = e + 1;
    }
    return pushes;
}

/* word onto out (len so far, cap with the NUL), a space before it when out has one already */
static int put(char *out, int cap, int len, const char *word)
{
    if (len && len < cap - 1) out[len++] = ' ';
    while (*word && len < cap - 1) out[len++] = *word++;
    out[len] = 0;
    return len;
}

int mc_format(const mc_set *s, char *out, int cap)
{
    char mhz[16];
    int len = 0;
    if (cap < 1) return 0;
    out[0] = 0;
    if (s->fpu != MC_NONE) len = put(out, cap, len, s->fpu == MC_FPU_EXTENDED ? "FPU=EXTENDED" : "FPU=DOUBLE");
    if (s->pace == MC_PACE_HYPER) len = put(out, cap, len, "PACE=HYPER");
    else if (s->pace == MC_PACE_A1200) len = put(out, cap, len, "PACE=A1200");
    else if (s->pace == MC_PACE_MHZ) { sprintf(mhz, "PACE=%u", s->mhz % 1000); len = put(out, cap, len, mhz); }
    if (s->chipplane != MC_NONE) len = put(out, cap, len, s->chipplane ? "CHIPPLANE=ON" : "CHIPPLANE=OFF");
    if (s->refresh == MC_REFRESH_AUTO) len = put(out, cap, len, "REFRESH=AUTO");
    else if (s->refresh == 50) len = put(out, cap, len, "REFRESH=50");
    else if (s->refresh == 60) len = put(out, cap, len, "REFRESH=60");
    return len;
}
