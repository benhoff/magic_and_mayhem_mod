#pragma once
#include "pixel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
struct MnmColourRect {
    uint32_t width, height, mode, source_x, source_y, visible_width, visible_height;
    uint16_t red, green, blue;
    const uint8_t *triples;
    int32_t x, y;
};
/* Canonical channel units, three little-endian WORDs per source pixel.
 * The entire source is admitted before the first write. */
int mnm_colour_rect(const struct MnmPixelPlane *, const struct MnmColourRect *, uint32_t *last_blue);
#ifdef __cplusplus
}
#endif
