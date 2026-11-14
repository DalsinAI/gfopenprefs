#!/bin/sh
# For the Cradle's stove_build (STOVE= and OUT= set, target this directory): build.sh into OUT.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
sh "$HERE/build.sh" "${OUT:-$HERE/build/os3}"
