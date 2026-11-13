#!/bin/sh
# Host tests for GFOpenUp Setup's unattended mode (src/setup_auto.h), with the host's cc.   sh Setup/tests/run.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -o "$T/t" "$HERE/test_setup_auto.c"
"$T/t"
