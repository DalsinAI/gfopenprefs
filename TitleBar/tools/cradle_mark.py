#!/usr/bin/env python3
# Copyright (c) 2026 AmiCradle Team. GFOpenPrefs, MIT licence.
"""The GFOS mark, the Deep cradle (Dale, 14 November 2026, 12:38-12:41: "deep cradle", "modern amiga colours";
amicradle-teams teams/DECISION-ICONS-SOFT3D-LOGO-CRADLE-20261114-1233.md): a deep arc, open about 124 degrees at
the top, holding a dot. Arc #0b5ed7, dot #ff7a1a. Our own drawing.

    cradle_mark.py png SIZE OUT.png [--bar]   the mark SIZE pixels high (drawn 16 times larger and cut down)
    cradle_mark.py h16 > src/logo.h           the 16 x 16 title-bar mark, as GFOpenTitle's logo16[]
    cradle_mark.py big > src/logo_big.h       the bar's picture, 44 rows, as logo_big[] (Common/barlogo.h's size)

--bar: for the blue glass title bar: the arc white (the decision's one-colour white version), the dot orange, as
#0b5ed7 doesn't hold on the bar's blue. At 16 px the mark is drawn pixel by pixel (MARK16) with the heavier
2 px stroke (the decision: "use the heavier stroke at that size only").
"""
import math
import sys

from PIL import Image, ImageDraw, ImageFilter

ARC = (0x0b, 0x5e, 0xd7)
DOT = (0xff, 0x7a, 0x1a)
OPEN = 124                      # the opening at the top, degrees
WHITE = (255, 255, 255)

# the 16 px mark, pixel by pixel: A the arc, o the dot
MARK16 = [
    "................",
    "................",
    ".AA..........AA.",
    "AAA..........AAA",
    "AA............AA",
    "AA....oooo....AA",
    "AA...oooooo...AA",
    "AA...oooooo...AA",
    "AA...oooooo...AA",
    "AAA...oooo...AAA",
    ".AA..........AA.",
    ".AAA........AAA.",
    "..AAAA....AAAA..",
    "...AAAAAAAAAA...",
    ".....AAAAAA.....",
    "................",
]


def geometry(h):
    """(width, circle radius, centre y, stroke) for a mark h pixels high: the arc's ends sit (90 - OPEN/2) degrees
    above its centre, so the mark is r * (1 + sin) high and 2r wide, plus the stroke"""
    heavy = h <= 16
    up = math.sin(math.radians(90 - OPEN / 2))
    stroke = h * (0.17 if heavy else 0.13)
    r = (h - stroke) / (1 + up)
    w = 2 * r + stroke
    cy = stroke / 2 + r * up
    return w, r, cy, stroke


def mark(h, bar=False):
    """the mark h pixels high, RGBA, its width from its shape"""
    k = 16
    w, r, cy, stroke = geometry(h)
    W = int(math.ceil(w))
    big = Image.new("RGBA", (W * k, h * k))
    d = ImageDraw.Draw(big)
    cx = W * k / 2
    R, S, CY = r * k, stroke * k, cy * k
    a0, a1 = -90 + OPEN / 2, 270 - OPEN / 2       # PIL: 0 at three o'clock, clockwise; the gap centred at the top
    box = [cx - R, CY - R, cx + R, CY + R]
    arc = WHITE if bar else ARC
    d.arc(box, a0, a1, fill=arc + (255,), width=int(S))
    for a in (a0, a1):                            # round ends
        ex = cx + (R - S / 2) * math.cos(math.radians(a))
        ey = CY + (R - S / 2) * math.sin(math.radians(a))
        d.ellipse([ex - S / 2, ey - S / 2, ex + S / 2, ey + S / 2], fill=arc + (255,))
    dr = (R - S) * 0.52                           # the dot, resting low in the arc
    dy = CY + (R - S) - dr - S * 0.55
    d.ellipse([cx - dr, dy - dr, cx + dr, dy + dr], fill=DOT + (255,))
    return big.resize((W, h), Image.LANCZOS)


def h16():
    colour = {"A": "0x%02x%02x%02x" % WHITE, "o": "0x%02x%02x%02x" % DOT, ".": "-1"}
    out = ["/* The GFOS mark, the Deep cradle, 16 x 16 for the title bar: 0xRRGGBB, or -1 where it is see-through",
           " * (tools/cradle_mark.py h16, MARK16) */",
           "static const long logo16[16 * 16] = {"]
    for row in MARK16:
        out.append("    " + ", ".join(colour[c] for c in row) + ",")
    out.append("};")
    return "\n".join(out)


def big():
    im = mark(44, bar=True)
    W, H = im.size
    out = ["/* The GFOS mark, the Deep cradle, %d x %d, 0xAARRGGBB (alpha in the top byte), for a bar %d pixels high or" % (W, H, H + 2),
           " * less: drawn at the bar's height less 2, each pixel the average of the ones it covers (tools/cradle_mark.py big). */",
           "#define LOGO_BIG_W %d" % W, "#define LOGO_BIG_H %d" % H,
           "static const ULONG logo_big[LOGO_BIG_W * LOGO_BIG_H] = {"]
    for y in range(H):
        out.append("    " + ", ".join("0x%02x%02x%02x%02x" % (a, r, g, b) for r, g, b, a in (im.getpixel((x, y)) for x in range(W))) + ",")
    out.append("};")
    return "\n".join(out)


if __name__ == "__main__":
    a = sys.argv[1:]
    if a and a[0] == "png" and len(a) >= 3:
        mark(int(a[1]), "--bar" in a).save(a[2])
    elif a == ["h16"]:
        print(h16())
    elif a == ["big"]:
        print(big())
    else:
        sys.exit(__doc__)
