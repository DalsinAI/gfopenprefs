/* GFOpenBlanker's music (1.6): what its PLAYMIDI and MIDIFILE settings mean.
 * Plain C with no Amiga calls, so the host tests read it too.
 *
 *   PLAYMIDI=TRUE   the show plays music (TRUE, YES, ON or 1, or the bare word)
 *   MIDIFILE=name   the file to play, or a drawer: a MIDI file is picked from
 *                   it afresh each show. Left out or empty: Media:midi/Dance
 *                   when that drawer is there, else Media:midi.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef BL_MUSIC_H
#define BL_MUSIC_H

#define BL_MUSIC_FIRST  "Media:midi/Dance"
#define BL_MUSIC_SECOND "Media:midi"

static int bl_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

static int bl_same(const char *a, const char *b)
{
    while (*a && bl_lower((unsigned char)*a) == bl_lower((unsigned char)*b)) { a++; b++; }
    return !*a && !*b;
}

/* a switch's value: NULL is off (the setting isn't there); the bare word is on */
static int bl_truth(const char *v)
{
    if (!v) return 0;
    while (*v == ' ') v++;
    return !*v || bl_same(v, "TRUE") || bl_same(v, "YES") || bl_same(v, "ON") || bl_same(v, "1");
}

/* a file the synthesiser plays, by its name's ending */
static int bl_is_midi(const char *name)
{
    static const char *const ends[] = { ".mid", ".midi", ".rmi", ".kar" };
    unsigned i, n = 0;
    while (name[n]) n++;
    for (i = 0; i < sizeof ends / sizeof ends[0]; i++) {
        unsigned e = 0;
        while (ends[i][e]) e++;
        if (n > e && bl_same(name + n - e, ends[i])) return 1;
    }
    return 0;
}

/* which of count files this show plays: a different one from last time when there is more than one */
static unsigned bl_pick(unsigned count, unsigned seed, unsigned last)
{
    unsigned p;
    if (count < 2) return 0;
    p = seed % count;
    return p == last % count ? (p + 1) % count : p;
}

/* MIDIFILE's value, or "" when it names nothing (left out, empty, blanks) */
static const char *bl_file_setting(const char *v)
{
    if (!v) return "";
    while (*v == ' ') v++;
    return v;
}

#endif
