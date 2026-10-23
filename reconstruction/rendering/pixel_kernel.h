#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* All extents are caller-owned. Admission precedes every destination write. */
struct MnmPixelPlane { uint16_t *pixels; uint32_t width, height, stride, format; };
/* Optional write-only storage backend; default model storage remains direct. */
typedef void (*MnmPixelStore16)(uint16_t *,uint16_t);
static inline void mnm_pixel_store16(MnmPixelStore16 store,uint16_t* target,uint16_t value){
 if(store)store(target,value);else *target=value;
}
struct MnmPcxInput { const uint8_t *encoded, *palette; uint32_t bytes, width, height; };
struct MnmDibInput { const uint8_t *info, *pixels; uint32_t info_bytes, pixel_bytes, usage; };
int mnm_pixel_plane(const struct MnmPixelPlane *);
int mnm_pcx_admit(const struct MnmPcxInput *);
int mnm_pcx_words(const struct MnmPixelPlane *, const struct MnmPcxInput *);
int mnm_pcx_indices(uint8_t *, uint32_t bytes, uint32_t stride, const struct MnmPcxInput *);
int mnm_dib_layout(const struct MnmDibInput *, uint32_t *width, uint32_t *height, uint32_t *stride);
int mnm_dib_words(const struct MnmPixelPlane *, const struct MnmDibInput *, int32_t x, int32_t y);
int mnm_rgb_add(const struct MnmPixelPlane *, int32_t left, int32_t top, int32_t right, int32_t bottom,
                uint16_t red, uint16_t green, uint16_t blue);
int mnm_pixel_whitewash(const struct MnmPixelPlane *);
/* Original row-forward alias ordering, with independently supplied row pitches. */
int mnm_pixel_copy(const struct MnmPixelPlane *, const struct MnmPixelPlane *, uint32_t width, uint32_t height);
#ifdef __cplusplus
}
#endif
