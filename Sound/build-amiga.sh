#!/bin/sh
# For the Cradle's stove_build (STOVE= and OUT= set, target this directory): build.sh into OUT, with
# gfopengadtools from the Cradle's own checkout (cradle-builds/DalsinAI__gfopengadtools) when it is there,
# else as build.sh finds it (OGT= to say where). As Menus/engine/build-amiga.sh.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
up=$(cd "$HERE/../.." && pwd)
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
if [ -z "${OGT:-}" ] && [ -d "$up/DalsinAI__gfopengadtools" ]; then OGT=$(tree "$up/DalsinAI__gfopengadtools" gfopengadtools); fi
export OGT
sh "$HERE/build.sh" "${OUT:-$HERE/build/os3}"
