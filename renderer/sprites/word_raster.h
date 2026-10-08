#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bounded direct-word SPR frame rasterization. Borrowed inputs are used only
 * during the call. This core has no legacy addresses, Qt or platform APIs.
 * Admission finishes before any destination byte is written. Clipped requests
 * are deliberately refused; the adapter retains the original clipping path. */
typedef struct {
    uint8_t *pixels;
    size_t bytes;
    uint32_t width, height, stride_words;
} MnmWordCanvas;
typedef struct {
    uint32_t width, height, opaque_pixels, last_run, last_run_x, last_run_y;
    int32_t left, top;
} MnmWordDraw;
enum { MNM_WORD_OK=0, MNM_WORD_INVALID=1, MNM_WORD_CLIPPED=2 };
int mnm_word_sprite_admit(const uint8_t *frame, size_t bytes,
                         const MnmWordCanvas *, int32_t anchor_x,
                         int32_t anchor_y, MnmWordDraw *);
int mnm_word_sprite_draw(const uint8_t *frame, size_t bytes,
                        const MnmWordCanvas *, int32_t anchor_x,
                        int32_t anchor_y, MnmWordDraw *);
#ifdef __cplusplus
}
#endif
