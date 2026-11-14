/* gfopensysbar.library 1.0: the system bar's items, kept here so they outlive the bar's owner
 * (designs/SYSTEM-BAR-LIBRARY-20261114.md in amicradle-teams, Dale's go 14 November 2026 03:08; SysBar/DESIGN.md).
 * The list itself is core/sb_core.c, host-tested; this file gives it exec's memory, a semaphore round every call,
 * the owner's signal, the check that an item's task is still there, and the events.
 * Built bare by build-amiga.sh (no startup code or C library). On disk in LIBS:; nothing of it is in the ROM.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <exec/semaphores.h>
#include <exec/tasks.h>
#include <utility/tagitem.h>
#define __NOLIBBASE__
#include <proto/exec.h>

#include <libraries/gfopensysbar.h>
#include "../core/sb_core.h"

#define REG(r, decl) register decl __asm(#r)
#define LIB_VERSION   1
#define LIB_REVISION  0

struct ExecBase *SysBase;

static const char lib_name[] = SYSBARNAME;
static const char lib_id[] = "gfopensysbar.library 1.0 (14.11.2026) AmiCradle Team\r\n";
static const char vstring[] __attribute__((used)) = "$VER: gfopensysbar.library 1.0 (14.11.2026) AmiCradle Team";

struct SysBarBase {
    struct Library lib;
    struct SignalSemaphore lock;
    sb_list list;
    BPTR seglist;
};

_Static_assert(sizeof(sb_tag) == sizeof(struct TagItem), "sb_tag is a TagItem");

static void *m_alloc(void *ctx, unsigned long n) { (void)ctx; return AllocVec(n, MEMF_PUBLIC | MEMF_CLEAR); }
static void m_free(void *ctx, void *p, unsigned long n) { (void)ctx; (void)n; FreeVec(p); }

/* the task is still there: the one running, or one on exec's lists */
static int on_list(struct List *l, struct Task *t)
{
    struct Node *n;
    for (n = l->lh_Head; n->ln_Succ; n = n->ln_Succ) if ((struct Task *)n == t) return 1;
    return 0;
}
static int alive(void *ctx, void *task)
{
    struct Task *t = task;
    int r;
    (void)ctx;
    Disable();
    r = t == SysBase->ThisTask || on_list(&SysBase->TaskReady, t) || on_list(&SysBase->TaskWait, t);
    Enable();
    return r;
}

/* after a change: the owner is told by its signal */
static void tell(struct SysBarBase *b, unsigned long gen)
{
    if (b->list.sl_Gen != gen && b->list.sl_Owner && b->list.sl_Owner != FindTask(NULL))
        Signal((struct Task *)b->list.sl_Owner, 1UL << b->list.sl_OwnerSig);
}

static struct SysBarItem *L_AddItemA(REG(a0, const struct TagItem *tags), REG(a6, struct SysBarBase *b))
{
    struct SysBarItem *it;
    unsigned long gen;
    int err;
    ObtainSemaphore(&b->lock);
    gen = b->list.sl_Gen;
    it = sb_add(&b->list, (const sb_tag *)tags, FindTask(NULL), &err);
    tell(b, gen);
    ReleaseSemaphore(&b->lock);
    return it;
}

static LONG L_SetItemA(REG(a0, struct SysBarItem *it), REG(a1, const struct TagItem *tags), REG(a6, struct SysBarBase *b))
{
    unsigned long gen;
    LONG r;
    ObtainSemaphore(&b->lock);
    gen = b->list.sl_Gen;
    r = sb_set(&b->list, it, (const sb_tag *)tags);
    tell(b, gen);
    ReleaseSemaphore(&b->lock);
    return r;
}

static LONG L_RemItem(REG(a0, struct SysBarItem *it), REG(a6, struct SysBarBase *b))
{
    unsigned long gen;
    LONG r;
    ObtainSemaphore(&b->lock);
    gen = b->list.sl_Gen;
    r = sb_remove(&b->list, it);
    tell(b, gen);
    ReleaseSemaphore(&b->lock);
    return r;
}

static LONG L_Own(REG(d0, ULONG sig), REG(a6, struct SysBarBase *b))
{
    LONG r;
    if (sig > 31) return SBR_BADARGS;
    ObtainSemaphore(&b->lock);
    sb_reap(&b->list, alive, NULL);            /* an owner that has gone lets go first */
    r = sb_own(&b->list, FindTask(NULL), sig);
    ReleaseSemaphore(&b->lock);
    return r;
}

static LONG L_Disown(REG(a6, struct SysBarBase *b))
{
    LONG r;
    ObtainSemaphore(&b->lock);
    r = sb_disown(&b->list, FindTask(NULL));
    ReleaseSemaphore(&b->lock);
    return r;
}

/* the owner reads: items whose task has gone are taken out first */
static ULONG L_LockList(REG(a6, struct SysBarBase *b))
{
    ObtainSemaphore(&b->lock);
    sb_reap(&b->list, alive, NULL);
    return sb_take_changes(&b->list);
}

static struct SysBarItem *L_NextItem(REG(a0, struct SysBarItem *prev), REG(a6, struct SysBarBase *b))
{
    return prev ? prev->si_Next : b->list.sl_First;
}

static void L_UnlockList(REG(a6, struct SysBarBase *b))
{
    ReleaseSemaphore(&b->lock);
}

static LONG L_SendEvent(REG(a0, struct SysBarItem *it), REG(d0, ULONG kind), REG(d1, ULONG entry), REG(a1, const WORD *box),
                        REG(a6, struct SysBarBase *b))
{
    struct SysBarEvent *ev;
    LONG r = SBR_OK;
    int i;
    ObtainSemaphore(&b->lock);
    if (!sb_known(&b->list, it)) r = SBR_UNKNOWN;
    else if (!it->si_Port || (kind == SBE_MENU && entry >= it->si_MenuCount) || kind < SBE_CLICK || kind > SBE_PLACE) r = SBR_BADARGS;
    else if (!(ev = AllocVec(sizeof *ev, MEMF_PUBLIC | MEMF_CLEAR))) r = SBR_NOMEM;
    else {
        ev->sbe_Message.mn_Node.ln_Type = NT_MESSAGE;
        ev->sbe_Message.mn_Length = sizeof *ev;
        ev->sbe_Magic = SBE_MAGIC;
        ev->sbe_Kind = (UWORD)kind;
        ev->sbe_Entry = (UWORD)entry;
        ev->sbe_Item = it;
        ev->sbe_UserData = it->si_UserData;
        for (i = 0; i < SB_NAMELEN; i++) ev->sbe_Name[i] = it->si_Name[i];
        if (box) { ev->sbe_Left = box[0]; ev->sbe_Top = box[1]; ev->sbe_Width = box[2]; ev->sbe_Height = box[3]; }
        PutMsg((struct MsgPort *)it->si_Port, &ev->sbe_Message);
    }
    ReleaseSemaphore(&b->lock);
    return r;
}

static void L_DoneEvent(REG(a0, struct SysBarEvent *ev), REG(a6, struct SysBarBase *b))
{
    (void)b;
    if (ev && ev->sbe_Magic == SBE_MAGIC) { ev->sbe_Magic = 0; FreeVec(ev); }
}

static STRPTR L_MenuEntry(REG(a0, struct SysBarItem *it), REG(d0, ULONG n), REG(a6, struct SysBarBase *b))
{
    (void)b;
    return (STRPTR)sb_menu_entry(it, n);
}

/* the bar's owner's task, or NULL: a program can tell whether anyone will draw its items (an owner that ended
 * counts as none) */
static APTR L_OwnerTask(REG(a6, struct SysBarBase *b))
{
    APTR t;
    ObtainSemaphore(&b->lock);
    if ((t = sb_owner(&b->list)) && !alive(NULL, t)) { sb_disown(&b->list, t); t = NULL; }
    ReleaseSemaphore(&b->lock);
    return t;
}

static struct SysBarItem *L_FindItem(REG(a0, CONST_STRPTR name), REG(a6, struct SysBarBase *b))
{
    return sb_find(&b->list, (const char *)name);
}

/* ---- the library itself ------------------------------------------------------------ */

static struct Library *L_Init(REG(d0, struct SysBarBase *b), REG(a0, BPTR seglist), REG(a6, struct ExecBase *sys))
{
    static const sb_mem mem = { m_alloc, m_free, 0 };
    SysBase = sys;
    b->lib.lib_Node.ln_Type = NT_LIBRARY;
    b->lib.lib_Node.ln_Name = (char *)lib_name;
    b->lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    b->lib.lib_Version = LIB_VERSION;
    b->lib.lib_Revision = LIB_REVISION;
    b->lib.lib_IdString = (APTR)lib_id;
    b->seglist = seglist;
    InitSemaphore(&b->lock);
    sb_init(&b->list, &mem);
    return &b->lib;
}

static struct Library *L_Open(REG(a6, struct SysBarBase *b))
{
    b->lib.lib_OpenCnt++;
    b->lib.lib_Flags &= ~LIBF_DELEXP;
    return &b->lib;
}

/* only with no openers and no items left: the items are the bar's, whoever opened it */
static BPTR L_Expunge(REG(a6, struct SysBarBase *b))
{
    BPTR seg;
    if (b->lib.lib_OpenCnt || b->list.sl_First || b->list.sl_Owner) { b->lib.lib_Flags |= LIBF_DELEXP; return 0; }
    seg = b->seglist;
    Remove(&b->lib.lib_Node);
    FreeMem((UBYTE *)b - b->lib.lib_NegSize, b->lib.lib_NegSize + b->lib.lib_PosSize);
    return seg;
}

static BPTR L_Close(REG(a6, struct SysBarBase *b))
{
    if (b->lib.lib_OpenCnt) b->lib.lib_OpenCnt--;
    if (!b->lib.lib_OpenCnt && (b->lib.lib_Flags & LIBF_DELEXP)) return L_Expunge(b);
    return 0;
}

static ULONG L_Null(void) { return 0; }

static const APTR lib_vectors[] = {
    (APTR)L_Open, (APTR)L_Close, (APTR)L_Expunge, (APTR)L_Null,
    (APTR)L_AddItemA,       /* -30 */
    (APTR)L_SetItemA,       /* -36 */
    (APTR)L_RemItem,        /* -42 */
    (APTR)L_Own,            /* -48 */
    (APTR)L_Disown,         /* -54 */
    (APTR)L_LockList,       /* -60 */
    (APTR)L_NextItem,       /* -66 */
    (APTR)L_UnlockList,     /* -72 */
    (APTR)L_SendEvent,      /* -78 */
    (APTR)L_DoneEvent,      /* -84 */
    (APTR)L_MenuEntry,      /* -90 */
    (APTR)L_FindItem,       /* -96 */
    (APTR)L_OwnerTask,      /* -102 */
    (APTR)-1,
};
static const struct { ULONG size; const APTR *vectors; APTR data; APTR init; } lib_inittable = {
    sizeof(struct SysBarBase), lib_vectors, NULL, (APTR)L_Init,
};
const struct Resident lib_romtag __attribute__((used)) = {
    RTC_MATCHWORD, (struct Resident *)&lib_romtag, (APTR)(&lib_romtag + 1),
    RTF_AUTOINIT, LIB_VERSION, NT_LIBRARY, 0, (char *)lib_name, (char *)lib_id, (APTR)&lib_inittable,
};
