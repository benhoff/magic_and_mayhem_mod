#ifndef MNM_RENDER_STREAM_V4_H
#define MNM_RENDER_STREAM_V4_H
#include "render_stream_v3.h"
/* V4 adds an ordered import of an independently acknowledged resident World.
 * Fields: destination ID, generation low/high, session nonce low/high.
 * The reference carries no pixels or process pointers. RGB565800x600 only.
 * All prior opcodes retain their prior layout; V3 copy/clipper policies apply. */
#define MNM_RENDER_STREAM_V4_MAGIC "MNMCMD04"
#define MNM_RENDER_STREAM_V4_VERSION 4u
#define MNM_RENDER_STREAM_V4_OPERATION_WORLD 19u
#define MNM_RENDER_STREAM_V4_WORLD_BYTES 20u
#endif
