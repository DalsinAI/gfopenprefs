#!/bin/sh
# The Default Actions page as GFOpenPrefs builds it (OpenPrefs/build.sh's page flags: LT_OP, opshift.h, its globals
# renamed), without the frame's gfopengadtools and gfopenlayout: the page's objects compile, and the page waits through
# the frame's lt_wait (as OpenPrefs/tests/run.sh checks every page).
#   DefAction/build-page.sh [OUT_DIR]   (default build/page)
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
P=$HERE/..
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
NM="$STOVE/prefix/bin/m68k-amigaos-nm"
OUT=${1:-$HERE/build/page}
mkdir -p "$OUT"
BASE="-noixemul ${CPU:--m68020} -std=gnu99 -Wall -Werror ${OPT:--O2} -fno-delete-null-pointer-checks -fno-common"
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then BASE="$BASE -fno-late-combine-instructions"; fi
TRUNC=""
for w in format-truncation stringop-truncation; do
  if echo 'int x;' | "$CC" -Werror -Wno-error=$w -fsyntax-only -x c - 2>/dev/null; then TRUNC="$TRUNC -Wno-error=$w"; fi
done
PAGE="$BASE -DLT_OP -DLT_ONE -I$P/Common -I$P/OpenPrefs/src$TRUNC -include $P/OpenPrefs/src/opshift.h"
DEFA="-I$HERE/src -DIconBase=defaction_IconBase -DDataTypesBase=defaction_DataTypesBase -DAslBase=defaction_AslBase -DGadToolsBase=defaction_GadToolsBase"
"$CC" $PAGE -Dmain=defaction_main -Dversion=defaction_version $DEFA -c -o "$OUT/defaction.o" "$HERE/src/defaction.c"
"$CC" $PAGE -I"$HERE/src" -c -o "$OUT/da_core.o" "$HERE/src/da_core.c"
"$NM" "$OUT/defaction.o" | grep -q -E ' U _?(lt_wait|lt_waitport)$' || { echo "FAIL defaction.o doesn't wait through lt_wait"; exit 1; }
"$NM" "$OUT/defaction.o" | grep -q -E ' T _?defaction_main$' || { echo "FAIL no defaction_main"; exit 1; }
echo "ok   the Default Actions page compiles as a page of GFOpenPrefs and waits through lt_wait ($("$CC" --version | head -1))"
