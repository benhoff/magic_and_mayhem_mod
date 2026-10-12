#pragma once
#include "menu_v13.h"
#define MNM_MENU_V14_MAGIC "MNMMCM14"
#define MNM_MENU_V14_SIZE 147456
#define MNM_MENU_V14_GRIMOIRE 131072
#define MNM_MENU_V14_GRIMOIRE_SIZE 32
#define MNM_MENU_GRIMOIRE_SCREEN 27
#define MNM_MENU_REGION_GRIMOIRE 33
#define MNM_MENU_GRIMOIRE_SELECT 34
#define MNM_MENU_GRIMOIRE_CLOSE 35
/* Semantic screen 27 is host policy: original book screen/initialized are zero.
 * LE words: chapter, section, original leaf cursor, available section mask,
 * parent screen, stack depth, context, reserved zero. No pointers or knowledge
 * levels. V14 admission is fresh Celtic Region Entry/Introduction and current first region only.
 * Select argument0..4 is Introduction section;5 is current region (1/1),
 * never arbitrary packed original action IDs. Available bits0..5.
 * All V13 lanes remain unchanged; V13 never enables book actions.
 */
