/* GFOpenPower: the host's power level and sensors, as accontrol.device 1.6's ACCMD_POWERLEVEL gives them (JSON text,
 * the host's /api/system/status "host.sensors": amicradle #1064), and the level's name in ENV:GFOpenPrefs/Power.
 * Plain C, no Amiga calls, so the host tests check it (Power/tests/pw_test.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef PW_INFO_H
#define PW_INFO_H

#define PW_CUSTOM   0                     /* set by hand outside AmiCradle */
#define PW_QUIET    1                     /* ACC_POWERLEVEL_QUIET, _BALANCED, _FULL: io_Offset to set it */
#define PW_BALANCED 2
#define PW_FULL     3

typedef struct {
    long temp_mc;                         /* tempMilliC; -1 not measured */
    long clock_khz, clock_max_khz;        /* clockKHz, clockMaxKHz; -1 not measured */
    long fan_rpm;                         /* fanRPM; -1 */
    int throttled, under_voltage;         /* -1 not measured, else 0 or 1 (a number: not 0) */
    int level;                            /* PW_*; -1 the host gave none */
    int can_set;                          /* powerLevelCanSet */
} pw_info;

/* the sensors' JSON into p; keys not there stay "not measured" */
void pw_parse(const char *json, pw_info *p);
/* "Quiet", "Balanced", "Full" (any case) to PW_*, "Custom" PW_CUSTOM; -1 for anything else */
int pw_level_named(const char *name);
/* the level's name; "Custom" for PW_CUSTOM */
const char *pw_level_name(int level);
/* "71.6 °C, 2.0 GHz of 2.0 GHz, not throttled" (Latin-1 degree sign); "" when nothing was measured */
int pw_sensors_text(const pw_info *p, char *out, int size);

#endif
