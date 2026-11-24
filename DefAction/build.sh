#!/bin/sh
# GFOpenDefAction alone, with the os32 stove (m68k-amigaos-gcc, NDK 3.2); GFOpenPrefs builds it in as the Default
# Actions page (OpenPrefs/build.sh).
#   DefAction/build.sh [OUT_DIR]        (default build/os3)
# CPU= and OPT= change -m68040 and -O2. MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
SPECS="-specs=$HERE/../specs/m68k-soft-m68040.specs"     # the 68040, soft float, as GFOpenTypes
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
OUT=${1:-$HERE/build/os3}
mkdir -p "$OUT"
LATE=""
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then LATE="-fno-late-combine-instructions"; fi
TRUNC=""
for w in format-truncation stringop-truncation; do
  if echo 'int x;' | "$CC" -Werror -Wno-error=$w -fsyntax-only -x c - 2>/dev/null; then TRUNC="$TRUNC -Wno-error=$w"; fi
done
"$CC" -noixemul ${CPU:--m68040} $SPECS -std=gnu99 -Wall -Werror $TRUNC ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common $LATE \
  -I"$HERE/src" -o "$OUT/GFOpenDefAction" "$HERE/src/defaction.c" "$HERE/src/da_core.c"
"$CC" --version | head -1 > "$OUT/GFOpenDefAction.compiler"
echo "$OUT/GFOpenDefAction ($(wc -c < "$OUT/GFOpenDefAction") bytes, $(cat "$OUT/GFOpenDefAction.compiler"))"
