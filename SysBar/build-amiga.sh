#!/bin/sh
# gfopensysbar.library for AmigaOS 3.x with an os32 stove, bare: no startup code or C library.
#   sh SysBar/build-amiga.sh            (OUT= where; default SysBar/build-amiga)
#     Libs/gfopensysbar.library      the library (core/sb_core.c, library/sysbar_lib.c)
#     lib/libgfopensysbar.a          the link stub (stub/sysbar_stub.c)
#     tests/SysBarTest               the lab's check of the library (tests/amiga/SysBarTest.c)
# STOVE= the stove (default ~/AmiCradle/stoves/os32); CPU= (default -m68040) and OPT= (default -Os).
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
STOVE=${STOVE:-"$HOME/AmiCradle/stoves/os32"}
P="$STOVE/prefix"
CC=${CC:-"$P/bin/m68k-amigaos-gcc"}
AR=${AR:-"$P/bin/m68k-amigaos-ar"}
NM=${NM:-"$P/bin/m68k-amigaos-nm"}
OUT=${OUT:-"$HERE/build-amiga"}
CPU=${CPU:--m68040}
OPT=${OPT:--Os}
mkdir -p "$OUT/Libs" "$OUT/lib" "$OUT/tests" "$OUT/obj"
INC="-I$HERE/include"
FLAGS="$CPU $OPT -std=gnu99 -fomit-frame-pointer -fno-lto -fno-builtin -ffreestanding
  -fno-delete-null-pointer-checks -fno-tree-loop-distribute-patterns -Wall -Wextra -Werror
  -Wno-unused-parameter $INC"
BARE="-nostartfiles -nostdlib"
# shellcheck disable=SC2086
"$CC" $FLAGS $BARE -fno-toplevel-reorder -c -o "$OUT/obj/start.o" "$HERE/library/start.c"
# shellcheck disable=SC2086
"$CC" $FLAGS $BARE -c -o "$OUT/obj/sysbar_lib.o" "$HERE/library/sysbar_lib.c"
# shellcheck disable=SC2086
"$CC" $FLAGS $BARE -c -o "$OUT/obj/sb_core.o" "$HERE/core/sb_core.c"
# shellcheck disable=SC2086
"$CC" $FLAGS $BARE -o "$OUT/Libs/gfopensysbar.library" "$OUT/obj/start.o" "$OUT/obj/sysbar_lib.o" "$OUT/obj/sb_core.o" -lgcc
und=$("$NM" -u "$OUT/Libs/gfopensysbar.library" || true)
[ -z "$und" ] || { echo "undefined in gfopensysbar.library: $und" >&2; exit 1; }
echo "$OUT/Libs/gfopensysbar.library ($(wc -c < "$OUT/Libs/gfopensysbar.library") bytes)"
# shellcheck disable=SC2086
"$CC" $FLAGS -noixemul -c -o "$OUT/obj/sysbar_stub.o" "$HERE/stub/sysbar_stub.c"
rm -f "$OUT/lib/libgfopensysbar.a"
"$AR" rcs "$OUT/lib/libgfopensysbar.a" "$OUT/obj/sysbar_stub.o"
echo "$OUT/lib/libgfopensysbar.a ($(wc -c < "$OUT/lib/libgfopensysbar.a") bytes)"
# shellcheck disable=SC2086
"$CC" $FLAGS -noixemul -o "$OUT/tests/SysBarTest" "$HERE/tests/amiga/SysBarTest.c" -L"$OUT/lib" -lgfopensysbar
echo "$OUT/tests/SysBarTest ($(wc -c < "$OUT/tests/SysBarTest") bytes)"
