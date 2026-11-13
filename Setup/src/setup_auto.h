/* GFOpenUp Setup's unattended mode (Dale, 13 Nov 2026: amicradle-teams
 * teams/DECISION-UNATTENDED-SETUP-20261113-1959.md): the parts with no Amiga
 * calls in them, so the host tests (Setup/tests) build them with the host's cc.
 *
 * Unattended means no window and no wait: the Simple path, nothing the user
 * has removed or overwritten, Setup.done written, one line in the log, and a
 * return code. It is on with any one of:
 *   - the AUTO switch (GFOpenUp-Setup AUTO), at any start;
 *   - at the first start (SYS:WBStartup or FIRSTSTART): an answers file,
 *     ENVARC:GFOpenUp/Answers;
 *   - at the first start: the host flag, ENV:AmiCradle/Unattended (or
 *     ENVARC:AmiCradle/Unattended), that the Cradle or the card writes for
 *     labs, test runs and freshly written cards.
 * Started by hand from SYS:Prefs (no FIRSTSTART, not from WBStartup) Setup
 * opens its window as before, answers file and host flag or not.
 *
 * The answers file has the wizard's own choices, one KEY=value a line; ";" or
 * "#" starts a comment; keys are any case; a key left out takes the wizard's
 * default:
 *   VIEW=simple|advanced     the view every GFOpenPrefs editor opens in
 *   PROFILE=<name>           a desktop profile in SYS:Prefs/Presets/GFOpenPrefs
 *   BYMACHINE=yes|no         "Choose by machine at every start"
 *   OFF=<words>              a line of GFOpenUp's startup block to switch off
 *                            (every line with these words in it; up to 16 OFF lines)
 * With no PROFILE and no BYMACHINE the Look settings are left as they are;
 * with no VIEW an existing ENVARC:GFOpenAmiga/PrefsView is left as it is; with
 * no OFF S:User-Startup isn't touched.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SETUP_AUTO_H
#define SETUP_AUTO_H

#include <string.h>

#define SA_ANSWERS "ENVARC:GFOpenUp/Answers"
#define SA_FLAG "ENV:AmiCradle/Unattended"
#define SA_FLAG_ARC "ENVARC:AmiCradle/Unattended"
#define SA_LOG "ENVARC:GFOpenUp/Setup.log"

#define SA_MAX_OFF 16
#define SA_WORDS 64

typedef struct sa_answers {
    int view;                         /* -1 not given, 0 simple, 1 advanced */
    int by_machine;                   /* -1 not given, 0 no, 1 yes */
    char profile[48];                 /* "" not given */
    int noff;
    char off[SA_MAX_OFF][SA_WORDS];
    int bad;                          /* lines not understood (unknown key, bad value, too long, too many OFF) */
} sa_answers;

/* How a start goes. */
enum { SA_SKIP, SA_WINDOW, SA_AUTO };
/* What turned unattended on (for the log line). */
enum { SA_BY_NONE, SA_BY_SWITCH, SA_BY_ANSWERS, SA_BY_FLAG };

static inline void sa_init(sa_answers *a)
{
    memset(a, 0, sizeof *a);
    a->view = -1;
    a->by_machine = -1;
}

static inline int sa_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

/* n characters of a and the whole of b, any case */
static inline int sa_is(const char *a, int n, const char *b)
{
    int i;
    for (i = 0; i < n; i++) if (!b[i] || sa_lower((unsigned char)a[i]) != sa_lower((unsigned char)b[i])) return 0;
    return b[n] == 0;
}

/* 1 yes, 0 no, -1 neither */
static inline int sa_yesno(const char *v, int n)
{
    if (sa_is(v, n, "yes") || sa_is(v, n, "on") || sa_is(v, n, "true") || sa_is(v, n, "1")) return 1;
    if (sa_is(v, n, "no") || sa_is(v, n, "off") || sa_is(v, n, "false") || sa_is(v, n, "0")) return 0;
    return -1;
}

/* The answers file's text into a (sa_init first). Returns the lines not understood (a->bad);
 * the rest still count. */
static inline int sa_parse(const char *text, sa_answers *a)
{
    const char *p = text;
    while (p && *p) {
        const char *e = strchr(p, '\n'), *eq, *k, *v;
        int len = e ? (int)(e - p) : (int)strlen(p), kn, vn;
        k = p;
        while (len && (*k == ' ' || *k == '\t')) { k++; len--; }
        while (len && (k[len - 1] == '\r' || k[len - 1] == ' ' || k[len - 1] == '\t')) len--;
        p = e ? e + 1 : NULL;
        if (!len || *k == ';' || *k == '#') continue;
        eq = memchr(k, '=', len);
        if (!eq) { a->bad++; continue; }
        kn = (int)(eq - k);
        while (kn && (k[kn - 1] == ' ' || k[kn - 1] == '\t')) kn--;
        v = eq + 1;
        vn = len - (int)(v - k);
        while (vn && (*v == ' ' || *v == '\t')) { v++; vn--; }
        if (vn >= 2 && v[0] == '"' && v[vn - 1] == '"') { v++; vn -= 2; }
        if (sa_is(k, kn, "view")) {
            if (sa_is(v, vn, "simple")) a->view = 0;
            else if (sa_is(v, vn, "advanced")) a->view = 1;
            else a->bad++;
        } else if (sa_is(k, kn, "bymachine")) {
            int y = sa_yesno(v, vn);
            if (y < 0) a->bad++; else a->by_machine = y;
        } else if (sa_is(k, kn, "profile")) {
            if (vn <= 0 || vn >= (int)sizeof a->profile) a->bad++;
            else { memcpy(a->profile, v, vn); a->profile[vn] = 0; }
        } else if (sa_is(k, kn, "off")) {
            if (vn <= 0 || vn >= SA_WORDS || a->noff >= SA_MAX_OFF) a->bad++;
            else { memcpy(a->off[a->noff], v, vn); a->off[a->noff][vn] = 0; a->noff++; }
        } else a->bad++;
    }
    return a->bad;
}

/* Which way a start goes.
 *   auto_switch  AUTO was given
 *   first        the first start: from SYS:WBStartup, or FIRSTSTART
 *   done         ENVARC:GFOpenAmiga/Setup.done is there
 *   answers      ENVARC:GFOpenUp/Answers is there
 *   flag         the host flag is there
 * *by says what turned unattended on. AUTO always runs (a script asked for it); a
 * first start after Setup.done does nothing, as before; a first start with an
 * answers file or the host flag runs unattended; anything else is the window. */
static inline int sa_mode(int auto_switch, int first, int done, int answers, int flag, int *by)
{
    *by = SA_BY_NONE;
    if (auto_switch) { *by = SA_BY_SWITCH; return SA_AUTO; }
    if (!first) return SA_WINDOW;
    if (done) return SA_SKIP;
    if (answers) { *by = SA_BY_ANSWERS; return SA_AUTO; }
    if (flag) { *by = SA_BY_FLAG; return SA_AUTO; }
    return SA_WINDOW;
}

/* 1 when the startup line has the words in it, any case */
static inline int sa_line_has(const char *line, const char *words)
{
    int n = (int)strlen(words), i;
    if (!n) return 0;
    for (; *line; line++) {
        for (i = 0; i < n && line[i] && sa_lower((unsigned char)line[i]) == sa_lower((unsigned char)words[i]); i++) ;
        if (i == n) return 1;
    }
    return 0;
}

/* The log's one line, without its newline:
 *   GFOpenUp Setup: unattended (answers) view=simple profile=Modern bymachine=yes off=1 ignored=0: ok
 * "kept" for what was left as it was. rc is the run's return code (0 ok). */
/* s onto buf (size bytes in all), cut to fit; no snprintf, so GCC 16's -Wformat-truncation has nothing to say */
static inline void sa_cat(char *buf, int size, const char *s)
{
    int n = (int)strlen(buf);
    while (*s && n < size - 1) buf[n++] = *s++;
    buf[n] = 0;
}

static inline void sa_catn(char *buf, int size, int v)
{
    char d[12];
    int i = sizeof d - 1;
    unsigned u = v < 0 ? 0u - (unsigned)v : (unsigned)v;
    d[i] = 0;
    do d[--i] = (char)('0' + u % 10); while ((u /= 10) && i > 1);
    if (v < 0) d[--i] = '-';
    sa_cat(buf, size, d + i);
}

static inline void sa_log_line(char *buf, int size, const sa_answers *a, int by, const char *profile_used, int rc)
{
    static const char *const why[] = { "?", "AUTO", "answers", "host flag" };
    int named = profile_used && *profile_used;
    if (size <= 0) return;
    buf[0] = 0;
    sa_cat(buf, size, "GFOpenUp Setup: unattended (");
    sa_cat(buf, size, why[by >= 0 && by <= SA_BY_FLAG ? by : 0]);
    sa_cat(buf, size, ") view=");
    sa_cat(buf, size, a->view < 0 ? "kept" : a->view ? "advanced" : "simple");
    sa_cat(buf, size, " profile=");
    sa_cat(buf, size, named ? profile_used : "kept");
    sa_cat(buf, size, " bymachine=");
    sa_cat(buf, size, a->by_machine < 0 ? (named ? "yes" : "kept") : a->by_machine ? "yes" : "no");
    sa_cat(buf, size, " off=");
    sa_catn(buf, size, a->noff);
    sa_cat(buf, size, " ignored=");
    sa_catn(buf, size, a->bad);
    sa_cat(buf, size, ": ");
    sa_cat(buf, size, rc == 0 ? "ok" : rc < 10 ? "done, some answers ignored" : "failed, Setup not marked done");
}

#endif
