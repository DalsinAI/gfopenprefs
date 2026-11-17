/* Host tests for GFOpenNotify's commands, programs and queue (src/nt_core.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "nt_core.h"
#include <stdio.h>
#include <string.h>

static int fails, checks;
#define CHECK(c, what) do { checks++; if (!(c)) { fails++; printf("FAIL %s (line %d)\n", what, __LINE__); } } while (0)

static nt_state st;
static char reply[NT_REPLY];
static nt_notice n;

static int says(const char *line, const char *want)
{
    int rc = nt_command(&st, line, reply);
    if (strcmp(reply, want)) printf("     \"%s\" answered \"%s\"\n", line, reply);
    return rc == NT_RC_OK && !strcmp(reply, want);
}

int main(void)
{
    int i;
    char line[4096];

    /* registering */
    nt_init(&st);
    CHECK(says("REGISTERAPP APP=YAM AREXXPORT=YAM ICON=Work:YAM/YAM TEXT An e-mail program", "OK, APP REGISTERED"), "a program registers");
    CHECK(st.napps == 1 && !strcmp(st.apps[0].port, "YAM") && !strcmp(st.apps[0].icon, "Work:YAM/YAM"), "its port and icon are kept");
    CHECK(!strcmp(st.apps[0].about, "An e-mail program"), "TEXT takes the rest of the line");
    CHECK(says("registerapp app yam", "ERROR: APP ALREADY REGISTERED"), "twice is refused, case aside");
    CHECK(says("REGISTERAPP TEXT nothing", "ERROR: NO APP NAME"), "no name is refused");
    CHECK(says("REGISTERAPP", "ERROR: NO APP NAME"), "a bare command is refused");
    CHECK(says("REGISTERAPP APP=\"My App\" TEXT=\"Does *\"things*\"\"", "OK, APP REGISTERED"), "a quoted name with a space");
    CHECK(nt_find_app(&st, "my app") && !strcmp(nt_find_app(&st, "my app")->about, "Does \"things\""), "quotes and *\" in the text");
    CHECK(nt_command(&st, "QUIT", reply) == NT_RC_UNKNOWN, "an unknown command says so");
    CHECK(nt_command(&st, 0, reply) == NT_RC_UNKNOWN && nt_command(&st, "   ", reply) == NT_RC_UNKNOWN, "an empty line is not a command");

    /* a notice */
    CHECK(says("RINGHIO APP=YAM TITLE=\"New mail\" IMG=Work:YAM/mail.iff IMGVALIGN=1 CLOSEONDC BACKRXMSG=\"SHOW\" TEXT 3 new messages", "OK, MSG QUEUED"), "a notice is queued");
    CHECK(nt_next(&st, &n), "and comes off the queue");
    CHECK(!strcmp(n.app, "YAM") && !strcmp(n.title, "New mail") && !strcmp(n.text, "3 new messages"), "its program, title and text");
    CHECK(!strcmp(n.img, "Work:YAM/mail.iff") && n.valign == 1 && !strcmp(n.back, "SHOW"), "its picture and message back");
    CHECK(n.flags == NT_F_CLOSEONDC && n.pri == NT_PRI_DEFAULT && n.id == 1, "its flags, priority and number");
    CHECK(!nt_next(&st, &n), "the queue is empty after");
    CHECK(says("RINGHIO APP YAM TITLE Hello Just text", "OK, MSG QUEUED") && nt_next(&st, &n) && !strcmp(n.title, "Hello") && !strcmp(n.text, "Just text"), "keywords without =, text without TEXT");
    CHECK(says("RINGHIO APP=YAM TEXT=\"Line one*NLine two\"", "OK, MSG QUEUED") && nt_next(&st, &n) && !strcmp(n.text, "Line one\nLine two"), "*N is a new line");
    CHECK(says("RINGHIO APP=YAM TEXT \"Said\" twice", "OK, MSG QUEUED") && nt_next(&st, &n) && !strcmp(n.text, "\"Said\" twice"), "text that only starts with a quote is kept as written");
    CHECK(says("RINGHIO APP=YAM TITLE=\"UPDATE\" TEXT x", "OK, MSG QUEUED") && nt_next(&st, &n) && !strcmp(n.title, "UPDATE") && !(n.flags & NT_F_UPDATE), "a value that looks like a switch is a value");
    CHECK(says("RINGHIO TEXT hello", "ERROR: NO APP NAME"), "no program named");
    CHECK(says("RINGHIO APP=Nobody TEXT hello", "ERROR: APP NOT REGISTERED"), "a program that never registered");

    /* limits */
    strcpy(line, "RINGHIO APP=YAM TITLE=\"");
    for (i = 0; i < 100; i++) strcat(line, "T");
    strcat(line, "\" TEXT ");
    for (i = 0; i < 900; i++) strcat(line, "x");
    CHECK(says(line, "OK, MSG QUEUED") && nt_next(&st, &n) && strlen(n.title) == 64 && strlen(n.text) == 160, "title cut at 64, text at 160");
    for (i = 0; i < 2000 - 900; i++) strcat(line, "y");
    CHECK(strlen(line) > NT_LINE && says(line, "OK, MSG QUEUED") && nt_next(&st, &n) && strlen(n.text) == 160, "a line longer than we read is cut, not overrun");
    CHECK(says("RINGHIO APP=YAM PRI=99 PERCENTAGE=250 IMGVALIGN=7 TEXT x", "OK, MSG QUEUED") && nt_next(&st, &n) && n.pri == 10 && n.percent == 100 && n.valign == 2, "numbers are held to their ranges");
    CHECK(says("RINGHIO APP=YAM PRI=high TEXT x", "OK, MSG QUEUED") && nt_next(&st, &n) && n.pri == NT_PRI_DEFAULT && !(n.flags & NT_F_PERCENT), "a number that isn't one is left out");
    CHECK(says("RINGHIO APP=YAM PERCENTAGE=0 TEXT x", "OK, MSG QUEUED") && nt_next(&st, &n) && (n.flags & NT_F_PERCENT) && n.percent == 0, "0 percent still draws the bar");

    /* order, update, the full queue */
    nt_init(&st);
    says("REGISTERAPP APP=A", "OK, APP REGISTERED"); says("REGISTERAPP APP=B", "OK, APP REGISTERED");
    nt_command(&st, "RINGHIO APP=A TEXT a1", reply); nt_command(&st, "RINGHIO APP=B PRI=8 TEXT b1", reply); nt_command(&st, "RINGHIO APP=A TEXT a2", reply); nt_command(&st, "RINGHIO APP=B PRI=8 TEXT b2", reply);
    CHECK(nt_next(&st, &n) && !strcmp(n.text, "b1") && nt_next(&st, &n) && !strcmp(n.text, "b2") && nt_next(&st, &n) && !strcmp(n.text, "a1") && nt_next(&st, &n) && !strcmp(n.text, "a2"), "highest priority first, the oldest of equals");
    nt_command(&st, "RINGHIO APP=A TEXT a1", reply); nt_command(&st, "RINGHIO APP=B TEXT b1", reply); nt_command(&st, "RINGHIO APP=A TEXT a2", reply);
    CHECK(says("RINGHIO APP=A UPDATE TEXT a3", "OK, MSG QUEUED") && st.nqueue == 2, "UPDATE takes the program's waiting notices away");
    CHECK(nt_next(&st, &n) && !strcmp(n.text, "b1") && nt_next(&st, &n) && !strcmp(n.text, "a3") && (n.flags & NT_F_UPDATE), "and leaves the others");
    for (i = 0; i < NT_QUEUE; i++) says("RINGHIO APP=A TEXT fill", "OK, MSG QUEUED");
    CHECK(st.nqueue == NT_QUEUE && says("RINGHIO APP=A TEXT one more", "ERROR: TOO MANY MESSAGES QUEUED"), "a full queue refuses");
    CHECK(says("RINGHIO APP=A LOGONLY TEXT quiet", "OK, MSG QUEUED") && st.nqueue == NT_QUEUE, "LOGONLY needs no room in it");
    CHECK(says("RINGHIO APP=B UPDATE TEXT b", "ERROR: TOO MANY MESSAGES QUEUED"), "UPDATE of another program finds no room");
    CHECK(says("RINGHIO APP=A UPDATE TEXT fresh", "OK, MSG QUEUED") && st.nqueue == 1, "UPDATE of the program that filled it does");

    /* the log */
    nt_init(&st);
    nt_command(&st, "REGISTERAPP APP=A", reply);
    nt_command(&st, "RINGHIO APP=A NOLOG TEXT shown only", reply); nt_command(&st, "RINGHIO APP=A LOGONLY TEXT logged only", reply); nt_command(&st, "RINGHIO APP=A TEXT both", reply);
    CHECK(st.nlog == 2 && !strcmp(st.log[0].text, "logged only") && !strcmp(st.log[1].text, "both"), "NOLOG stays out of the log");
    CHECK(st.nqueue == 2 && !strcmp(st.queue[0].text, "shown only") && !strcmp(st.queue[1].text, "both"), "LOGONLY stays off the screen");
    nt_next(&st, &n); nt_next(&st, &n);
    for (i = 0; i < NT_LOG + 5; i++) { sprintf(line, "RINGHIO APP=A LOGONLY TEXT n%d", i); says(line, "OK, MSG QUEUED"); }
    sprintf(line, "n%d", NT_LOG + 4);
    CHECK(st.nlog == NT_LOG && !strcmp(st.log[NT_LOG - 1].text, line) && !strcmp(st.log[0].text, "n5"), "the log keeps the newest, the oldest goes");

    /* blocking, leaving, a full list of programs */
    nt_init(&st);
    nt_command(&st, "REGISTERAPP APP=A", reply); nt_command(&st, "REGISTERAPP APP=B", reply);
    CHECK(nt_block_app(&st, "a", 1) && says("RINGHIO APP=A TEXT x", "ERROR: APP NOT ALLOWED"), "a blocked program is refused");
    CHECK(nt_block_app(&st, "A", 0) && says("RINGHIO APP=A TEXT x", "OK, MSG QUEUED") && !nt_block_app(&st, "C", 1), "and allowed again");
    nt_command(&st, "RINGHIO APP=B TEXT y", reply);
    CHECK(nt_unregister_app(&st, "A") && st.napps == 1 && st.nqueue == 1 && !strcmp(st.queue[0].app, "B"), "a program that leaves takes its notices");
    CHECK(!nt_unregister_app(&st, "A") && says("RINGHIO APP=A TEXT x", "ERROR: APP NOT REGISTERED"), "and is gone");
    nt_init(&st);
    for (i = 0; i < NT_APPS; i++) { sprintf(line, "REGISTERAPP APP=P%d", i); says(line, "OK, APP REGISTERED"); }
    CHECK(says("REGISTERAPP APP=Last", "ERROR: OUT OF MEMORY") && st.napps == NT_APPS, "the list of programs is bounded");

    /* how long a notice stays */
    memset(&n, 0, sizeof n);
    n.pri = 5; CHECK(nt_show_seconds(&n, 6) == 6, "priority 5 stays the set time");
    n.pri = 0; CHECK(nt_show_seconds(&n, 6) == 3, "priority 0 half of it");
    n.pri = 9; CHECK(nt_show_seconds(&n, 6) == 8, "priority 9 longer");
    n.pri = 10; CHECK(nt_show_seconds(&n, 6) == 0, "priority 10 until closed");
    n.pri = 0; CHECK(nt_show_seconds(&n, 0) == 1, "never less than a second");

    printf("%d checks, %d failed\n", checks, fails);
    return fails ? 1 : 0;
}
