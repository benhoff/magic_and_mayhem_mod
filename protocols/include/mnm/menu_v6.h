#pragma once
#include "menu_v5.h"
#define MNM_MENU_V6_MAGIC "MNMMCMD6"
#define MNM_MENU_V6_SIZE 77824
#define MNM_MENU_V6_PREFERENCES 44708
#define MNM_MENU_V6_PREFERENCES_SIZE 48
#define MNM_MENU_V6_HOST_PREFERENCES 44756
#define MNM_MENU_PREFERENCES_SCREEN 10
#define MNM_MENU_OPEN_PREFERENCES 18
#define MNM_MENU_PREFERENCES_OK 19
#define MNM_MENU_PREFERENCES_CANCEL 20
#define MNM_MENU_PREFERENCES_PREVIEW 21
/* V1-V5 offsets unchanged. Engine payload: actions (OK=1/Cancel=2),
 * availability (radio indices 0..11, music=12, effects=13), parent ID=3,
 * stack depth; seven semantic values (music 0..15, effects -2500..0,
 * resolution High=0/Low=1, animation Full=0/Cut=1, dialogue and game
 * Fast=0/Medium=1/Slow=2, border On=1/Off=0); reserved zero.
 * Host payload: same seven values; included in the host lane's seqlock.
 * PREVIEW argument identifies music=0/effects=1; OK consumes all values.
 * No pointers. Main caller only. Original device/file/display work retained.
 */
