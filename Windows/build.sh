#!/bin/sh
# GFOpenPrefs Windows and the GFOpenWindows commodity, with the os32 stove
# (m68k-amigaos-gcc, NDK 3.2).
#   Windows/build.sh [OUT_DIR]     (default $OUT, else build/os3)
# MIT, Copyright (c) 2026 AmiCradle Team.
# CPU= and OPT= change -m68040 and -O2 (the ROM's build: OPT=-Os).
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
# The 68040, soft float: the specs file links libnix's soft libm (maths through
# the mathieee libraries), not its FPU libm881, so one program runs on any 68040.
SPECS="-specs=$HERE/../specs/m68k-soft-m68040.specs"
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
OUT=${1:-${OUT:-$HERE/build/os3}}   # the stove build (Cradle) sets OUT=
mkdir -p "$OUT"
# GFOpenAmigaGCC 16.2.0-open3's late-combine pass can store a copy one place late (GCC PR 123853; Common/wbclose.h's
# strcpy of "OpenWBNotify" came out so): off until open4 (gfopengcc #29), on a compiler that knows the option.
LATE=""
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then LATE="-fno-late-combine-instructions"; fi
"$CC" -noixemul ${CPU:--m68040} $SPECS -std=gnu99 -Wall -Werror ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common $LATE -o "$OUT/Windows" "$HERE/src/windows.c"
"$CC" -noixemul ${CPU:--m68040} $SPECS -std=gnu99 -Wall -Werror ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common $LATE -o "$OUT/GFOpenWindows" "$HERE/src/openwindows.c" "$HERE/src/gestures.c" "$HERE/src/places.c" "$HERE/src/wheel.c"
echo "$OUT/Windows ($(wc -c < "$OUT/Windows") bytes), $OUT/GFOpenWindows ($(wc -c < "$OUT/GFOpenWindows") bytes)"
# 0.11: WheelTest (tests/amiga/wheeltest.c), the scrollers the wheel moves for a program that doesn't know it
"$CC" -noixemul ${CPU:--m68040} $SPECS -std=gnu99 -Wall -Werror ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common $LATE -o "$OUT/WheelTest" "$HERE/tests/amiga/wheeltest.c"
