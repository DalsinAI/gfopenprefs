#!/bin/sh
# GFOpenBlanker's host tests: the wordmark (bl_wordmark.h), and no old name,
# split or joined, in its source or, given as $1, its built binary.
#   sh Blanker/tests/run.sh [build/os3/GFOpenBlanker]
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
B=$HERE/..
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$B/src" -o "$T/w" "$HERE/wordmark_test.c"
"$T/w" > "$T/w.txt" || true
if [ "$(tail -n 1 "$T/w.txt")" = PASS ]; then echo "ok   the wordmark: $(grep -c '^ok' "$T/w.txt") checks"; else grep FAIL "$T/w.txt"; exit 1; fi
# the scanner itself: it must catch the old name joined, spaced, and split into two strings
printf 'x AmigaChrome y\n' > "$T/a.txt"; printf 'Amiga Chrome\n' > "$T/b.txt"; printf 'GFOS\nThe AmiCradle Team\n' > "$T/c.txt"
printf '\001\002Amiga\000Chrome\000\003' > "$T/d.bin"; printf '\001\002GFOS\000AmiCradle\000\003' > "$T/e.bin"
for f in a.txt b.txt d.bin; do if sh "$HERE/oldname.sh" "$T/$f" > /dev/null; then echo "FAIL the scanner missed $f"; exit 1; fi; done
for f in c.txt e.bin; do sh "$HERE/oldname.sh" "$T/$f" > /dev/null || { echo "FAIL the scanner flagged $f"; exit 1; }; done
echo "ok   the old-name scanner: 5 checks"
sh "$HERE/oldname.sh" "$B"/src/*.c "$B"/src/*.h || { echo "FAIL the old name is in Blanker/src"; exit 1; }
echo "ok   no old name in Blanker/src"
if [ $# -gt 0 ]; then
    sh "$HERE/oldname.sh" "$1" || { echo "FAIL the old name is in $1"; exit 1; }
    strings -a "$1" | grep -q 'GFOpenBlanker 1.5' || { echo "FAIL $1 isn't 1.5"; exit 1; }
    echo "ok   no old name in $1, and it is 1.5"
fi
echo PASS
