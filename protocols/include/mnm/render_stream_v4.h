#ifndef MNM_RENDER_STREAM_V4_H
#define MNM_RENDER_STREAM_V4_H
#include "render_stream_v3.h"
#include "render_stream_v2.h"
/* V4 adds an ordered import of an independently acknowledged resident World.
 * Fields: destination ID, generation low/high, session nonce low/high.
 * The reference carries no pixels or process pointers. RGB565800x600 only.
 * All prior opcodes retain their prior layout; V3 copy/clipper policies apply. */
#define MNM_RENDER_STREAM_V4_MAGIC "MNMCMD04"
#define MNM_RENDER_STREAM_V4_VERSION 4u
#define MNM_RENDER_STREAM_V4_OPERATION_WORLD 19u
#define MNM_RENDER_STREAM_V4_WORLD_BYTES 20u

/* Tight RGB565 patch followed by one0/1 coverage byte per pixel. No unchanged
 * CPU word may replace resident GPU history merely because it is inside a dirty
 * bounding rectangle. */
#define MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE 20u

#endif
