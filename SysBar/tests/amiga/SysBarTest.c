/* SysBarTest: gfopensysbar.library on a real Amiga, for the lab. Run from the Shell; prints ok/FAIL lines and
 * PASS or FAIL last (rc 0 or 10). It needs no bar owner running, and leaves the list as it found it.
 *   SysBarTest            the library's calls, an owner's signal, an event, and a child task's items going
 * with the task. Linked with libgfopensysbar.a (the link stub).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#define SYSBAR_NO_INLINE
#include <proto/gfopensysbar.h>
#include <stdio.h>
#include <string.h>

struct Library *SysBarBase;
static int fails, checks;
#define CHECK(c, what) do { checks++; if (c) printf("ok   %s\n", what); else { fails++; printf("FAIL %s\n", what); } } while (0)

static volatile int child_done;
static void child(void)
{
    struct TagItem t[] = { { SBA_Name, (ULONG)"T-child" }, { TAG_DONE, 0 } };
    SB_AddItemA(t);
    child_done = 1;                            /* and the task ends with its item still in the list */
}

int main(void)
{
    struct MsgPort *port;
    struct SysBarItem *a, *c, *it;
    BYTE sig;
    LONG err = -1;
    ULONG ch;
    int n, found;
    const char *menu[] = { "Show", "Hide", NULL };
    WORD box[4] = { 600, 0, 20, 12 };
    struct SysBarEvent *ev;
    if (!(SysBarBase = OpenLibrary((CONST_STRPTR)SYSBARNAME, SYSBAR_VERSION))) { printf("FAIL no %s\n", SYSBARNAME); return 10; }
    port = CreateMsgPort();
    sig = AllocSignal(-1);
    {
        struct TagItem t[] = { { SBA_Name, (ULONG)"T-one" }, { SBA_Text, (ULONG)"one" }, { SBA_Order, 3 },
                               { SBA_Menu, (ULONG)menu }, { SBA_Port, (ULONG)port }, { SBA_UserData, 42 }, { TAG_DONE, 0 } };
        a = SB_AddItemA(t);
    }
    CHECK(a != NULL, "an item is added");
    {
        struct TagItem t[] = { { SBA_Name, (ULONG)"T-one" }, { SBA_Error, (ULONG)&err }, { TAG_DONE, 0 } };
        CHECK(!SB_AddItemA(t) && err == SBR_DUP, "the same name again is refused, SBA_Error says why");
    }
    {
        LONG r = SB_Own((ULONG)sig);
        if (r == SBR_OWNED) printf("     (the bar has an owner already: the owner checks are skipped)\n");
        else {
            struct TagItem t[] = { { SBA_Text, (ULONG)"two" }, { TAG_DONE, 0 } };
            CHECK(r == SBR_OK && SB_OwnerTask() == FindTask(NULL), "this task owns the bar, and SB_OwnerTask says so");
            ch = SB_LockList();
            SB_UnlockList();
            SetSignal(0, 1UL << sig);
            SB_SetItemA(a, t);
            CHECK(!(SetSignal(0, 0) & (1UL << sig)), "the owner's own change doesn't signal it");
            ch = SB_LockList();
            for (n = 0, found = 0, it = SB_NextItem(NULL); it; it = SB_NextItem(it), n++) if (it == a) found = 1;
            CHECK(found && (ch & SBC_TEXT) && !strcmp(a->si_Text, "two"), "the owner reads the item and its change");
            CHECK(!strcmp((char *)SB_MenuEntry(a, 1), "Hide") && SB_FindItem((CONST_STRPTR)"T-one") == a, "menu and find");
            CHECK(SB_SendEvent(a, SBE_MENU, 1, box) == SBR_OK, "a menu event is sent");
            CHECK(SB_SendEvent(a, SBE_MENU, 5, box) == SBR_BADARGS, "past the menu is refused");
            SB_UnlockList();
            WaitPort(port);
            ev = (struct SysBarEvent *)GetMsg(port);
            CHECK(ev && ev->sbe_Magic == SBE_MAGIC && ev->sbe_Kind == SBE_MENU && ev->sbe_Entry == 1 && ev->sbe_UserData == 42 &&
                  ev->sbe_Left == 600 && !strcmp(ev->sbe_Name, "T-one"), "the event arrives with the item's name, data and box");
            if (ev) SB_DoneEvent(ev);
            /* a child task's item goes when the task ends */
            child_done = 0;
            if (CreateNewProcTags(NP_Entry, (ULONG)child, NP_Name, (ULONG)"SysBarTest child", TAG_DONE)) {
                for (n = 0; n < 250 && !child_done; n++) Delay(1);
                Delay(10);
                SB_LockList();
                SB_UnlockList();
                SB_LockList();
                c = SB_FindItem((CONST_STRPTR)"T-child");
                SB_UnlockList();
                CHECK(child_done && !c, "a task's item goes when the task has gone");
                /* an item added by a task that then ended, before any lock: SendEvent refuses, never PutMsgs into its
                 * gone port (Marcus's review) */
                child_done = 0;
                if (CreateNewProcTags(NP_Entry, (ULONG)child, NP_Name, (ULONG)"SysBarTest child", TAG_DONE)) {
                    for (n = 0; n < 250 && !child_done; n++) Delay(1);
                    Delay(10);
                    c = SB_FindItem((CONST_STRPTR)"T-child");
                    CHECK(!c || SB_SendEvent(c, SBE_CLICK, 0, box) == SBR_UNKNOWN, "an event for a gone task's item is refused");
                }
            } else CHECK(0, "the child task starts");
            CHECK(SB_Disown() == SBR_OK, "the owner lets go");
        }
    }
    CHECK(SB_RemItem(a) == SBR_OK && SB_RemItem(a) == SBR_UNKNOWN, "removed once; again is refused");
    FreeSignal(sig);
    DeleteMsgPort(port);
    CloseLibrary(SysBarBase);
    printf("%d checks, %d failed\n%s\n", checks, fails, fails ? "FAIL" : "PASS");
    return fails ? 10 : 0;
}
