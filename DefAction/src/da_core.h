/* da_core.h: GFOpenDefAction's rules, in plain C so the host can test them (DefAction/tests/da_test.c).
 *
 * A project icon with no default tool (or, when asked, one naming a program that isn't on this machine) is
 * offered one by the kind of file it belongs to. The kind is one of seven groups; each group's program is the
 * page's rule when it has one, else what the kind's default icon (ENV:Sys/def_<kind>.info) names.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef DA_CORE_H
#define DA_CORE_H

#include <stddef.h>

enum { DA_NONE = -1, DA_TEXT, DA_GUIDE, DA_PICTURE, DA_MOVIE, DA_SOUND, DA_MUSIC, DA_DOCUMENT, DA_NGROUPS };

#define DA_TOOL_MAX 200
#define DA_DRAWERS_MAX 256

/* datatypes.library's groups (datatypes/datatypes.h's GID_*), as their four letters */
#define DA_ID(a, b, c, d) (((unsigned long)(a) << 24) | ((unsigned long)(b) << 16) | ((unsigned long)(c) << 8) | (unsigned long)(d))
#define DA_GID_SYSTEM DA_ID('s', 'y', 's', 't')
#define DA_GID_TEXT DA_ID('t', 'e', 'x', 't')
#define DA_GID_DOCUMENT DA_ID('d', 'o', 'c', 'u')
#define DA_GID_SOUND DA_ID('s', 'o', 'u', 'n')
#define DA_GID_INSTRUMENT DA_ID('i', 'n', 's', 't')
#define DA_GID_MUSIC DA_ID('m', 'u', 's', 'i')
#define DA_GID_PICTURE DA_ID('p', 'i', 'c', 't')
#define DA_GID_ANIMATION DA_ID('a', 'n', 'i', 'm')
#define DA_GID_MOVIE DA_ID('m', 'o', 'v', 'i')

typedef struct {
    char tool[DA_NGROUPS][DA_TOOL_MAX];  /* "": the kind's default icon names it */
    char drawers[DA_DRAWERS_MAX];        /* where Scan looks, spaces between ("SYS: Work:") */
    int missing;                         /* 1: icons naming a program that isn't here are offered one too */
} da_prefs;

/* The group's name as the page shows it ("Text", "Pictures"), and its word in the prefs file ("TEXT") */
const char *da_group_label(int group);
const char *da_group_word(int group);
/* The default icon a group falls back to when its own kind has none: def_ascii, def_picture... */
const char *da_group_defname(int group);

/* The group of a type name: DefIcons' (icon.library's identify hook: "ascii", "ilbm", "mpeg") or a datatype's
 * ("ILBM", "AmigaGuide", "gfopenpng"), letters in either case; DA_NONE when not one of ours */
int da_group_of_type(const char *type);
/* The group of a datatype's group ID (DA_GID_*); DA_NONE for system files and those not known */
int da_group_of_gid(unsigned long gid);
/* The group of a file by its name alone: "Nostalgia.readme", "ReadMe", "pic.iff", "tune.mod"; DA_NONE */
int da_group_of_name(const char *name);

/* 1 when an icon's default tool is no tool: NULL, empty or only spaces */
int da_tool_blank(const char *tool);
/* 1 when the icon is to be offered a tool: no tool, or (missing on) one that isn't here */
int da_wants_offer(const char *tool, int tool_found, int missing);

/* The prefs: built in (every group from its default icon, SYS: and Work:, missing on), and as the file holds
 * them ("TEXT=SYS:Utilities/MultiView", "DRAWERS=SYS: Work:", "MISSING=ON", one a line, any case for the words;
 * a line it doesn't know is passed over). Parse returns how many lines it took. */
void da_prefs_default(da_prefs *p);
int da_prefs_parse(const char *text, da_prefs *p);
/* p as the file's lines; the length (cut to cap, NUL-ended) */
int da_prefs_format(const da_prefs *p, char *out, size_t cap);

/* The list of changes (ENVARC:GFOpenDefAction/Changes): one line an icon, "<file>\t<old tool>\n", the old tool
 * empty when it had none. Format returns the length; parse cuts line at its tab and returns the old tool, or
 * NULL for a line that isn't one. */
int da_change_line(const char *file, const char *old_tool, char *out, size_t cap);
const char *da_change_parse(char *line);

/* The next drawer of a "SYS: Work:" list into out (quotes keep spaces: "\"Work:My Files\""); returns where the
 * rest starts, or NULL when there are no more */
const char *da_next_drawer(const char *list, char *out, size_t cap);

#endif
