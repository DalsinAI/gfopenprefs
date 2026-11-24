/* da_test: GFOpenDefAction's rules (DefAction/src/da_core.c): which group a file is by its type, its datatype's
 * group and its name, which icons are offered a tool, the prefs file and the list of changes.
 *   cc -std=c99 -Wall -Werror -IDefAction/src -o da_test DefAction/tests/da_test.c DefAction/src/da_core.c
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "da_core.h"

static int checks, fails;
#define CHECK(what, c) do { checks++; if (!(c)) { fails++; printf("FAIL %s\n", what); } } while (0)

int main(void)
{
    da_prefs p, q;
    char out[2048], tiny[16], line[300], d[64];
    const char *rest;
    int g;

    /* types as DefIcons (icon.library's identify hook) and datatypes name them, either case */
    CHECK("ascii is text", da_group_of_type("ascii") == DA_TEXT);
    CHECK("ILBM is a picture", da_group_of_type("ILBM") == DA_PICTURE);
    CHECK("gfopenpng is a picture", da_group_of_type("gfopenpng") == DA_PICTURE);
    CHECK("AmigaGuide is a guide", da_group_of_type("AmigaGuide") == DA_GUIDE);
    CHECK("anim is a movie", da_group_of_type("anim") == DA_MOVIE);
    CHECK("mpeg is a movie", da_group_of_type("mpeg") == DA_MOVIE);
    CHECK("8SVX is a sound", da_group_of_type("8SVX") == DA_SOUND);
    CHECK("gfopenmodule is music", da_group_of_type("gfopenmodule") == DA_MUSIC);
    CHECK("gfopendoc is a document, not text", da_group_of_type("gfopendoc") == DA_DOCUMENT);
    CHECK("pdf is a document", da_group_of_type("pdf") == DA_DOCUMENT);
    CHECK("a part of a word is not the word", da_group_of_type("asc") == DA_NONE && da_group_of_type("pictures") == DA_NONE);
    CHECK("rexx, lha and none are ours to leave", da_group_of_type("rexx") == DA_NONE && da_group_of_type("lha") == DA_NONE
          && da_group_of_type("") == DA_NONE && da_group_of_type(NULL) == DA_NONE);

    /* datatypes' groups */
    CHECK("GID_TEXT", da_group_of_gid(DA_GID_TEXT) == DA_TEXT);
    CHECK("GID_PICTURE", da_group_of_gid(DA_GID_PICTURE) == DA_PICTURE);
    CHECK("GID_ANIMATION and GID_MOVIE", da_group_of_gid(DA_GID_ANIMATION) == DA_MOVIE && da_group_of_gid(DA_GID_MOVIE) == DA_MOVIE);
    CHECK("GID_SOUND and GID_INSTRUMENT", da_group_of_gid(DA_GID_SOUND) == DA_SOUND && da_group_of_gid(DA_GID_INSTRUMENT) == DA_SOUND);
    CHECK("GID_MUSIC", da_group_of_gid(DA_GID_MUSIC) == DA_MUSIC);
    CHECK("GID_DOCUMENT", da_group_of_gid(DA_GID_DOCUMENT) == DA_DOCUMENT);
    CHECK("GID_SYSTEM is left", da_group_of_gid(DA_GID_SYSTEM) == DA_NONE && da_group_of_gid(0) == DA_NONE);

    /* names */
    CHECK("Nostalgia.readme", da_group_of_name("Work:Docs/Nostalgia.readme") == DA_TEXT);
    CHECK("ReadMe alone", da_group_of_name("DH1:Game/ReadMe") == DA_TEXT && da_group_of_name("READ_ME") == DA_TEXT);
    CHECK("pic.IFF", da_group_of_name("pic.IFF") == DA_PICTURE);
    CHECK("intro.anim", da_group_of_name("intro.anim") == DA_MOVIE);
    CHECK("film.mp4", da_group_of_name("RAM:film.mp4") == DA_MOVIE);
    CHECK("bang.8svx and hit.wav", da_group_of_name("bang.8svx") == DA_SOUND && da_group_of_name("hit.wav") == DA_SOUND);
    CHECK("mod.tune and tune.mod", da_group_of_name("Music:mod.space_debris") == DA_MUSIC && da_group_of_name("tune.mod") == DA_MUSIC);
    CHECK("Help.guide", da_group_of_name("Help.guide") == DA_GUIDE);
    CHECK("manual.pdf", da_group_of_name("manual.pdf") == DA_DOCUMENT);
    CHECK("programs and archives are left", da_group_of_name("Game.lha") == DA_NONE && da_group_of_name("Install") == DA_NONE
          && da_group_of_name(".hidden") == DA_NONE && da_group_of_name("mod.") == DA_NONE && da_group_of_name("x.") == DA_NONE);
    CHECK("a drawer's dots aren't the file's", da_group_of_name("Work:v1.mod/Install") == DA_NONE);

    /* which icons are offered one */
    CHECK("no tool", da_tool_blank(NULL) && da_tool_blank("") && da_tool_blank("  "));
    CHECK("a tool", !da_tool_blank("MultiView"));
    CHECK("an empty tool is offered one", da_wants_offer("", 0, 0) && da_wants_offer(NULL, 1, 1));
    CHECK("a tool not here, missing on", da_wants_offer("SYS:Utilities/Visage", 0, 1));
    CHECK("a tool not here, missing off", !da_wants_offer("SYS:Utilities/Visage", 0, 0));
    CHECK("a tool that is here stays", !da_wants_offer("SYS:Utilities/MultiView", 1, 1));

    /* the prefs */
    da_prefs_default(&p);
    CHECK("built in: from the default icons, SYS: and Work:, missing on", !p.tool[DA_TEXT][0] && !strcmp(p.drawers, "SYS: Work:") && p.missing);
    CHECK("four of six lines taken", da_prefs_parse("pictures=C:GFOpenView\nTEXT = SYS:Tools/TextEdit\r\nDrawers=DH1: \"Work:My Files\"\n"
                                                "MISSING=off\nFOO=bar\nno equals\n", &p) == 4);
    CHECK("pictures", !strcmp(p.tool[DA_PICTURE], "C:GFOpenView"));
    CHECK("text, spaces round the = gone", !strcmp(p.tool[DA_TEXT], "SYS:Tools/TextEdit"));
    CHECK("drawers kept with quotes", !strcmp(p.drawers, "DH1: \"Work:My Files\""));
    CHECK("missing off", p.missing == 0);
    da_prefs_format(&p, out, sizeof out);
    da_prefs_default(&q);
    da_prefs_parse(out, &q);
    CHECK("the file read back is the same", !memcmp(&p, &q, sizeof p));
    CHECK("the file names every group", strstr(out, "MOVIES=\n") && strstr(out, "PICTURES=C:GFOpenView\n") && strstr(out, "MISSING=OFF\n"));
    CHECK("cut to its buffer, NUL-ended", da_prefs_format(&p, tiny, sizeof tiny) == (int)sizeof tiny - 1 && strlen(tiny) == sizeof tiny - 1);
    for (g = 0; g < DA_NGROUPS; g++) CHECK("each group has a label, a word and a default icon",
                                           *da_group_label(g) && *da_group_word(g) && *da_group_defname(g));
    CHECK("no group, no names", !*da_group_label(DA_NONE) && !*da_group_word(DA_NGROUPS));

    /* the list of changes */
    CHECK("a line", da_change_line("Work:a.readme", "", line, sizeof line) > 0 && !strcmp(line, "Work:a.readme\t\n"));
    CHECK("read back: no old tool", !strcmp(da_change_parse(line), "") && !strcmp(line, "Work:a.readme"));
    da_change_line("DH1:p.iff", "SYS:Utilities/Visage", line, sizeof line);
    CHECK("read back: the old tool", !strcmp(da_change_parse(line), "SYS:Utilities/Visage") && !strcmp(line, "DH1:p.iff"));
    CHECK("a name with a tab is never listed", da_change_line("a\tb", "", line, sizeof line) == 0 && !line[0]);
    CHECK("too long for the line is never listed half", da_change_line("Work:long-name.readme", "x", tiny, sizeof tiny) == 0);
    {
        char bad[] = "no tab here";
        CHECK("a line without a tab", da_change_parse(bad) == NULL);
    }

    /* the drawers */
    rest = da_next_drawer("SYS:  \"Work:My Files\",DH1:", d, sizeof d);
    CHECK("first SYS:", rest && !strcmp(d, "SYS:"));
    rest = da_next_drawer(rest, d, sizeof d);
    CHECK("then the quoted one", rest && !strcmp(d, "Work:My Files"));
    rest = da_next_drawer(rest, d, sizeof d);
    CHECK("then DH1:", rest && !strcmp(d, "DH1:"));
    CHECK("then no more", da_next_drawer(rest, d, sizeof d) == NULL && da_next_drawer("   ", d, sizeof d) == NULL);

    printf("%d/%d GFOpenDefAction checks pass\n", checks - fails, checks);
    return fails != 0;
}
