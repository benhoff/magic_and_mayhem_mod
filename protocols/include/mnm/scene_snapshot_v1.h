#pragma once
/* Immutable little-endian observation files; offsets only, no host structs.
 * Header 64, records 32, then contiguous 16-byte blob headers + frame bytes.
 * Tokens are 1..blob_count, local to one snapshot, never process addresses.
 */
#define MNM_SCENE_V1_MAGIC "MNMSCNE1"
#define MNM_SCENE_V1_VERSION 1u
#define MNM_SCENE_V1_HEADER 64u
#define MNM_SCENE_V1_RECORD 32u
#define MNM_SCENE_V1_BLOB_HEADER 16u
#define MNM_SCENE_V1_MAX_BYTES (8u*1024u*1024u)
#define MNM_SCENE_V1_MAX_DRAWS 12320u
#define MNM_SCENE_V1_MAX_BLOBS 4096u
#define MNM_SCENE_V1_MAX_FRAME (1024u*1024u)
#define MNM_SCENE_V1_BUILD 0x40209ca7u
#define MNM_SCENE_V1_HIDDEN 1u
#define MNM_SCENE_V1_NO_FRAME 2u
#define MNM_SCENE_V1_INDEXED 1u
