#pragma once
#include "menu_v16.h"
#define MNM_MENU_V17_MAGIC "MNMMCM17"
#define MNM_MENU_V17_SIZE 196608
#define MNM_MENU_V17_LOBBY 180224
#define MNM_MENU_V17_LOBBY_SIZE 544
#define MNM_MENU_LOBBY_SCREEN 24
#define MNM_MENU_LOBBY_CANCEL 42
/* V16 lanes unchanged. Host-only, normal Create caller and one local player.
 * LE words parent15/depth5/context2/provider1..3/actions Cancel1/map/localSlot0/
 * reserved0, then13 rules, four48-byte player rows (active,portrait,colour,
 * handicap,name32), game name128 and original map label128; trailing zero12.
 * Native view/Cancel only. Peer/guest admission, rules/chat/map/start actions
 * remain original controls; no networking/lobby/gameplay replacement.
 */
