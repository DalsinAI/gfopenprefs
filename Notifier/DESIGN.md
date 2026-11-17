# GFOpenNotifier

Pop-up notices on the desktop: a program says something has happened (an update is ready, a new game is listed,
mail has come) and a small panel shows it for a few seconds without stopping anything or taking the keyboard.
Dale asked for it on 17 November 2026: "i would create a new openversion of it but have ranchero compatiblity".
He named it the same evening ("we jsut call it GFOPenNotifer") and asked that it be compatible with both Ranchero and Ringhio. MIT, Copyright (c) 2026 AmiCradle Team.

## What it is compatible with

Ranchero (Rob Cranley, 2018, Aminet `util/cdity/Ranchero`) is a commodity for OS 3 that shows such notices. Its
readme says it was written to match the ARexx interface of Ringhio, OS 4.1's notification server, so the same
script runs on both. That interface is two ARexx commands, published on the OS 4 wiki
("Ringhio Notifications Arexx Interface"):

```
REGISTERAPP APP/K,AREXXPORT/K,ICON/K,TEXT/F
RINGHIO     APP/K,UPDATE/S,CLOSEONDC/S,PRI/K,SCREEN/K,IMG/K,IMGVALIGN/K/N,LOGONLY/S,NOLOG/S,TITLE/K,
            BACKRXMSG/K,PAUSEBACKRXMSG/K,STOPBACKRXMSG/K,PERCENTAGE/K,TEXT/F
```

Compatible means: GFOpenNotifier opens the same ARexx port, takes both commands with every argument above, and
gives the same answers (`OK, APP REGISTERED`, `OK, MSG QUEUED`, `ERROR: APP NOT REGISTERED` and the rest). A
script or program written for Ranchero or Ringhio then works with ours unchanged, and ours takes Ranchero's
place: only one of them can own the port.

We go further than Ranchero where Ringhio's interface does: Ranchero's readme lists as not done the blocking of
programs, the message back to a program, the URL to a browser and more than one notice at once.

Written from those two published descriptions only. None of Ranchero's or Ringhio's code or files is used or
shipped.

Ringhio's other interface, application.library's `Notify()`, is an OS 4 PowerPC library call; no 68k program can make it, so on our machines the ARexx commands are the whole of both interfaces. Every Ringhio argument is taken, including those Ranchero leaves out.

Still to check on a machine, since the archive itself could not be fetched from here: the port's name (taken
as `RINGHIO`), the return code a script sees with each answer, and which arguments Ranchero 1.0 ignores.

## The parts

| Part | What | State |
| --- | --- | --- |
| `src/nt_core.c` | Reads a command line, keeps the registered programs (32), the queue (16) and a log (32), gives the answer. No Amiga calls | **Step 1, this** |
| `GFOpenNotifier` | The commodity, started from WBStartup: owns the port, shows each notice as a pop-up panel under the system bar in the Open look (`Common/popuptheme.h`), word-wrapped, with the picture through datatypes; sends the message back on a double-click | Step 2 |
| The systray | Its broker shows with the other commodities; a bell item on the system bar (`SB_AddItemA`) whose menu lists the last notices and has "Quiet" | Step 3 |
| `C:Notify` and `notify.h` | For our own programs and scripts that have no ARexx of their own: `Notify APP=GFOpenUpdate TITLE="Updates" TEXT 3 parts have newer versions` sends the same command to the port | Step 4 |
| Settings | Seconds a notice stays, the corner, quiet, blocked programs, in `ENVARC:GFOpenNotifier/` and a Prefs page | Step 5 |

## Step 1 rules (tested in `tests/test_nt_core.c`, `sh Notifier/tests/run.sh`)

- Commands and keywords in any case; `KEY=value`, `KEY value`, `"quoted values"` with `*"` and `*N`; `TEXT`
  takes the rest of the line, with or without its keyword.
- The title is cut at 64 characters and the text at 160, as Ringhio shows them. Numbers are held to their
  ranges (priority 0 to 10, percentage 0 to 100, picture alignment 0 to 2).
- The highest priority is shown first, the oldest of equals. Priority 10 stays until closed; priority 5 stays
  the set time, 0 half of it.
- `UPDATE` takes the program's waiting notices away first. `LOGONLY` goes to the log only, `NOLOG` to the
  screen only. A full queue answers `ERROR: TOO MANY MESSAGES QUEUED`.
- A program must register before it sends. A blocked program is answered `ERROR: APP NOT ALLOWED`.
- A line longer than 1023 characters is cut, never overrun.
