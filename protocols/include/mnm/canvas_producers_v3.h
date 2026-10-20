#pragma once
#include "canvas_producers_v1.h"
// Opt-in extended guarded World prefix. V1 operations and byte cap apply.
// This is a finite diagnostic policy, not an original engine limit.
#define MNM_PRODUCER_V3_MAGIC "MNMPRO03"
#define MNM_PRODUCER_V3_QUEUES 32u
#define MNM_PRODUCER_V3_MAX_RECORDS 131072u
// Recorder-only extended header budget; replacement defaults retain131072.
#define MNM_PRODUCER_V3_TRACE_MAX_RECORDS 262144u
// Definition25 owns the full closed font/raster payload. A V3 font/raster
// with word4 !=0 and no wire payload references that earlier definition sequence.
// Canonical decode restores word4=0 and full extents before batch hashing.
#define MNM_PRODUCER_PAYLOAD_SOURCE 25u
// Public V3 prefix may carry the V2 owned four-orientation minimap
// operation24 and terrain operation22 marked with word19=0xfffffffe.
// Unmarked historical V3 terrain retains the orientation-zero policy.
// V3 operation26 owns a bounded RGB565 colour rectangle: word14 format0,
// word15 mode0..3, words19/20 width/height1..64; tight WORD pixels.
// No source pointers cross the channel. V1/V2 continue to refuse this operation.
#define MNM_PRODUCER_COLOUR_RECT 26u
/* Native-owned COPY metadata: word23 marker plus exact word21 NS_RECT.
 * Records the failed call; it never authorizes an out-of-bounds pixel write. */
#define MNM_PRODUCER_NATIVE_COPY_RECT_REFUSED 0x4e435231u
#define MNM_PRODUCER_NATIVE_COPY_RECT_HRESULT 0x88760096u
#define MNM_PRODUCER_V3_SOURCES 2048u
#define MNM_PRODUCER_V3_SOURCE_BYTES (16u*1024u*1024u)

// Tagged owned effect rasters: see world_effect_raster_v1.h.
