#!/bin/sh
# GFOpenPrefs's host checks:
#   - every section name maps to a page;
#   - the pages' names (opframe.c's lt_label) are the 23 the user named, each one once;
#   - with a stove (STOVE=, default ~/AmiCradle/stoves/os32-gcc16): the app's first $VER is GFOpenPrefs's, and its
#     version is higher than every page's own $VER inside it and every editor's in this repository, and every
#     page object calls the frame's lt_wait (opshift.h routes the pages' Wait and WaitPort through it, so
#     Ctrl-C ends GFOpenPrefs: GFOpenBench's Quit).
#   sh OpenPrefs/tests/run.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/.." && pwd)
P=$HERE/..
fail=0
python3 - "$HERE/src/openprefs.c" "$P/Common/looktabs.h" <<'EOF' || fail=1
import re, sys
src = open(sys.argv[1]).read()
lt = open(sys.argv[2]).read()
op = lt[lt.index('#ifdef LT_OP'):lt.index('#else', lt.index('#ifdef LT_OP'))]
pages = re.findall(r'\b(LT_[A-Z]+)\b', op[op.index('enum {'):op.index('LT_N };')])
used = set(re.findall(r'\{ "[A-Z]+", (LT_[A-Z]+) \}', src))
missing = [p for p in pages if p not in used]
cases = set(re.findall(r'case (LT_[A-Z]+):', src))
nocase = [p for p in pages if p not in cases]
if missing or nocase:
    print(f'FAIL sections: no name for {missing}, no page for {nocase}')
    sys.exit(1)
print(f'ok   {len(pages)} sections, each with a name and a page')
EOF
# the pages' names in the list, as the user named them (salvor/Design-No-ROM-Volume-20261024.md, section 8,
# question Q9): GFOpenPrefs takes them for a section too ("Date & Time", a project icon's name), so each is one
python3 - "$HERE/src/opframe.c" <<'EOF' || fail=1
import re, sys
src = open(sys.argv[1]).read()
m = re.search(r'lt_label\[LT_N\] = \{(.*?)\};', src, re.S)
labels = re.findall(r'"([^"]*)"', m.group(1)) if m else []
want = {"Theme", "Screen Mode", "Pointer", "Fonts", "Colours", "Backdrops", "Workbench", "File Requesters", "Windows",
        "Title Bar", "Menus", "IControl", "Dock", "Sound", "Input", "Gamepads", "Date & Time", "Serial", "File Types",
        "System", "Blanker", "Icons", "Printing"}
if set(labels) != want or len({l.lower() for l in labels}) != len(labels):
    print(f'FAIL page names: {labels}, missing {sorted(want - set(labels))}, not asked for {sorted(set(labels) - want)}')
    sys.exit(1)
print(f'ok   {len(labels)} page names, the user\'s, each one')
EOF
# a page that can't start (a library it needs isn't there) is named with that library and version, and the window
# stays (Instance-40, 4 November 2026: no iffparse.library, nine pages ended GFOpenPrefs with no word)
HCC=${HOSTCC:-cc}
if command -v "$HCC" >/dev/null 2>&1; then
  W=$(mktemp -d)
  if "$HCC" -std=c99 -Wall -Werror -I"$HERE/src" -o "$W/why_test" "$HERE/tests/why_test.c" "$HERE/src/op_why.c" && "$W/why_test"; then :; else fail=1; fi
  rm -rf "$W"
else
  echo "skipped why_test: no host compiler ($HCC)"
fi
python3 - "$HERE/src/openprefs.c" "$HERE/src/opshift.h" <<'EOF' || fail=1
import re, sys
src = open(sys.argv[1]).read()
sh = open(sys.argv[2]).read()
rw = src[src.index('static int run_window('):]
rw = rw[:rw.index('\n}\n')]
bad = []
if 'lt_page_failed_to_start(rc)' not in rw or 'lt_page_say(' not in rw:
    bad.append('run_window does not check for a page that can\'t start')
if not re.search(r'continue;', rw):
    bad.append('run_window does not stay on the window after a page that can\'t start')
if not re.search(r'#define OpenLibrary\(n, v\) lt_openlib', sh) or 'lt_libfail(name, version)' not in sh:
    bad.append('opshift.h does not note the pages\' failed OpenLibrary')
if bad:
    print('FAIL ' + '; '.join(bad)); sys.exit(1)
print('ok   a page that can\'t start is named on a requester and the window stays')
EOF
# the Theme page's profiles: SYS:Prefs/Presets/GFOpenPrefs, else the drawer an install from before the rename has
# (SYS:Prefs/Presets/OpenPrefs: Instance-32, 4 November 2026); a page whose presets drawer is empty says so
python3 - "$P/Look/src/look.c" "$P/Look/src/appearance.c" <<'EOF' || fail=1
import re, sys
look, app = open(sys.argv[1]).read(), open(sys.argv[2]).read()
bad = []
if '"SYS:Prefs/Presets/OpenPrefs"' not in look or not re.search(r'if \(!nprofiles\) \{\s*profile_dir = PROFILE_DIR_OLD;', look):
    bad.append('Theme does not fall back to SYS:Prefs/Presets/OpenPrefs for its profiles')
if re.search(r'PROFILE_DIR "/%s\.profile"', look):
    bad.append('a profile is read from PROFILE_DIR, not the drawer it was listed from')
if 'if (!nthemes) strcpy(status_text' not in look:
    bad.append('Theme says nothing when no theme is installed')
if '!sets.n)' not in app:
    bad.append('Pointer says nothing when no pointer set is installed')
if bad:
    print('FAIL ' + '; '.join(bad)); sys.exit(1)
print('ok   the profiles\' old drawer is read, and an empty presets drawer is said')
EOF
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16}
if [ ! -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then echo "skipped the build: no stove at $STOVE"; exit $fail; fi
OGT=${OGT:-$P/../gfopengadtools}
[ -f "$OGT/lib/ogt_theme.c" ] || { echo "skipped the build: no gfopengadtools at $OGT (OGT=)"; exit $fail; }
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
STOVE=$STOVE OGT=$OGT CPU=-m68040 OPT=-Os sh "$HERE/build.sh" "$T/out" > "$T/build.log" 2>&1 || { tail -20 "$T/build.log"; echo "FAIL the build"; exit 1; }
python3 - "$T/out" "$P" <<'EOF' || fail=1
import os, re, sys
out, repo = sys.argv[1], sys.argv[2]
def vers(data):
    return [(m.group(1).decode(), tuple(int(x) for x in m.group(2).decode().split('.')))
            for m in re.finditer(rb'\$VER: ([A-Za-z-]+) ([0-9]+(?:\.[0-9]+)*)', data)]
app = open(os.path.join(out, 'GFOpenPrefs'), 'rb').read()
v = vers(app)
bad = 0
if not v or v[0][0] != 'GFOpenPrefs':
    print(f'FAIL the first $VER is {v[:1]}'); bad = 1
top = v[0][1]
for name, ver in v[1:]:
    if ver >= top:
        print(f'FAIL {name} {ver} inside is not older than GFOpenPrefs {top}'); bad = 1
for root, _, fs in os.walk(repo):
    if '/.git' in root or '/build' in root:
        continue
    for f in fs:
        if f.endswith('.c'):
            for name, ver in vers(open(os.path.join(root, f), 'rb').read()):
                if name != 'GFOpenPrefs' and ver >= top and name in ('Look', 'GFOpenLook', 'Windows', 'TitleBar', 'Menus',
                        'GFOpenDock-Prefs', 'GFOpenSound', 'GFOpenInput', 'Gamepads', 'GFOpenDateTime', 'GFOpenSerial', 'GFOpenTypes', 'GFOpenSystem'):
                    print(f'FAIL {name} {ver} ({f}) is not older than GFOpenPrefs {top}'); bad = 1
print(f'ok   GFOpenPrefs {".".join(map(str, top))} is the first $VER and the highest' if not bad else 'FAIL versions')
sys.exit(bad)
EOF
# every page that waits does so through the frame (Ctrl-C): its object refers to lt_wait or lt_waitport
NM="$STOVE/prefix/bin/m68k-amigaos-nm"
nowait=""
for o in "$T/out/obj/app/"*.o; do
  if "$NM" "$o" | grep -q -E ' U _?(lt_wait|lt_waitport)$'; then :; else
    case $(basename "$o") in look.o|windows.o|titlebar.o|menus.o|appearance.o|dockprefs.o|sound.o|openinput.o|gamepads.o|\
      opendatetime.o|openserial.o|opentypes.o|opensystem.o|blankerprefs.o|screenmode.o|icontrol.o|printing.o) nowait="$nowait $(basename "$o")" ;;
    esac
  fi
done
if [ -z "$nowait" ]; then echo "ok   every page waits through the frame (Ctrl-C ends GFOpenPrefs)"; else echo "FAIL pages that wait on their own:$nowait"; fail=1; fi
# every page object that opens a library notes a failure for the frame (opshift.h lt_openlib): pf_amiga.o is the
# iffparse.library the nine pages need; an object that opens none has no OpenLibrary call to note
nolib=""
for o in "$T/out/obj/app/"*.o; do
  case $(basename "$o") in pf_amiga.o|appearance.o|menus.o|dockprefs.o|openinput.o|gamepads.o|opendatetime.o|opentypes.o|\
    opensystem.o|windows.o|sp_amiga.o)
    "$NM" "$o" | grep -q -E ' U _?lt_libfail$' || nolib="$nolib $(basename "$o")" ;;
  esac
done
if [ -z "$nolib" ]; then echo "ok   every page's OpenLibrary notes a failure for the frame"; else echo "FAIL pages whose failed OpenLibrary goes unsaid:$nolib"; fail=1; fi
exit $fail
