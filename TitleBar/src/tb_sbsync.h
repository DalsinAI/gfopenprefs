/* GFOpenTitle 0.10: what the row does with one of gfopensysbar.library's items at a read of the list, and which
 * library event a taskspace event becomes. Plain C, host-tested (tests/test_tb.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef TB_SBSYNC_H
#define TB_SBSYNC_H

enum {
    TBS_SKIP,       /* not shown: no picture, or the taskspace already has an icon of that name */
    TBS_NEW,        /* a new icon in a free slot */
    TBS_FULL,       /* new, but every slot is taken */
    TBS_KEEP,       /* shown as it is (its state and help may still change) */
    TBS_REDRAW,     /* the same place, a new picture */
    TBS_MOVE        /* a new width or order: closed and opened again where it now goes */
};

/* slot: the icon of that name in the row, or -1; slot_is_lib: that icon is a library item's; free_slot: a slot is
 * free; has_picture: the item has pixels; same_place: its width and order are the icon's; same_picture: its pixels,
 * height, states and colours are */
static inline int tbs_plan(int slot, int slot_is_lib, int free_slot, int has_picture, int same_place, int same_picture)
{
    if (!has_picture) return TBS_SKIP;
    if (slot < 0) return free_slot ? TBS_NEW : TBS_FULL;
    if (!slot_is_lib) return TBS_SKIP;
    if (!same_place) return TBS_MOVE;
    return same_picture ? TBS_KEEP : TBS_REDRAW;
}

/* a taskspace event as the library's (SBE_CLICK 1, SBE_PLACE 3); 0: none (TSE_GONE: the item stays the library's) */
static inline int tbs_event(int tse_kind)
{
    return tse_kind == 1 ? 1 : tse_kind == 3 ? 3 : 0;
}

/* after the lock (drawing or closing a window under it would hold up every SB_SetItemA): what a library icon's
 * window gets. seen: 0 gone from the list, 1 still there, 2 there and moved; has_win: it has a window now */
enum { TBA_NONE, TBA_DROP, TBA_REOPEN, TBA_OPEN, TBA_DRAW };
static inline int tbs_after(int is_lib, int seen, int has_win)
{
    if (!is_lib) return TBA_NONE;
    if (!seen) return TBA_DROP;
    if (!has_win) return TBA_OPEN;
    return seen == 2 ? TBA_REOPEN : TBA_DRAW;
}

/* every match takes the list's item and task again: a program that removed its item and added one of the same name
 * between two reads has a new item and maybe a new task (Marcus's review). 1: the task changed (the systray looks
 * again) */
static inline int tbs_rebind(void **sbi, void **task, void *item, void *item_task)
{
    int moved = *task != item_task;
    *sbi = item;
    *task = item_task;
    return moved;
}

#endif
