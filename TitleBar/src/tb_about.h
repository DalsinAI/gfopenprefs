/* GFOpenTitle's About window: a click on the GFOS mark at the bar's left opens GFOpenLayout's olabout (the mark, the
 * name and version, these lines, the Team's credit, OK), without stopping the clock and the tray: olabout_open and
 * olabout_handle in GFOpenTitle's own loop (Team 5, 14 November 2026: About windows with the GFOS mark). Plain C
 * (tests/test_tb.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef TB_ABOUT_H
#define TB_ABOUT_H

#define TB_ABOUT_NAME "GFOpenTitle"
#define TB_ABOUT_VERSION "0.11.2"
#define TB_ABOUT_TEXT "The title bar and the system tray for AmigaOS 3.2.\nPart of GFOpenPrefs.\n" \
    "MIT, Copyright (c) 2026 AmiCradle Team."

/* what a click on the mark does: open the About window, or close it when it is open (1: open it, 0: close it) */
static inline int tb_about_click(int open_now) { return !open_now; }

#endif
