/* gfopensysbar.library's core: the system bar's item list, kept by the library so items outlive the bar's
 * owner (designs/SYSTEM-BAR-LIBRARY-20261114.md in amicradle-teams; Dale's go, 14 November 2026 03:08).
 * Plain C with no AmigaOS calls, so the host tests (tests/test_sb_core.c) run it as the library does; the
 * library (library/sysbar_lib.c) puts its semaphore round every call and gives it exec's memory.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SB_CORE_H
#define SB_CORE_H

#include "../include/libraries/gfopensysbar_items.h"

/* tag walking, as utility.library's NextTagItem has it */
#define SB_TAG_DONE   0UL
#define SB_TAG_IGNORE 1UL
#define SB_TAG_MORE   2UL
#define SB_TAG_SKIP   3UL

/* the layout of a struct TagItem (ULONG and pointers are 32 bits on the Amiga; long on the host holds a pointer) */
typedef struct sb_tag { unsigned long tag, data; } sb_tag;

typedef struct sb_mem {
    void *(*alloc)(void *ctx, unsigned long size);     /* cleared memory, or NULL */
    void  (*free)(void *ctx, void *p, unsigned long size);
    void  *ctx;
} sb_mem;

typedef struct sb_list {
    struct SysBarItem *sl_First;
    unsigned short sl_Count;
    unsigned long  sl_Seq;
    unsigned long  sl_Gen;                     /* goes up on every change */
    unsigned long  sl_Changed;                 /* SBC_* since the owner last took them */
    void          *sl_Owner;                   /* the owner's task, or NULL */
    unsigned long  sl_OwnerSig;                /* the owner's signal number */
    sb_mem         mem;
} sb_list;

/* the fields a tag list gives, before they are checked */
typedef struct sb_spec {
    unsigned long have;                        /* SBC_* of what the tags named; SB_HAVE_NAME for the name */
    const char *name, *help, *text;
    long width, height, states, state, order;
    const unsigned char *pixels;
    const unsigned long *rgb;
    const char *const *menu;
    void *port;
    unsigned long userdata;
    int  have_userdata;
    long *errp;                                /* SBA_Error */
} sb_spec;
#define SB_HAVE_NAME 0x8000

void sb_init(sb_list *l, const sb_mem *mem);
void sb_spec_tags(sb_spec *s, const sb_tag *tags);            /* fills s from the tags (s is cleared first) */
struct SysBarItem *sb_add(sb_list *l, const sb_tag *tags, void *task, int *err);
int  sb_set(sb_list *l, struct SysBarItem *it, const sb_tag *tags);
int  sb_remove(sb_list *l, struct SysBarItem *it);
int  sb_reap(sb_list *l, int (*alive)(void *ctx, void *task), void *ctx);   /* items whose task has gone: how many */
int  sb_remove_task(sb_list *l, void *task);                   /* a program's items, all at once */
int  sb_known(const sb_list *l, const struct SysBarItem *it);
struct SysBarItem *sb_find(const sb_list *l, const char *name);
const char *sb_menu_entry(const struct SysBarItem *it, unsigned n);   /* NULL past the last */
int  sb_own(sb_list *l, void *task, unsigned long sig);
int  sb_disown(sb_list *l, void *task);
unsigned long sb_take_changes(sb_list *l);                     /* the owner's: the SBC_* bits, cleared */
void sb_free_all(sb_list *l);

#endif
