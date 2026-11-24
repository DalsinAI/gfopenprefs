/* da_core.c: GFOpenDefAction's rules (da_core.h). Plain C: no Amiga headers, tested on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "da_core.h"

static const char *const labels[DA_NGROUPS] = { "Text and readmes", "AmigaGuide", "Pictures", "Movies and animations",
                                                "Sounds", "Music", "Documents" };
static const char *const words[DA_NGROUPS] = { "TEXT", "GUIDE", "PICTURES", "MOVIES", "SOUNDS", "MUSIC", "DOCUMENTS" };
static const char *const defnames[DA_NGROUPS] = { "ascii", "amigaguide", "picture", "anim", "sound", "music", "pdf" };

const char *da_group_label(int g) { return g >= 0 && g < DA_NGROUPS ? labels[g] : ""; }
const char *da_group_word(int g) { return g >= 0 && g < DA_NGROUPS ? words[g] : ""; }
const char *da_group_defname(int g) { return g >= 0 && g < DA_NGROUPS ? defnames[g] : ""; }

static char lower(char c) { return c >= 'A' && c <= 'Z' ? (char)(c + 32) : c; }

static int same(const char *a, const char *b)      /* case-blind */
{
    for (; *a && *b; a++, b++)
        if (lower(*a) != lower(*b)) return 0;
    return *a == *b;
}

static int starts(const char *s, const char *with)  /* case-blind */
{
    for (; *with; s++, with++)
        if (lower(*s) != lower(*with)) return 0;
    return 1;
}

/* a word in a list of words, each between spaces: " ascii text " */
static int listed(const char *list, const char *w)
{
    size_t n = strlen(w);
    const char *p = list;
    if (!n) return 0;
    while ((p = strchr(p, ' '))) {
        p++;
        if (!*p) break;
        {
            size_t i;
            for (i = 0; i < n && p[i] && lower(p[i]) == lower(w[i]); i++) ;
            if (i == n && p[n] == ' ') return 1;
        }
    }
    return 0;
}

/* The type names each group takes, between spaces. DefIcons' (its own names: ascii, ilbm, mpeg...), the OS's
 * datatypes', and ours (gfopen<name>; gfopendoc is GFOpenDoc's office documents, not text). */
static const char *const types[DA_NGROUPS] = {
    " ascii text ftxt txt readme doc ",
    " amigaguide guide ",
    " picture ilbm jpeg jpg png gif bmp tga tiff tif webp pcx ham gfopenpicture gfopenpng gfopentga gfopengif gfopenwebp ",
    " anim animation anim5 anim7 anim8 video movie mpeg mpg avi quicktime mov mp4 cdxl webm gfopenvideo ",
    " sound 8svx wav wave aiff aif mp3 ogg flac audio gfopensound gfopenwave gfopenaiff ",
    " music mod module protracker med xm s3m midi mid smus gfopenmodule gfopenmidi ",
    " pdf document officedocs rtf odt gfopendoc ",
};

int da_group_of_type(const char *type)
{
    int g;
    if (!type || !*type) return DA_NONE;
    for (g = 0; g < DA_NGROUPS; g++)
        if (listed(types[g], type)) return g;
    return DA_NONE;
}

int da_group_of_gid(unsigned long gid)
{
    if (gid == DA_GID_TEXT) return DA_TEXT;
    if (gid == DA_GID_DOCUMENT) return DA_DOCUMENT;
    if (gid == DA_GID_SOUND || gid == DA_GID_INSTRUMENT) return DA_SOUND;
    if (gid == DA_GID_MUSIC) return DA_MUSIC;
    if (gid == DA_GID_PICTURE) return DA_PICTURE;
    if (gid == DA_GID_ANIMATION || gid == DA_GID_MOVIE) return DA_MOVIE;
    return DA_NONE;
}

/* By the name's end (after its last dot); Amiga modules by the start ("mod.tune"); readmes by the whole name */
static const char *const suffixes[DA_NGROUPS] = {
    " readme txt text doc asc ascii nfo diz 1st me faq man ",
    " guide amigaguide ag ",
    " iff ilbm lbm pic jpg jpeg png gif bmp tga tif tiff webp pcx ham ",
    " anim anm mpg mpeg mp4 m4v avi mov mkv webm cdxl flc fli ",
    " 8svx svx wav aif aiff aifc mp3 mp2 ogg oga flac snd au voc ",
    " mod med mmd0 mmd1 mmd2 mmd3 xm s3m it mid midi smus okt ahx sid ",
    " pdf ps rtf odt docx ",
};
static const char *const prefixes[] = { "mod.", "med.", "p61.", "xm.", "s3m.", "ahx.", NULL };
static const char *const readmes[] = { "readme", "read.me", "read_me", "read-me", "readme.first", "liesmich", "lisezmoi",
                                       "leggimi", "leesmij", NULL };

int da_group_of_name(const char *name)
{
    const char *dot, *p;
    int g, i;
    if (!name || !*name) return DA_NONE;
    if ((p = strrchr(name, '/'))) name = p + 1;    /* the file's own name */
    if ((p = strrchr(name, ':'))) name = p + 1;
    for (i = 0; readmes[i]; i++)
        if (same(name, readmes[i])) return DA_TEXT;
    for (i = 0; prefixes[i]; i++)
        if (starts(name, prefixes[i]) && name[strlen(prefixes[i])]) return DA_MUSIC;
    if (!(dot = strrchr(name, '.')) || !dot[1] || dot == name) return DA_NONE;
    for (g = 0; g < DA_NGROUPS; g++)
        if (listed(suffixes[g], dot + 1)) return g;
    return DA_NONE;
}

int da_tool_blank(const char *tool)
{
    if (!tool) return 1;
    for (; *tool; tool++)
        if (*tool != ' ' && *tool != '\t') return 0;
    return 1;
}

int da_wants_offer(const char *tool, int tool_found, int missing)
{
    if (da_tool_blank(tool)) return 1;
    return missing && !tool_found;
}

static void copy(char *to, const char *from, size_t cap)
{
    size_t n = strlen(from);
    if (!cap) return;
    if (n >= cap) n = cap - 1;
    memcpy(to, from, n);
    to[n] = 0;
}

void da_prefs_default(da_prefs *p)
{
    memset(p, 0, sizeof *p);
    copy(p->drawers, "SYS: Work:", sizeof p->drawers);
    p->missing = 1;
}

int da_prefs_parse(const char *text, da_prefs *p)
{
    int taken = 0;
    while (text && *text) {
        const char *end = strchr(text, '\n'), *eq;
        size_t len = end ? (size_t)(end - text) : strlen(text);
        char line[DA_DRAWERS_MAX + 32], *v;
        int g;
        if (len >= sizeof line) len = sizeof line - 1;
        memcpy(line, text, len);
        line[len] = 0;
        while (len && (line[len - 1] == '\r' || line[len - 1] == ' ' || line[len - 1] == '\t')) line[--len] = 0;
        text = end ? end + 1 : text + strlen(text);
        if (!(eq = strchr(line, '='))) continue;
        v = line + (eq - line);
        *v++ = 0;
        while (*v == ' ' || *v == '\t') v++;
        {   /* the word without the spaces before the = */
            size_t k = strlen(line);
            while (k && (line[k - 1] == ' ' || line[k - 1] == '\t')) line[--k] = 0;
        }
        for (g = 0; g < DA_NGROUPS; g++)
            if (same(line, words[g])) { copy(p->tool[g], v, sizeof p->tool[g]); taken++; break; }
        if (g < DA_NGROUPS) continue;
        if (same(line, "DRAWERS")) { copy(p->drawers, v, sizeof p->drawers); taken++; }
        else if (same(line, "MISSING")) {
            if (same(v, "ON") || same(v, "YES") || same(v, "1")) { p->missing = 1; taken++; }
            else if (same(v, "OFF") || same(v, "NO") || same(v, "0")) { p->missing = 0; taken++; }
        }
    }
    return taken;
}

/* s onto out at *at, never past cap - 1 */
static void put(char *out, size_t cap, size_t *at, const char *s)
{
    while (*s && *at + 1 < cap) out[(*at)++] = *s++;
    out[*at] = 0;
}

int da_prefs_format(const da_prefs *p, char *out, size_t cap)
{
    size_t at = 0;
    int g;
    if (!cap) return 0;
    out[0] = 0;
    for (g = 0; g < DA_NGROUPS; g++) {
        put(out, cap, &at, words[g]); put(out, cap, &at, "="); put(out, cap, &at, p->tool[g]); put(out, cap, &at, "\n");
    }
    put(out, cap, &at, "DRAWERS="); put(out, cap, &at, p->drawers); put(out, cap, &at, "\n");
    put(out, cap, &at, p->missing ? "MISSING=ON\n" : "MISSING=OFF\n");
    return (int)at;
}

int da_change_line(const char *file, const char *old_tool, char *out, size_t cap)
{
    size_t at = 0;
    if (!cap) return 0;
    out[0] = 0;
    if (strchr(file, '\t') || strchr(file, '\n') || (old_tool && (strchr(old_tool, '\t') || strchr(old_tool, '\n'))))
        return 0;                                    /* a name the list can't hold: never listed half */
    if (strlen(file) + (old_tool ? strlen(old_tool) : 0) + 3 > cap) return 0;
    put(out, cap, &at, file); put(out, cap, &at, "\t"); put(out, cap, &at, old_tool ? old_tool : ""); put(out, cap, &at, "\n");
    return (int)at;
}

const char *da_change_parse(char *line)
{
    char *tab, *nl;
    if (!line || !(tab = strchr(line, '\t')) || tab == line) return NULL;
    *tab = 0;
    if ((nl = strchr(tab + 1, '\n'))) *nl = 0;
    if ((nl = strchr(tab + 1, '\r'))) *nl = 0;
    return tab + 1;
}

const char *da_next_drawer(const char *list, char *out, size_t cap)
{
    size_t n = 0;
    if (!cap) return NULL;
    out[0] = 0;
    if (!list) return NULL;
    while (*list == ' ' || *list == '\t' || *list == ',') list++;
    if (!*list) return NULL;
    if (*list == '"') {
        list++;
        while (*list && *list != '"') { if (n + 1 < cap) out[n++] = *list; list++; }
        if (*list == '"') list++;
    } else
        while (*list && *list != ' ' && *list != '\t' && *list != ',') { if (n + 1 < cap) out[n++] = *list; list++; }
    out[n] = 0;
    return list;
}
