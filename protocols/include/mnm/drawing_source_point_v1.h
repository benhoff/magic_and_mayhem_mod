#pragma once
/* Portable scalar source-point metadata. A COM pointer is accepted only inside
 * the original process; it never enters this64byte little-endian payload. */
#define MNM_DRAW_SOURCE_POINT_VERSION 1u
#define MNM_DRAW_SOURCE_POINT_BYTES 64u
#define MNM_DRAW_SOURCE_PAIR_BYTES 128u
#define MNM_DRAW_SOURCE_POINT_TRACE 1u
#define MNM_DRAW_SOURCE_POINT_LIVE 2u
#define MNM_DRAW_SOURCE_POINT_STOPPED 4u
#define MNM_DRAW_SOURCE_POINT_AMBIGUOUS 8u
#define MNM_DRAW_SOURCE_POINT_MATCH 16u
/* Words: bytes, version, flags, producer sequence, queue, failures, canvas ID,
 * width, height, WORD stride, dirty generation, sampled generation, outstanding
 * return frames, caller's primitive nesting, local caller thread ID, reserved. */
