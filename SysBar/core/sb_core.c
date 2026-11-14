/* gfopensysbar.library's core: the item list (sb_core.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include "sb_core.h"

static unsigned sb_len(const char *s) { unsigned n = 0; while (s[n]) n++; return n; }
static int sb_same(const char *a, const char *b) { while (*a && *a == *b) a++, b++; return *a == *b; }
static void sb_put(char *d, const char *s, unsigned max)    /* s, cut to max - 1 */
{
    unsigned i = 0;
    for (; s && s[i] && i < max - 1; i++) d[i] = s[i];
    d[i] = 0;
}
/* byte loops, not structure copies: the library is built bare, with no memcpy or memset to call */
static void sb_copy(void *d, const void *s, unsigned long n)
{
    unsigned char *a = d; const unsigned char *b = s;
    while (n--) *a++ = *b++;
}
static void sb_clear(void *d, unsigned long n)
{
    volatile unsigned char *a = d;
    while (n--) *a++ = 0;
}

void sb_init(sb_list *l, const sb_mem *mem)
{
    sb_clear(l, sizeof *l);
    sb_copy(&l->mem, mem, sizeof *mem);
}

void sb_spec_tags(sb_spec *s, const sb_tag *t)
{
    sb_clear(s, sizeof *s);
    while (t) {
        unsigned long d = t->data;
        switch (t->tag) {
        case SB_TAG_DONE: return;
        case SB_TAG_IGNORE: break;
        case SB_TAG_MORE: t = (const sb_tag *)d; continue;
        case SB_TAG_SKIP: t += d + 1; continue;
        case SBA_Name:     s->name = (const char *)d; s->have |= SB_HAVE_NAME; break;
        case SBA_Help:     s->help = (const char *)d; s->have |= SBC_HELP; break;
        case SBA_Text:     s->text = (const char *)d; s->have |= SBC_TEXT; break;
        case SBA_Width:    s->width = (long)d; s->have |= SBC_PICTURE; break;
        case SBA_Height:   s->height = (long)d; s->have |= SBC_PICTURE; break;
        case SBA_States:   s->states = (long)d; s->have |= SBC_PICTURE; break;
        case SBA_Pixels:   s->pixels = (const unsigned char *)d; s->have |= SBC_PICTURE; break;
        case SBA_RGB:      s->rgb = (const unsigned long *)d; s->have |= SBC_PICTURE; break;
        case SBA_State:    s->state = (long)d; s->have |= SBC_STATE; break;
        case SBA_Menu:     s->menu = (const char *const *)d; s->have |= SBC_MENU; break;
        case SBA_Order:    s->order = (long)(short)d; s->have |= SBC_ORDER; break;
        case SBA_Port:     s->port = (void *)d; s->have |= SBC_PORT; break;
        case SBA_UserData: s->userdata = d; s->have_userdata = 1; break;
        case SBA_Error:    s->errp = (long *)d; break;
        default: break;                       /* a tag of a later version: left alone */
        }
        t++;
    }
}

static void sb_unlink(sb_list *l, struct SysBarItem *it)
{
    struct SysBarItem **p = &l->sl_First;
    while (*p && *p != it) p = &(*p)->si_Next;
    if (*p) *p = it->si_Next;
    it->si_Next = 0;
}

/* in the row's order: by si_Order, then by when it was added */
static void sb_link(sb_list *l, struct SysBarItem *it)
{
    struct SysBarItem **p = &l->sl_First;
    while (*p && ((*p)->si_Order < it->si_Order || ((*p)->si_Order == it->si_Order && (*p)->si_Seq < it->si_Seq)))
        p = &(*p)->si_Next;
    it->si_Next = *p;
    *p = it;
}

static unsigned long sb_pixbytes(long w, long h, long n) { return (unsigned long)(w * h * n); }

static unsigned long sb_menubytes(const char *const *m, unsigned *count)
{
    unsigned long n = 0;
    unsigned c = 0;
    while (m[c]) {
        unsigned k = sb_len(m[c]);
        n += (k > SB_MENULEN - 1 ? SB_MENULEN - 1 : k) + 1;
        c++;
        if (c > SB_MENUMAX) break;
    }
    *count = c;
    return n;
}

static void sb_free_menu(sb_list *l, struct SysBarItem *it)
{
    if (it->si_Menu) {
        const char *p = it->si_Menu;
        unsigned i;
        for (i = 0; i < it->si_MenuCount; i++) p += sb_len(p) + 1;
        l->mem.free(l->mem.ctx, it->si_Menu, (unsigned long)(p - it->si_Menu));
    }
    it->si_Menu = 0;
    it->si_MenuCount = 0;
}

static void sb_free_item(sb_list *l, struct SysBarItem *it)
{
    if (it->si_Pixels) l->mem.free(l->mem.ctx, it->si_Pixels, sb_pixbytes(it->si_Width, it->si_Height, it->si_States));
    sb_free_menu(l, it);
    l->mem.free(l->mem.ctx, it, sizeof *it);
}

/* s onto it: everything checked and every buffer had before anything changes, so a failure changes nothing */
static int sb_apply(sb_list *l, struct SysBarItem *it, const sb_spec *s, unsigned long *changed)
{
    long w = it->si_Width, h = it->si_Height, n = it->si_States ? it->si_States : 1, st = it->si_State;
    unsigned char *pix = 0;
    char *menu = 0;
    unsigned long menun = 0;
    unsigned count = 0;
    unsigned long ch = 0;
    if (s->have & SBC_PICTURE) {
        int sized = 0;
        if (s->width)  { w = s->width; sized = 1; }
        if (s->height) { h = s->height; sized = 1; }
        if (s->states) { n = s->states; sized = 1; }
        if (w < 1 || w > SB_MAXW || h < 1 || h > SB_MAXH || n < 1 || n > SB_STATES) return SBR_BADARGS;
        if (sized && !s->pixels && it->si_Pixels) return SBR_BADARGS;       /* a new size needs new pixels */
        if (s->pixels) {
            if (!(pix = l->mem.alloc(l->mem.ctx, sb_pixbytes(w, h, n)))) return SBR_NOMEM;
            sb_copy(pix, s->pixels, sb_pixbytes(w, h, n));
        }
        if (st >= n) st = 0;
        ch |= SBC_PICTURE;
    }
    if (s->have & SBC_STATE) {
        if (s->state < 0 || s->state >= n) goto bad;
        if (s->state != it->si_State) ch |= SBC_STATE;
        st = s->state;
    }
    if ((s->have & SBC_MENU) && s->menu) {
        menun = sb_menubytes(s->menu, &count);
        if (count > SB_MENUMAX) goto bad;
        if (count) {
            char *p;
            unsigned i;
            if (!(menu = l->mem.alloc(l->mem.ctx, menun))) {
                if (pix) l->mem.free(l->mem.ctx, pix, sb_pixbytes(w, h, n));
                return SBR_NOMEM;
            }
            for (p = menu, i = 0; i < count; i++) { sb_put(p, s->menu[i], SB_MENULEN); p += sb_len(p) + 1; }
        }
    }
    /* nothing can fail from here */
    if (s->have & SBC_PICTURE) {
        if (pix) {
            if (it->si_Pixels) l->mem.free(l->mem.ctx, it->si_Pixels, sb_pixbytes(it->si_Width, it->si_Height, it->si_States));
            it->si_Pixels = pix;
        }
        it->si_Width = (unsigned short)w; it->si_Height = (unsigned short)h; it->si_States = (unsigned short)n;
        if (s->rgb) sb_copy(it->si_RGB, s->rgb, sizeof it->si_RGB);
    }
    it->si_State = (unsigned short)st;
    if (s->have & SBC_MENU) { sb_free_menu(l, it); it->si_Menu = menu; it->si_MenuCount = (unsigned short)count; ch |= SBC_MENU; }
    if (s->have & SBC_HELP) { sb_put(it->si_Help, s->help, SB_HELPLEN); ch |= SBC_HELP; }
    if (s->have & SBC_TEXT) {
        char t[SB_TEXTLEN];
        sb_put(t, s->text, SB_TEXTLEN);
        if (!sb_same(t, it->si_Text)) { sb_put(it->si_Text, t, SB_TEXTLEN); ch |= SBC_TEXT; }
    }
    if (s->have & SBC_PORT) { it->si_Port = s->port; ch |= SBC_PORT; }
    if (s->have_userdata) it->si_UserData = s->userdata;
    if ((s->have & SBC_ORDER) && s->order != it->si_Order) {
        it->si_Order = (short)s->order;
        ch |= SBC_ORDER;
    }
    *changed = ch;
    return SBR_OK;
bad:
    if (pix) l->mem.free(l->mem.ctx, pix, sb_pixbytes(w, h, n));
    return SBR_BADARGS;
}

static void sb_changed(sb_list *l, unsigned long ch)
{
    if (!ch) return;
    l->sl_Changed |= ch;
    l->sl_Gen++;
}

static struct SysBarItem *sb_add_spec(sb_list *l, const sb_spec *sp, void *task, int *err)
{
    sb_spec s;
    struct SysBarItem *it;
    unsigned long ch = 0;
    int r;
    sb_copy(&s, sp, sizeof s);
    if (!s.name || !s.name[0] || sb_len(s.name) > SB_NAMELEN - 1) { *err = SBR_BADARGS; return 0; }
    if (sb_find(l, s.name)) { *err = SBR_DUP; return 0; }
    if (l->sl_Count >= SB_MAXITEMS) { *err = SBR_FULL; return 0; }
    if ((s.have & SBC_PICTURE) && !s.pixels) { *err = SBR_BADARGS; return 0; }   /* a picture comes with its pixels */
    if (!(it = l->mem.alloc(l->mem.ctx, sizeof *it))) { *err = SBR_NOMEM; return 0; }
    sb_put(it->si_Name, s.name, SB_NAMELEN);
    it->si_Task = task;
    it->si_Seq = l->sl_Seq;
    if ((r = sb_apply(l, it, &s, &ch)) != SBR_OK) { sb_free_item(l, it); *err = r; return 0; }
    l->sl_Seq++;
    sb_link(l, it);
    l->sl_Count++;
    sb_changed(l, SBC_ADD);
    *err = SBR_OK;
    return it;
}

int sb_known(const sb_list *l, const struct SysBarItem *it)
{
    const struct SysBarItem *p;
    for (p = l->sl_First; p; p = p->si_Next) if (p == it) return 1;
    return 0;
}

struct SysBarItem *sb_add(sb_list *l, const sb_tag *tags, void *task, int *err)
{
    sb_spec s;
    struct SysBarItem *it;
    sb_spec_tags(&s, tags);
    it = sb_add_spec(l, &s, task, err);
    if (s.errp) *s.errp = *err;
    return it;
}

static int sb_set_spec(sb_list *l, struct SysBarItem *it, sb_spec *sp)
{
    sb_spec s;
    unsigned long ch = 0;
    int r;
    if (!it || !sb_known(l, it)) return SBR_UNKNOWN;
    sb_copy(&s, sp, sizeof s);
    s.have &= ~SB_HAVE_NAME;                   /* a name is the item's for good */
    if ((r = sb_apply(l, it, &s, &ch)) != SBR_OK) return r;
    if (ch & SBC_ORDER) { sb_unlink(l, it); sb_link(l, it); }
    sb_changed(l, ch);
    return SBR_OK;
}

int sb_set(sb_list *l, struct SysBarItem *it, const sb_tag *tags)
{
    sb_spec s;
    int r;
    sb_spec_tags(&s, tags);
    r = sb_set_spec(l, it, &s);
    if (s.errp) *s.errp = r;
    return r;
}

int sb_remove(sb_list *l, struct SysBarItem *it)
{
    if (!it || !sb_known(l, it)) return SBR_UNKNOWN;
    sb_unlink(l, it);
    l->sl_Count--;
    sb_free_item(l, it);
    sb_changed(l, SBC_REMOVE);
    return SBR_OK;
}

int sb_reap(sb_list *l, int (*alive)(void *ctx, void *task), void *ctx)
{
    struct SysBarItem *p = l->sl_First, *nx;
    int n = 0;
    for (; p; p = nx) {
        nx = p->si_Next;
        if (p->si_Task && !alive(ctx, p->si_Task)) { sb_remove(l, p); n++; }
    }
    if (l->sl_Owner && !alive(ctx, l->sl_Owner)) l->sl_Owner = 0;
    return n;
}

int sb_remove_task(sb_list *l, void *task)
{
    struct SysBarItem *p = l->sl_First, *nx;
    int n = 0;
    for (; p; p = nx) {
        nx = p->si_Next;
        if (p->si_Task == task) { sb_remove(l, p); n++; }
    }
    return n;
}

struct SysBarItem *sb_find(const sb_list *l, const char *name)
{
    struct SysBarItem *p;
    if (!name) return 0;
    for (p = l->sl_First; p; p = p->si_Next) if (sb_same(p->si_Name, name)) return p;
    return 0;
}

const char *sb_menu_entry(const struct SysBarItem *it, unsigned n)
{
    const char *p;
    if (!it || n >= it->si_MenuCount) return 0;
    for (p = it->si_Menu; n; n--) p += sb_len(p) + 1;
    return p;
}

int sb_own(sb_list *l, void *task, unsigned long sig)
{
    if (l->sl_Owner && l->sl_Owner != task) return SBR_OWNED;
    l->sl_Owner = task;
    l->sl_OwnerSig = sig;
    l->sl_Changed |= SBC_ADD;                  /* a new owner reads the whole list */
    return SBR_OK;
}

int sb_disown(sb_list *l, void *task)
{
    if (l->sl_Owner != task) return SBR_OWNED;
    l->sl_Owner = 0;
    return SBR_OK;
}

void *sb_owner(const sb_list *l) { return l->sl_Owner; }

unsigned long sb_take_changes(sb_list *l)
{
    unsigned long c = l->sl_Changed;
    l->sl_Changed = 0;
    return c;
}

void sb_free_all(sb_list *l)
{
    while (l->sl_First) sb_remove(l, l->sl_First);
}
