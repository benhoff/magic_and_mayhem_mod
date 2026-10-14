#pragma once
#include "menu_v20.h"
#define MNM_MENU_V21_MAGIC "MNMMCM21"
#define MNM_MENU_V21_SIZE 458752
#define MNM_MENU_V21_SPELL_CONTEXT 393216
#define MNM_MENU_V21_SPELL_CONTEXT_SIZE 32
/* V3 spell lane remains unchanged. Eight LE metadata words:
 * original mode (0 campaign,1 Quick Battle), parent(4/18 for campaign),
 * depth(4/5 for campaign),context(5 for campaign),four reserved zero words.
 * Quick Battle parent/depth/context are zero (not projected).
 * Campaign owner0..79; campaign seconds=-1 has no native countdown.
 * Original Done/Escape and teardown return to original campaign caller,
 * handoff4; they never produce Quick Battle handoff2.
 */
