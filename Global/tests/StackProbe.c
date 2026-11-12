/* StackProbe: prints the stack this program was started with, to check Global Settings' default stack size
 * (GFOpenDOS's floor) in a lab: from the Shell after "Stack 4096", and from its icon with Stack 4096.
 *   StackProbe          prints "StackProbe: stack N bytes (asked for M)" to the Shell, or to a window from Workbench
 * N is the stack it runs on (tc_SPUpper - tc_SPLower); M is what it was asked to have (the Shell's Stack,
 * cli_DefaultStack, or the icon's, pr_StackSize). No C library start-up, so nothing swaps the stack first.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;

LONG start(void)
{
    struct Process *pr;
    struct WBStartup *wb = NULL;
    struct CommandLineInterface *cli;
    BPTR out, win = 0;
    ULONG got, asked;
    LONG args[2];
    SysBase = *(struct ExecBase **)4;
    pr = (struct Process *)FindTask(NULL);
    if (!pr->pr_CLI) {
        WaitPort(&pr->pr_MsgPort);
        wb = (struct WBStartup *)GetMsg(&pr->pr_MsgPort);
    }
    if (!(DOSBase = (struct DosLibrary *)OpenLibrary((STRPTR)"dos.library", 36))) goto end;
    got = (ULONG)pr->pr_Task.tc_SPUpper - (ULONG)pr->pr_Task.tc_SPLower;
    cli = pr->pr_CLI ? (struct CommandLineInterface *)BADDR(pr->pr_CLI) : NULL;
    asked = cli ? (ULONG)cli->cli_DefaultStack * 4 : (ULONG)pr->pr_StackSize;
    out = wb ? (win = Open((STRPTR)"CON:40/40/440/60/StackProbe/CLOSE/WAIT", MODE_NEWFILE)) : Output();
    args[0] = (LONG)got;
    args[1] = (LONG)asked;
    if (out) VFPrintf(out, (STRPTR)"StackProbe: stack %lu bytes (asked for %lu)\n", args);
    if (win) { Delay(250); Close(win); }
    CloseLibrary((struct Library *)DOSBase);
end:
    if (wb) { Forbid(); ReplyMsg((struct Message *)wb); }
    return 0;
}
