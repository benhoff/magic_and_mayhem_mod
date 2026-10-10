#pragma once
#include "canvas_producers_v1.h"
// Opt-in extended guarded World prefix. V1 operations and byte cap apply.
// This is a finite diagnostic policy, not an original engine limit.
#define MNM_PRODUCER_V3_MAGIC "MNMPRO03"
#define MNM_PRODUCER_V3_QUEUES 32u
#define MNM_PRODUCER_V3_MAX_RECORDS 131072u
// Definition25 owns the full closed font/raster payload. A V3 font/raster
// with word4 !=0 and no wire payload references that earlier definition sequence.
// Canonical decode restores word4=0 and full extents before batch hashing.
#define MNM_PRODUCER_PAYLOAD_SOURCE 25u
#define MNM_PRODUCER_V3_SOURCES 2048u
#define MNM_PRODUCER_V3_SOURCE_BYTES (16u*1024u*1024u)
