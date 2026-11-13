/* Host test for GFOpenUp Setup's unattended mode: the answers file and the choice of mode (src/setup_auto.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "setup_auto.h"

static int fails, checks;
#define CHECK(c, msg) do { checks++; if (!(c)) { fails++; printf("FAIL %s\n", msg); } } while (0)

static void parse_tests(void)
{
    sa_answers a;
    char big[400];

    sa_init(&a);
    CHECK(sa_parse("", &a) == 0, "empty file: nothing bad");
    CHECK(a.view == -1 && a.by_machine == -1 && !a.profile[0] && a.noff == 0, "empty file: every key takes the default");

    sa_init(&a);
    CHECK(sa_parse("; GFOpenUp Setup answers\n# a comment\n\n   \nVIEW=advanced\nPROFILE=Modern\nBYMACHINE=no\nOFF=GFOpenMenus\n", &a) == 0,
          "every key: nothing bad");
    CHECK(a.view == 1, "VIEW=advanced");
    CHECK(!strcmp(a.profile, "Modern"), "PROFILE=Modern");
    CHECK(a.by_machine == 0, "BYMACHINE=no");
    CHECK(a.noff == 1 && !strcmp(a.off[0], "GFOpenMenus"), "OFF=GFOpenMenus");

    sa_init(&a);
    sa_parse("view = Simple\r\n  profile = \"Lite 68040\"  \r\nbymachine=YES\r\n", &a);
    CHECK(a.view == 0, "keys and values any case, spaces round =, CRLF");
    CHECK(!strcmp(a.profile, "Lite 68040"), "quoted value with a space");
    CHECK(a.by_machine == 1, "BYMACHINE=YES");

    sa_init(&a);
    CHECK(sa_parse("BYMACHINE=on\n", &a) == 0 && a.by_machine == 1, "on");
    sa_init(&a);
    CHECK(sa_parse("BYMACHINE=0", &a) == 0 && a.by_machine == 0, "0, and no newline at the end");

    sa_init(&a);
    CHECK(sa_parse("VIEW=fancy\nCOLOUR=red\njust words\nBYMACHINE=maybe\nPROFILE=\nVIEW=advanced\n", &a) == 5,
          "five lines not understood");
    CHECK(a.view == 1, "a good line after bad ones still counts");
    CHECK(a.by_machine == -1 && !a.profile[0], "bad values leave the default");

    sa_init(&a);
    memset(big, 'x', sizeof big); memcpy(big, "PROFILE=", 8); big[sizeof big - 1] = 0;
    CHECK(sa_parse(big, &a) == 1 && !a.profile[0], "a profile name too long is refused, not cut");

    sa_init(&a);
    for (int i = 0; i < SA_MAX_OFF + 2; i++) sa_parse("OFF=GFOpenDock\n", &a);
    CHECK(a.noff == SA_MAX_OFF && a.bad == 2, "OFF lines past the limit are counted as ignored");

    sa_init(&a);
    sa_parse("VIEW=simple\nVIEW=advanced\n", &a);
    CHECK(a.view == 1, "the last of a key wins");
}

static void mode_tests(void)
{
    int by;
    /* auto_switch, first, done, answers, flag */
    CHECK(sa_mode(0, 0, 0, 0, 0, &by) == SA_WINDOW && by == SA_BY_NONE, "by hand from Prefs: the window");
    CHECK(sa_mode(0, 0, 0, 1, 1, &by) == SA_WINDOW, "by hand: the window even with answers and the host flag");
    CHECK(sa_mode(0, 0, 1, 0, 0, &by) == SA_WINDOW, "by hand after Setup.done: the window");
    CHECK(sa_mode(0, 1, 0, 0, 0, &by) == SA_WINDOW, "first start, nothing set: the window, as before");
    CHECK(sa_mode(0, 1, 1, 0, 0, &by) == SA_SKIP, "first start after Setup.done: nothing");
    CHECK(sa_mode(0, 1, 1, 1, 1, &by) == SA_SKIP, "first start after Setup.done: nothing, answers or not");
    CHECK(sa_mode(0, 1, 0, 1, 0, &by) == SA_AUTO && by == SA_BY_ANSWERS, "first start with answers: unattended");
    CHECK(sa_mode(0, 1, 0, 0, 1, &by) == SA_AUTO && by == SA_BY_FLAG, "first start with the host flag: unattended");
    CHECK(sa_mode(0, 1, 0, 1, 1, &by) == SA_AUTO && by == SA_BY_ANSWERS, "both: the answers are named");
    CHECK(sa_mode(1, 0, 0, 0, 0, &by) == SA_AUTO && by == SA_BY_SWITCH, "AUTO by hand: unattended");
    CHECK(sa_mode(1, 1, 1, 0, 0, &by) == SA_AUTO && by == SA_BY_SWITCH, "AUTO after Setup.done: unattended again");
}

static void line_tests(void)
{
    CHECK(sa_line_has("Run >NIL: C:GFOpenMenus", "GFOpenMenus"), "OFF words found");
    CHECK(sa_line_has("Run >NIL: C:GFOpenMenus", "gfopenmenus"), "OFF words any case");
    CHECK(!sa_line_has("Run >NIL: C:GFOpenDock", "GFOpenMenus"), "other lines stay");
    CHECK(!sa_line_has("Run >NIL: C:GFOpenDock", ""), "empty words match nothing");
}

static void log_tests(void)
{
    sa_answers a;
    char buf[300];
    sa_init(&a);
    a.view = 0;
    sa_log_line(buf, sizeof buf, &a, SA_BY_FLAG, "", 0);
    CHECK(!strcmp(buf, "GFOpenUp Setup: unattended (host flag) view=simple profile=kept bymachine=kept off=0 ignored=0: ok"), buf);
    sa_init(&a);
    sa_parse("PROFILE=Modern\nOFF=GFOpenMenus\nCOLOUR=red\n", &a);
    sa_log_line(buf, sizeof buf, &a, SA_BY_ANSWERS, "Modern", 5);
    CHECK(!strcmp(buf, "GFOpenUp Setup: unattended (answers) view=kept profile=Modern bymachine=yes off=1 ignored=1: done, some answers ignored"), buf);
    sa_init(&a);
    sa_log_line(buf, sizeof buf, &a, SA_BY_SWITCH, NULL, 10);
    CHECK(strstr(buf, "(AUTO)") && strstr(buf, ": failed, Setup not marked done"), buf);
    CHECK(!strchr(buf, '\n'), "one line");
}

int main(void)
{
    parse_tests();
    mode_tests();
    line_tests();
    log_tests();
    printf("%s   %d checks (GFOpenUp Setup unattended: answers file, mode, log line)\n", fails ? "FAIL" : "ok", checks);
    return fails != 0;
}
