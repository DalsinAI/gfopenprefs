#!/bin/sh
# For the Cradle's stove_build (STOVE= and OUT= set, target this directory): StackProbe, for the lab check of Global
# Settings' default stack size. MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${OUT:-$HERE/build}
mkdir -p "$OUT"
"$STOVE/prefix/bin/m68k-amigaos-gcc" -noixemul -nostartfiles -m68040 -Os -Wall -Werror -o "$OUT/StackProbe" "$HERE/StackProbe.c"
cp "$HERE/StackProbe.info" "$OUT/"       # a tool icon with Stack 4096, for the Workbench check
ls -l "$OUT"
