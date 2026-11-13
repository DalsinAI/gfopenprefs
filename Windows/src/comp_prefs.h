/* comp_prefs.h: layers.library's composited-mode lines in ENV:GFOpenPrefs/Windows that the Windows editor
 * reads and writes, and its words for why a screen isn't composited. Portable C, no Amiga headers:
 * tests/test_comp_prefs.c.
 *
 * screen.mode (row 11, Dale 13 Nov 05:57): "direct", screen direct mode, is the default until the compositor
 * is within about 5% of it on Franklin's bench; "composited" asks for it. The editor has no gadget for it and
 * keeps it as written. layers.library 47.109 answers LAYERS_WHY_DIRECT (9) for a screen it leaves direct.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef COMP_PREFS_H
#define COMP_PREFS_H
#include <string.h>

/* 1 for "composited", 0 for anything else ("direct") */
static inline int comp_prefs_mode(const char *v) { return !strncmp(v, "composited", 10); }
static inline const char *comp_prefs_mode_name(int composited) { return composited ? "composited" : "direct"; }

/* LAYERS_COMPOSE_WHY's answers 1-9 (gfopengfx include/graphics/gfopenlayers.h) as the switch's words;
 * 0 for 0 (composited) or one this editor doesn't know */
static inline const char *comp_prefs_why(unsigned long w)
{
    static const char why[] = "not asked\0switched off\0not RTG\0" "8-bit\0pixel layout\0no GPU driver\0no GFOpenGPU\0"
                              "no memory\0screen direct mode";
    const char *p = why;
    if (w < 1 || w > 9) return 0;
    while (--w) p += strlen(p) + 1;
    return p;
}
#endif
