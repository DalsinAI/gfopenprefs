/* Anti-aliased text for the bar and its panels: GFOpenLayout 19's OLUI_AmigaText, drawn from the screen font's
 * outline face (sans-serif when it has none) at the font's height, its grey levels blended onto what the RastPort
 * holds (on a 16 to 256-pen screen from the screen's own pens, none taken). Where it can't (gfopenlayout.library
 * older than 19 or missing, fewer than 16 pens, no gfopenfont.library, text that isn't plain ASCII) the same box
 * is drawn with Text() as before, so nothing moves.
 *   aat_open()/aat_close()            once, from the program's process
 *   aat_length(rp, s, n)              the width either way draws it
 *   aat_box(rp, scr, x, top, h, s, n) s at x, centred in the rows top..top+h-1, in rp's APen's colour
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef GFOPENPREFS_AATEXT_H
#define GFOPENPREFS_AATEXT_H

/* ---- the parts the host tests (Common/tests/aatext_test.c) ---- */

/* 1 when the n bytes are plain ASCII: what the bar has is Latin-1 and OLUI_AmigaText takes UTF-8 */
static inline int aat_ascii(const char *s, unsigned long n)
{
    while (n--)
        if ((unsigned char)*s++ & 0x80) return 0;
    return 1;
}

/* GetRGB32's three 32-bit guns as 0xRRGGBB */
static inline unsigned long aat_argb(const unsigned long rgb[3])
{
    return ((rgb[0] >> 24) << 16) | ((rgb[1] >> 24) << 8) | (rgb[2] >> 24);
}

/* the baseline that centres ascent + descent in h rows from top (as the bar centres Text()'s TxHeight) */
static inline int aat_baseline(int top, int h, int ascent, int descent)
{
    return top + (h - (ascent + descent)) / 2 + ascent;
}

#ifndef AAT_HOST
#include <exec/types.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <inline/macros.h>

typedef struct { int width, ascent, descent; } aat_size;   /* gfopenlayout_ui_amiga.h's olui_amiga_text_size */

static struct Library *aat_base;

/* gfopenlayout.library 19's two calls (include/inline/gfopenlayout.h) */
#define AAT_Text(rp, x, y, utf8, length, family, px, argb, flags, shadow) \
    LP10(0x41a, int, OLUI_AmigaText, struct RastPort *, (rp), a0, int, (x), d0, int, (y), d1, const char *, (utf8), a1, \
         unsigned long, (length), d2, const char *, (family), a2, int, (px), d3, unsigned long, (argb), d4, unsigned, \
         (flags), d5, unsigned long, (shadow), d6, , aat_base)
#define AAT_TextWidth(rp, utf8, length, family, px, flags, size) \
    LP7(0x420, int, OLUI_AmigaTextWidth, struct RastPort *, (rp), a0, const char *, (utf8), a1, unsigned long, \
        (length), d0, const char *, (family), a2, int, (px), d1, unsigned, (flags), d2, aat_size *, (size), a3, , aat_base)

static inline void aat_open(void)
{
    if (!aat_base) aat_base = OpenLibrary((CONST_STRPTR)"gfopenlayout.library", 19);
}

static inline void aat_close(void)
{
    if (aat_base) { CloseLibrary(aat_base); aat_base = NULL; }
}

/* the face: rp's font's own name (diskfont's, "DejaVuSans.font"); one with no outline face gives sans-serif */
#define AAT_FAMILY(rp) ((const char *)(rp)->Font->tf_Message.mn_Node.ln_Name)

/* the outline text's size at rp's font's height; 0 where it isn't drawn so. A RastPort of no BitMap (one
 * InitRastPort made only to measure) is measured without a screen: given as it is, it measured nothing and
 * the box came out Text()'s width, too narrow for the outline text (the clock's first letter cut off). */
static inline int aat_size_of(struct RastPort *rp, const char *s, unsigned long n, aat_size *sz)
{
    return aat_base && rp->Font && aat_ascii(s, n)
           && AAT_TextWidth(rp->BitMap ? rp : NULL, s, n, AAT_FAMILY(rp), rp->Font->tf_YSize, 0, sz);
}

/* the width aat_box draws s at: the wider of the two, so a box sized by it holds either */
static inline WORD aat_length(struct RastPort *rp, const char *s, unsigned long n)
{
    aat_size sz;
    WORD w = TextLength(rp, (STRPTR)s, n);
    if (aat_size_of(rp, s, n, &sz) && sz.width > w) w = sz.width;
    return w;
}

/* s at x, centred in the rows top..top+h-1, in rp's APen's colour; JAM1 */
static inline void aat_box(struct RastPort *rp, struct Screen *scr, WORD x, WORD top, WORD h, const char *s, unsigned long n)
{
    aat_size sz;
    ULONG rgb[3];
    if (scr && aat_size_of(rp, s, n, &sz)) {
        GetRGB32(scr->ViewPort.ColorMap, GetAPen(rp), 1, rgb);
        if (AAT_Text(rp, x, aat_baseline(top, h, sz.ascent, sz.descent), s, n, AAT_FAMILY(rp), rp->Font->tf_YSize,
                     aat_argb((const unsigned long *)rgb), 0, 0))
            return;
    }
    SetDrMd(rp, JAM1);
    Move(rp, x, aat_baseline(top, h, rp->TxBaseline, rp->TxHeight - rp->TxBaseline));
    Text(rp, (STRPTR)s, n);
}
#endif
#endif
