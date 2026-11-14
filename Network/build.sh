#!/bin/sh
# GFOpenNetwork, with the os32 stove (m68k-amigaos-gcc, NDK 3.2).
#   Network/build.sh [OUT_DIR]     (default build/os3)
# MIT, Copyright (c) 2026 AmiCradle Team.
# CPU= and OPT= change -m68040 and -O2.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
# The 68040, soft float (as every GFOpenPrefs program): one program runs on any 68040.
SPECS="-specs=$HERE/../specs/m68k-soft-m68040.specs"
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
OGT=${OGT:-$HERE/../../gfopengadtools}      # a checkout of DalsinAI/gfopengadtools: its theme reader (lib/ogt_theme.c)
OUT=${1:-$HERE/build/os3}
CFLAGS="-noixemul ${CPU:--m68040} $SPECS -std=gnu99 -Wall -Werror ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common -I$HERE/src -I$HERE/../TitleBar/include -I$HERE/../SysBar/include -I$HERE/../Common -I$OGT/lib"
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then CFLAGS="$CFLAGS -fno-late-combine-instructions"; fi
mkdir -p "$OUT"
"$CC" $CFLAGS -o "$OUT/GFOpenNetwork" "$HERE/src/opennetwork.c" "$OGT/lib/ogt_theme.c"
echo "$OUT/GFOpenNetwork ($(wc -c < "$OUT/GFOpenNetwork") bytes)"
