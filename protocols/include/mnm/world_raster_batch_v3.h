#pragma once
/* V3 queue completion. Little-endian64-byte header; requests append count
 * descriptors of16bytes: producer sequence, exact entry, low16AX, FNV-1a32
 * of the entire owned96-byte producer record plus payload. Producer V1 stays
 * unchanged. Header words4..10 and14..15 must be echoed exactly.
 * 4 transfer ordinal,5 queue,6 last published producer sequence,7 canvas,
 * 8 width,9 height,10 tight stride,11 payload bytes,12 payload FNV,13 success1,
 * 14 queue raster count,15 cumulative skipped raster count. The request's
 * success1 asserts the canvas guard was active throughout admitted traversal.
 * Replies append tight RGB565 canvas words. Publish requests only after all
 * owned sources are emitted; replies use atomic rename. No host pointers. */
#define MNM_WORLD_BATCH_REQUEST "MNMWBQ03"
#define MNM_WORLD_BATCH_REPLY "MNMWBR03"
#define MNM_WORLD_BATCH_VERSION 3u
#define MNM_WORLD_BATCH_HEADER 64u
#define MNM_WORLD_BATCH_DESCRIPTOR 16u
#define MNM_WORLD_BATCH_MAX 12320u
