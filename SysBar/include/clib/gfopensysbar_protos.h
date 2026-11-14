/* gfopensysbar.library's calls (libraries/gfopensysbar.h has the items and the events). LVOs from -30, in this
 * order. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef CLIB_GFOPENSYSBAR_PROTOS_H
#define CLIB_GFOPENSYSBAR_PROTOS_H

#include <utility/tagitem.h>
#include <libraries/gfopensysbar.h>

/* programs */
struct SysBarItem *SB_AddItemA(const struct TagItem *tags);                 /* -30  a0; NULL and SBA_Error say why */
LONG   SB_SetItemA(struct SysBarItem *item, const struct TagItem *tags);    /* -36  a0 a1; SBR_* */
LONG   SB_RemItem(struct SysBarItem *item);                                 /* -42  a0 */
/* the owner */
LONG   SB_Own(ULONG signal);                                                /* -48  d0: told of changes by this signal */
LONG   SB_Disown(void);                                                     /* -54 */
ULONG  SB_LockList(void);                                                   /* -60  the SBC_* since the last lock */
struct SysBarItem *SB_NextItem(struct SysBarItem *prev);                    /* -66  a0; NULL: the first */
void   SB_UnlockList(void);                                                 /* -72 */
LONG   SB_SendEvent(struct SysBarItem *item, ULONG kind, ULONG entry, const WORD *box);   /* -78  a0 d0 d1 a1 */
/* both */
void   SB_DoneEvent(struct SysBarEvent *event);                             /* -84  a0 */
STRPTR SB_MenuEntry(struct SysBarItem *item, ULONG n);                      /* -90  a0 d0; under the lock */
struct SysBarItem *SB_FindItem(CONST_STRPTR name);                          /* -96  a0; under the lock */
APTR   SB_OwnerTask(void);                                                  /* -102 the owner's task, or NULL */

#endif
