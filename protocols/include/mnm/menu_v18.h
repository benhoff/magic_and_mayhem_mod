#pragma once
#include "menu_v17.h"
#define MNM_MENU_V18_MAGIC "MNMMCM18"
#define MNM_MENU_V18_SIZE 262144
#define MNM_MENU_V18_CHARACTER 196608
#define MNM_MENU_V18_CHARACTER_SIZE 256
#define MNM_MENU_CHARACTER_SCREEN 20
#define MNM_MENU_CHARACTER_OK 43
#define MNM_MENU_CHARACTER_CANCEL 44
#define MNM_MENU_CHARACTER_ADJUST 45
/* V17 lanes unchanged. Experimental Character lane: eight LE words
 * parent4/18,depth4/5,context5,wizard0..79,draftXP,actions3,adjustMask12,reserved0;
 * six24-byte rows opening/current/maximum/increment/decreasePrice/increasePrice;
 * original Realm wizard label32, then48 zero bytes. No host pointers or grants.
 * Adjust arg0..11 is original minus/plus row order; OK/Cancel require arg0.
 * Live admission remains disabled by default pending progressed-caller proof.
 */
