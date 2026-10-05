#pragma once
#include "menu_v7.h"
#define MNM_MENU_V8_MAGIC "MNMMCMD8"
#define MNM_MENU_V8_SIZE 86016
#define MNM_MENU_REGION_ENTER 25
/* V7 payload/offsets retained. V8 adds Enter=2 to region actions and
 * handoff=3 for an accepted original fresh campaign battle request.
 * Acceptance is not proof of completed loading: original gameplay ticks
 * are validated separately. Auxiliary/loaded Realm paths remain unsupported.
 */
