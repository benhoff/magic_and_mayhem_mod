#pragma once
#include "drawing_caller_state_v4.h"
/* V5 retains V4's848-byte layout. Return-observer state is sampled before
 * observer work, while outgoing queue/sequence/epoch identities are published
 * after its source command emission. Native glyphs complete at their actual
 * normal return observer; other native bypasses still describe the saved frame
 * consumed by shared POP/RET. This distinguishes state sampling from source
 * publication and makes prefix-drain joins exact. V1..V4 keep their old scopes. */
#define MNM_CALLER_STATE_V5_MAGIC "MNMCSA05"
#define MNM_CALLER_STATE_V5_VERSION 5u
#define MNM_CALLER_STATE_V5_ROW_BYTES 848u
