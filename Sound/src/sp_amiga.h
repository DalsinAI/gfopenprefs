/* sp_amiga: what the Sound editor and GFOpenSpeaker share on the Amiga: the
 * settings files, and the ACAHI board's levels.
 *
 * With the board (AmiCradle), the levels go to its LEVELS register, which
 * the PC's mixer applies to Paula's sound and the board's. Without it (a
 * real Amiga), the volume is AHI's output volume on every unit, and Paula's
 * level is the speakers' knob.
 *
 * MIT, Copyright (c) 2026 AmigaChrome Team. */
#ifndef SP_AMIGA_H
#define SP_AMIGA_H

#include <exec/types.h>
#include "sp_core.h"

#define SP_ENV        "ENV:GFOpenAmiga/Sound"
#define SP_ENVARC     "ENVARC:GFOpenAmiga/Sound"
#define SP_SOUND_ENV  "ENV:Sys/sound.prefs"
#define SP_SOUND_ARC  "ENVARC:Sys/sound.prefs"
#define SP_AHI_ENV    "ENV:Sys/ahi.prefs"
#define SP_AHI_ARC    "ENVARC:Sys/ahi.prefs"
#define SP_SPEAKER_CMD  "C:GFOpenSpeaker"
#define SP_SPEAKER_PORT "GFOpenSpeaker"     /* GFOpenSpeaker's port: Ctrl-F to its task reads the levels again */

/* The ACAHI board with LEVELS (protocol acahi-v1, vendor/amigachrome-guest
 * common/protocol/acahi.h), or NULL. */
volatile ULONG *sp_board(void);
ULONG sp_board_levels(volatile ULONG *b);           /* the LEVELS register */
ULONG sp_board_seq(volatile ULONG *b);              /* LEVEL_SEQ: moves on every change, from either side */
void sp_board_set(volatile ULONG *b, ULONG levels);

/* Everything from ENV: (the files, and the board's live levels over the
 * file's when there is a board). from: another ENV:GFOpenAmiga/Sound, or NULL. */
void sp_load(sp_settings *s, const char *from);
/* The files into ENV: (and ENVARC: when save): our own, sound.prefs, and
 * ahi.prefs (its units' mode for the mix; without the board, the volume). */
void sp_store(sp_settings *s, int save);
/* The levels, now: to the board, or to AHI's units without one (in
 * ENV:Sys/ahi.prefs, heard when a program next opens AHI). */
void sp_apply_levels(sp_settings *s, volatile ULONG *board);
/* Only our own file (ENV:GFOpenAmiga/Sound), with s's levels and the rest as
 * the file has them; ENVARC: too when save. The menu bar speaker keeps the
 * volume this way, as a knob keeps its place. */
void sp_save_levels(const sp_settings *s, int save);
/* GFOpenSpeaker, if it runs, reads the levels again; started when it doesn't
 * run and s says to show it. */
void sp_tell_speaker(void);
void sp_start_speaker(void);

#endif
