#pragma once
#include "menu_v9.h"
#define MNM_MENU_V10_MAGIC "MNMMCM10"
#define MNM_MENU_V10_SIZE 94208
/* V9 offsets retained. Campaign Mini offers Cancel|Quit (mask 5).
 * Quit dispatches original local index 3; confirmation stays in original viewport.
 * Modal snapshots offer no actions. No host command selects Yes or No.
 */
