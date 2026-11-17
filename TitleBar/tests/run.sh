#!/bin/sh
# Host tests for GFOpenTitle's settings (src/tb_prefs.h), its see-through bar's fade (src/tb_fade.h) and where it reads
# the bar's colour (../Common/barstrip.h), with the
# host's cc.
#   sh TitleBar/tests/run.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -I"$HERE/../../Common" -o "$T/t" "$HERE/test_tb.c"
"$T/t"
python3 "$HERE/test_mark.py"
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -I"$HERE/../../Common" -o "$T/tl" "$HERE/test_logo.c"
"$T/tl"
