/* Common/hotkeys.h: ENV:GFOpenPrefs/Hotkeys, the GFOpenMonitor hotkeys' switch, as the Input page reads and writes
 * it. Prints "ok"/"FAIL" lines, then PASS or FAIL.
 * Copyright (c) 2026 AmiCradle Team. MIT. */
#include <stdio.h>
#include <string.h>
#include "hotkeys.h"

static int fails;

static void check(const char *what, int ok)
{
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) fails++;
}

int main(void)
{
    char out[HK_MAX], small[16];
    int n;

    /* reading */
    check("read: no file is on", hk_monitor_on(NULL) == 1);
    check("read: an empty file is on", hk_monitor_on("") == 1);
    check("read: monitor on / off", hk_monitor_on("monitor on\n") == 1 && hk_monitor_on("monitor off\n") == 0);
    check("read: no and 0 are off, yes is on", hk_monitor_on("monitor no\n") == 0 && hk_monitor_on("monitor 0\n") == 0 &&
          hk_monitor_on("monitor yes\n") == 1);
    check("read: no newline at the end", hk_monitor_on("monitor off") == 0);
    check("read: a tab after the word", hk_monitor_on("monitor\toff\n") == 0);
    check("read: a comment doesn't count", hk_monitor_on("; monitor off\n") == 1);
    check("read: another word starting monitor doesn't count", hk_monitor_on("monitors off\n") == 1);
    check("read: the last monitor line wins", hk_monitor_on("monitor off\nmonitor on\n") == 1 &&
          hk_monitor_on("monitor on\nother 1\nmonitor off\n") == 0);

    /* writing */
    n = hk_put_monitor(NULL, 0, out, sizeof out);
    check("write: a new file has its comment line, then monitor off",
          n > 0 && out[0] == ';' && strstr(out, "\nmonitor off\n") && (int)strlen(out) == n && hk_monitor_on(out) == 0);
    n = hk_put_monitor("", 1, out, sizeof out);
    check("write: an empty file the same, monitor on", n > 0 && out[0] == ';' && hk_monitor_on(out) == 1);
    n = hk_put_monitor("; kept\nother 1\nmonitor off\nlast x\n", 1, out, sizeof out);
    check("write: other lines kept in order, the monitor line replaced at the end",
          n > 0 && !strcmp(out, "; kept\nother 1\nlast x\nmonitor on\n"));
    n = hk_put_monitor("monitor on\nmonitor off\n", 0, out, sizeof out);
    check("write: every old monitor line goes", n > 0 && !strcmp(out, "monitor off\n"));
    n = hk_put_monitor("; monitor off\nother 1", 1, out, sizeof out);
    check("write: a commented monitor line is kept; a last line with no newline gets one",
          n > 0 && !strcmp(out, "; monitor off\nother 1\nmonitor on\n") && hk_monitor_on(out) == 1);
    check("write and read back: on and off", hk_put_monitor("x 1\n", 1, out, sizeof out) > 0 && hk_monitor_on(out) == 1 &&
          hk_put_monitor(out, 0, out + 512, 512) > 0 && hk_monitor_on(out + 512) == 0);
    check("write: too small is -1", hk_put_monitor(NULL, 1, small, sizeof small) == -1 &&
          hk_put_monitor("a long line kept\n", 1, small, sizeof small) == -1 && hk_put_monitor("", 1, small, 0) == -1);
    n = hk_put_monitor("x\n", 1, small, sizeof small);
    check("write: just fits", n == 13 && !strcmp(small, "x\nmonitor on\n"));

    printf("%s\n", fails ? "FAIL" : "PASS");
    return fails != 0;
}
