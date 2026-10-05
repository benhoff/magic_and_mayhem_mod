#pragma once
#include "menu_v6.h"
#define MNM_MENU_V7_MAGIC "MNMMCMD7"
#define MNM_MENU_V7_SIZE 81920
#define MNM_MENU_V7_REGION 44784
#define MNM_MENU_V7_REGION_SIZE 320
#define MNM_MENU_REGION_SCREEN 18
#define MNM_MENU_NEW_GAME 22
#define MNM_MENU_REGION_DIFFICULTY 23
#define MNM_MENU_REGION_CANCEL 24
/* V1-V6 lanes/offsets unchanged. Engine region payload: caller, depth,
 * different-region flag, difficulty, available radios (bits 0..3), actions
 * (Cancel=1), region number, reserved zero; realm and title are bounded
 * 128-byte NUL-terminated CP1252 strings; trailing 32 bytes reserved zero.
 * DIFFICULTY uses the existing host argument, range 0..3. No pointers.
 * Initial integration supports fresh Celtic region 1, Realm caller 4 only.
 * Enter, auxiliary menus and loaded Realm callers remain original-owned.
 */
