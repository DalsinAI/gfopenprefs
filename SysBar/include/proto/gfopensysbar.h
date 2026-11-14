/* gfopensysbar.library for programs: the prototypes and, with GCC on the 68k, the calls. Define SYSBAR_BASE_NAME
 * to use a base of another name. Or link SysBar's libgfopensysbar.a and call the functions.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef PROTO_GFOPENSYSBAR_H
#define PROTO_GFOPENSYSBAR_H

#include <clib/gfopensysbar_protos.h>
#ifndef __NOLIBBASE__
extern struct Library *SysBarBase;
#endif
#if defined(__GNUC__) && defined(__mc68000__) && !defined(SYSBAR_NO_INLINE)
#include <inline/gfopensysbar.h>
#endif

#endif
