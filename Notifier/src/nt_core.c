/* nt_core: GFOpenNotifier's commands, its list of programs and its queue (see nt_core.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "nt_core.h"
#include <string.h>

/* ---- small helpers ---- */

static int nt_upper(int c) { return (c >= 'a' && c <= 'z') ? c - 32 : c; }

static int nt_same(const char *a, const char *b)                  /* equal, letters' case aside */
{
    while (*a && *b) { if (nt_upper((unsigned char)*a) != nt_upper((unsigned char)*b)) return 0; a++; b++; }
    return *a == *b;
}

static void nt_copy(char *to, int size, const char *from)         /* cut to fit, always ended */
{
    int n = 0;
    while (from[n] && n < size - 1) { to[n] = from[n]; n++; }
    to[n] = 0;
}

static int nt_number(const char *s, int *out)                     /* a plain number, an optional sign */
{
    int neg = 0, n = 0, any = 0;
    if (*s == '-') { neg = 1; s++; } else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') { if (n < 100000) n = n * 10 + (*s - '0'); s++; any = 1; }
    if (!any || *s) return 0;
    *out = neg ? -n : n;
    return 1;
}

static int nt_clamp(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* ---- the argument reader: AmigaDOS templates of /K, /S and one /F ---- */

enum { NT_K, NT_S, NT_F };
typedef struct nt_key { const char *name; int kind; } nt_key;
typedef struct nt_args { char val[16][NT_PATH]; unsigned char set[16]; } nt_args;

static const char *nt_skip(const char *p) { while (*p == ' ' || *p == '\t') p++; return p; }

/* One word or one "quoted string" from p into out. Inside quotes *" is a quote, *N a new line, ** a star,
 * as AmigaDOS has them. Stops at '=' outside quotes when stop_eq is set. Returns where it stopped. */
static const char *nt_word(const char *p, char *out, int size, int stop_eq)
{
    int n = 0;
    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            char c = *p++;
            if (c == '*' && *p) {
                c = *p++;
                if (c == 'N' || c == 'n') c = '\n';
                else if (c == 'E' || c == 'e') c = 27;
            }
            if (n < size - 1) out[n++] = c;
        }
        if (*p == '"') p++;
    } else {
        while (*p && *p != ' ' && *p != '\t' && !(stop_eq && *p == '=')) { if (n < size - 1) out[n++] = *p; p++; }
    }
    out[n] = 0;
    return p;
}

static int nt_key_index(const nt_key *keys, int nkeys, const char *word)
{
    int i;
    for (i = 0; i < nkeys; i++) if (nt_same(keys[i].name, word)) return i;
    return -1;
}

/* The rest of the line for a /F: one quoted string loses its quotes, anything else is taken as written. */
static void nt_rest(const char *p, char *out, int size)
{
    int n;
    p = nt_skip(p);
    if (*p == '"') {
        const char *end = nt_word(p, out, size, 0);
        if (!*nt_skip(end)) return;                               /* the quoted string was all of it */
    }
    nt_copy(out, size, p);
    n = (int)strlen(out);
    while (n > 0 && (out[n - 1] == ' ' || out[n - 1] == '\t')) out[--n] = 0;
}

static void nt_read_args(const char *p, const nt_key *keys, int nkeys, nt_args *a)
{
    char word[NT_PATH];
    int rest = -1, i;
    memset(a, 0, sizeof *a);
    for (i = 0; i < nkeys; i++) if (keys[i].kind == NT_F) rest = i;
    for (;;) {
        const char *start, *q;
        int k;
        p = nt_skip(p);
        if (!*p) return;
        start = p;
        if (*p == '"') k = -1;                                    /* a quoted string is never a keyword */
        else { q = nt_word(p, word, sizeof word, 1); k = nt_key_index(keys, nkeys, word); }
        if (k < 0 || keys[k].kind == NT_F) {                      /* the text starts here */
            if (rest < 0) return;
            if (k >= 0) { start = nt_skip(q); if (*start == '=') start++; }
            nt_rest(start, a->val[rest], NT_PATH);
            a->set[rest] = 1;
            return;
        }
        p = q;
        a->set[k] = 1;
        if (keys[k].kind == NT_S) continue;
        if (*p == '=') p++; else p = nt_skip(p);
        p = nt_word(p, a->val[k], NT_PATH, 0);
    }
}

/* ---- programs ---- */

void nt_init(nt_state *st) { memset(st, 0, sizeof *st); st->next_id = 1; }

nt_app *nt_find_app(nt_state *st, const char *name)
{
    int i;
    for (i = 0; i < st->napps; i++) if (nt_same(st->apps[i].name, name)) return &st->apps[i];
    return 0;
}

int nt_block_app(nt_state *st, const char *name, int blocked)
{
    nt_app *a = nt_find_app(st, name);
    if (!a) return 0;
    a->blocked = blocked ? 1 : 0;
    return 1;
}

static void nt_drop(nt_state *st, int i)                          /* queue[i] out, the order kept */
{
    memmove(&st->queue[i], &st->queue[i + 1], (size_t)(st->nqueue - i - 1) * sizeof st->queue[0]);
    st->nqueue--;
}

static void nt_drop_app(nt_state *st, const char *name)
{
    int i = 0;
    while (i < st->nqueue) if (nt_same(st->queue[i].app, name)) nt_drop(st, i); else i++;
}

int nt_unregister_app(nt_state *st, const char *name)
{
    nt_app *a = nt_find_app(st, name);
    if (!a) return 0;
    nt_drop_app(st, name);
    memmove(a, a + 1, (size_t)(&st->apps[st->napps] - (a + 1)) * sizeof *a);
    st->napps--;
    return 1;
}

/* REGISTERAPP APP/K,AREXXPORT/K,ICON/K,TEXT/F */
static const nt_key nt_reg_keys[] = { { "APP", NT_K }, { "AREXXPORT", NT_K }, { "ICON", NT_K }, { "TEXT", NT_F } };

static const char *nt_register(nt_state *st, const char *p)
{
    static nt_args a;                                             /* too big for a 4 KB stack */
    nt_app *app;
    nt_read_args(p, nt_reg_keys, 4, &a);
    if (!a.val[0][0]) return "ERROR: NO APP NAME";
    if (nt_find_app(st, a.val[0])) return "ERROR: APP ALREADY REGISTERED";
    if (st->napps >= NT_APPS) return "ERROR: OUT OF MEMORY";
    app = &st->apps[st->napps++];
    memset(app, 0, sizeof *app);
    nt_copy(app->name, sizeof app->name, a.val[0]);
    nt_copy(app->port, sizeof app->port, a.val[1]);
    nt_copy(app->icon, sizeof app->icon, a.val[2]);
    nt_copy(app->about, sizeof app->about, a.val[3]);
    return "OK, APP REGISTERED";
}

/* ---- notices ---- */

/* RINGHIO APP/K,UPDATE/S,CLOSEONDC/S,PRI/K,SCREEN/K,IMG/K,IMGVALIGN/K/N,LOGONLY/S,NOLOG/S,TITLE/K,
 *         BACKRXMSG/K,PAUSEBACKRXMSG/K,STOPBACKRXMSG/K,PERCENTAGE/K,TEXT/F */
enum { RA_APP, RA_UPDATE, RA_CLOSEONDC, RA_PRI, RA_SCREEN, RA_IMG, RA_IMGVALIGN, RA_LOGONLY, RA_NOLOG, RA_TITLE,
       RA_BACK, RA_PAUSE, RA_STOP, RA_PERCENT, RA_TEXT, RA_COUNT };
static const nt_key nt_ring_keys[RA_COUNT] = {
    { "APP", NT_K }, { "UPDATE", NT_S }, { "CLOSEONDC", NT_S }, { "PRI", NT_K }, { "SCREEN", NT_K },
    { "IMG", NT_K }, { "IMGVALIGN", NT_K }, { "LOGONLY", NT_S }, { "NOLOG", NT_S }, { "TITLE", NT_K },
    { "BACKRXMSG", NT_K }, { "PAUSEBACKRXMSG", NT_K }, { "STOPBACKRXMSG", NT_K }, { "PERCENTAGE", NT_K },
    { "TEXT", NT_F }
};

static void nt_log_add(nt_state *st, const nt_notice *n)
{
    if (st->nlog == NT_LOG) { memmove(&st->log[0], &st->log[1], (NT_LOG - 1) * sizeof st->log[0]); st->nlog--; }
    st->log[st->nlog++] = *n;
}

static const char *nt_ring(nt_state *st, const char *p)
{
    static nt_args a;
    static nt_notice n;
    nt_app *app;
    int v;
    nt_read_args(p, nt_ring_keys, RA_COUNT, &a);
    if (!a.val[RA_APP][0]) return "ERROR: NO APP NAME";
    app = nt_find_app(st, a.val[RA_APP]);
    if (!app) return "ERROR: APP NOT REGISTERED";
    if (app->blocked) return "ERROR: APP NOT ALLOWED";

    memset(&n, 0, sizeof n);
    nt_copy(n.app, sizeof n.app, app->name);
    nt_copy(n.title, sizeof n.title, a.val[RA_TITLE]);
    nt_copy(n.text, sizeof n.text, a.val[RA_TEXT]);
    nt_copy(n.screen, sizeof n.screen, a.val[RA_SCREEN]);
    nt_copy(n.img, sizeof n.img, a.val[RA_IMG]);
    nt_copy(n.back, sizeof n.back, a.val[RA_BACK]);
    nt_copy(n.pause, sizeof n.pause, a.val[RA_PAUSE]);
    nt_copy(n.stop, sizeof n.stop, a.val[RA_STOP]);
    n.pri = NT_PRI_DEFAULT;
    if (a.set[RA_PRI] && nt_number(a.val[RA_PRI], &v)) n.pri = nt_clamp(v, 0, NT_PRI_STAY);
    if (a.set[RA_IMGVALIGN] && nt_number(a.val[RA_IMGVALIGN], &v)) n.valign = nt_clamp(v, 0, 2);
    if (a.set[RA_PERCENT] && nt_number(a.val[RA_PERCENT], &v)) { n.percent = nt_clamp(v, 0, 100); n.flags |= NT_F_PERCENT; }
    if (a.set[RA_UPDATE]) n.flags |= NT_F_UPDATE;
    if (a.set[RA_CLOSEONDC]) n.flags |= NT_F_CLOSEONDC;
    if (a.set[RA_LOGONLY]) n.flags |= NT_F_LOGONLY;
    if (a.set[RA_NOLOG]) n.flags |= NT_F_NOLOG;

    if (n.flags & NT_F_UPDATE) nt_drop_app(st, app->name);        /* the commodity closes the one on screen */
    if (!(n.flags & NT_F_LOGONLY) && st->nqueue >= NT_QUEUE) return "ERROR: TOO MANY MESSAGES QUEUED";
    n.id = st->next_id++;
    if (!(n.flags & NT_F_NOLOG)) nt_log_add(st, &n);
    if (!(n.flags & NT_F_LOGONLY)) st->queue[st->nqueue++] = n;
    return "OK, MSG QUEUED";
}

int nt_next(nt_state *st, nt_notice *out)
{
    int i, best = 0;
    if (!st->nqueue) return 0;
    for (i = 1; i < st->nqueue; i++) if (st->queue[i].pri > st->queue[best].pri) best = i;
    *out = st->queue[best];
    nt_drop(st, best);
    return 1;
}

int nt_show_seconds(const nt_notice *n, int base)
{
    int s;
    if (n->pri >= NT_PRI_STAY) return 0;
    if (base < 1) base = 1;
    s = base * (n->pri + 5) / 10;                                 /* 0 half the time, 5 the time, 9 a little under double */
    return s < 1 ? 1 : s;
}

/* ---- the port ---- */

int nt_command(nt_state *st, const char *line, char *reply)
{
    static char buf[NT_LINE];
    char cmd[32];
    const char *p, *answer;
    nt_copy(buf, sizeof buf, line ? line : "");
    p = nt_word(nt_skip(buf), cmd, sizeof cmd, 0);
    if (nt_same(cmd, "REGISTERAPP")) answer = nt_register(st, p);
    else if (nt_same(cmd, "RINGHIO")) answer = nt_ring(st, p);
    else { nt_copy(reply, NT_REPLY, "ERROR: UNDEFINED ERROR"); return NT_RC_UNKNOWN; }
    nt_copy(reply, NT_REPLY, answer);
    return NT_RC_OK;
}
