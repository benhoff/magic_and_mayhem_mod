#pragma once
#include "menu_v18.h"
#define MNM_MENU_V19_MAGIC "MNMMCM19"
#define MNM_MENU_V19_SIZE 327680
#define MNM_MENU_V19_REALM 262144
#define MNM_MENU_V19_REALM_SIZE 512
#define MNM_MENU_REALM_SCREEN 4
#define MNM_MENU_REALM_ACTIVATE 46
#define MNM_MENU_REALM_CHARACTER 47
#define MNM_MENU_REALM_GRIMOIRE 48
#define MNM_MENU_REALM_SPELLBOX 49
/* Experimental settled Realm lane; earlier lanes retain their offsets.
 * Header words: parent3,depth3,context5,kind1..3,visible1..19,player0..79,
 * playerLocation,destination,availableRegionMask,auxiliaryMask,reserved[2].
 * Nineteen rows of six LE words: region1..19,owner,owned,adjacent,x,y.
 * Unused rows and final eight bytes are zero. No pointers or campaign writes.
 * Activation delegates one scheduled original tick's release branch. Auxiliary
 * requests delegate the original callback. Default-off pending live proof.
 */
