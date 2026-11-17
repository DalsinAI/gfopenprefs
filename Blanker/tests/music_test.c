/* GFOpenBlanker's music settings (bl_music.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include "bl_music.h"

static int fails;
#define CHECK(what, ok) do { if (ok) printf("ok   %s\n", what); else { printf("FAIL %s\n", what); fails++; } } while (0)

int main(void)
{
    unsigned i, seen = 0, last = 0, same = 0;
    CHECK("PLAYMIDI left out is off", !bl_truth(NULL));
    CHECK("PLAYMIDI=TRUE is on, in any case", bl_truth("TRUE") && bl_truth("true") && bl_truth("True"));
    CHECK("YES, ON and 1 are on", bl_truth("YES") && bl_truth("on") && bl_truth("1"));
    CHECK("the bare word PLAYMIDI is on", bl_truth("") && bl_truth("  "));
    CHECK("FALSE, NO, OFF, 0 and anything else are off", !bl_truth("FALSE") && !bl_truth("no") && !bl_truth("OFF") && !bl_truth("0") && !bl_truth("TRUEISH") && !bl_truth("TRU"));
    CHECK("a value after blanks counts", bl_truth(" TRUE"));
    CHECK("names that end .mid, .midi, .rmi or .kar are MIDI files", bl_is_midi("a.mid") && bl_is_midi("Tune.MIDI") && bl_is_midi("x.rmi") && bl_is_midi("song.Kar"));
    CHECK("other names are not", !bl_is_midi("a.mod") && !bl_is_midi("mid") && !bl_is_midi(".mid") && !bl_is_midi("a.mid.info") && !bl_is_midi("") && !bl_is_midi("SOURCES.tsv"));
    CHECK("MIDIFILE left out, empty or blank names nothing", !*bl_file_setting(NULL) && !*bl_file_setting("") && !*bl_file_setting("   "));
    CHECK("MIDIFILE's value is taken after its blanks", !strcmp(bl_file_setting(" Media:midi/Joplin/entertainer.mid"), "Media:midi/Joplin/entertainer.mid"));
    CHECK("one file or none: the first", bl_pick(0, 7, 0) == 0 && bl_pick(1, 7, 0) == 0);
    for (i = 0; i < 1000; i++) {
        unsigned p = bl_pick(5, i * 2654435761u, last);
        if (p >= 5) fails++;
        if (i && p == last) same++;
        seen |= 1u << p;
        last = p;
    }
    CHECK("a pick is always one of the files, and every file gets its turn", seen == 31);
    CHECK("never the same file twice running", same == 0);
    puts(fails ? "FAIL" : "PASS");
    return fails != 0;
}
