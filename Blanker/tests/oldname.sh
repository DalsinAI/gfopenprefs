#!/bin/sh
# The old name, split or joined, in any file given: "AmigaChrome", "Amiga Chrome",
# "Amiga" and "Chrome" as two strings next to each other in a binary, or
# "Chrome" alone as a word. Prints each hit as file:line (or file: strings
# offset for a binary) and exits 1 if there is one.
#   sh Blanker/tests/oldname.sh FILE...
# MIT, Copyright (c) 2026 AmiCradle Team.
set -u
hit=0
tmp=$(mktemp); trap 'rm -f "$tmp"' EXIT
for f in "$@"; do
    if grep -Iq . "$f" 2>/dev/null; then
        grep -n -i -E 'amiga[^a-z0-9]{0,3}chrome|(^|[^a-z0-9_])chrome([^a-z0-9_]|$)' "$f" > "$tmp" && { sed "s|^|$f:|" "$tmp"; hit=1; }
    else
        # a binary: its strings, one per line, so two strings side by side are seen together too
        strings -a -n 3 "$f" | tr '\n' ' ' | grep -o -i -E '.{0,20}(amiga[^a-z0-9]{0,3}chrome|(^|[^a-z0-9_])chrome([^a-z0-9_]|$)).{0,20}' \
            > "$tmp" && { sed "s|^|$f: |" "$tmp"; hit=1; }
    fi
done
exit $hit
