#!/bin/sh
# Host tests for the Screen mode section's fields (Display/src/dp_prefs.c) on Look's PREF reader and writer.
# And gfosdisplay.prefs's GDSP chunk (Display/src/gd_prefs.c), the display profiles.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${TMPDIR:-/tmp}/dp_test
${CC:-cc} -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -I"$HERE/../../Look/src" -o "$OUT" \
    "$HERE/test_dp.c" "$HERE/../src/dp_prefs.c" "$HERE/../../Look/src/pf_iff.c"
"$OUT"
OUT2=${TMPDIR:-/tmp}/gd_test
${CC:-cc} -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -I"$HERE/../../Look/src" -o "$OUT2" \
    "$HERE/test_gd.c" "$HERE/../src/gd_prefs.c" "$HERE/../../Look/src/pf_iff.c"
"$OUT2"
