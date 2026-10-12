#pragma once
/* Portable scalar cache association. Addresses remain inside the process. */
#define MNM_DRAW_STORAGE_POINT_VERSION 1u
#define MNM_DRAW_STORAGE_POINT_BYTES 64u
#define MNM_DRAW_STORAGE_POINT_TRACE 1u
#define MNM_DRAW_STORAGE_POINT_LIVE 2u
#define MNM_DRAW_STORAGE_POINT_STOPPED 4u
#define MNM_DRAW_STORAGE_POINT_AMBIGUOUS 8u
#define MNM_DRAW_STORAGE_POINT_MATCH 16u
#define MNM_DRAW_STORAGE_POINT_EXTENT_MISMATCH 32u
/* Words: bytes/version/flags/sequence/queue/failures/canvas/width/height,
 * WORD stride/dirty/sampled/outstanding frames/nesting/thread/reserved. */
