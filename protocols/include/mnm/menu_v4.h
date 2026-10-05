#pragma once
#include "menu_v3.h"
/* Offline preparation only. Both native shell and runtime default to disabled.
 * Enable only in a dedicated validation build; no normal-launch CLI switch.
 */
#ifndef MNM_MENU_MINI_EXPERIMENTAL
#define MNM_MENU_MINI_EXPERIMENTAL 0
#endif
#define MNM_MENU_V4_MAGIC "MNMMCMD4"
#define MNM_MENU_V4_SIZE 69632
#define MNM_MENU_V4_MINI 42000
#define MNM_MENU_V4_MINI_SIZE 32
/* LE words: battle-mode boolean, confirmation boolean, stack depth,
 * parent screen ID, offered action bitset, context value, reserved, reserved.
 * These are observed state, not an assertion that simulation is paused.
 */
#define MNM_MENU_MINI_SCREEN 17
#define MNM_MENU_MINI_CANCEL 13
#define MNM_MENU_MINI_PREFERENCES 14
#define MNM_MENU_MINI_QUIT 15
#define MNM_MENU_MINI_CAN_CANCEL 1
#define MNM_MENU_MINI_CAN_PREFERENCES 2
#define MNM_MENU_MINI_CAN_QUIT 4
