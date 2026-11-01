# GFOpenUp Setup

The first-start wizard (openamigaup DESIGN.md section 6). It runs once from
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

### For the GFOpenUp part (to add in openamigaup)

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
gets it back. This is what GFOpenUpTool does since openamigaup #179. Files on
the RAM disk (ENV:, T:) are written in place as before. GFOpenBench's
built-in GFOpenUp-Setup changes with GFOpenBench's next build.

## Next

GFOpenUp Picks (ticking programs to add) joins as a page once the Picks list
exists. Screen mode with GFOpenRTG's boot to RTG joins the Devices page when
GFOpenRTG has its own editor.
