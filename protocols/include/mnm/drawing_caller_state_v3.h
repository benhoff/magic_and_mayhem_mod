#pragma once
#include "drawing_caller_state_v2.h"
/* V3 appends 16 DWORDs to the V2 row (832 bytes). Fields0..3: entry World
 * generation and next source-chunk ordinal, each low/high. Fields4..7: exit
 * identities. Fields8/9: session nonce; field10: continuous channel present;
 * fields11/12: World open at entry/exit; fields13..15 reserved/zero.
 * Without a continuous channel the entire extension is zero. Sequence resets
 * may be interpreted only with these exact identities. The extension is raw
 * observation; it supplies neither source data nor expected state/pixels. */
#define MNM_CALLER_STATE_V3_MAGIC "MNMCSA03"
#define MNM_CALLER_STATE_V3_VERSION 3u
#define MNM_CALLER_STATE_V3_ROW_BYTES 832u
