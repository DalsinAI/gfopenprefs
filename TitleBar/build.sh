#!/bin/sh
# GFOpenPrefs Title bar and GFOpenTitle, with the os32 stove (m68k-amigaos-gcc, NDK 3.2).
#   TitleBar/build.sh [OUT_DIR]     (default build/os3)
# src/logo.h and src/logo_big.h come from tools/cradle_mark.py (the GFOS mark); they are kept in
# the repository, so the build needs no Python.
# MIT, Copyright (c) 2026 AmiCradle Team.
# CPU= and OPT= change -m68040 and -O2 (the ROM's build: OPT=-Os).
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
# The 68040, soft float: the specs file links libnix's soft libm (maths through
# the mathieee libraries), not its FPU libm881, so one program runs on any 68040.
SPECS="-specs=$HERE/../specs/m68k-soft-m68040.specs"
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
OGT=${OGT:-$HERE/../../gfopengadtools}      # a checkout of DalsinAI/gfopengadtools: its theme reader (lib/ogt_theme.c)
LAYOUT=${LAYOUT:-$HERE/../../gfopenlayout}  # a checkout of DalsinAI/gfopenlayout (master): GFOpenTitle draws through its oldraw
OUT=${1:-$HERE/build/os3}
CFLAGS="-noixemul ${CPU:--m68040} $SPECS -std=gnu99 -Wall -Werror ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common -I$HERE/src -I$HERE/include -I$HERE/../Common -I$OGT/lib"
# GFOpenAmigaGCC 16.2.0-open3's late-combine pass can store a copy one place late (GCC PR 123853; Common/wbclose.h's
# strcpy of "OpenWBNotify" came out so): off until open4 (gfopengcc #29), on a compiler that knows the option.
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then CFLAGS="$CFLAGS -fno-late-combine-instructions"; fi
mkdir -p "$OUT"
"$CC" $CFLAGS -o "$OUT/TitleBar" "$HERE/src/titlebar.c"
# GFOpenTitle reads where GFOpenMenus' bar is with the Menus editor's om_prefs.c, and draws what 0.9 added through
# GFOpenLayout's oldraw (include/gfopenlayout_draw.h; draw/oldraw.c and amiga/oldraw_amiga.c, as its build-amiga.sh
# puts them in libgfopenlayout_draw.a)
[ -f "$LAYOUT/include/gfopenlayout_draw.h" ] || { echo "build.sh: no gfopenlayout at $LAYOUT (LAYOUT= to say where)" >&2; exit 1; }
"$CC" $CFLAGS -I"$HERE/../Menus/src" -I"$HERE/../SysBar/include" -I"$LAYOUT/include" -DOLDRAW_AMIGA -I"$LAYOUT/draw" -o "$OUT/GFOpenTitle" \
    "$HERE/src/opentitle.c" "$HERE/../Menus/src/om_prefs.c" "$OGT/lib/ogt_theme.c" "$LAYOUT/draw/oldraw.c" \
    "$LAYOUT/amiga/oldraw_amiga.c"
echo "$OUT/TitleBar ($(wc -c < "$OUT/TitleBar") bytes), $OUT/GFOpenTitle ($(wc -c < "$OUT/GFOpenTitle") bytes)"
