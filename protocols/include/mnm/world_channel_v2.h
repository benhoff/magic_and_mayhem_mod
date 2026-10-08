#pragma once
#include "world_channel_v1.h"
/* Opt-in finite, ordered startup journal. No original pointer crosses the wire.
 * Each slot is published once; no drop, reuse or superseding. */
#define MNM_WCH_HISTORY_MAGIC "MNMWCH02"
#define MNM_WCH_HISTORY 2u
#define MNM_WCH_HISTORY_MAX 16u
#define MNM_WCH_HISTORY_SLOT (32u+MNM_WORLD_MAX_BYTES+MNM_WCH_ORACLE)
#define MNM_WCH_HISTORY_SIZE(n) (MNM_WCH_HEADER+(n)*MNM_WCH_HISTORY_SLOT)
#define MNM_WCH_NATIVE_ZERO_RESET 1u
/* Header v1 offsets remain, version=2, flags=HISTORY|optional VERIFY,
 * slots52=target68 (1..16), remaining reserved72..127=0.
 * Slot: ownership0, publication4, input8, oracle12, logical canvas16,
 * actual source queue20, reset flags24, reserved28=0, inputs32,
 * oracle32+input-cap. Source queues must cover exactly1..target.
 * RESET is an explicit native zero policy, never original destination data. */
