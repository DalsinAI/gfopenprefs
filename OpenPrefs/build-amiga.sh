#!/bin/sh
# Cradle's stove build (stove_build: STOVE= and OUT= set): GFOpenPrefs with OpenPrefs/build.sh, the theme reader
# from a gfopengadtools checkout beside this one (OGT=, default ../gfopengadtools), else a shallow clone of it in OUT.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${OUT:-$HERE/build/os3}
OGT=${OGT:-$HERE/../../gfopengadtools}
if [ ! -f "$OGT/lib/ogt_theme.c" ]; then
  rm -rf "$OUT/gfopengadtools"
  git clone -q --depth 1 https://github.com/DalsinAI/gfopengadtools "$OUT/gfopengadtools"
  OGT=$OUT/gfopengadtools
fi
OGT=$OGT STOVE=$STOVE sh "$HERE/build.sh" "$OUT"
rm -rf "$OUT/obj" "$OUT/gfopengadtools"
ls -l "$OUT"
