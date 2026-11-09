#pragma once
#include "drawing_caller_state_v1.h"
/* V2 retains the V1 header/row/semantic-FX layouts and bounds. Row DWORD190/191
 * become optional enclosing normal-return context kind/PC: 0 absent (both0),
 * 1 additive/colour wrapper (entry stack word3), 2 opaque wrapper (word8/10),
 * 3 checked sprite wrapper (production parent_word), 4 trail wrapper (word11).
 * These are passive raw stack observations, not caller admission or proof that
 * a recorded enclosing PC is required/supported. An acceptance consumer must
 * bind the actual PC, pinned body/stack derivation and continuation liveness.
 * V1 remains independently readable with those two words reserved/zero. */
#define MNM_CALLER_STATE_V2_MAGIC "MNMCSA02"
#define MNM_CALLER_STATE_V2_VERSION 2u
