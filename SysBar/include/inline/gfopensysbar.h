/* gfopensysbar.library's calls for GCC on the 68k (clib/gfopensysbar_protos.h). The base is SYSBAR_BASE_NAME
 * (default SysBarBase). Arguments go into temporaries before the registers are loaded, so an argument may itself
 * be a library call. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef INLINE_GFOPENSYSBAR_H
#define INLINE_GFOPENSYSBAR_H

#ifndef SYSBAR_BASE_NAME
#define SYSBAR_BASE_NAME SysBarBase
#endif

#define __SB_B  register struct Library *_b __asm("a6") = (struct Library *)(__sb)
#define __SB_X  "d1", "cc", "memory"

#define SB_AddItemA(tags) ({ const struct TagItem *__t = (tags); void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    register struct SysBarItem *_r __asm("d0"); register const struct TagItem *_a0 __asm("a0") = __t; \
    __asm__ __volatile__ ("jsr -30(%%a6)" : "=r"(_r), "+r"(_a0) : "r"(_b) : "a1", __SB_X); _r; })

#define SB_SetItemA(item, tags) ({ struct SysBarItem *__i = (item); const struct TagItem *__t = (tags); \
    void *__sb = (SYSBAR_BASE_NAME); __SB_B; register LONG _r __asm("d0"); \
    register struct SysBarItem *_a0 __asm("a0") = __i; register const struct TagItem *_a1 __asm("a1") = __t; \
    __asm__ __volatile__ ("jsr -36(%%a6)" : "=r"(_r), "+r"(_a0), "+r"(_a1) : "r"(_b) : __SB_X); _r; })

#define SB_RemItem(item) ({ struct SysBarItem *__i = (item); void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    register LONG _r __asm("d0"); register struct SysBarItem *_a0 __asm("a0") = __i; \
    __asm__ __volatile__ ("jsr -42(%%a6)" : "=r"(_r), "+r"(_a0) : "r"(_b) : "a1", __SB_X); _r; })

#define SB_Own(sig) ({ ULONG __s = (sig); void *__sb = (SYSBAR_BASE_NAME); __SB_B; register LONG _r __asm("d0") = (LONG)__s; \
    __asm__ __volatile__ ("jsr -48(%%a6)" : "+r"(_r) : "r"(_b) : "a0", "a1", __SB_X); _r; })

#define SB_Disown() ({ void *__sb = (SYSBAR_BASE_NAME); __SB_B; register LONG _r __asm("d0"); \
    __asm__ __volatile__ ("jsr -54(%%a6)" : "=r"(_r) : "r"(_b) : "a0", "a1", __SB_X); _r; })

#define SB_LockList() ({ void *__sb = (SYSBAR_BASE_NAME); __SB_B; register ULONG _r __asm("d0"); \
    __asm__ __volatile__ ("jsr -60(%%a6)" : "=r"(_r) : "r"(_b) : "a0", "a1", __SB_X); _r; })

#define SB_NextItem(prev) ({ struct SysBarItem *__p = (prev); void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    register struct SysBarItem *_r __asm("d0"); register struct SysBarItem *_a0 __asm("a0") = __p; \
    __asm__ __volatile__ ("jsr -66(%%a6)" : "=r"(_r), "+r"(_a0) : "r"(_b) : "a1", __SB_X); _r; })

#define SB_UnlockList() ({ void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    __asm__ __volatile__ ("jsr -72(%%a6)" : : "r"(_b) : "d0", "a0", "a1", __SB_X); })

#define SB_SendEvent(item, kind, entry, box) ({ struct SysBarItem *__i = (item); ULONG __k = (kind), __e = (entry); \
    const WORD *__bx = (box); void *__sb = (SYSBAR_BASE_NAME); __SB_B; register LONG _r __asm("d0") = (LONG)__k; \
    register ULONG _d1 __asm("d1") = __e; register struct SysBarItem *_a0 __asm("a0") = __i; \
    register const WORD *_a1 __asm("a1") = __bx; \
    __asm__ __volatile__ ("jsr -78(%%a6)" : "+r"(_r), "+r"(_d1), "+r"(_a0), "+r"(_a1) : "r"(_b) : "cc", "memory"); _r; })

#define SB_DoneEvent(ev) ({ struct SysBarEvent *__v = (ev); void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    register struct SysBarEvent *_a0 __asm("a0") = __v; \
    __asm__ __volatile__ ("jsr -84(%%a6)" : "+r"(_a0) : "r"(_b) : "d0", "a1", __SB_X); })

#define SB_MenuEntry(item, n) ({ struct SysBarItem *__i = (item); ULONG __n = (n); void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    register STRPTR _r __asm("d0") = (STRPTR)__n; register struct SysBarItem *_a0 __asm("a0") = __i; \
    __asm__ __volatile__ ("jsr -90(%%a6)" : "+r"(_r), "+r"(_a0) : "r"(_b) : "a1", __SB_X); _r; })

#define SB_FindItem(name) ({ CONST_STRPTR __n = (name); void *__sb = (SYSBAR_BASE_NAME); __SB_B; \
    register struct SysBarItem *_r __asm("d0"); register CONST_STRPTR _a0 __asm("a0") = __n; \
    __asm__ __volatile__ ("jsr -96(%%a6)" : "=r"(_r), "+r"(_a0) : "r"(_b) : "a1", __SB_X); _r; })

#define SB_OwnerTask() ({ void *__sb = (SYSBAR_BASE_NAME); __SB_B; register APTR _r __asm("d0"); \
    __asm__ __volatile__ ("jsr -102(%%a6)" : "=r"(_r) : "r"(_b) : "a0", "a1", __SB_X); _r; })

#endif
