/* gfopensysbar.library: the system bar, the right end of the screen's title bar, as items that programs ask
 * for and one owner (GFOpenTitle) draws. The items are in libraries/gfopensysbar_items.h; this header adds the
 * events the owner sends to an item's port. See SysBar/DESIGN.md.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef LIBRARIES_GFOPENSYSBAR_H
#define LIBRARIES_GFOPENSYSBAR_H

#include <exec/types.h>
#include <exec/ports.h>
#include <libraries/gfopensysbar_items.h>

#define SYSBARNAME     "gfopensysbar.library"
#define SYSBAR_VERSION 1

#define SBE_MAGIC  0x53424556UL               /* 'SBEV' */
#define SBE_CLICK  1                          /* the item was clicked */
#define SBE_MENU   2                          /* sbe_Entry of its menu was chosen */
#define SBE_PLACE  3                          /* where the item is on the screen now (sbe_Left...) */

/* An event, on the item's SBA_Port. Not replied: give it back with SB_DoneEvent once read (ReplyMsg would leak it:
 * it has no reply port). */
struct SysBarEvent {
    struct Message sbe_Message;
    ULONG  sbe_Magic;                         /* SBE_MAGIC */
    UWORD  sbe_Kind;                          /* SBE_* */
    UWORD  sbe_Entry;                         /* SBE_MENU: the entry, from 0 */
    struct SysBarItem *sbe_Item;              /* which item; it may have gone by now, and its memory be another's:
                                               * compare it with yours, never read it. sbe_Name is the one to trust */
    ULONG  sbe_UserData;                      /* the item's SBA_UserData */
    char   sbe_Name[SB_NAMELEN];              /* the item's name */
    WORD   sbe_Left, sbe_Top, sbe_Width, sbe_Height;   /* the item's box on the screen */
};

#endif
