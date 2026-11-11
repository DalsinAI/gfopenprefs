/* The tray's grid at the menu bar's right end (Dale, 11 November 2026: the
 * speaker and the depth gadget "appear stuck on"; "a wee bit more space").
 * Every tray icon (GFOpenTitle's network icons, cog and the taskspace's
 * icons, and GFOpenSpeaker's speaker) sits in a box TRAY_ICON wide, with
 * TRAY_PAD on each side, so each icon is TRAY_CELL wide and the gap between
 * two glyphs is 2 * TRAY_PAD wherever they come from. A program's own
 * picture wider than TRAY_ICON keeps its width and the same pads.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef GFOPENPREFS_TRAYGRID_H
#define GFOPENPREFS_TRAYGRID_H

#define TRAY_ICON 16                                /* an icon's box: the network icons' width */
#define TRAY_PAD  7                                 /* each side of it (6 before 11 November 2026) */
#define TRAY_CELL (TRAY_ICON + 2 * TRAY_PAD)        /* one icon's room on the bar */

/* the room for a picture w wide: at least the icon's box */
#define TRAY_ROOM(w) (((w) > TRAY_ICON ? (w) : TRAY_ICON) + 2 * TRAY_PAD)

#endif
