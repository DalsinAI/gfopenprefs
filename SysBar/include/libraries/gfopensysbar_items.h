/* gfopensysbar.library: the system bar's items (the right end of the screen's title bar), kept by the library
 * so they outlive the bar's owner. Plain C types, so the library's host tests share this header.
 * See SysBar/DESIGN.md. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef LIBRARIES_GFOPENSYSBAR_ITEMS_H
#define LIBRARIES_GFOPENSYSBAR_ITEMS_H

#define SB_NAMELEN   16      /* with its 0: an item's name, unique */
#define SB_HELPLEN   48      /* what the item is, in plain words */
#define SB_TEXTLEN   32      /* a text item's text (the clock is one) */
#define SB_MAXW      16      /* the grid's cell (Common/traygrid.h) */
#define SB_MAXH      16
#define SB_STATES    4       /* pictures an item can have */
#define SB_COLOURS   12      /* the RGB of pixels 4 to 15, as the taskspace has them; 0 to 3 are the bar's pens */
#define SB_MENUMAX   16      /* menu entries */
#define SB_MENULEN   32      /* with its 0: one entry */
#define SB_MAXITEMS  32

/* Tags (SB_AddItemA, SB_SetItemA). TAG_USER is 1 << 31. */
#define SB_TAGBASE     0x80535400UL                /* TAG_USER | 'S' 'T' */
#define SBA_Name       (SB_TAGBASE + 1)            /* const char *, AddItem only, required */
#define SBA_Help       (SB_TAGBASE + 2)            /* const char * */
#define SBA_Text       (SB_TAGBASE + 3)            /* const char *: a text item, "" for none */
#define SBA_Width      (SB_TAGBASE + 4)            /* 1 to SB_MAXW */
#define SBA_Height     (SB_TAGBASE + 5)            /* 1 to SB_MAXH */
#define SBA_States     (SB_TAGBASE + 6)            /* 1 to SB_STATES */
#define SBA_Pixels     (SB_TAGBASE + 7)            /* const unsigned char *: States * Width * Height, one byte a pixel */
#define SBA_RGB        (SB_TAGBASE + 8)            /* const unsigned long *: SB_COLOURS of 0x00RRGGBB */
#define SBA_State      (SB_TAGBASE + 9)            /* the picture shown, 0 to States - 1 */
#define SBA_Menu       (SB_TAGBASE + 10)           /* const char *const *: NULL-ended entries; NULL for no menu */
#define SBA_Order      (SB_TAGBASE + 11)           /* where in the row: lower is further left; same order, added first */
#define SBA_Port       (SB_TAGBASE + 12)           /* struct MsgPort *: where the item's events go, NULL for none */
#define SBA_UserData   (SB_TAGBASE + 13)           /* yours, given back in each event */
#define SBA_Error      (SB_TAGBASE + 14)           /* long *: SB_AddItemA and SB_SetItemA put their SBR_* there */

/* results */
#define SBR_OK       0
#define SBR_BADARGS  1       /* no name, a size or state out of range, pixels without a size */
#define SBR_DUP      2       /* the name is taken */
#define SBR_FULL     3       /* SB_MAXITEMS already */
#define SBR_NOMEM    4
#define SBR_UNKNOWN  5       /* not an item of this list */
#define SBR_OWNED    6       /* another owner has the bar */

/* change bits: what SB_SetItemA (or an add or a remove) changed, or'd into the list's sl_Changed for the owner */
#define SBC_ADD      0x0001
#define SBC_REMOVE   0x0002
#define SBC_PICTURE  0x0004  /* pixels, size, colours or states */
#define SBC_STATE    0x0008
#define SBC_TEXT     0x0010
#define SBC_HELP     0x0020
#define SBC_MENU     0x0040
#define SBC_ORDER    0x0080
#define SBC_PORT     0x0100

/* One item. The owner reads these between SB_LockList and SB_UnlockList; programs change them only through
 * SB_SetItemA. */
struct SysBarItem {
    struct SysBarItem *si_Next;                /* in the row's order */
    char           si_Name[SB_NAMELEN];
    char           si_Help[SB_HELPLEN];
    char           si_Text[SB_TEXTLEN];
    short          si_Order;
    unsigned short si_Width, si_Height, si_States, si_State;
    unsigned char *si_Pixels;                  /* the library's copy, or NULL */
    unsigned long  si_RGB[SB_COLOURS];
    unsigned short si_MenuCount;
    char          *si_Menu;                    /* the library's copy: si_MenuCount entries, each ended by its 0 */
    void          *si_Port;                    /* struct MsgPort * */
    unsigned long  si_UserData;
    void          *si_Task;                    /* who added it (struct Task *): it goes when the task has gone */
    unsigned long  si_Seq;                     /* added this many items after the list began */
};

#endif
