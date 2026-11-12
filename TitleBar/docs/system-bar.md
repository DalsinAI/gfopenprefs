# The system bar: one owner, items asked for, commodities shown

Status: decided, 12 November 2026 (Dale 00:57: every commodity shown; 00:59: GFOpenTitle on disk owns the bar). Nothing here is built yet.

## Why

Dale, 11 November 2026 19:39: "we probably need a library option to allow icons
and controls and menus to come from that end of the screen title. ... wedging
them in is the wrong approach, part of the screen title is the system bar that
has time, and applications can request to go in it, perhaps the approach is lean
into commodities and render the commodities in that space."

Today the right end of the Workbench screen's title bar is drawn by up to four
programs, each placing itself by `ENV:GFOpenMenus/Tray` files and redrawing on a
timer:

| Drawn by | What | How it places itself |
| --- | --- | --- |
| GFOpenTitle (disk) | clock, network, cog, taskspace icons | its own windows over the bar, tray files |
| GFOpenSpeaker (disk) | speaker and volume panel | its own window over the bar, tray file `Speaker` |
| GFOpenBar (ROM, common/openbar) | clock, network, speaker, cog | draws them itself, steps aside when the disk tools' ports are there |
| Intuition | the screen's depth gadget | the screen's own gadget |

That is how the speaker went missing (nothing started GFOpenSpeaker on the open
ROM, gfopenup #238) and why the items looked "stuck on": each one is a separate
window laid over the bar.

## The proposal

1. **One owner draws the system bar.** The right part of the screen's title bar
   is the system bar. One program draws every item in it, in the bar's own
   refresh, on one grid (Common/traygrid.h), with the theme's pens and the same
   text path as the title. No item opens a window over the bar.
2. **Programs ask for a place with a library call.** A small API (below) adds,
   changes and removes an item: a picture (one per state), a help line, an
   optional text (the clock is a text item), an optional menu, and where in the
   row it wants to be. The owner lays the row out and closes it up when an item
   goes.
3. **Commodities are shown there.** Every commodity is already registered with
   commodities.library (CxBroker), with a name, a title and a description, and
   understands Exchange's commands (show, hide, enable, disable, quit). The
   owner lists the brokers and gives every running commodity an item (Dale,
   12 Nov 00:57: every commodity; under GFOS we run so few).
   The item's menu is Exchange's commands for that commodity; a click sends
   CXCMD_APPEAR. So GFOpenBlanker, GFOpenSnap, GFOpenWindows, GFOpenSpeaker and
   any third-party commodity can show in the bar without each of them drawing
   anything.
4. **Menus come from the item.** An item's menu is a list of entries the program
   gives (or Exchange's commands for a commodity). The owner shows it as a
   drop-down under the item, drawn through GFOpenLayout like the other menus,
   and sends the chosen entry back to the program's port.
5. **The depth gadget is the bar's end.** It is drawn as part of the bar
   (amicradle-guest #707), so the row ends at it with the grid's spacing.

### Who owns it

GFOpenTitle on disk owns the bar (Dale, 12 November 2026 00:59). It already
hosts other programs' icons (taskspace); it draws every item in one row window
instead of one window per item. GFOpenSpeaker adds the speaker item through the
port and keeps only its volume panel. The ROM's GFOpenBar is unchanged: it stays
the fallback that draws the clock, network, speaker and cog while GFOpenTitle
isn't running, so the ROM doesn't grow.

### The API

Two ways in, both answered by the same owner:

- **commodities.library**: every broker gets an item, with no flag and no
  change to the commodity (Dale, 12 Nov 00:57: every commodity). The owner
  takes the broker's title for the item's name and a picture from the
  commodity's icon (its tool types may name a smaller one); a commodity that
  wants its own live picture sends it through taskspace version 2.
- **A port message for other programs**: the taskspace protocol
  (TitleBar/include/taskspace.h, version 1) grows into version 2: text items, a
  menu, and up to the grid's 16 x 16 pictures in 32-bit colour on RTG screens.
  Version 1 messages keep working.

The tray files in `ENV:GFOpenMenus/Tray` stay only for GFOpenMenus' bar at a
screen edge, written by the owner.

## Text: anti-aliased, hinted, through GFOpenLayout

Team 5's note (teams/team3/inbox/20261111-1948-from-team5-fonts.md, Dale 19:44):
icon labels and the screen title are drawn one bit per pixel from DejaVuSans,
unhinted, with uneven strokes.

1. **One text path.** Labels, window titles, the screen title and the system
   bar, menus and requesters measure and place text through GFOpenLayout (the
   UI drawing layer, Dale 11 November 15:56) and draw glyphs from
   openfont.library (OF_RasterGlyphMatrix), the same engine as bullet.library.
2. **Grey levels on RTG.** On a 15, 16, 24 or 32-bit screen each glyph is drawn
   as 8-bit coverage, blended onto what is under it (the bar's gradient, the
   backdrop picture), never scaled from a bitmap.
3. **Hinting at the real pixel size.** The glyph is rasterised at the screen's
   pixel size (the font prefs' size and the screen's DPI), with the outline's
   hints applied, so the stems of "m", "s" and "a" come out even.
4. **Shadow or outline over the backdrop.** A Workbench prefs switch: none, a
   1-pixel shadow (down right) or a 1-pixel outline, in the theme's shadow pen,
   for icon labels on a picture.
5. **Planar screens keep the 1-bit path.** On chip-RAM screens (8 to 256 pens),
   text stays as today's 1-bit rendering.

Before and after crops come from the 8 GB card (Team 5) once the first step is
built.

## Steps (each its own PR, each tested in a lab)

1. The owner: GFOpenTitle draws every item from one list, in one window; the speaker, network,
   cog and clock become items. GFOpenTitle and GFOpenSpeaker add theirs through
   the port instead of drawing.
2. Taskspace version 2 (text items, menus, 32-bit pictures).
3. Commodities in the bar (every broker, Exchange's commands as the item's
   menu).
4. The text path: AA and hinting for the bar and the screen title first, then
   icon labels, window titles, menus and requesters.

Team 3
