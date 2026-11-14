/* libgfopensysbar.a: gfopensysbar.library's calls as functions, for a program that links them in place of the
 * inline header. The program keeps SysBarBase, opened with OpenLibrary(SYSBARNAME, SYSBAR_VERSION). Each name is
 * in brackets where it is defined, so the inline header's macro of that name isn't used there.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#define SYSBAR_NO_INLINE
#include <proto/gfopensysbar.h>
#include <inline/gfopensysbar.h>

struct SysBarItem *(SB_AddItemA)(const struct TagItem *tags) { return SB_AddItemA(tags); }
LONG (SB_SetItemA)(struct SysBarItem *item, const struct TagItem *tags) { return SB_SetItemA(item, tags); }
LONG (SB_RemItem)(struct SysBarItem *item) { return SB_RemItem(item); }
LONG (SB_Own)(ULONG signal) { return SB_Own(signal); }
LONG (SB_Disown)(void) { return SB_Disown(); }
ULONG (SB_LockList)(void) { return SB_LockList(); }
struct SysBarItem *(SB_NextItem)(struct SysBarItem *prev) { return SB_NextItem(prev); }
void (SB_UnlockList)(void) { SB_UnlockList(); }
LONG (SB_SendEvent)(struct SysBarItem *item, ULONG kind, ULONG entry, const WORD *box) { return SB_SendEvent(item, kind, entry, box); }
void (SB_DoneEvent)(struct SysBarEvent *event) { SB_DoneEvent(event); }
STRPTR (SB_MenuEntry)(struct SysBarItem *item, ULONG n) { return SB_MenuEntry(item, n); }
struct SysBarItem *(SB_FindItem)(CONST_STRPTR name) { return SB_FindItem(name); }
