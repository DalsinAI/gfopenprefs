/* GFOpenPrefs' About window (Dale, 14 November 2026, 15:20: "GFOpenPrefs About row under the sections list: go"):
 * a button under the sections list (or beside the Section cycle gadget on a narrow screen) opens GFOpenLayout's
 * olabout with the GFOS mark, the name and version, these lines, the Team's credit and OK. Plain C
 * (OpenPrefs/tests/about_test.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OP_ABOUT_H
#define OP_ABOUT_H

#include "openprefs.h"

#define OP_ABOUT_NAME "GFOpenPrefs"
#define OP_ABOUT_VERSION OP_VERSION
#define OP_ABOUT_TEXT "Every preferences editor in one window.\nPart of GFOS for AmigaOS 3.2.\n" \
    "MIT, Copyright (c) 2026 AmiCradle Team."
#define OP_ABOUT_LABEL "About"

/* Where the About button stands under the list: its top, a gap below the list's recessed box (2 pixels round
 * the list), and the inner height the window needs for it (inner_top: where the window's inside starts). */
#define OP_ABOUT_GAP 6
static inline int op_about_y(int list_top, int list_h) { return list_top + list_h + 2 + OP_ABOUT_GAP; }
static inline int op_about_min_h(int list_top, int list_h, int row_h, int inner_top)
{
    return op_about_y(list_top, list_h) + row_h + 2 + 6 - inner_top;
}

#endif
