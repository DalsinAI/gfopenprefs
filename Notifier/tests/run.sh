#!/bin/sh
# Host tests for GFOpenNotifier's commands, programs and queue (nt_core.c), with the host's cc.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${TMPDIR:-/tmp}/nt_core_test
${CC:-cc} -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -o "$OUT" "$HERE/test_nt_core.c" "$HERE/../src/nt_core.c"
"$OUT"
