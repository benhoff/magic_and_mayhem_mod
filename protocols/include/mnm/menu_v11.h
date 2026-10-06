#pragma once
#include "menu_v10.h"
#define MNM_MENU_V11_MAGIC "MNMMCM11"
#define MNM_MENU_V11_SIZE 102400
#define MNM_MENU_V11_DEFEAT 94208
#define MNM_MENU_V11_DEFEAT_SIZE 5648
#define MNM_MENU_DEFEAT_SCREEN 6
#define MNM_MENU_DEFEAT_OK 26
/* Existing lanes retained. LE header: actions (OK=1), context=5,
 * depth=5, outcome=0 (defeat only). Original title and 21 original display
 * strings follow, each 256-byte NUL-terminated CP1252. No scores computed.
 * Fresh campaign Quit only. Victory and other outcomes remain original-owned.
 */
