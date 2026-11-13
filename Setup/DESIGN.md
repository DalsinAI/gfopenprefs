# GFOpenUp Setup

The first-start wizard (gfopenup DESIGN.md section 6). It runs once from
`SYS:WBStartup` the first time Workbench starts after GFOpenUp is installed,
and stays in `SYS:Prefs` to run again. MIT, Copyright (c) 2026 AmiCradle Team.

## 0.1 (6 October 2026)

| Page | What it offers |
| --- | --- |
| 1. Welcome | What this machine is (`ENV:GFOpenUp/Summary` from `GFOpenUpTool DETECT`, or the CPU and FPU when that is missing). **Simple** or **Advanced**. "Don't show this at start again" |
| 2. Your desktop | The desktop profiles in `SYS:Prefs/Presets/GFOpenPrefs` (Modern, Lite 68040, Classic 3.2, and any added), each with its own line saying what it is for. "Choose by machine at every start". In Advanced, one button each for Look, Menus, Windows, Dock and File types (greyed when that editor isn't installed) |
| 3. Screen, printer and network (Advanced) | Screen mode, GFOpenPrint (`SYS:Utilities/GFOpenPrint/Printers and Queue`), Network (`SYS:Tools/Commodities/GFOpenSocketControl`), Fonts: the editors themselves (Printer went in 0.1.6) |
| 4. What starts with the machine (Advanced) | Every line in GFOpenUp's marked block in `S:User-Startup`, in plain words ("The theme on windows and gadgets (GFOpenLook)"), each with a switch |
| 5. All set | Finish |

Simple goes Welcome, Desktop, Done: choose a theme and go. Nothing changes
until **Finish**. **Later** (or the close gadget) leaves everything as it
was.

Setup keeps no settings of its own. On Finish:

- The profile is applied through Look, with
  `SYS:Prefs/Look FROM <profile> SAVE`, adding `profile auto` when "by
  machine" is ticked. Look owns `ENV:GFOpenGadTools/Look`.
- The view is written to `ENV:GFOpenAmiga/PrefsView` and `ENVARC:`, so every
  GFOpenPrefs editor opens in the same view (Menus DESIGN.md, "Simple and
  Advanced").
- The startup switches are applied (see below).
- `ENVARC:GFOpenAmiga/Setup.done` is written. "Don't show this at start
  again" writes it on Later too.

`GFOpenUp-Setup [FIRSTSTART] [ADVANCED]`. With FIRSTSTART, or when started
from `SYS:WBStartup`, it does nothing once `Setup.done` exists.

### Startup switches (a proposal for GFOpenUpTool)

A line that is switched off stays in the block with `;OFF ` in front, so
AmigaDOS skips it:

    ;BEGIN GFOpenUp
    ; Added by GFOpenUp. GFOpenUpTool UNINSTALL takes it out.
    Run >NIL: C:GFOpenLook
    ;OFF Run >NIL: C:GFOpenMenus
    ;END GFOpenUp

Setup also lists the switched-off commands in `ENVARC:GFOpenAmiga/Startup.off`,
one per line, with `;` comments allowed. `GFOpenUpTool STARTUP` writes the
block again from the manifest. For it to keep the switches, its
`write_startup` needs one more step: put `;OFF ` before every `startup`
entry whose command is a line of `Startup.off`. Then a reinstall or a new
part keeps what Dale switched off. Until GFOpenUpTool does this, a new part's
install puts every line back on.

### Building

`Setup/build.sh [OUT_DIR]` uses the os32 stove (or `CC=`) and builds
`GFOpenUp-Setup` for a 68040 (built with -m68040), with no FPU needed.

### For the GFOpenUp part (to add in gfopenup)

    setup = Part(top, "GFOpenUpSetup", "0.1", "GFOpenUp Setup: the first-start wizard, with the startup switches")
    setup.add("Prefs/GFOpenUp-Setup", (a.openamigaprefs / "Setup/build/os3/GFOpenUp-Setup").read_bytes(), "SYS:Prefs/GFOpenUp-Setup")
    setup.add("Prefs/GFOpenUp-Setup.info", icons.icon(icons.WBTOOL, "page", stack=16384), "SYS:Prefs/GFOpenUp-Setup.info")
    # its first start: a project icon in WBStartup (Setup sees it was started from there)
    setup.add("WBStartup/GFOpenUp-Setup", b"", "SYS:WBStartup/GFOpenUp-Setup")   # an empty project file
    setup.add("WBStartup/GFOpenUp-Setup.info", <a project icon: default tool SYS:Prefs/GFOpenUp-Setup, tooltype DONOTWAIT>, "SYS:WBStartup/GFOpenUp-Setup.info")

## 0.1.3 (12 October 2026)

Page 3's **GFOpenPrint...** and **Network...** were always greyed: they looked
for `SYS:Prefs/GFOpenPrint` and `SYS:Prefs/GFOpenSocket`, which no part
installs. They now open what the title bar's cog opens: GFOpenPrint's
Printers and Queue (`SYS:Utilities/GFOpenPrint/Printers and Queue`) and
GFOpenSocketControl (`SYS:Tools/Commodities/GFOpenSocketControl`). The same
code is GFOpenBench's built-in GFOpenUp-Setup, so it changes there with
GFOpenBench's next build.

## 0.1.6 (24 October 2026)

Page 3 has no **Printer...** button. Printing is GFOpenPrint's now (the plan
"Retiring the legacy printing, in favour of GFOpenPrint", PR 5): the
GFOpenPrinter part archives the OS's Printer, PrinterGfx and PrinterPS
editors, so the button would only have been greyed. **GFOpenPrint...** is the
page's printing button; its other three are Screen mode, Network and Fonts,
two to a row. Paper settings are going to a GFOpenPrefs Printing page.
GFOpenBench's built-in GFOpenUp-Setup changes with GFOpenBench's next build.

## 0.1.7 (1 November 2026)

Finish writes S:User-Startup (and its other files) whole or not at all. It
used to open the file with MODE_NEWFILE, which cuts it to nothing first, and
didn't check Close, so a full or write-protected SYS:, or a reset part way,
left S:User-Startup empty or cut short, and the next boot missed the user's
own lines. Now the new text goes to a short name in the same drawer
(`Setup`, the task's address and a count in hex, `.tmp`), every Write and
the Close are checked, and only then the old file steps aside as `.bak`,
the new one takes its name (with the old one's protection bits and comment)
and the `.bak` is deleted. If the new one can't take the name, the old one
gets it back. This is what GFOpenUpTool does since gfopenup #179. Files on
the RAM disk (ENV:, T:) are written in place as before. GFOpenBench's
built-in GFOpenUp-Setup changes with GFOpenBench's next build.

## 0.1.8 (9 November 2026)

The Desktop page (step 2 of 5, Advanced) shown a second time locked the
Amiga. Dale's PiStorm card (GFOS on GFOpenKick full38) sat with the 68k flat
out at one address for over 75 seconds; a lab copy of Instance-40 locks the
same way with 0.1.7 after Advanced, Next, Back, Next. Every page's gadgets
were made once, the Desktop page's in two lists (the page's and the Advanced
buttons), and went in and out of the window with AddGList and
RemoveGList(-1). RemoveGList(-1) of the page's list took the buttons with
it and left the page's Next linked to them, so the next AddGList(-1) put the
buttons in twice and the window's gadgets became a ring. Now each page's
gadgets, the Advanced buttons with them, are one list made when the page is
shown and freed when it goes; the choices on a page (the profile, by
machine, Simple or Advanced, don't show again, the startup line picked) are
kept in their own variables and the next list starts from them.

A click in the list of profiles, and Simple or Advanced, no longer erase the
whole page: only the words beside the list, or the step count, are written
again. On the GFOpenKick ROM RefreshGList doesn't draw a listview's lines
again, so the list went blank at the first click.

GFOpenBench's built-in GFOpenUp-Setup (the one SYS:WBStartup starts where
GFOpenBench is the Workbench, as on GFOS) changes with GFOpenBench's next
build: until then the hang stays there.

## 0.1.9 (13 November 2026): unattended

Dale's rule of 13 November (amicradle-teams
`teams/DECISION-UNATTENDED-SETUP-20261113-1959.md`): a CM5 card booted on the
Pi 5 stopped at step 1 waiting for a click. Setup now has an unattended mode:
no window, no wait, the Simple path, nothing of the user's removed or written
over, `Setup.done` written, one line in the log and a return code.

    GFOpenUp-Setup [FIRSTSTART] [ADVANCED] [AUTO]

| Start | What happens |
| --- | --- |
| `AUTO` (any start, even after `Setup.done`) | unattended |
| first start (`SYS:WBStartup` or `FIRSTSTART`) after `Setup.done` | nothing, as before |
| first start with `ENVARC:GFOpenUp/Answers` | unattended, with those answers |
| first start with the host flag `ENV:AmiCradle/Unattended` (or `ENVARC:AmiCradle/Unattended`) | unattended, the defaults |
| first start with none of them | the window, as before |
| by hand from `SYS:Prefs` | the window, answers file and host flag or not |

The host flag is a file (any contents) that the Cradle writes on a lab or
test run's disk and the card builder on a freshly written card.

The answers file has the wizard's choices, `KEY=value` a line, `;` or `#`
for comments, keys any case; a key left out takes the default:

    ; ENVARC:GFOpenUp/Answers
    VIEW=simple          ; or advanced: the view the GFOpenPrefs editors open in
    PROFILE=Modern       ; a profile in SYS:Prefs/Presets/GFOpenPrefs
    BYMACHINE=yes        ; "Choose by machine at every start" (yes when PROFILE is given)
    OFF=GFOpenMenus      ; switch off every startup-block line with these words (up to 16)

The defaults keep what is there: with no `PROFILE` and no `BYMACHINE` the
Look settings stay as they are (GFOpenLook's default theme where there are
none); with no `VIEW` an existing `PrefsView` stays (`simple` is written
where there is none); with no `OFF` `S:User-Startup` isn't touched.

It appends one line to `ENVARC:GFOpenUp/Setup.log` (and prints it when run
from a Shell), for example

    GFOpenUp Setup: unattended (host flag) view=simple profile=kept bymachine=kept off=0 ignored=0: ok

and returns 0 (done), 5 (done, some answers not understood or a profile not
found) or 10 (something couldn't be written; `Setup.done` isn't written, so
the next start tries again). The parsing and the choice of mode are in
`src/setup_auto.h`, tested on the host by `Setup/tests/run.sh`.
GFOpenBench's built-in GFOpenUp-Setup gets it with GFOpenBench's next build
(the built-in has no `AUTO`; the answers file and the host flag work there).

## Next

GFOpenUp Picks (ticking programs to add) joins as a page once the Picks list
exists. Screen mode with GFOpenRTG's boot to RTG joins the Devices page when
GFOpenRTG has its own editor.
