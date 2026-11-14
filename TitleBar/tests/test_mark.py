#!/usr/bin/env python3
# Copyright (c) 2026 AmiCradle Team. GFOpenPrefs, MIT licence.
"""The GFOS mark (tools/cradle_mark.py) in GFOpenTitle: src/logo.h and src/logo_big.h are what the tool makes,
the picture's size is Common/barlogo.h's, and the 16 px mark is the Deep cradle's shape.
    python3 TitleBar/tests/test_mark.py"""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
TB = os.path.join(HERE, "..")
sys.path.insert(0, os.path.join(TB, "tools"))
import cradle_mark as cm

fails = 0
def check(what, ok, more=""):
    global fails
    print(("ok   " if ok else "FAIL ") + what + ("" if ok else f": {more}"))
    fails += not ok

def read(p):
    return open(os.path.join(TB, p), encoding="utf-8").read()

check("src/logo.h is tools/cradle_mark.py h16's", read("src/logo.h").strip() == cm.h16().strip())
check("src/logo_big.h is tools/cradle_mark.py big's", read("src/logo_big.h").strip() == cm.big().strip())
bl = read("../Common/barlogo.h")
pw = int(re.search(r"#define BARLOGO_PIC_W (\d+)", bl).group(1))
ph = int(re.search(r"#define BARLOGO_PIC_H (\d+)", bl).group(1))
big = read("src/logo_big.h")
bw = int(re.search(r"#define LOGO_BIG_W (\d+)", big).group(1))
bh = int(re.search(r"#define LOGO_BIG_H (\d+)", big).group(1))
check("logo_big is barlogo.h's size (opentitle.c's #if)", (bw, bh) == (pw, ph), f"{bw}x{bh} vs {pw}x{ph}")
vals = re.findall(r"0x[0-9a-f]{8}", big)
check("logo_big holds W x H pixels", len(vals) == bw * bh, str(len(vals)))
m = cm.MARK16
check("MARK16: 16 rows of 16", len(m) == 16 and all(len(r) == 16 for r in m))
check("MARK16: arc and dot only", set("".join(m)) <= set("Ao."))
check("MARK16: symmetric left to right", all(r == r[::-1] for r in m))
check("MARK16: open at the top (no arc between its two ends)", all("A" not in r[3:13] for r in m[:5]))
check("MARK16: closed at the bottom", "AAAAAA" in m[14])
dots = [(y, x) for y, r in enumerate(m) for x, c in enumerate(r) if c == "o"]
arc_rows = [y for y, r in enumerate(m) if "A" in r]
check("MARK16: the dot inside the arc, clear of it",
      all(r[x - 1] != "A" and r[x + 1] != "A" for y, x in dots for r in [m[y]]) and min(arc_rows) < min(y for y, _ in dots))
logo = read("src/logo.h")
cols = set(re.findall(r"0x[0-9a-f]{6}\b", logo))
check("logo16: white arc and orange dot on the bar", cols == {"0xffffff", "0xff7a1a"}, str(cols))
for h in (16, 22, 44, 96):
    im = cm.mark(h)
    check(f"mark {h}: {h} high, wider than high, both colours", im.height == h and im.width > h, str(im.size))
print("FAILED" if fails else "all good", f"({fails} failed)")
sys.exit(1 if fails else 0)
