#!/bin/sh
# Host tests for GFOpenNetwork's pictures, with the host's cc.   sh Network/tests/run.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
"$CC" -std=gnu99 -Wall -Wextra -Werror -O2 -fsanitize=address,undefined -I"$HERE/../src" -o "$T/t" "$HERE/test_np.c"
"$T/t"
