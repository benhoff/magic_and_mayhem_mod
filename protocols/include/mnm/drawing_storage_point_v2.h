#pragma once
#include "drawing_storage_point_v1.h"
/* V2 has the same64byte scalar envelope. Word15 is the exact query ordinal
 * in the process-local version3 canvas storage-transition journal, or zero
 * when the recorder is stopped/inactive. It is never an allocation identity. */
#define MNM_DRAW_STORAGE_POINT_V2_VERSION 2u
#define MNM_DRAW_STORAGE_POINT_V2_BYTES 64u
