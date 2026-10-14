#pragma once
#include "menu_v15.h"
#define MNM_MENU_V16_MAGIC "MNMMCM16"
#define MNM_MENU_V16_SIZE 180224
#define MNM_MENU_V16_SESSIONS 163840
#define MNM_MENU_V16_SESSIONS_SIZE (40+100*128)

#define MNM_MENU_SESSION_SCREEN 21
#define MNM_MENU_SESSION_JOIN 40
#define MNM_MENU_SESSION_CANCEL 41
/* V15 lanes unchanged. LE words: parent16, depth5, context2, transport1..3,
 * original action mask OK1/Cancel2, enumeration active, first-enumeration busy,
 * original deferred join, count0..100, selected row+1 (0 absent), then100
 * original CP1252 names128 each at40.
 * Original names and row ordinals; no GUID identity or network handles claimed.
 * Join argument row+1; Cancel0. Automatic lobby launch is excluded. Original
 * enumeration, joins, dialogs, lobby and networking remain authoritative.
 */
