# gfopensysbar.library

The system bar is the right end of the screen's title bar: the time, the systray, Prefs and Power, then the
screen's depth gadget. One owner draws it (GFOpenTitle since 0.8). This library keeps the bar's **items**, so
programs ask for a place with a library call instead of sending GFOpenTitle a message by hand, and the items
outlive the owner.

It follows designs/SYSTEM-BAR-LIBRARY-20261114.md in amicradle-teams (Dale's go, 14 November 2026 03:08), which
followed his words of 11 November 19:39: "a library option to allow icons and controls and menus to come from that
end of the screen title".

## What it keeps

An item has:
- a name, unique, up to 15 letters;
- a help line;
- an optional text (a text item, as the clock is);
- up to 6 pictures of up to 16 x 16 pixels, one byte a pixel (0 to 3 the bar's pens, 4 to 15 its 12 colours, as
  the taskspace has them), and the one shown;
- an optional menu of up to 16 entries;
- an order in the row (lower is further left; the same order, added first);
- the port its events go to, and the program's own data.

The library copies everything it is given. Up to 32 items at once.

## The calls (clib/gfopensysbar_protos.h)

| Call | LVO | Who | What |
| --- | --- | --- | --- |
| `SB_AddItemA(tags)` | -30 | a program | Adds an item, its task's. NULL if refused: `SBA_Error` says why (`SBR_*`). |
| `SB_SetItemA(item, tags)` | -36 | a program | Changes anything but the name. A refused change changes nothing. |
| `SB_RemItem(item)` | -42 | a program | Removes it. An item also goes once its task has ended. |
| `SB_Own(signal)` | -48 | the owner | Takes the bar; told of every change by that signal. One owner at a time; an owner whose task ended lets go. |
| `SB_Disown()` | -54 | the owner | Lets go. The items stay for the next owner. |
| `SB_LockList()` | -60 | the owner | Locks the list and gives the changes since the last lock (`SBC_*`; 0 to anyone but the owner, so nobody else takes them). Items whose task has gone are taken out first. |
| `SB_NextItem(prev)` | -66 | the owner | The row in order; NULL gives the first. |
| `SB_UnlockList()` | -72 | the owner | |
| `SB_SendEvent(item, kind, entry, box)` | -78 | the owner | A click, a menu entry or the item's place, to its port. An item whose task has ended is taken out first, and the answer is SBR_UNKNOWN: its port is gone with it. |
| `SB_DoneEvent(event)` | -84 | a program | Gives an event back once read. Never ReplyMsg one: it has no reply port, and would leak. Trust `sbe_Name`, not `sbe_Item`, which may be stale. |
| `SB_MenuEntry(item, n)` | -90 | anyone | The menu's entry n, or NULL (also for an item gone). |
| `SB_FindItem(name)` | -96 | anyone | An item by its name. |
| `SB_OwnerTask()` | -102 | a program | The owner's task, or NULL: whether anyone draws the bar (GFOpenSpeaker keeps its own window until then). |

The inline header (inline/gfopensysbar.h, through proto/gfopensysbar.h) calls it straight; or link
`libgfopensysbar.a` (stub/) and call the functions.

## What it doesn't touch

- The ROM: nothing of it is in the ROM; it lives in LIBS:.
- commodities.library: the systray keeps reading the brokers itself.
- The taskspace port (TitleBar/include/taskspace.h): versions 1 and 2 keep working. GFOpenTitle passes them on to
  the library in the next step.

## Steps

1. **This:** the library, the link stub, host tests (`sh SysBar/tests/run.sh`) and the lab's SysBarTest.
2. GFOpenTitle owns the bar through `SB_Own` and passes taskspace messages on.
3. GFOpenSpeaker and Tata add their items with `SB_AddItemA`.
4. The rest of GFOpenTitle's drawing onto GFOpenLayout's oldraw.
5. Lab: each item, a menu pick and an owner restart, as WBGapTest rows.

## Build

`sh SysBar/build-amiga.sh` (STOVE= the os32 stove). It gives Libs/gfopensysbar.library, lib/libgfopensysbar.a and
tests/SysBarTest.

MIT, Copyright (c) 2026 AmiCradle Team.
