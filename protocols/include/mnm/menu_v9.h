#pragma once
#include "menu_v8.h"
#define MNM_MENU_V9_MAGIC "MNMMCMD9"
#define MNM_MENU_V9_SIZE 90112
/* Previous offsets retained. Campaign gameplay Mini: battle=0, context=5,
 * depth=5, parent=2, word6=mode2; only Cancel bit1 is offered. Word7 zero.
 * Opening and resume stay original; this contract does not assert timer pause.
 */
