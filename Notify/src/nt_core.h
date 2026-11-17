/* nt_core: GFOpenNotify's commands, its list of programs and its queue of notices.
 *
 * GFOpenNotify is our own pop-up notice server. It answers the two ARexx commands of OS 4's Ringhio,
 * REGISTERAPP and RINGHIO, which are also what Ranchero (Rob Cranley, Aminet util/cdity/Ranchero) answers on
 * OS 3, so a script written for either works with ours unchanged. Written from the published description of
 * the commands only; none of their code or files.
 *
 * Plain C with no Amiga calls: the same code is tested on the host (tests/test_nt_core.c) and runs in the
 * commodity. The commodity owns the port, the pop-up and the clock; this file only reads a command line,
 * keeps the state and gives the answer.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef NT_CORE_H
#define NT_CORE_H

#define NT_PORT "RINGHIO"          /* the ARexx port scripts address */

#define NT_APPS      32            /* programs registered at once */
#define NT_QUEUE     16            /* notices waiting to be shown */
#define NT_LOG       32            /* notices kept for the list (oldest goes first) */
#define NT_NAME      64            /* with the ending 0 */
#define NT_PATH     256
#define NT_TITLE     65            /* "only the first 64 characters are displayed" */
#define NT_TEXT     161            /* "only the first 160 characters are displayed" */
#define NT_RX       128            /* a message sent back to the program */
#define NT_LINE    1024            /* the longest command line read */
#define NT_REPLY     48

#define NT_PRI_DEFAULT 5
#define NT_PRI_STAY   10           /* stays until the user closes it */

/* rc of nt_command(): what the ARexx message is answered with (the text goes back as RESULT either way) */
#define NT_RC_OK       0
#define NT_RC_UNKNOWN 10           /* not one of our commands */

enum {                             /* nt_notice.flags */
    NT_F_UPDATE    = 1,            /* took the place of the program's earlier notices */
    NT_F_CLOSEONDC = 2,            /* a double-click closes it */
    NT_F_LOGONLY   = 4,
    NT_F_NOLOG     = 8,
    NT_F_PERCENT   = 16            /* percent is set: draw a progress bar */
};

typedef struct nt_app {
    char name[NT_NAME];
    char port[NT_NAME];            /* its ARexx port, for the messages back; "" none */
    char icon[NT_PATH];            /* its icon, without .info; "" none */
    char about[NT_TEXT];           /* what the program is for */
    int blocked;                   /* the user turned its notices off */
} nt_app;

typedef struct nt_notice {
    unsigned long id;              /* counts up from 1 */
    char app[NT_NAME];
    char title[NT_TITLE];
    char text[NT_TEXT];
    char screen[NT_NAME];          /* public screen; "" Workbench, "FRONT" the front one */
    char img[NT_PATH];             /* a picture up to 32 x 32, read through datatypes; "" none */
    char back[NT_RX];              /* sent to the program on a double-click, or "URL:..." */
    char pause[NT_RX];             /* set: a Pause button that sends it */
    char stop[NT_RX];              /* set: a Stop button that sends it */
    int pri;                       /* 0 to 10 */
    int valign;                    /* the picture: 0 top, 1 middle, 2 bottom */
    int percent;                   /* 0 to 100 with NT_F_PERCENT */
    int flags;
} nt_notice;

typedef struct nt_state {
    nt_app apps[NT_APPS];
    int napps;
    nt_notice queue[NT_QUEUE];     /* queue[0] is shown next */
    int nqueue;
    nt_notice log[NT_LOG];         /* log[0] the oldest */
    int nlog;
    unsigned long next_id;
} nt_state;

void nt_init(nt_state *st);

/* One command line as it arrives on the port. reply (NT_REPLY bytes) gets the answer, "OK, ..." or
 * "ERROR: ...". Returns NT_RC_OK for a command we know (whatever the answer), NT_RC_UNKNOWN otherwise. */
int nt_command(nt_state *st, const char *line, char *reply);

/* The next notice to show, taken off the queue: the highest priority first, the oldest of equals.
 * 0 when nothing waits. */
int nt_next(nt_state *st, nt_notice *out);

/* How long a notice stays up, in seconds: 0 means until the user closes it (priority 10).
 * base is the user's setting for an ordinary notice (priority 5). */
int nt_show_seconds(const nt_notice *n, int base);

nt_app *nt_find_app(nt_state *st, const char *name);
int nt_block_app(nt_state *st, const char *name, int blocked);    /* 1 done, 0 no such program */
int nt_unregister_app(nt_state *st, const char *name);            /* its waiting notices go too */

#endif
