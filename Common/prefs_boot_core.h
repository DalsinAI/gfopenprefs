/* GFOpenIntuition: what the prefs task applies at boot from three plain
 * files the Preferences editors write (salvor/Design-No-ROM-Volume-20261024.md,
 * section 9.3; prefs_boot.c is the Amiga side). Each is read as the editor
 * that writes it reads it, in gfopenprefs: Sound (Sound/src/sp_core.c,
 * sp_parse and sp_levels_word), System (System/src/opensystem.c,
 * load_settings and clamp_kb) and Gamepads (gfopeninput's library: the
 * patch is on when the file's first byte is '1').
 * Copyright (c) 2026 AmiCradle Team. MIT.
 *
 * Plain C: no Amiga call, no library, no globals. Text in (its length
 * given: the files have no NUL), register values out.
 *
 * A copy of amicradle-guest's libraries/openintuition/prefs_boot_core.c
 * and .h (commit f842038, 2 November 2026), where intuition's prefs task
 * runs it at boot (prefs_boot.c): keep the two the same. One set of
 * vectors checks both (here Common/tests/prefs_boot_core_test.c; there
 * tests/m68k/prefs_boot_core_test.c), and Common/tests/run.sh checks the
 * Sound reader against Sound's own (sp_core.c). */
#ifndef OI_PREFS_BOOT_CORE_H
#define OI_PREFS_BOOT_CORE_H

typedef unsigned int pbc_u32;          /* 32 bits on the 68k and the hosts the tests run on */

/* ENV:GFOpenAmiga/Sound, Sound's format 1: "key value" lines, a ';' line a
 * comment, unknown keys skipped. The ACAHI board's LEVELS word:
 *   volume | paula << 8 | ahi << 16 | mute << 24
 * each level 0-100 (a number as atoi reads it, then clamped), mute "on",
 * "yes" or "1" (and "off", "no" or "0"; anything else keeps it). Missing
 * keys: 100, 100, 100, unmuted, the board's own start. */
pbc_u32 pbc_sound_levels(const char *text, pbc_u32 len);

/* ENV:GFOpenAmiga/Stack: "CAP kb", then "PROGRAM name kb" lines. A kb is
 * rounded up to whole 64 KB and kept to 64-65536 (a PROGRAM line's must be
 * above 0); a name is 1-31 characters with no blank, ':' or '/', its case
 * ignored, the last line for a name counting; at most 32 programs (the
 * runtime keeps 32). No CAP line: 1024. */
#define PBC_MAXPROGS 32
#define PBC_NAMELEN  32                 /* the runtime's name register: 31 characters and a NUL */
#define PBC_CAP_DEFAULT_KB 1024
struct pbc_prog { char name[PBC_NAMELEN]; pbc_u32 kb; };
struct pbc_stack {
    pbc_u32 cap_kb;
    int nprogs;
    struct pbc_prog prog[PBC_MAXPROGS];
};
void pbc_stack_parse(struct pbc_stack *s, const char *text, pbc_u32 len);

/* ENV:GFOpenGadTools/Look, as GFOpenPrefs Look writes it: its "scale <percent>"
 * line (not the first, which names the theme), kept to 50-400. No line: 100. */
pbc_u32 pbc_look_scale(const char *text, pbc_u32 len);

/* Its "oldapp <program> <2-4>" lines (docs/design/old-apps-scale.md): the
 * programs Intuition scales whole. A name is 1-31 characters, quoted or one
 * word, its case ignored, the last line for a name counting; a factor that
 * isn't 2, 3 or 4 (or none) is 2; at most PBC_MAXOLD. Returns how many. */
#define PBC_MAXOLD 32
struct pbc_oldapp { char name[PBC_NAMELEN]; pbc_u32 factor; };
int pbc_look_oldapps(struct pbc_oldapp *out, const char *text, pbc_u32 len);

/* ENV:GFOpenInput/LowLevelPatch: 1 when the patch is on (the first byte '1') */
int pbc_lowlevel_on(const char *text, pbc_u32 len);

#endif
