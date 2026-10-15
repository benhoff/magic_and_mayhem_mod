#pragma once
#include "pixel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Little-endian native-source cache V1. No native pointer is serialized. */
int mnm_jpeg_catalog_admit(const uint8_t *, uint32_t bytes);
int mnm_jpeg_record_words(const uint8_t *record, uint32_t record_bytes, const struct MnmPixelPlane *);
int mnm_jpeg_catalog_words(const uint8_t *, uint32_t bytes, const char *relative_path,
                           const struct MnmPixelPlane *);
#ifdef __cplusplus
}
#endif
