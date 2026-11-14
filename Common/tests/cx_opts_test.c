/* Host test for Common/cx_opts.h: CX_PRIORITY and CX_POPUP. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "cx_opts.h"

static int fails;
#define CHECK(c, msg) do { if (c) printf("ok   %s\n", msg); else { fails++; printf("FAIL %s\n", msg); } } while (0)

int main(void)
{
    cxo_opts o;
    cxo_defaults(&o, 0);
    CHECK(o.pri == 0 && !o.popup, "defaults: the priority given, no popup");
    CHECK(cxo_word(&o, "CX_PRIORITY=5") && o.pri == 5, "CX_PRIORITY=5");
    CHECK(cxo_word(&o, "cx_priority = -3") && o.pri == -3, "letter case and spaces");
    CHECK(cxo_word(&o, "CX_PRIORITY=300") && o.pri == 127, "held to 127");
    CHECK(cxo_word(&o, "CX_PRIORITY=-999999") && o.pri == -128, "held to -128");
    CHECK(cxo_word(&o, "CX_PRIORITY=abc") && o.pri == -128, "not a number: as it was");
    CHECK(!cxo_word(&o, "CX_PRIORITYX=1") && o.pri == -128, "CX_PRIORITYX isn't ours");
    CHECK(cxo_word(&o, "CX_POPUP=YES") && o.popup, "CX_POPUP=YES");
    CHECK(cxo_word(&o, "CX_POPUP=NO") && !o.popup, "CX_POPUP=NO");
    CHECK(cxo_word(&o, "CX_POPUP") && o.popup, "CX_POPUP alone is yes");
    CHECK(cxo_word(&o, "cx_popup=on") && o.popup, "on");
    CHECK(cxo_word(&o, "CX_POPUP=OFF") && !o.popup, "OFF is no");
    CHECK(!cxo_word(&o, "DONOTWAIT") && !cxo_word(&o, NULL) && !cxo_word(&o, ""), "other words, none, empty");
    printf("%s\n", fails ? "FAIL" : "PASS");
    return fails != 0;
}
