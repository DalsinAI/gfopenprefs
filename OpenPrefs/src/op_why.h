/* op_why.h: what GFOpenPrefs says when a page can't start (op_why.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OP_WHY_H
#define OP_WHY_H
#include <stddef.h>

void op_note_lib(char *list, size_t size, const char *name, unsigned long version);
void op_why(char *text, size_t size, const char *page, int rc, const char *missing);

#endif
