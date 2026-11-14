#!/bin/sh
# Host tests for the Sound editor's settings and files (sp_core.c), with the host's cc.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${TMPDIR:-/tmp}/sp_core_test
${CC:-cc} -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -o "$OUT" "$HERE/test_sp_core.c" "$HERE/../src/sp_core.c"
"$OUT" "$HERE"
# GFOpenSpeaker 0.3's commodity options and pictures (src/sp_cx.h)
${CC:-cc} -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -I"$HERE/../../Common" -o "$OUT.cx" "$HERE/test_sp_cx.c"
"$OUT.cx" | grep -v '^ok' || true
"$OUT.cx" > /dev/null
