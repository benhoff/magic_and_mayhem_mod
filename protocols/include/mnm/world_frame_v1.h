#pragma once
/* Owned raster inputs only. Original pixels live in separate oracle files. */
#define MNM_WORLD_MAGIC "MNMWRLD1"
#define MNM_WORLD_HEADER 80u
#define MNM_WORLD_RECORD 64u
#define MNM_WORLD_MAX_BYTES (32u*1024u*1024u)
#define MNM_WORLD_MAX_DRAWS 32768u
#define MNM_WORLD_MAX_FRAME (1024u*1024u)
#define MNM_WORLD_BUILD 0x40209ca7u
/* Diagnostic capability refusal, never an admitted partial raster frame. */
#define MNM_WORLD_UNSUPPORTED_WAVE 7u
#define MNM_WORLD_UNSUPPORTED_KIND 8u
/* copy, integer half blend, integer quarter-source blend, displacement */
#define MNM_WORLD_COPY 0u
#define MNM_WORLD_HALF 1u
#define MNM_WORLD_QUARTER 2u
#define MNM_WORLD_WAVE 3u
#define MNM_WORLD_QUARTER_DESTINATION 4u
#define MNM_WORLD_PROJECTED_SHADOW 5u
