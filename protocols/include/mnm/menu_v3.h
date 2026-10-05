#pragma once
#include "menu_v2.h"
/* V3 preserves V2 lanes/payload. Host assignments at 40000 are covered by
 * the host seqlock. Empty = UINT32_MAX; 63 physical slots, 21 per alignment;
 * only the first recovered count (<=7) of each alignment can be assigned.
 * Engine spell payload uses the engine seqlock. No pointers or object layouts.
 */
#define MNM_MENU_V3_MAGIC "MNMMCMD3"
#define MNM_MENU_V3_SIZE 65536
#define MNM_MENU_V3_HOST_SLOTS 40000
#define MNM_MENU_V3_SPELL 18000
#define MNM_MENU_V3_SPELL_SIZE 12288
#define MNM_MENU_V3_COUNTS 16
#define MNM_MENU_V3_SLOTS 28
#define MNM_MENU_V3_SHELVES 280
#define MNM_MENU_V3_RECIPES 380
#define MNM_MENU_V3_ITEM_NAMES 632
#define MNM_MENU_V3_SPELL_NAMES 3320
#define MNM_MENU_SPELL_FINISH 12
