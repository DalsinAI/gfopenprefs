/* GFOpenWindows' centring and snapshot gadget defaults, shared by the commodity (openwindows.c) and its editor
 * (windows.c), plain C so the host tests read the same rule.
 *
 * Dale, 4 November 2026: windows open centred until snapped and the snapshot gadget shows, by default. GFOpenWindows
 * 0.10 and 0.11 (GFOpenUp 0.8.14) had both off, and their editor (Windows 0.7) wrote "centre off" on every Save or
 * Use whether or not the user had touched it, so a "centre" or "snapshot.gadget" line from then says nothing of a
 * choice. From 0.12 the editor writes "centre.v 2" beside them, and only lines with that mark are a choice: without
 * it centring is on and the gadget follows it (on), for new installs and for upgrades alike.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef CENTRE_PREFS_H
#define CENTRE_PREFS_H

#define CENTRE_PREFS_V 2      /* "centre.v 2": the centre and snapshot.gadget lines beside it were chosen */

/* ver: the centre.v line's number (0 with none); centre_line, gadget_line: 1 on, 0 off, -1 no line.
 * *centre: 1 or 0; *gadget: 1, 0, or -1 (follows centring). */
static inline void centre_prefs_resolve(int ver, int centre_line, int gadget_line, int *centre, int *gadget)
{
    int chosen = ver >= CENTRE_PREFS_V;
    *centre = chosen && centre_line >= 0 ? centre_line : 1;
    *gadget = chosen && gadget_line >= 0 ? gadget_line : -1;
}

#endif
