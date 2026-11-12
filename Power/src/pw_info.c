/* GFOpenPower: the sensors' JSON and the level's names (pw_info.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "pw_info.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* the text after "key": (spaces skipped), or NULL */
static const char *value_of(const char *json, const char *key)
{
    size_t n = strlen(key);
    const char *s = json;
    while ((s = strchr(s, '"'))) {
        s++;
        if (!strncmp(s, key, n) && s[n] == '"') {
            s += n + 1;
            while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
            if (*s != ':') continue;
            s++;
            while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
            return s;
        }
        while (*s && *s != '"') { if (*s == '\\' && s[1]) s++; s++; }   /* past this string */
        if (*s) s++;
    }
    return NULL;
}

static long number_of(const char *json, const char *key)
{
    const char *v = value_of(json, key);
    if (!v || !((*v >= '0' && *v <= '9') || *v == '-')) return -1;
    return strtol(v, NULL, 10);
}

/* true/false, or a number (vcgencmd's throttled bits: not 0 is yes); -1 when not there */
static int flag_of(const char *json, const char *key)
{
    const char *v = value_of(json, key);
    if (!v) return -1;
    if (!strncmp(v, "true", 4)) return 1;
    if (!strncmp(v, "false", 5)) return 0;
    if (*v == '"') v++;                   /* "0x50005" as a string */
    if (*v >= '0' && *v <= '9') return strtol(v, NULL, 0) != 0;
    return -1;
}

int pw_level_named(const char *name)
{
    static const char *const names[4] = { "Custom", "Quiet", "Balanced", "Full" };
    int i;
    for (i = 0; i < 4; i++) {
        const char *a = names[i], *b = name;
        for (; *a; a++, b++) if ((*a | 32) != (*b | 32)) break;
        if (!*a && (!*b || *b == '"' || *b == '\n' || *b == '\r' || *b == ' ')) return i;
    }
    return -1;
}

const char *pw_level_name(int level)
{
    return level == PW_QUIET ? "Quiet" : level == PW_BALANCED ? "Balanced" : level == PW_FULL ? "Full" : "Custom";
}

void pw_parse(const char *json, pw_info *p)
{
    const char *v;
    p->temp_mc = number_of(json, "tempMilliC");
    p->clock_khz = number_of(json, "clockKHz");
    p->clock_max_khz = number_of(json, "clockMaxKHz");
    p->fan_rpm = number_of(json, "fanRPM");
    p->throttled = flag_of(json, "throttled");
    p->under_voltage = flag_of(json, "underVoltage");
    p->can_set = flag_of(json, "powerLevelCanSet") == 1;
    p->level = (v = value_of(json, "powerLevel")) && *v == '"' ? pw_level_named(v + 1) : -1;
}

/* kHz as "2.0 GHz" or "800 MHz" */
static int clock_text(long khz, char *out, int size)
{
    long tenths = (khz + 50000) / 100000;
    if (khz >= 1000000) return snprintf(out, size, "%ld.%ld GHz", tenths / 10, tenths % 10);
    return snprintf(out, size, "%ld MHz", (khz + 500) / 1000);
}

int pw_sensors_text(const pw_info *p, char *out, int size)
{
    char c1[24], c2[24];
    int n = 0;
    out[0] = 0;
#define ADD(...) do { if (n < size) n += snprintf(out + n, size - n, __VA_ARGS__); } while (0)
    if (p->temp_mc >= 0) {
        long t = (p->temp_mc + 50) / 100;
        ADD("%ld.%ld \xb0" "C", t / 10, t % 10);
    }
    if (p->clock_khz > 0) {
        clock_text(p->clock_khz, c1, sizeof c1);
        if (p->clock_max_khz > 0) { clock_text(p->clock_max_khz, c2, sizeof c2); ADD("%s%s of %s", n ? ", " : "", c1, c2); }
        else ADD("%s%s", n ? ", " : "", c1);
    }
    if (p->fan_rpm >= 0) ADD("%sfan %ld rpm", n ? ", " : "", p->fan_rpm);
    if (p->throttled >= 0) ADD("%s%s", n ? ", " : "", p->throttled ? "throttled" : "not throttled");
    if (p->under_voltage == 1) ADD("%sunder-voltage", n ? ", " : "");
#undef ADD
    return n < size ? n : size - 1;
}
