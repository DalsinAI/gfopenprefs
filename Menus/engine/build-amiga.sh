#!/bin/sh
# For the Cradle's stove_build (STOVE= and OUT= set, target Menus/engine): build.sh into OUT,
# with opengadtools from the Cradle's own checkout (cradle-builds/DalsinAI__opengadtools)
# when it is there, else as build.sh finds it (OGT= to say where).
# MIT, Copyright (c) 2026 AmigaChrome Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
up=$(cd "$HERE/../../.." && pwd)
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
# A Cradle checkout can be a git directory with no files out: take its tree.
tree() {
    [ -n "$(ls "$1" 2>/dev/null)" ] && { echo "$1"; return; }
    for r in HEAD FETCH_HEAD origin/main main; do
        if git -C "$1" rev-parse -q --verify "$r^{tree}" >/dev/null 2>&1; then
            mkdir -p "$TMP/$2" && git -C "$1" archive "$r" | tar -x -C "$TMP/$2" && { echo "$TMP/$2"; return; }
        fi
    done
    echo "$1"
}
if [ -z "${OGT:-}" ] && [ -d "$up/DalsinAI__opengadtools" ]; then OGT=$(tree "$up/DalsinAI__opengadtools" opengadtools); fi
export OGT
OS32_GCC16=${OS32_GCC16:-${STOVE:-}}
export OS32_GCC16
sh "$HERE/build.sh" "${OUT:-$HERE/../build/os3}"
