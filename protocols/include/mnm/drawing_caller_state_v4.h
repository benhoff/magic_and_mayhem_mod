#pragma once
#include "drawing_caller_state_v3.h"
/* V4: V2 base plus20 DWORDs,848-byte row. Fields0..12 of the
 * extension retain V3 identities. Fields13..19 are raw entry-time fixed-build
 * globals: wave initializer guard5f14d0, source-row guard5f1450, glyph coverage
 * sentinel6f5eac, phase6c4830, amplitude5e15f8, pixel format6e1f88, stride6a2dc8.
 * Epoch fields are zero without a continuous channel; these seven raw input
 * observations remain meaningful. They are not expected outputs/admission.
 * V1/V2/V3 retain their exact independently readable formats and zero fields. */
#define MNM_CALLER_STATE_V4_MAGIC "MNMCSA04"
#define MNM_CALLER_STATE_V4_VERSION 4u
#define MNM_CALLER_STATE_V4_ROW_BYTES 848u
