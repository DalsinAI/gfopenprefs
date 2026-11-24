<!-- GFOpenDefAction's design, 24 November 2026 (AC-266). Copyright (c) 2026 AmiCradle Team, MIT licence (../LICENSE). -->

# GFOpenDefAction

Dale, 24 November 2026, 20:54: "ac-232 go ahead, we need a scanner to auto offer a tool for them, GFOpenDefAction",
"so it sits on icon.library", "with its own preference page".

AC-232 gave open-ROM machines the text, picture, sound and animation classes; the Archive work set default tools on
readme, picture and movie icons by hand. GFOpenDefAction is the scanner for the rest: it finds project icons with no
default tool and offers each the program for its kind of file.

## What it does

- **Which icons:** a project's icon (`WBPROJECT`) whose default tool is empty. With "Also icons naming a program
  that isn't here" (MISSING, on by default, as the Archive work did) also those naming a program found nowhere
  Workbench looks: beside the project, then `C:`, `SYS:Utilities`, `SYS:Tools`, `SYS:System`, `SYS:Preferences`,
  `SYS:Prefs`, `SYS:Tools/Commodities`, `SYS:Apps` (GFOpenBench's `find_tool`).
- **Its kind:** one of seven: text and readmes, AmigaGuide, pictures, movies and animations, sounds, music,
  documents (`src/da_core.c`). Asked of **icon.library** first, its identify hook (DefIcons, `ICONGETA_IdentifyBuffer`
  with `ICONGETA_IdentifyOnly`, icon.library 47); then, on an ACFS volume, what the file's own bytes say
  (AC-270: `ACTION_ACFS_KINDS`, one packet a page of the drawer, then `ACTION_ACFS_KIND` for a file not on it; its
  type, then its family, so one rule takes every picture); then datatypes.library (the datatype's name, then its
  group); then the file's name (`Nostalgia.readme`, `ReadMe`, `mod.tune`). Another filesystem answers
  `ERROR_ACTION_NOT_KNOWN` and is asked nothing more for that drawer. Programs, archives, scripts and anything not
  one of the seven are left alone.
- **The program offered:** the kind's program on the page when one is set, else what the kind's **default icon**
  names (icon.library's `ICONGETA_GetDefaultName`: `def_<type>`, then `def_ascii`, `def_picture`, ...: the icons
  GFOpenTypes edits), else `SYS:Utilities/MultiView`. Only a program that is on the machine is offered.
- **Offered, not forced:** Scan lists every icon with the program it would get, all ticked; a click unticks one;
  "Give programs" writes the ticked ones.
- **Never a silent change:** each icon's old default tool goes on a line of `ENVARC:GFOpenPrefs/DefActionChanges`
  before it is written; Undo all (`UNDO`) gives every one back, newest first, and forgets the list.
- **icon.library only:** icons are read with `GetIconTagList` (no pictures while scanning) and changed with
  `GetDiskObject` and `PutDiskObject`, as GFOpenTypes changes them; nothing is patched.
- Requesters are off while it scans, so a volume an icon names that isn't mounted asks nothing; Ctrl-C stops a scan.

## Where it is

- **Its own page of GFOpenPrefs, Default Actions**, after File Types (`GFOpenPrefs DEFACTION`, or `DEFAULTACTIONS`,
  `DEFAULTTOOLS`). GFOpenPrefs run as `GFOpenDefAction` opens it, with its arguments.
- **Alone:** `DefAction/build.sh` builds `GFOpenDefAction` with the os32 stove; `DefAction/build-page.sh` compiles
  the page as GFOpenPrefs does, without the frame.

## The Shell

    GFOpenDefAction                                   the window
    GFOpenDefAction SCAN [IN] <drawers...>             the icons with no tool there, and the program each is offered
    GFOpenDefAction SCAN [IN] <drawers...> APPLY       and gives them those programs
    GFOpenDefAction UNDO                              every program it gave, taken back
    GFOpenDefAction KIND <kind> TO <program>|DEFAULT [MISSING ON|OFF] [USE|SAVE]

Kinds: `TEXT`, `GUIDE`, `PICTURES`, `MOVIES`, `SOUNDS`, `MUSIC`, `DOCUMENTS`. Without drawers it scans the page's
(`SYS: Work:` built in).

## Settings

`ENV:GFOpenPrefs/DefAction` (Use) and `ENVARC:GFOpenPrefs/DefAction` (Save), one line each:
`TEXT=`, `GUIDE=`, `PICTURES=`, `MOVIES=`, `SOUNDS=`, `MUSIC=`, `DOCUMENTS=` (empty: the default icon's program),
`DRAWERS=SYS: Work:` (quotes keep a space), `MISSING=ON`.

## Tests

`DefAction/tests/da_test.c` (71 checks, run by `OpenPrefs/tests/run.sh`): the kinds by type, ACFS kind (type, then family), datatype group and name;
which icons are offered one; the settings file read and written; the list of changes; the drawers' list.
