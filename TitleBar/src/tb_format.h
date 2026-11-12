/* GFOpenTitle 0.9: the words the bar shows, kept apart so the host tests check them (tests/test_tb.c).
 *
 * Memory (CM4 item 17, Dale, 12 November 2026): "3 GB RAM (0.3 MB in use)", the total first, then what is in use,
 * each in the biggest unit that keeps the number small: KB, MB or GB, never 1,000,000.
 * The time: "Nov 12 1059", 24-hour, no weekday, no colon; "1059" with the date off. The locale doesn't change it yet.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef TB_FORMAT_H
#define TB_FORMAT_H

#include <stdio.h>

/* bytes as "3 GB", "2.9 GB", "0.15 GB", "0.3 MB", "20 KB": the biggest unit the size is at least a tenth of;
 * two decimals below 1, one below 10, none above, and no trailing 0 */
static __attribute__((unused)) int tb_size(unsigned long long bytes, char *out, int size)
{
    static const char *const units[3] = { "KB", "MB", "GB" };
    unsigned long long unit = 1ULL << 30, h;
    int u = 2;
    while (u > 0 && bytes * 10 < unit) { unit >>= 10; u--; }
    h = (bytes * 100 + unit / 2) / unit;          /* hundredths */
    if (h >= 1000) return snprintf(out, size, "%llu %s", (h + 50) / 100, units[u]);
    if (h >= 100) h = (h + 5) / 10 * 10;           /* one decimal from 1 up */
    if (h % 100 == 0) return snprintf(out, size, "%llu %s", h / 100, units[u]);
    if (h % 10 == 0) return snprintf(out, size, "%llu.%llu %s", h / 100, h / 10 % 10, units[u]);
    return snprintf(out, size, "%llu.%02llu %s", h / 100, h % 100, units[u]);
}

/* "3 GB RAM (0.3 MB in use)" (put together by hand: the sizes are short, and snprintf's checks can't know it) */
static __attribute__((unused)) int tb_memory(unsigned long long total, unsigned long long used, char *out, int size)
{
    char t[32], u[32];
    const char *parts[5];
    int n = 0, i, k;
    tb_size(total, t, sizeof t);
    tb_size(used, u, sizeof u);
    parts[0] = t; parts[1] = " RAM ("; parts[2] = u; parts[3] = " in use)"; parts[4] = NULL;
    for (i = 0; parts[i]; i++)
        for (k = 0; parts[i][k] && n < size - 1; k++) out[n++] = parts[i][k];
    if (size > 0) out[n] = 0;
    return n;
}

/* "Nov 12 1059", or "1059" without the date; month 1 to 12 */
static __attribute__((unused)) int tb_clock(int month, int mday, int hour, int min, int date, char *out, int size)
{
    static const char *const months[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    if (date) return snprintf(out, size, "%s %d %02d%02d", months[(month + 11) % 12], mday, hour, min);
    return snprintf(out, size, "%02d%02d", hour, min);
}

/* the same words with every digit a 0: what the bar's room is measured on, so it changes only with the digits' count */
static __attribute__((unused)) void tb_zeros(const char *in, char *out, int size)
{
    int i;
    for (i = 0; i < size - 1 && in[i]; i++) out[i] = in[i] >= '0' && in[i] <= '9' ? '0' : in[i];
    out[i] = 0;
}

#endif
