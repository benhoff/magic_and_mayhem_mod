#pragma once
#include "menu_v22.h"
#define MNM_MENU_V23_MAGIC "MNMMCM23"
#define MNM_MENU_V23_SIZE 589824
#define MNM_MENU_V23_BASE 524288
#define MNM_MENU_MINI_RESTART 52
#define MNM_MENU_MINI_CAN_RESTART 32
/* V23 wraps an explicitly selected V13..V22 base contract. The immutable LE
 * word at BASE selects that version; all its offsets and capability gates stay
 * unchanged. Mini actions may additionally offer RESTART32, only while this
 * process holds a checkpoint of the currently entered campaign level.
 * RESTART52 has no argument: original Mini Load and original Load lifecycle
 * restore a private engine-written checkpoint. No paths or pointers on wire.
 * Checkpoints are session-local; loading an older save does not invent a start.
 */
