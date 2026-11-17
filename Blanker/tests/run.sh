#!/bin/sh
# GFOpenBlanker's host tests: the logo and the credit line (bl_logo.h, bl_wordmark.h), the music settings
# (bl_music.h), quit while the show is up (bl_quit.h), and no old name,
# split or joined, in its source or, given as $1, its built binary.
#   sh Blanker/tests/run.sh [build/os3/GFOpenBlanker]
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
B=$HERE/..
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$B/src" -o "$T/w" "$HERE/logo_test.c"
"$T/w" > "$T/w.txt" || true
if [ "$(tail -n 1 "$T/w.txt")" = PASS ]; then echo "ok   the logo and the credit line: $(grep -c '^ok' "$T/w.txt") checks"; else grep FAIL "$T/w.txt"; exit 1; fi
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$B/src" -o "$T/m" "$HERE/music_test.c"
"$T/m" > "$T/m.txt" || true
if [ "$(tail -n 1 "$T/m.txt")" = PASS ]; then echo "ok   the music settings: $(grep -c '^ok' "$T/m.txt") checks"; else grep FAIL "$T/m.txt"; exit 1; fi
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$B/src" -o "$T/q" "$HERE/quit_test.c"
"$T/q" > "$T/q.txt" || true
if [ "$(tail -n 1 "$T/q.txt")" = PASS ]; then echo "ok   quit while the show is up: $(grep -c '^ok' "$T/q.txt") checks"; else grep FAIL "$T/q.txt"; exit 1; fi
# 1.8: the show's frame looks for a Break and the commodity's messages, not only a key or the mouse
grep -q 'SetSignal(0, SIGBREAKF_CTRL_C)' "$B/src/openblanker.c" && grep -q 'cx_messages();' "$B/src/openblanker.c" \
    && grep -q 'return bl_show_goes_on(woken, quit);' "$B/src/openblanker.c" || { echo "FAIL the show's frame doesn't look for a quit"; exit 1; }
echo "ok   the show's frame looks for a Break and Exchange's Kill"
# the scanner itself: it must catch the old name joined, spaced, and split into two strings
printf 'x AmigaChrome y\n' > "$T/a.txt"; printf 'Amiga Chrome\n' > "$T/b.txt"; printf 'GFOS\nThe AmiCradle Team\n' > "$T/c.txt"
printf '\001\002Amiga\000Chrome\000\003' > "$T/d.bin"; printf '\001\002GFOS\000AmiCradle\000\003' > "$T/e.bin"
for f in a.txt b.txt d.bin; do if sh "$HERE/oldname.sh" "$T/$f" > /dev/null; then echo "FAIL the scanner missed $f"; exit 1; fi; done
for f in c.txt e.bin; do sh "$HERE/oldname.sh" "$T/$f" > /dev/null || { echo "FAIL the scanner flagged $f"; exit 1; }; done
echo "ok   the old-name scanner: 5 checks"
sh "$HERE/oldname.sh" "$B"/src/*.c "$B"/src/*.h || { echo "FAIL the old name is in Blanker/src"; exit 1; }
echo "ok   no old name in Blanker/src"
# 1.7: the show draws no "GFOS" (the string, in the source or the binary)
if grep -q '"GFOS"' "$B"/src/openblanker.c "$B"/src/bl_wordmark.h; then echo "FAIL the show still has the GFOS wordmark"; exit 1; fi
echo "ok   no GFOS wordmark in the show"
if [ $# -gt 0 ]; then
    sh "$HERE/oldname.sh" "$1" || { echo "FAIL the old name is in $1"; exit 1; }
    if strings -a "$1" | grep -qx 'GFOS'; then echo "FAIL $1 still draws GFOS"; exit 1; fi
    strings -a "$1" | grep -q 'GFOpenBlanker 1.8' || { echo "FAIL $1 isn't 1.8"; exit 1; }
    echo "ok   no old name in $1, and it is 1.8"
fi
echo PASS
