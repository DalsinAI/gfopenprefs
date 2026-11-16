/* gd_prefs: ENV:Sys/gfosdisplay.prefs, the display profiles' settings (amicradle-teams
 * designs/DISPLAY-PROFILES-20261113.md, section 4), read from and written into its GDSP chunk.
 *
 * The chunk is six records of GD_RECORD bytes, profile 1 to 6 in order, big-endian, as GFOpenIntuition
 * reads it (amicradle-guest libraries/openintuition/screen_core.h, OI_GDSP_):
 *    0  profile number (1-6)       1  reserved, 0
 *    2  word: GD_KEEP_ bits, a set bit keeps the shipped value
 *    4  screen font name, 32 bytes with its NUL     36  its YSize (word)
 *   38  system default font name, 32 bytes         70  its YSize
 *   72  icon font name, 32 bytes                   104  its YSize
 *  106  image set name, 16 bytes: "lores", "hires12", "medres", "large"
 *  122  title bar: pixels over the font            123  frame
 *  124  border across                              125  border down
 *  126  grey-level text edges, 0 or 1              127  reserved, 0
 * As dp_prefs does, a put writes the fields it knows and leaves every other byte as the file had it (the
 * reserved bytes, anything after the six records), so settings put back unchanged give the file's bytes.
 * Plain C: tested on the host by Display/tests/run.sh.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef GD_PREFS_H
#define GD_PREFS_H

#include "pf_iff.h"

#define GD_GDSP PF_ID('G', 'D', 'S', 'P')
#define GD_PROFILES 6
#define GD_RECORD 128
#define GD_LEN (GD_PROFILES * GD_RECORD)
#define GD_NAME 32                        /* a font name's bytes, its NUL in them */
#define GD_SET 16                         /* the image set name's */

#define GD_KEEP_SCREENFONT 0x0001
#define GD_KEEP_SYSFONT    0x0002
#define GD_KEEP_ICONFONT   0x0004
#define GD_KEEP_IMAGES     0x0008
#define GD_KEEP_TITLEBAR   0x0010
#define GD_KEEP_FRAME      0x0020
#define GD_KEEP_GREY       0x0040
#define GD_KEEP_ALL        0x007F

/* the profiles (GFOpenIntuition's OI_PROFILE_) */
enum { GD_LORES = 1, GD_HIRES, GD_LACED, GD_RTG_SMALL, GD_RTG_1080, GD_RTG_LARGE };

typedef struct gd_font {
    char name[GD_NAME];           /* "" : the system font (the chipset profiles' screen font today) */
    int ysize;
} gd_font;

typedef struct gd_profile {
    unsigned keep;                /* GD_KEEP_: the fields that are the shipped ones */
    gd_font screen, sys, icon;
    char set[GD_SET];             /* "lores", "hires12", "medres", "large" */
    int titlebar, frame, border_x, border_y;
    int grey;                     /* grey-level text edges */
} gd_profile;

/* profile n's shipped row (1-6; any other n gives 5's), its keep bits all set */
void gd_shipped(int n, gd_profile *out);
/* The six rows from f's GDSP: each field the file keeps, or can't be used, is the shipped one and its keep
 * bit set. 1 when the chunk was there and whole; 0 (no chunk, a short one, or a record out of order) and
 * out all shipped. */
int gd_get(const pf_file *f, gd_profile out[GD_PROFILES]);
/* The six rows into f's GDSP (added after PRHD's chunks when missing). A kept field's bytes stay as the
 * file had them. 1, or 0 with no memory or a full file. */
int gd_put(pf_file *f, const gd_profile in[GD_PROFILES]);
/* The profile GFOpenIntuition gives a screen of that DisplayID, DisplayInfo Resolution (ticks; 0: square) and
 * size (screen_core.c oi_profile_of): an RTG mode (monitor $00100000 and up) by width, up to 1024, up to 1920,
 * wider; a chipset one by its pixels' shape. The page opens on the profile of the screen it is on. */
#define GD_MONITOR_MASK 0xFFFF1000UL
#define GD_RTG_FIRST 0x00100000UL
int gd_profile_of(uint32_t id, unsigned res_x, unsigned res_y, int width, int height);
/* 1 when s is one of the four image sets */
int gd_set_known(const char *s);
/* the row with every field set to the shipped one again */
void gd_reset(int n, gd_profile *p);

#endif /* GD_PREFS_H */
