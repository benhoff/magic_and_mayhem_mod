#pragma once
#include "pixel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
struct MnmRasterPixels {
    const uint8_t *frame;
    uint32_t bytes, indexed, palette_step, mode, strict_horizontal, contiguous, legacy_wave_clip, paired_palette;
    const uint8_t *palette;
    int32_t left, top, clip_left, clip_top, clip_right, clip_bottom;
    int32_t offsets[16];
    uint32_t period;
};
/* Modes:0copy,1half,2destination75,3displacement,4source75,
 * 5projected shadow,6grayscale,7black silhouette. Atomic admission. */
int mnm_raster_pixels(const struct MnmPixelPlane *, const struct MnmRasterPixels *);
/* Optional native storage-boundary decision, separate from recovered strict
 * admission. Called only for a valid displacement whose tap leaves ownership.
 * Return1 to retain that destination without reading/writing it. The preflight
 * call (write=0) must have no side effects; write=1 follows complete admission. */
typedef int (*MnmRasterOutsideSample)(void *, uint32_t, uint32_t, int);
int mnm_raster_pixels_with_sample_boundary(const struct MnmPixelPlane *,
    const struct MnmRasterPixels *, MnmRasterOutsideSample, void *);
#ifdef __cplusplus
}
#endif
