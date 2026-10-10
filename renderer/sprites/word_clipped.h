#pragma once
#include "word_raster.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct { int32_t left, top, right, bottom; } MnmWordClip;
typedef struct {
    uint32_t width, height, opaque_pixels, visible_pixels;
    int32_t left, top;
    uint32_t source_left, source_top, source_right, source_bottom;
} MnmWordClippedDraw;

/* Half-open clipping of bounded, contiguous, direct-word frames. No auxiliary
 * planes or indexed input. Empty dimensions and hidden frames are successful
 * no-ops. Validate the complete frame, including hidden rows, before writing.
 * Invalid input leaves pixels and output untouched. Output/descriptors must not
 * alias storage; borrowed frame and descriptors must remain stable throughout.
 * This native service has no original addresses or workspace dependencies. */
int mnm_word_sprite_clip_admit(const uint8_t *, size_t, const MnmWordCanvas *,
                             const MnmWordClip *, int32_t, int32_t,
                             MnmWordClippedDraw *);
int mnm_word_sprite_clip_draw(const uint8_t *, size_t, const MnmWordCanvas *,
                            const MnmWordClip *, int32_t, int32_t,
                            MnmWordClippedDraw *);
#ifdef __cplusplus
}
#endif
