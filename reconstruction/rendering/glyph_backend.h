#ifndef MNM_GLYPH_BACKEND_H
#define MNM_GLYPH_BACKEND_H
#include "glyph_backend_state.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Owned, nonaliasing storage is required. FX images are 512 bytes, aligned16.
 * This isolated kernel does not inspect process memory or install a hook. */
typedef struct MnmGlyphBackend {
    MnmGlyphStateInput state;
    uint16_t *pixels;
    uint32_t width,height,stride;
    uint32_t *coverage;
    const uint8_t *incoming_fx;
    uint8_t *outgoing_fx;
    MnmGlyphState result;
} MnmGlyphBackend;
/* Atomic preflight: on refusal pixels, coverage, outgoing_fx and result retain
 * their contents. On success only selected x87 state is contracted; saved
 * instruction/data pointers and empty x87 register contents are unspecified. */
int mnm_glyph_backend(MnmGlyphBackend *);
#ifdef __cplusplus
}
#endif
#endif
