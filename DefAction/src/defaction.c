/* GFOpenDefAction: icons with no default tool are offered one, by the kind of file they belong to (Dale,
 * 24 November 2026: "we need a scanner to auto offer a tool for them, GFOpenDefAction", "so it sits on
 * icon.library", "with its own preference page"; AC-266, after AC-232's missing datatypes).
 *
 *   GFOpenDefAction                                   the window (GFOpenPrefs' Default Actions page)
 *   GFOpenDefAction SCAN [IN] <drawers...>             the icons with no tool there, and the tool each is offered
 *   GFOpenDefAction SCAN [IN] <drawers...> APPLY       and gives them those tools
 *   GFOpenDefAction UNDO                              puts back every tool GFOpenDefAction gave
 *   GFOpenDefAction KIND <kind> TO <program>|DEFAULT [MISSING ON|OFF] [USE|SAVE]
 *                                                     a kind's program (DEFAULT: its default icon's), and whether
 *                                                     icons naming a program that isn't here are offered one too
 *
 * - An icon is offered a tool when it is a project's (WBPROJECT) and its default tool is empty, or, with MISSING
 *   on (the built-in), names a program found nowhere Workbench looks (GFOpenBench's find_tool: as named, then C:,
 *   SYS:Utilities, SYS:Tools, SYS:System, SYS:Preferences, SYS:Prefs, SYS:Tools/Commodities, SYS:Apps).
 * - Its kind (DefAction/src/da_core.c: text and readmes, AmigaGuide, pictures, movies and animations, sounds,
 *   music, documents) is asked of icon.library first: its identify hook (DefIcons, ICONGETA_IdentifyBuffer with
 *   ICONGETA_IdentifyOnly, icon.library 47), then datatypes.library (the datatype's name, then its group), then
 *   the file's name (Nostalgia.readme, mod.tune).
 * - The tool offered is the kind's program on this page when one is set, else what the kind's default icon names
 *   (icon.library's ICONGETA_GetDefaultName: def_<type>, then def_ascii, def_picture...), else MultiView; only a
 *   program that is here is offered.
 * - Icons are read and written with icon.library only (GetIconTagList, GetDiskObject, PutDiskObject). Never a
 *   silent change: every icon given a tool is listed first, ticked to choose, and written with its old tool to
 *   ENVARC:GFOpenPrefs/DefActionChanges, so UNDO (Undo all) puts every one back.
 * - Settings: ENV:GFOpenPrefs/DefAction (Use) and ENVARC: too (Save), lines as da_core.h says.
 *
 * Built into GFOpenPrefs (LT_OP) it is a page, as GFOpenTypes is; built alone (DefAction/build.sh) it is its own
 * program.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/asl.h>
#include <proto/icon.h>
#include <proto/datatypes.h>
#include <proto/utility.h>

#include <string.h>
#include <stdio.h>
#include "da_core.h"
#ifdef LT_ONE
#include "looktabs.h"                       /* a page of GFOpenPrefs (LT_OP): its frame and sections */
#define LT_REFRESH(w) lt_draw((w), -1)
#else
#define LT_REFRESH(w) ((void)0)
#define LT_DIRTY 1
#define lt_note_to(f) ((void)0)
#endif

static const char version[] __attribute__((used)) = "$VER: GFOpenDefAction 0.1 (24.11.2026) GFOpenPrefs, MIT, Copyright (c) 2026 AmiCradle Team";
#ifndef LT_ONE
unsigned long __stack = 32768;              /* the window and the walk on 32 KB, however it is started (GFOpenTypes' measure) */
#endif

#ifndef ICONGETA_IdentifyBuffer             /* icon.library 47 (NDK 3.2 workbench/icon.h) */
#define ICONGETA_IdentifyBuffer (ICONA_Dummy + 122)
#define ICONGETA_IdentifyOnly (ICONA_Dummy + 123)
#endif

#define PREFS_ENV "ENV:GFOpenPrefs/DefAction"
#define PREFS_ENVARC "ENVARC:GFOpenPrefs/DefAction"
#define CHANGES "ENVARC:GFOpenPrefs/DefActionChanges"
#define FALLBACK "SYS:Utilities/MultiView"
#define MAXFOUND 4000
#define MAXDEPTH 16

struct Library *IconBase, *DataTypesBase, *AslBase, *GadToolsBase;

static da_prefs prefs;

/* ---- what is on this machine ----------------------------------------------------------------- */

static int here(const char *path)
{
    BPTR l = Lock((STRPTR)path, ACCESS_READ);
    if (l) UnLock(l);
    return l != 0;
}

/* The tools named with a volume (SYS:Utilities/MultiView) found or not, asked once a scan: most icons name a few */
#define NSEEN 16
static struct { char tool[DA_TOOL_MAX]; int found; } seen[NSEEN];
static int nseen, seen_next;

/* 1 when a default tool is here, as Workbench finds it (GFOpenBench desktop/openbench.c find_tool): as named when
 * it names a volume; else beside the project (project NULL: not asked), then where Workbench's tools are. Requesters
 * are off for the whole scan, so a volume that isn't there asks nothing. */
static int tool_found_for(const char *tool, const char *project)
{
    static const char *const places[] = { "C:", "SYS:Utilities/", "SYS:Tools/", "SYS:System/", "SYS:Preferences/",
                                          "SYS:Prefs/", "SYS:Tools/Commodities/", "SYS:Apps/", NULL };
    char try_[560];
    int i, f;
    if (da_tool_blank(tool) || strlen(tool) >= DA_TOOL_MAX) return 0;
    if (project && !strchr(tool, ':') && strlen(project) < sizeof try_) {
        strcpy(try_, project);
        *(char *)PathPart((STRPTR)try_) = 0;
        if (AddPart((STRPTR)try_, (STRPTR)tool, sizeof try_) && here(try_)) return 1;
    }
    if (strchr(tool, ':')) {
        for (i = 0; i < nseen; i++)
            if (!strcmp(seen[i].tool, tool)) return seen[i].found;
        f = here(tool);
        i = nseen < NSEEN ? nseen++ : (seen_next++ % NSEEN);
        strcpy(seen[i].tool, tool);
        seen[i].found = f;
        return f;
    }
    if (strchr(tool, '/')) return 0;        /* a path from the project's drawer, not there */
    for (i = 0; places[i]; i++) {
        strcpy(try_, places[i]);
        strcat(try_, tool);
        if (here(try_)) return 1;
    }
    return 0;
}

static int tool_found(const char *tool) { return tool_found_for(tool, NULL); }

/* What a default icon (ENV:Sys/def_<name>.info, through icon.library) names, asked once a name a run */
#define NDEFS 48
static struct { char name[32]; char tool[DA_TOOL_MAX]; } defs[NDEFS];
static int ndefs;

static const char *def_tool(const char *name)
{
    struct DiskObject *dob;
    int i;
    if (!name || !*name || strlen(name) >= sizeof defs[0].name) return "";
    for (i = 0; i < ndefs; i++)
        if (!Stricmp((STRPTR)defs[i].name, (STRPTR)name)) return defs[i].tool;
    if (ndefs >= NDEFS) return "";
    strcpy(defs[ndefs].name, name);
    defs[ndefs].tool[0] = 0;
    if ((dob = GetIconTags(NULL, ICONGETA_GetDefaultName, (ULONG)name, ICONGETA_GetPaletteMappedIcon, FALSE, TAG_DONE))) {
        if (dob->do_Type == WBPROJECT && dob->do_DefaultTool && tool_found((char *)dob->do_DefaultTool)) {
            strncpy(defs[ndefs].tool, (char *)dob->do_DefaultTool, DA_TOOL_MAX - 1);
            defs[ndefs].tool[DA_TOOL_MAX - 1] = 0;
        }
        FreeDiskObject(dob);
    }
    return defs[ndefs++].tool;
}

/* The program a group's file of this type is offered: the page's, else the type's default icon's, else the
 * group's default icon's, else MultiView; only one that is here. "" for none. */
static const char *offer_for(int group, const char *type)
{
    const char *t;
    if (prefs.tool[group][0]) return tool_found(prefs.tool[group]) ? prefs.tool[group] : "";
    if (type && *type && *(t = def_tool(type))) return t;
    if (*(t = def_tool(da_group_defname(group)))) return t;
    return tool_found(FALLBACK) ? FALLBACK : "";
}

/* The kind of a file, asked of icon.library's identify hook, then datatypes.library, then its name; type gets
 * the type's name when one was found (for its default icon) */
static int kind_of(const char *path, char *type, int cap)
{
    int g = DA_NONE;
    type[0] = 0;
    if (IconBase->lib_Version >= 47) {
        char buf[260];
        struct DiskObject *dob;
        buf[0] = 0;
        dob = GetIconTags((STRPTR)path, ICONGETA_IdentifyBuffer, (ULONG)buf, ICONGETA_IdentifyOnly, TRUE,
                          ICONGETA_GetPaletteMappedIcon, FALSE, TAG_DONE);
        if (dob) FreeDiskObject(dob);       /* an icon.library that doesn't know IdentifyOnly hands the icon back */
        buf[sizeof buf - 1] = 0;
        if (buf[0] && (g = da_group_of_type(buf)) != DA_NONE) { strncpy(type, buf, cap - 1); type[cap - 1] = 0; return g; }
    }
    if (DataTypesBase) {
        BPTR l = Lock((STRPTR)path, ACCESS_READ);
        struct DataType *dtn;
        if (l) {
            if ((dtn = ObtainDataTypeA(DTST_FILE, (APTR)l, NULL))) {
                struct DataTypeHeader *dth = dtn->dtn_Header;
                if ((g = da_group_of_type((char *)dth->dth_Name)) != DA_NONE) {
                    strncpy(type, (char *)dth->dth_Name, cap - 1);
                    type[cap - 1] = 0;
                } else
                    g = da_group_of_gid(dth->dth_GroupID);
                ReleaseDataType(dtn);
            }
            UnLock(l);
        }
        if (g != DA_NONE) return g;
    }
    return da_group_of_name(path);
}

/* ---- the icons found --------------------------------------------------------------------------- */

typedef struct found_icon {
    struct Node node;                       /* for the window's list */
    struct found_icon *next;
    int on, group;
    const char *tool;                       /* the tool offered: prefs' or defs' (both outlive the list) */
    char *label;
    char path[1];                           /* the file (its icon is path.info) */
} found_icon;

static found_icon *found, **found_end;
static int nfound, nnone, scanned, stopped;
static struct List found_list;

static void free_found(void)
{
    while (found) {
        found_icon *f = found;
        found = f->next;
        if (f->label) FreeVec(f->label);
        FreeVec(f);
    }
    found_end = &found;
    nfound = nnone = scanned = stopped = 0;
}

static void add_found(const char *path, int group, const char *tool)
{
    found_icon *f;
    if (nfound >= MAXFOUND || !(f = AllocVec(sizeof *f + strlen(path), MEMF_CLEAR))) return;
    strcpy(f->path, path);
    f->group = group;
    f->tool = tool;
    f->on = 1;
    *found_end = f;
    found_end = &f->next;
    nfound++;
}

/* One path for the whole walk, not one a level: the Shell gives a command 4 KB of stack (GFOpenTypes, 5 Oct 2026) */
static char walk_path[512];

static void walk(int depth)
{
    BPTR l;
    struct FileInfoBlock *fib;
    int len = strlen(walk_path);
    if (depth > MAXDEPTH || stopped || !(l = Lock((STRPTR)walk_path, ACCESS_READ))) return;
    if (!(fib = AllocDosObject(DOS_FIB, NULL))) { UnLock(l); return; }
    if (Examine(l, fib) && fib->fib_DirEntryType > 0) {
        while (!stopped && ExNext(l, fib)) {
            int n = strlen((char *)fib->fib_FileName);
            if (SetSignal(0, 0) & SIGBREAKF_CTRL_C) { SetSignal(0, SIGBREAKF_CTRL_C); stopped = 1; break; }
            walk_path[len] = 0;
            if (!AddPart((STRPTR)walk_path, fib->fib_FileName, sizeof walk_path - 8)) continue;
            if (fib->fib_DirEntryType > 0) {
                if (fib->fib_DirEntryType != ST_SOFTLINK) walk(depth + 1);
            } else if (n > 5 && !Stricmp(fib->fib_FileName + n - 5, (STRPTR)".info")) {
                struct DiskObject *dob;
                BPTR fl;
                walk_path[strlen(walk_path) - 5] = 0;                      /* the file the icon belongs to */
                if (!(fl = Lock((STRPTR)walk_path, ACCESS_READ))) continue;  /* an icon with no file */
                UnLock(fl);
                scanned++;
                dob = GetIconTags((STRPTR)walk_path, ICONGETA_FailIfUnavailable, TRUE, ICONGETA_GetPaletteMappedIcon, FALSE, TAG_DONE);
                if (dob) {
                    const char *tool = (const char *)dob->do_DefaultTool;
                    int want = dob->do_Type == WBPROJECT &&
                               da_wants_offer(tool, !da_tool_blank(tool) && tool_found_for(tool, walk_path), prefs.missing);
                    FreeDiskObject(dob);
                    if (want) {
                        char type[64];
                        int g = kind_of(walk_path, type, sizeof type);
                        if (g != DA_NONE) {
                            const char *t = offer_for(g, type);
                            if (*t) add_found(walk_path, g, t); else nnone++;
                        }
                    }
                }
            }
        }
    }
    walk_path[len] = 0;
    FreeDosObject(DOS_FIB, fib);
    UnLock(l);
}

/* Every drawer of the list (prefs.drawers, or the Shell's); requesters off so a volume that isn't there asks nothing */
static void scan(const char *const *drawers)
{
    struct Process *me = (struct Process *)FindTask(NULL);
    APTR was = me->pr_WindowPtr;
    char one[256];
    const char *rest = prefs.drawers;
    free_found();
    ndefs = nseen = seen_next = 0;          /* default icons and tools asked again a scan: they may have changed */
    me->pr_WindowPtr = (APTR)-1;
    for (;;) {
        if (drawers) { if (!*drawers) break; strncpy(one, *drawers++, sizeof one - 1); one[sizeof one - 1] = 0; }
        else if (!(rest = da_next_drawer(rest, one, sizeof one))) break;
        strncpy(walk_path, one, sizeof walk_path - 8);
        walk_path[sizeof walk_path - 8] = 0;
        walk(0);
        if (stopped) break;
    }
    me->pr_WindowPtr = was;
}

/* ---- giving them tools, and taking them back ------------------------------------------------------ */

static int write_text(const char *path, const char *text, LONG mode)
{
    BPTR fh = Open((STRPTR)path, mode), lock;
    LONG len = strlen(text), got;
    if (!fh) {                              /* no GFOpenPrefs drawer there yet */
        char dir[40];
        strncpy(dir, path, sizeof dir - 1);
        dir[sizeof dir - 1] = 0;
        *(char *)PathPart((STRPTR)dir) = 0;
        if ((lock = CreateDir((STRPTR)dir))) UnLock(lock);
        if (!(fh = Open((STRPTR)path, mode))) return 0;
    }
    if (mode == MODE_READWRITE) Seek(fh, 0, OFFSET_END);
    got = Write(fh, (APTR)text, len);
    Close(fh);
    return got == len;
}

/* An icon's default tool set, with icon.library, its old one listed first; 1 done */
static int give_tool(const char *path, const char *tool)
{
    static char line[700];
    struct DiskObject *dob = GetDiskObject((STRPTR)path);
    STRPTR old;
    int ok;
    if (!dob) return 0;
    old = dob->do_DefaultTool;
    if (!da_change_line(path, (const char *)old, line, sizeof line) || !write_text(CHANGES, line, MODE_READWRITE)) {
        FreeDiskObject(dob);                /* never a change that isn't listed */
        return 0;
    }
    dob->do_DefaultTool = (STRPTR)tool;
    ok = PutDiskObject((STRPTR)path, dob);
    dob->do_DefaultTool = old;
    FreeDiskObject(dob);
    return ok;
}

static int apply_found(void)
{
    found_icon *f;
    int done = 0;
    for (f = found; f; f = f->next)
        if (f->on && give_tool(f->path, f->tool)) done++;
    return done;
}

/* Every tool given back, the newest first (an icon given two keeps its first old one), and the list forgotten */
static int undo_all(int *count)
{
    BPTR fh = Open((STRPTR)CHANGES, MODE_OLDFILE);
    struct FileInfoBlock *fib;
    char *buf, *p;
    LONG size = 0;
    int problems = 0;
    *count = 0;
    if (!fh) return 1;
    if ((fib = AllocDosObject(DOS_FIB, NULL))) { if (ExamineFH(fh, fib)) size = fib->fib_Size; FreeDosObject(DOS_FIB, fib); }
    if (size <= 0 || !(buf = AllocVec(size + 2, MEMF_ANY))) { Close(fh); return size <= 0; }
    size = Read(fh, buf, size);
    Close(fh);
    if (size < 0) { FreeVec(buf); return 0; }
    buf[size] = 0;
    for (p = buf + size; p > buf;) {        /* from the last line back */
        char *line, *end = p;
        const char *old;
        struct DiskObject *dob;
        if (end[-1] == '\n') end[-1] = 0;
        for (line = end - 1; line > buf && line[-1] != '\n'; line--) ;
        p = line;
        if (!(old = da_change_parse(line))) continue;
        if ((dob = GetDiskObject((STRPTR)line))) {
            STRPTR now = dob->do_DefaultTool;
            dob->do_DefaultTool = (STRPTR)old;
            if (PutDiskObject((STRPTR)line, dob)) (*count)++; else problems++;
            dob->do_DefaultTool = now;
            FreeDiskObject(dob);
        } else
            problems++;                     /* the icon has gone: nothing to give back */
    }
    FreeVec(buf);
    if (!problems) DeleteFile((STRPTR)CHANGES);
    return !problems;
}

/* ---- the settings ------------------------------------------------------------------------------------ */

static void load_prefs(void)
{
    static char buf[2048];
    BPTR fh = Open((STRPTR)PREFS_ENV, MODE_OLDFILE);
    LONG n;
    da_prefs_default(&prefs);
    if (!fh) fh = Open((STRPTR)PREFS_ENVARC, MODE_OLDFILE);
    if (!fh) return;
    n = Read(fh, buf, sizeof buf - 1);
    Close(fh);
    buf[n > 0 ? n : 0] = 0;
    da_prefs_parse(buf, &prefs);
}

static int save_prefs(int save)
{
    static char buf[2048];
    da_prefs_format(&prefs, buf, sizeof buf);
    if (save && !write_text(PREFS_ENVARC, buf, MODE_NEWFILE)) return 0;
    return write_text(PREFS_ENV, buf, MODE_NEWFILE);
}

/* ---- the Shell ------------------------------------------------------------------------------------------ */

static int group_named(const char *s)
{
    int g;
    for (g = 0; g < DA_NGROUPS; g++)
        if (!Stricmp((STRPTR)s, (STRPTR)da_group_word(g)) || !Stricmp((STRPTR)s, (STRPTR)da_group_label(g))) return g;
    /* the words as one might type them: TEXT, PICTURE, MOVIE, SOUND, DOCUMENT */
    for (g = 0; g < DA_NGROUPS; g++) {
        const char *w = da_group_word(g);
        int n = strlen(w);
        if (w[n - 1] == 'S' && (int)strlen(s) == n - 1 && !Strnicmp((STRPTR)s, (STRPTR)w, n - 1)) return g;
    }
    return DA_NONE;
}

/* SCAN/S,APPLY/S,UNDO/S,KIND/K,TO/K,MISSING/K,USE/S,SAVE/S,IN/M */
static int shell(LONG *a)
{
    found_icon *f;
    if (a[2]) {
        int n;
        if (!undo_all(&n)) {
            Printf((STRPTR)"GFOpenDefAction: %ld tool%s given back; some icons couldn't be, they stay in " CHANGES "\n",
                   (LONG)n, (LONG)(n == 1 ? "" : "s"));
            return RETURN_WARN;
        }
        Printf((STRPTR)"GFOpenDefAction: %ld icon%s as they were\n", (LONG)n, (LONG)(n == 1 ? "" : "s"));
        return RETURN_OK;
    }
    if (a[3]) {
        int g = group_named((const char *)a[3]);
        if (g == DA_NONE) {
            PutStr((STRPTR)"GFOpenDefAction: KIND is TEXT, GUIDE, PICTURES, MOVIES, SOUNDS, MUSIC or DOCUMENTS\n");
            return RETURN_ERROR;
        }
        if (!a[4]) { PutStr((STRPTR)"GFOpenDefAction: KIND needs TO <program> (or TO DEFAULT)\n"); return RETURN_ERROR; }
        if (!Stricmp((STRPTR)a[4], (STRPTR)"DEFAULT")) prefs.tool[g][0] = 0;
        else {
            strncpy(prefs.tool[g], (const char *)a[4], DA_TOOL_MAX - 1);
            prefs.tool[g][DA_TOOL_MAX - 1] = 0;
            if (!tool_found(prefs.tool[g])) Printf((STRPTR)"GFOpenDefAction: note: %s isn't here (yet)\n", a[4]);
        }
    }
    if (a[5]) {
        if (!Stricmp((STRPTR)a[5], (STRPTR)"ON")) prefs.missing = 1;
        else if (!Stricmp((STRPTR)a[5], (STRPTR)"OFF")) prefs.missing = 0;
        else { PutStr((STRPTR)"GFOpenDefAction: MISSING is ON or OFF\n"); return RETURN_ERROR; }
    }
    if (a[6] || a[7]) {
        if (!save_prefs(a[7] != 0)) { PutStr((STRPTR)"GFOpenDefAction: the settings couldn't be written\n"); return RETURN_ERROR; }
        Printf((STRPTR)"GFOpenDefAction: settings %s\n", (LONG)(a[7] ? "saved" : "in use until reboot"));
    }
    if (!a[0] && !a[1]) {
        {
            int g;                          /* the settings as they are now */
            for (g = 0; g < DA_NGROUPS; g++)
                Printf((STRPTR)"%-10s %s\n", (LONG)da_group_word(g), (LONG)(prefs.tool[g][0] ? prefs.tool[g] : "(its default icon's)"));
            Printf((STRPTR)"DRAWERS    %s\nMISSING    %s\n", (LONG)prefs.drawers, (LONG)(prefs.missing ? "ON" : "OFF"));
        }
        return RETURN_OK;
    }
    scan(a[8] ? (const char *const *)a[8] : NULL);
    for (f = found; f; f = f->next)
        Printf((STRPTR)"  %s\t%s\t%s\n", (LONG)f->path, (LONG)da_group_word(f->group), (LONG)f->tool);
    Printf((STRPTR)"GFOpenDefAction: %ld icon%s looked at, %ld to give a tool", (LONG)scanned, (LONG)(scanned == 1 ? "" : "s"), (LONG)nfound);
    if (nnone) Printf((STRPTR)", %ld with no program here for their kind", (LONG)nnone);
    PutStr((STRPTR)(stopped ? " (stopped)\n" : "\n"));
    if (a[1] && nfound) {
        int n = apply_found();
        Printf((STRPTR)"GFOpenDefAction: %ld icon%s given a tool (UNDO puts them back)\n", (LONG)n, (LONG)(n == 1 ? "" : "s"));
        if (n < nfound) { free_found(); return RETURN_WARN; }
    }
    free_found();
    return RETURN_OK;
}

/* ---- the window -------------------------------------------------------------------------------------------- */

enum { G_KINDS = 1, G_PROGRAM, G_CHOOSE, G_DEFAULT, G_NOW, G_DRAWERS, G_SCAN, G_MISSING, G_FOUND, G_STATUS,
       G_GIVE, G_UNDO, G_SAVE, G_USE, G_CANCEL, G_COUNT };

static struct Window *win;
static struct Gadget *gad[G_COUNT];
static struct List kind_list;
static struct Node kind_nodes[DA_NGROUPS];
static char kind_labels[DA_NGROUPS][80], status_text[200], now_text[240];
static int sel;

#define SET(id, ...) GT_SetGadgetAttrs(gad[id], win, NULL, __VA_ARGS__, TAG_DONE)

static void status(const char *s)
{
    strncpy(status_text, s, sizeof status_text - 1);
    status_text[sizeof status_text - 1] = 0;
    if (win && gad[G_STATUS]) SET(G_STATUS, GTTX_Text, (ULONG)status_text);
}

static void new_list(struct List *l)
{
    l->lh_Head = (struct Node *)&l->lh_Tail; l->lh_Tail = NULL; l->lh_TailPred = (struct Node *)&l->lh_Head;
}

static void label_kinds(void)
{
    int g;
    new_list(&kind_list);
    for (g = 0; g < DA_NGROUPS; g++) {
        const char *t = prefs.tool[g][0] ? prefs.tool[g] : def_tool(da_group_defname(g));
        snprintf(kind_labels[g], sizeof kind_labels[g], "%s: %s", da_group_label(g),
                 *t ? (const char *)FilePart((STRPTR)t) : "(nothing here)");
        kind_nodes[g].ln_Name = kind_labels[g];
        AddTail(&kind_list, &kind_nodes[g]);
    }
}

static void relabel_kinds(void)
{
    if (!win) return;
    SET(G_KINDS, GTLV_Labels, ~0UL);
    label_kinds();
    SET(G_KINDS, GTLV_Labels, (ULONG)&kind_list, GTLV_Selected, sel >= 0 ? sel : ~0UL);
}

static void show_now(void)
{
    if (sel < 0) snprintf(now_text, sizeof now_text, "Choose a kind of file, then the program that opens it.");
    else if (prefs.tool[sel][0])
        snprintf(now_text, sizeof now_text, "%s %s", prefs.tool[sel], tool_found(prefs.tool[sel]) ? "" : "(isn't here)");
    else {
        const char *t = def_tool(da_group_defname(sel));
        snprintf(now_text, sizeof now_text, "From its default icon: %s", *t ? t : tool_found(FALLBACK) ? FALLBACK : "nothing here");
    }
    if (win) SET(G_NOW, GTTX_Text, (ULONG)now_text);
}

static void label_found(void)
{
    found_icon *f;
    new_list(&found_list);
    for (f = found; f; f = f->next) {
        int n = strlen(f->path) + strlen(f->tool) + 12;
        if (!f->label && !(f->label = AllocVec(n, MEMF_ANY))) continue;
        snprintf(f->label, n, "[%c] %s > %s", f->on ? 'x' : ' ', f->path, (const char *)FilePart((STRPTR)f->tool));
        f->node.ln_Name = f->label;
        AddTail(&found_list, &f->node);
    }
}

static void show_found(int at)
{
    if (!win) return;
    SET(G_FOUND, GTLV_Labels, ~0UL);
    label_found();
    SET(G_FOUND, GTLV_Labels, (ULONG)&found_list, GTLV_MakeVisible, at);
    SET(G_GIVE, GA_Disabled, nfound == 0);
}

static void program_from_string(void)
{
    if (sel < 0) return;
    strncpy(prefs.tool[sel], (char *)((struct StringInfo *)gad[G_PROGRAM]->SpecialInfo)->Buffer, DA_TOOL_MAX - 1);
    prefs.tool[sel][DA_TOOL_MAX - 1] = 0;
    relabel_kinds();
    show_now();
}

static int gui(void)
{
    struct Screen *scr = LockPubScreen(NULL);
    APTR vi = scr ? GetVisualInfoA(scr, NULL) : NULL;
    struct Gadget *glist = NULL, *g;
    struct NewGadget ng;
    int fh, lh, top, row, quit = 0, rc = RETURN_OK, W = 620, i, lw, rx, rw;
    if (!vi) { if (scr) UnlockPubScreen(NULL, scr); return RETURN_FAIL; }
    memset(gad, 0, sizeof gad);
    sel = -1;
    fh = scr->Font->ta_YSize;
    lh = fh + 6;
    top = scr->WBorTop + fh + 1 + 8;
    lw = 250;                               /* the kinds' list */
    rx = 10 + lw + 10 + TextLength(&scr->RastPort, (STRPTR)"Opens with", 10) + 8;
    rw = W - 10 - rx;
    ndefs = 0;
    label_kinds();
    new_list(&found_list);
    strcpy(status_text, "Scan finds icons with no program, and offers each the one for its kind.");
    show_now();

    g = CreateContext(&glist);
    memset(&ng, 0, sizeof ng);
    ng.ng_TextAttr = scr->Font;
    ng.ng_VisualInfo = vi;
#define G(kind, id, x, y, w, h, text, flags, ...) \
    (ng.ng_LeftEdge = (x), ng.ng_TopEdge = (y), ng.ng_Width = (w), ng.ng_Height = (h), ng.ng_GadgetText = (STRPTR)(text), \
     ng.ng_Flags = (flags), ng.ng_GadgetID = (id), g = gad[id] = CreateGadget(kind, g, &ng, __VA_ARGS__, TAG_DONE))
#ifdef LT_ONE
    lt_make(NULL, NULL, 10, top, W - 20, lh, LT_DEFACTION);   /* the frame's sections (GFOpenPrefs) */
    top += lt_space(scr->Font, W - 20, lh);
#endif
    row = top + fh + 4;
    G(LISTVIEW_KIND, G_KINDS, 10, row, lw, 7 * (fh + 1) + 4, "Kinds of file", PLACETEXT_ABOVE, GTLV_Labels, (ULONG)&kind_list,
      GTLV_ShowSelected, NULL);
    G(STRING_KIND, G_PROGRAM, rx, row, rw - 90, lh, "Opens with", PLACETEXT_LEFT, GTST_MaxChars, DA_TOOL_MAX - 1, GA_Disabled, TRUE);
    G(BUTTON_KIND, G_CHOOSE, W - 10 - 86, row, 86, lh, "Choose...", 0, GA_Disabled, TRUE);
    G(BUTTON_KIND, G_DEFAULT, rx, row + lh + 4, rw, lh, "Use its default icon's program", 0, GA_Disabled, TRUE);
    G(TEXT_KIND, G_NOW, 10 + lw + 10, row + 2 * (lh + 4), W - 20 - lw - 10, lh, NULL, 0, GTTX_Text, (ULONG)now_text);
    G(CHECKBOX_KIND, G_MISSING, 10 + lw + 10, row + 3 * (lh + 4) + 2, 26, fh + 3, "Also icons naming a program that isn't here",
      PLACETEXT_RIGHT, GTCB_Checked, prefs.missing);
    row += 7 * (fh + 1) + 4 + 8;
    i = TextLength(&scr->RastPort, (STRPTR)"Look in", 7) + 18;
    G(STRING_KIND, G_DRAWERS, 10 + i, row, W - 20 - i - 96, lh, "Look in", PLACETEXT_LEFT, GTST_String, (ULONG)prefs.drawers,
      GTST_MaxChars, DA_DRAWERS_MAX - 1);
    G(BUTTON_KIND, G_SCAN, W - 10 - 86, row, 86, lh, "Scan", 0, TAG_IGNORE, 0);
    row += lh + 4;
    G(LISTVIEW_KIND, G_FOUND, 10, row, W - 20, 8 * (fh + 1) + 4, NULL, 0, GTLV_Labels, (ULONG)&found_list);
    row += 8 * (fh + 1) + 4 + 6;
    G(TEXT_KIND, G_STATUS, 10, row, W - 20, lh, NULL, 0, GTTX_Text, (ULONG)status_text, GTTX_Border, TRUE);
    row += lh + 6;
    {
        static const char *const names[5] = { "Give programs", "Undo all", "Save", "Use", "Cancel" };
        static const int ids[5] = { G_GIVE, G_UNDO, G_SAVE, G_USE, G_CANCEL };
        int bw = (W - 20 - 4 * 8) / 5;
        for (i = 0; i < 5; i++) G(BUTTON_KIND, ids[i], 10 + i * (bw + 8), row, bw, lh, names[i], 0, GA_Disabled, ids[i] == G_GIVE);
        row += lh + 8;
    }
    if (!g) { rc = RETURN_FAIL; goto out; }
#ifdef LT_ONE
    win = lt_open(scr, 30, scr->BarHeight + 10, W - scr->WBorLeft - scr->WBorRight, row - (scr->WBorTop + fh + 1) - scr->WBorBottom,
                  glist, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | LISTVIEWIDCMP | BUTTONIDCMP | CHECKBOXIDCMP | STRINGIDCMP | TEXTIDCMP,
                  "GFOpenDefAction 0.1");
#else
    win = OpenWindowTags(NULL, WA_Title, (ULONG)"GFOpenDefAction: a program for icons that have none", WA_PubScreen, (ULONG)scr,
                         WA_Left, 30, WA_Top, scr->BarHeight + 10, WA_Width, W, WA_Height, row,
                         WA_Gadgets, (ULONG)glist, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_Activate, TRUE,
                         WA_SmartRefresh, TRUE,
                         WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | LISTVIEWIDCMP | BUTTONIDCMP | CHECKBOXIDCMP | STRINGIDCMP | TEXTIDCMP,
                         TAG_DONE);
#endif
    if (!win) { rc = RETURN_FAIL; goto out; }
    GT_RefreshWindow(win, NULL);
    lt_note_to(status);

    while (!quit) {
        struct IntuiMessage *m;
        WaitPort(win->UserPort);
        while (!quit && (m = GT_GetIMsg(win->UserPort))) {
            ULONG cls = m->Class;
            UWORD code = m->Code;
            struct Gadget *gg = (struct Gadget *)m->IAddress;
            GT_ReplyIMsg(m);
            if (cls == IDCMP_CLOSEWINDOW) quit = 1;
            else if (cls == IDCMP_REFRESHWINDOW) { GT_BeginRefresh(win); LT_REFRESH(win); GT_EndRefresh(win, TRUE); }
            else if (cls == IDCMP_GADGETUP) switch (gg->GadgetID) {
            case G_KINDS:
                if (code < DA_NGROUPS) {
                    sel = code;
                    SET(G_PROGRAM, GTST_String, (ULONG)prefs.tool[sel], GA_Disabled, FALSE);
                    SET(G_CHOOSE, GA_Disabled, FALSE);
                    SET(G_DEFAULT, GA_Disabled, FALSE);
                    show_now();
                }
                break;
            case G_PROGRAM: program_from_string(); break;
            case G_CHOOSE:
                if (sel >= 0 && (AslBase || (AslBase = OpenLibrary((STRPTR)"asl.library", 37)))) {
                    struct FileRequester *fr = AllocAslRequestTags(ASL_FileRequest, ASLFR_Window, (ULONG)win,
                                                                   ASLFR_TitleText, (ULONG)"The program that opens these files",
                                                                   ASLFR_InitialDrawer, (ULONG)"SYS:Utilities", TAG_DONE);
                    if (fr && AslRequest(fr, NULL)) {
                        char pick[DA_TOOL_MAX];
                        strncpy(pick, (char *)fr->fr_Drawer, sizeof pick - 1);
                        pick[sizeof pick - 1] = 0;
                        AddPart((STRPTR)pick, fr->fr_File, sizeof pick);
                        strcpy(prefs.tool[sel], pick);
                        SET(G_PROGRAM, GTST_String, (ULONG)prefs.tool[sel]);
                        relabel_kinds();
                        show_now();
                    }
                    if (fr) FreeAslRequest(fr);
                }
                break;
            case G_DEFAULT:
                if (sel >= 0) {
                    prefs.tool[sel][0] = 0;
                    SET(G_PROGRAM, GTST_String, (ULONG)"");
                    relabel_kinds();
                    show_now();
                }
                break;
            case G_MISSING: prefs.missing = (gg->Flags & GFLG_SELECTED) != 0; break;
            case G_DRAWERS:
                strncpy(prefs.drawers, (char *)((struct StringInfo *)gad[G_DRAWERS]->SpecialInfo)->Buffer, DA_DRAWERS_MAX - 1);
                prefs.drawers[DA_DRAWERS_MAX - 1] = 0;
                break;
            case G_SCAN: {
                char line[200];
                strncpy(prefs.drawers, (char *)((struct StringInfo *)gad[G_DRAWERS]->SpecialInfo)->Buffer, DA_DRAWERS_MAX - 1);
                prefs.drawers[DA_DRAWERS_MAX - 1] = 0;
                status("Scanning... (Ctrl-C stops it)");
                SetWindowPointer(win, WA_BusyPointer, TRUE, TAG_DONE);
                SET(G_FOUND, GTLV_Labels, ~0UL);
                scan(NULL);
                show_found(0);
                SetWindowPointer(win, TAG_DONE);
                snprintf(line, sizeof line, nfound ? "%d of %d icons have no program: [x] ones get the one shown%s%s"
                                                   : "%d of %d icons have no program for their kind%s%s",
                         nfound, scanned, nnone ? "; some kinds have nothing here" : "", stopped ? " (stopped)" : "");
                status(line);
                break;
            }
            case G_FOUND: {
                found_icon *f = found;
                for (i = 0; f && i < code; i++) f = f->next;
                if (f) { f->on = !f->on; show_found(code); }
                break;
            }
            case G_GIVE: {
                char line[120];
                int n;
                SetWindowPointer(win, WA_BusyPointer, TRUE, TAG_DONE);
                n = apply_found();
                SET(G_FOUND, GTLV_Labels, ~0UL);
                free_found();
                show_found(0);
                SetWindowPointer(win, TAG_DONE);
                snprintf(line, sizeof line, "%d icon%s given a program. Undo all puts them back.", n, n == 1 ? "" : "s");
                status(line);
                break;
            }
            case G_UNDO: {
                char line[120];
                int n, ok;
                SetWindowPointer(win, WA_BusyPointer, TRUE, TAG_DONE);
                ok = undo_all(&n);
                SetWindowPointer(win, TAG_DONE);
                if (!ok) snprintf(line, sizeof line, "%d given back; some icons couldn't be (the list is kept).", n);
                else snprintf(line, sizeof line, n ? "Every icon GFOpenDefAction gave a program is back as it was." : "GFOpenDefAction hasn't changed any icons.");
                status(line);
                break;
            }
            case G_SAVE:
            case G_USE:
                if (sel >= 0) program_from_string();
                strncpy(prefs.drawers, (char *)((struct StringInfo *)gad[G_DRAWERS]->SpecialInfo)->Buffer, DA_DRAWERS_MAX - 1);
                prefs.drawers[DA_DRAWERS_MAX - 1] = 0;
                if (save_prefs(gg->GadgetID == G_SAVE)) quit = 1;
                else status("The settings couldn't be written.");
                break;
            case G_CANCEL: quit = 1; break;
#ifdef LT_ONE
            default: {                      /* another section: the settings changed kept, as Use, when there are any */
                int to = lt_hit(gg, code);
                if (to < 0) break;
                if (LT_DIRTY) save_prefs(0);
                lt_go(win, to);
                quit = 1;
                break;
            }
#endif
            }
        }
    }
out:
#ifdef LT_ONE
    if (win) { lt_close(win, glist); win = NULL; }
#else
    if (win) { CloseWindow(win); win = NULL; }
#endif
    FreeGadgets(glist);
    FreeVisualInfo(vi);
    UnlockPubScreen(NULL, scr);
    free_found();
    return rc;
}

int main(int argc, char **argv)
{
    LONG a[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    struct RDArgs *rd = NULL;
    int rc = RETURN_FAIL;
    (void)argv;
    (void)version;
    win = NULL;                             /* its statics outlive a run inside GFOpenPrefs */
    found = NULL;
    found_end = &found;
    nfound = nnone = scanned = stopped = 0;
    ndefs = 0;
    IconBase = OpenLibrary((STRPTR)"icon.library", 44);
    DataTypesBase = OpenLibrary((STRPTR)"datatypes.library", 39);   /* not needed: icon.library and the names do without */
    GadToolsBase = OpenLibrary((STRPTR)"gadtools.library", 39);
    AslBase = NULL;                         /* opened when Choose... is first pressed */
    if (!IconBase || !GadToolsBase) {
        if (argc > 0) PutStr((STRPTR)"GFOpenDefAction needs icon.library 44 (AmigaOS 3.5 or later) and gadtools.library 39\n");
        goto out;
    }
    load_prefs();
    /* from Workbench (argc 0) there are no arguments: ReadArgs would wait on an empty console */
    if (argc > 0 && !(rd = ReadArgs((STRPTR)"SCAN/S,APPLY/S,UNDO/S,KIND/K,TO/K,MISSING/K,USE/S,SAVE/S,IN/M", a, NULL))) {
        PrintFault(IoErr(), (STRPTR)"GFOpenDefAction");
        goto out;
    }
    if (rd && (a[0] || a[1] || a[2] || a[3] || a[5] || a[6] || a[7])) rc = shell(a);
    else rc = gui();                        /* no command: the window, from the Shell as from Workbench */
out:
    if (rd) FreeArgs(rd);
    free_found();
    if (AslBase) CloseLibrary(AslBase);
    if (GadToolsBase) CloseLibrary(GadToolsBase);
    if (DataTypesBase) CloseLibrary(DataTypesBase);
    if (IconBase) CloseLibrary(IconBase);
    AslBase = GadToolsBase = DataTypesBase = IconBase = NULL;
    return rc;
}
