#pragma once
#include "menu_v11.h"
#define MNM_MENU_V12_MAGIC "MNMMCM12"
#define MNM_MENU_V12_SIZE 106496
/* Existing lanes/payloads retained. Campaign Mini offers Cancel|Quit|Preferences,
 * mask 7. Preferences retains V6 fields and admits parent ID17, depth6, only
 * beneath the exact fresh mode2/context5 campaign Mini/World/Realm/Main stack.
 * Preview/Cancel/OK remain semantic actions21/20/19; original availability
 * controls mutations. Other callers and pause cadence remain unsupported.
 */
