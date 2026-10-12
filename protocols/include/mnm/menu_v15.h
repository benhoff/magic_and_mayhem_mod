#pragma once
#include "menu_v14.h"
#define MNM_MENU_V15_MAGIC "MNMMCM15"
#define MNM_MENU_V15_SIZE 163840
#define MNM_MENU_V15_HOST_NAMES 147456
#define MNM_MENU_V15_NAMES_SIZE 80
#define MNM_MENU_V15_MULTIPLAYER 151552
#define MNM_MENU_V15_MULTIPLAYER_SIZE 128
#define MNM_MENU_CREATE_SCREEN 15
#define MNM_MENU_JOIN_SCREEN 16
#define MNM_MENU_OPEN_CREATE 36
#define MNM_MENU_OPEN_JOIN 37
#define MNM_MENU_MULTIPLAYER_SUBMIT 38
#define MNM_MENU_MULTIPLAYER_CANCEL 39
/* V14 lanes unchanged. LE engine words: parent22, depth4, context2,
 * selected transport0..2, transport availability bits0..2, action bits
 * OK1/Cancel2, modal, deferred submit, original game/user edit limits,
 * reserved0, reserved0; then game name64 and user name16 (CP1252).
 * Host names are game64/user16 NUL-terminated printable ASCII, preserving
 * spaces. Game name63 is a bounded native policy; original edit limit256
 * and allocation256 are not safe justification for a 256-byte setter copy.
 * Join has no game name. Submit argument is transport; Cancel argument0.
 * Only normal Main/Quick callers, excluding automatic lobby launches.
 * Original networking, session selection, lobby and errors own the viewport.
 */
