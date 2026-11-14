#!/bin/sh
# Host tests for gfopensysbar.library's core (core/sb_core.c), with the host's cc.
#   sh SysBar/tests/run.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../core" -o "$T/t" \
    "$HERE/test_sb_core.c" "$HERE/../core/sb_core.c"
"$T/t"
