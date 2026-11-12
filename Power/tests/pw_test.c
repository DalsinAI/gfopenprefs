/* Host tests for pw_info.c: the sensors' JSON as the CM4 and daletop sent it (Team 5, 12 November 2026, 20:20).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "pw_info.h"
#include <stdio.h>
#include <string.h>

static int checks, failed;
#define CHECK(x) do { checks++; if (!(x)) { failed++; printf("FAILED line %d: %s\n", __LINE__, #x); } } while (0)

int main(void)
{
    pw_info p;
    char t[128];
    pw_parse("{\"tempMilliC\":71575,\"clockKHz\":2000000,\"clockMaxKHz\":2000000,\"underVoltage\":false,"
             "\"powerLevel\":\"Balanced\",\"powerLevelCanSet\":false}", &p);
    CHECK(p.temp_mc == 71575 && p.clock_khz == 2000000 && p.clock_max_khz == 2000000);
    CHECK(p.under_voltage == 0 && p.throttled == -1 && p.fan_rpm == -1);
    CHECK(p.level == PW_BALANCED && !p.can_set);
    pw_sensors_text(&p, t, sizeof t);
    CHECK(!strcmp(t, "71.6 \xb0" "C, 2.0 GHz of 2.0 GHz"));
    pw_parse("{ \"tempMilliC\": 70625, \"clockKHz\": 3865000, \"clockMaxKHz\": 3600000, \"powerLevel\": \"Full\", "
             "\"powerLevelCanSet\": true, \"throttled\": 0, \"fanRPM\": 1200 }", &p);
    CHECK(p.level == PW_FULL && p.can_set && p.throttled == 0 && p.fan_rpm == 1200);
    pw_sensors_text(&p, t, sizeof t);
    CHECK(!strcmp(t, "70.6 \xb0" "C, 3.9 GHz of 3.6 GHz, fan 1200 rpm, not throttled"));
    pw_parse("{\"powerLevel\":\"Custom\",\"throttled\":\"0x50005\",\"underVoltage\":true}", &p);
    CHECK(p.level == PW_CUSTOM && p.throttled == 1 && p.under_voltage == 1 && !p.can_set);
    pw_sensors_text(&p, t, sizeof t);
    CHECK(!strcmp(t, "throttled, under-voltage"));
    pw_parse("{}", &p);                   /* a real 68k: nothing measured */
    CHECK(p.level == -1 && p.temp_mc == -1);
    CHECK(pw_sensors_text(&p, t, sizeof t) == 0 && !t[0]);
    pw_parse("{\"name\":\"powerLevel\",\"clockKHz\":800000}", &p);   /* a key's name as a value isn't the key */
    CHECK(p.level == -1 && p.clock_khz == 800000);
    pw_sensors_text(&p, t, sizeof t);
    CHECK(!strcmp(t, "800 MHz"));
    CHECK(pw_level_named("quiet") == PW_QUIET && pw_level_named("FULL\n") == PW_FULL && pw_level_named("Fullest") == -1);
    CHECK(!strcmp(pw_level_name(PW_BALANCED), "Balanced") && !strcmp(pw_level_name(PW_CUSTOM), "Custom"));
    printf("%s   %d checks%s\n", failed ? "FAIL" : "ok", checks, failed ? ", some failed" : " (Power: the sensors and the levels)");
    return failed != 0;
}
