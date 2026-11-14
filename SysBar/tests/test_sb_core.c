/* Host tests for gfopensysbar.library's core (core/sb_core.c): adding, the row's order, changing, menus, tags,
 * failures that change nothing, an owner restarting and a program's task ending.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sb_core.h"

static int fails, checks;
#define CHECK(c, what) do { checks++; if (c) printf("ok   %s\n", what); else { fails++; printf("FAIL %s (line %d)\n", what, __LINE__); } } while (0)

/* memory that counts, and can be made to fail */
static long live, fail_after = -1;
static void *t_alloc(void *ctx, unsigned long n)
{
    (void)ctx;
    if (fail_after == 0) return NULL;
    if (fail_after > 0) fail_after--;
    live++;
    return calloc(1, n);
}
static void t_free(void *ctx, void *p, unsigned long n) { (void)ctx; (void)n; if (p) { live--; free(p); } }
static const sb_mem mem = { t_alloc, t_free, NULL };

#define T(t, d) { (unsigned long)(t), (unsigned long)(d) }
#define END { SB_TAG_DONE, 0 }

static int task_a, task_b, task_owner;
static int dead_b, dead_owner;
static int t_alive(void *ctx, void *task)
{
    (void)ctx;
    if (task == &task_b) return !dead_b;
    if (task == &task_owner) return !dead_owner;
    return 1;
}

static const char *order_of(sb_list *l, char *buf)
{
    struct SysBarItem *p;
    buf[0] = 0;
    for (p = l->sl_First; p; p = p->si_Next) { if (buf[0]) strcat(buf, " "); strcat(buf, p->si_Name); }
    return buf;
}

int main(void)
{
    sb_list l;
    char buf[256];
    int err;
    unsigned char pix[2 * 16 * 16];
    unsigned long rgb[SB_COLOURS];
    struct SysBarItem *clock, *spk, *tata, *x;
    int i;
    for (i = 0; i < (int)sizeof pix; i++) pix[i] = (unsigned char)(i & 15);
    for (i = 0; i < SB_COLOURS; i++) rgb[i] = 0x010203UL * (unsigned long)i;
    sb_init(&l, &mem);

    /* adding */
    {
        sb_tag t[] = { T(SBA_Name, "Clock"), T(SBA_Text, "Nov 14 0310"), T(SBA_Order, 10), T(SBA_Help, "The time"), END };
        clock = sb_add(&l, t, &task_a, &err);
    }
    CHECK(clock && err == SBR_OK, "a text item is added");
    CHECK(clock && !strcmp(clock->si_Text, "Nov 14 0310") && !strcmp(clock->si_Help, "The time"), "its text and help are kept");
    CHECK(clock && clock->si_Task == &task_a, "it is the adding task's");
    CHECK((sb_take_changes(&l) & SBC_ADD) && l.sl_Gen == 1, "the owner is told of the add");
    {
        sb_tag t[] = { T(SBA_Name, "Speaker"), T(SBA_Width, 16), T(SBA_Height, 16), T(SBA_States, 2), T(SBA_Pixels, pix),
                       T(SBA_RGB, rgb), T(SBA_Order, 5), T(SBA_UserData, 77), END };
        spk = sb_add(&l, t, &task_b, &err);
    }
    CHECK(spk && spk->si_States == 2 && spk->si_Pixels && spk->si_Pixels != pix, "a picture item gets its own copy of the pixels");
    CHECK(spk && !memcmp(spk->si_Pixels, pix, 2 * 16 * 16) && spk->si_RGB[3] == 0x010203UL * 3, "pixels and colours copied");
    CHECK(spk && spk->si_UserData == 77, "user data kept");
    CHECK(!strcmp(order_of(&l, buf), "Speaker Clock"), "the row is in order: lower order further left");
    {
        sb_tag t[] = { T(SBA_Name, "Tata"), T(SBA_Order, 5), END };
        tata = sb_add(&l, t, &task_a, &err);
    }
    CHECK(tata && !strcmp(order_of(&l, buf), "Speaker Tata Clock"), "the same order: the one added first stays left");
    CHECK(l.sl_Count == 3 && sb_find(&l, "Tata") == tata && !sb_find(&l, "tata"), "found by its exact name");

    /* refusals */
    {
        sb_tag t[] = { T(SBA_Name, "Clock"), END };
        CHECK(!sb_add(&l, t, &task_a, &err) && err == SBR_DUP, "a name taken is refused");
    }
    {
        long e = -1, e2 = -1;
        sb_tag t[] = { T(SBA_Name, "Clock"), T(SBA_Error, &e), END };
        sb_tag u[] = { T(SBA_State, 9), T(SBA_Error, &e2), END };
        sb_add(&l, t, &task_a, &err);
        sb_set(&l, clock, u);
        CHECK(e == SBR_DUP && e2 == SBR_BADARGS, "SBA_Error says why an add or a change was refused");
    }
    {
        sb_tag t[] = { T(SBA_Help, "no name"), END };
        CHECK(!sb_add(&l, t, &task_a, &err) && err == SBR_BADARGS, "no name is refused");
    }
    {
        sb_tag t[] = { T(SBA_Name, "SixteenLettersXX"), END };
        CHECK(!sb_add(&l, t, &task_a, &err) && err == SBR_BADARGS, "a name of 16 letters is too long");
    }
    {
        sb_tag t[] = { T(SBA_Name, "Big"), T(SBA_Width, 17), T(SBA_Height, 16), T(SBA_Pixels, pix), END };
        CHECK(!sb_add(&l, t, &task_a, &err) && err == SBR_BADARGS, "a picture wider than the grid is refused");
    }
    {
        sb_tag t[] = { T(SBA_Name, "NoPix"), T(SBA_Width, 8), T(SBA_Height, 8), END };
        CHECK(!sb_add(&l, t, &task_a, &err) && err == SBR_BADARGS, "a size with no pixels is refused");
    }
    {
        sb_tag t[] = { T(SBA_Name, "St"), T(SBA_Width, 8), T(SBA_Height, 8), T(SBA_Pixels, pix), T(SBA_State, 1), END };
        CHECK(!sb_add(&l, t, &task_a, &err) && err == SBR_BADARGS, "a state past the pictures is refused");
    }
    {
        long before = live;
        sb_tag t[] = { T(SBA_Name, "Mem"), T(SBA_Width, 8), T(SBA_Height, 8), T(SBA_Pixels, pix), END };
        fail_after = 1;                        /* the item, then not its pixels */
        x = sb_add(&l, t, &task_a, &err);
        fail_after = -1;
        CHECK(!x && err == SBR_NOMEM && live == before && l.sl_Count == 3, "out of memory: nothing added, nothing left over");
    }
    sb_take_changes(&l);
    {
        sb_list m;
        char nm[SB_MAXITEMS + 1][8];
        int n = 0;
        sb_init(&m, &mem);
        for (i = 0; i <= SB_MAXITEMS; i++) {
            sb_tag t[] = { T(SBA_Name, nm[i]), END };
            snprintf(nm[i], sizeof nm[i], "i%d", i);
            if (sb_add(&m, t, &task_a, &err)) n++;
        }
        CHECK(n == SB_MAXITEMS && err == SBR_FULL, "the 33rd item is refused: the bar is full");
        sb_free_all(&m);
    }

    /* changing */
    {
        sb_tag t[] = { T(SBA_State, 1), END };
        CHECK(sb_set(&l, spk, t) == SBR_OK && spk->si_State == 1 && (sb_take_changes(&l) == SBC_STATE), "a state change says only that");
    }
    {
        sb_tag t[] = { T(SBA_State, 2), END };
        CHECK(sb_set(&l, spk, t) == SBR_BADARGS && spk->si_State == 1, "a state past the pictures changes nothing");
    }
    {
        sb_tag t[] = { T(SBA_Text, "Nov 14 0310"), END };
        sb_set(&l, clock, t);
        CHECK(!(sb_take_changes(&l) & SBC_TEXT), "the same text again is no change");
    }
    {
        sb_tag t[] = { T(SBA_Text, "Nov 14 0311 and far too long a text for the bar"), END };
        CHECK(sb_set(&l, clock, t) == SBR_OK && strlen(clock->si_Text) == SB_TEXTLEN - 1 && (sb_take_changes(&l) & SBC_TEXT),
              "a text longer than the room is cut");
    }
    {
        sb_tag t[] = { T(SBA_Width, 8), END };
        CHECK(sb_set(&l, spk, t) == SBR_BADARGS && spk->si_Width == 16, "a new size without new pixels changes nothing");
    }
    {
        sb_tag t[] = { T(SBA_Width, 8), T(SBA_Height, 8), T(SBA_States, 1), T(SBA_Pixels, pix + 1), END };
        CHECK(sb_set(&l, spk, t) == SBR_OK && spk->si_Width == 8 && spk->si_States == 1 && spk->si_State == 0 &&
              spk->si_Pixels[0] == 1, "new pixels at a new size; the state comes back inside the pictures");
    }
    {
        sb_tag t[] = { T(SBA_Order, 20), END };
        sb_take_changes(&l);
        CHECK(sb_set(&l, spk, t) == SBR_OK && !strcmp(order_of(&l, buf), "Tata Clock Speaker") && (sb_take_changes(&l) & SBC_ORDER),
              "a new order moves the item in the row");
    }
    {
        sb_tag t[] = { T(SBA_Name, "Other"), T(SBA_Help, "renamed?"), END };
        CHECK(sb_set(&l, tata, t) == SBR_OK && !strcmp(tata->si_Name, "Tata"), "a name can't be changed");
    }
    {
        sb_tag t[] = { END };
        CHECK(sb_set(&l, (struct SysBarItem *)&task_a, t) == SBR_UNKNOWN && sb_set(&l, NULL, t) == SBR_UNKNOWN, "an item not in the list is refused");
    }

    /* menus */
    {
        const char *menu[] = { "Show", "Hide", "A very long entry that does not fit in the menu's room", NULL };
        sb_tag t[] = { T(SBA_Menu, menu), END };
        CHECK(sb_set(&l, tata, t) == SBR_OK && tata->si_MenuCount == 3, "a menu of three");
        CHECK(!strcmp(sb_menu_entry(tata, 0), "Show") && !strcmp(sb_menu_entry(tata, 1), "Hide"), "entries in their order");
        CHECK(strlen(sb_menu_entry(tata, 2)) == SB_MENULEN - 1 && !sb_menu_entry(tata, 3), "a long entry is cut; none past the last");
    }
    {
        const char *menu[SB_MENUMAX + 2];
        sb_tag t[] = { T(SBA_Menu, menu), END };
        for (i = 0; i <= SB_MENUMAX; i++) menu[i] = "x";
        menu[SB_MENUMAX + 1] = NULL;
        CHECK(sb_set(&l, tata, t) == SBR_BADARGS && tata->si_MenuCount == 3, "17 entries are refused and the old menu stays");
    }
    {
        long before = live;
        const char *menu[] = { "Quit", NULL };
        sb_tag t[] = { T(SBA_Menu, menu), END };
        sb_set(&l, tata, t);
        CHECK(tata->si_MenuCount == 1 && live == before, "a new menu frees the old one");
    }
    {
        sb_tag t[] = { T(SBA_Menu, NULL), END };
        CHECK(sb_set(&l, tata, t) == SBR_OK && tata->si_MenuCount == 0 && !sb_menu_entry(tata, 0), "a NULL menu takes it away");
    }

    /* tags as utility.library walks them */
    {
        sb_tag more[] = { T(SBA_Help, "from MORE"), END };
        sb_tag t[] = { T(SB_TAG_IGNORE, 0), T(SB_TAG_SKIP, 1), T(SBA_Help, "skipped"), T(0x8053FFFFUL, 5), T(SB_TAG_MORE, more) };
        CHECK(sb_set(&l, clock, t) == SBR_OK && !strcmp(clock->si_Help, "from MORE"), "TAG_IGNORE, TAG_SKIP, TAG_MORE and an unknown tag");
    }

    /* the owner */
    CHECK(sb_own(&l, &task_owner, 20) == SBR_OK && l.sl_Owner == &task_owner && l.sl_OwnerSig == 20, "an owner takes the bar");
    CHECK(sb_own(&l, &task_a, 21) == SBR_OWNED, "a second owner is refused");
    CHECK(sb_disown(&l, &task_a) == SBR_OWNED, "only the owner lets go");
    CHECK(sb_disown(&l, &task_owner) == SBR_OK && !l.sl_Owner, "the owner lets go");
    sb_take_changes(&l);
    CHECK(sb_own(&l, &task_owner, 22) == SBR_OK && (sb_take_changes(&l) & SBC_ADD) && l.sl_Count == 3,
          "an owner coming back finds every item still there and is told to read them");
    dead_owner = 1;
    sb_reap(&l, t_alive, NULL);
    CHECK(!l.sl_Owner && l.sl_Count == 3, "an owner whose task ended lets go; the items stay");
    CHECK(sb_own(&l, &task_a, 23) == SBR_OK, "and another can own the bar");

    /* a program's task ending */
    dead_b = 1;
    CHECK(sb_reap(&l, t_alive, NULL) == 1 && !sb_find(&l, "Speaker") && l.sl_Count == 2, "the speaker goes with its task");
    CHECK(sb_remove_task(&l, &task_a) == 2 && l.sl_Count == 0 && !l.sl_First, "a program's items go all at once");
    {
        sb_tag t[] = { T(SBA_Name, "Again"), END };
        x = sb_add(&l, t, &task_a, &err);
        CHECK(x && sb_remove(&l, x) == SBR_OK && sb_remove(&l, x) == SBR_UNKNOWN, "removed once; a second remove is refused");
    }
    sb_free_all(&l);
    CHECK(live == 0, "nothing left allocated");

    printf("%d checks, %d failed\n%s\n", checks, fails, fails ? "FAIL" : "PASS");
    return fails != 0;
}
