/* hotkeys.h: ENV:GFOpenPrefs/Hotkeys, the OS-wide hotkeys' switches (Dale, 3 November 2026: "an OS wide option to
 * bring up monitor by pressing ctrl alt help on an amiga and ctrl alt f12 on a pc keyboard").
 *
 * "key value" lines, as ENV:GFOpenPrefs/TitleBar and Windows have. One key so far:
 *   monitor on|off     Ctrl+Alt+Help (Ctrl+Alt+F12 on a PC keyboard) opens GFOpenMonitor; on with no file or no line
 * GFOpenUpMenu (gfopenup src/openupmenu.c, src/openup_hotkeys.h) holds the keys and reads the file; the Input page
 * writes it (ENVARC: too on Save) and then sends Ctrl-F to the task of GFOpenUpMenu's public port.
 *
 * Plain C with no Amiga headers, so Common/tests builds it on the host as it is.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <string.h>

#define HK_ENV "ENV:GFOpenPrefs/Hotkeys"
#define HK_ENVARC "ENVARC:GFOpenPrefs/Hotkeys"
#define HK_PORT "GFOpenUpMenu"                   /* the task that holds the keys: Ctrl-F reads the file again */
#define HK_MAX 1024                              /* the file's most bytes this page reads or writes */

/* the line's word is key, followed by a space or a tab */
static inline const char *hk_value(const char *line, const char *key)
{
    size_t n = strlen(key);
    if (strncmp(line, key, n) || (line[n] != ' ' && line[n] != '\t')) return NULL;
    line += n;
    while (*line == ' ' || *line == '\t') line++;
    return line;
}

/* The file's text (NULL: no file): 1 the GFOpenMonitor hotkeys are on. "off", "no" or "0" turns them off; the last
 * monitor line wins. GFOpenUpMenu reads it the same way (openup_hotkeys.h's ouh_monitor_on). */
static inline int hk_monitor_on(const char *text)
{
    int on = 1;
    const char *p = text, *v;
    while (p && *p) {
        const char *line = p;
        while (*p && *p != '\n') p++;
        if (*p) p++;
        if (*line == ';' || !(v = hk_value(line, "monitor"))) continue;
        on = !(!strncmp(v, "off", 3) || !strncmp(v, "no", 2) || *v == '0');
    }
    return on;
}

/* The file to write: old's lines (old NULL: none) with every monitor line taken out and "monitor on" or "off" put
 * at the end; a first line saying what the file is when old had none. The length written to out, or -1 when it
 * doesn't fit in size bytes. */
static inline int hk_put_monitor(const char *old, int on, char *out, int size)
{
    const char *p = old;
    int n = 0, len;
    const char *head = "; GFOpenPrefs' hotkeys: monitor on|off (Ctrl+Alt+Help, Ctrl+Alt+F12 on a PC: GFOpenMonitor)\n";
    if (size <= 0) return -1;
    out[0] = 0;
    if (!old || !*old) {
        len = (int)strlen(head);
        if (len >= size) return -1;
        memcpy(out, head, len);
        n = len;
    }
    while (p && *p) {
        const char *line = p;
        while (*p && *p != '\n') p++;
        len = (int)(p - line);
        if (*p) p++;
        if (*line != ';' && hk_value(line, "monitor")) continue;
        if (n + len + 1 >= size) return -1;
        memcpy(out + n, line, len);
        n += len;
        out[n++] = '\n';
    }
    len = on ? 11 : 12;
    if (n + len >= size) return -1;
    memcpy(out + n, on ? "monitor on\n" : "monitor off\n", len);
    n += len;
    out[n] = 0;
    return n;
}

#endif /* HOTKEYS_H */
