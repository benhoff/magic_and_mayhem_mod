#ifndef MNM_GLYPH_BACKEND_STATE_H
#define MNM_GLYPH_BACKEND_STATE_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* PE32 addresses are supplied identities, never host pointers to dereference. */
typedef struct MnmGlyphStateInput {
    const uint8_t *frame; size_t frame_bytes;
    uint32_t frame_address, coverage_address, entry_sp, incoming_flags;
    int32_t left,top,right,bottom,x,y;
    uint32_t red,green,blue,cold,rounding;
} MnmGlyphStateInput;
typedef struct MnmGlyphState {
    uint32_t eax,ecx,edx,arguments[4],flags,branch,visible_rows,opaque_pixels;
} MnmGlyphState;
enum { MNM_GLYPH_EMPTY, MNM_GLYPH_LEFT_OUT, MNM_GLYPH_TOP_OUT,
       MNM_GLYPH_RIGHT_OUT, MNM_GLYPH_BOTTOM_OUT, MNM_GLYPH_HORIZONTAL,
       MNM_GLYPH_NO_ROWS, MNM_GLYPH_ROWS };
/* Exact positive integer times original float constant0x3c820821, rounded
 * once to binary32. No host floating environment dependency. */
int mnm_glyph_coverage_bits(uint32_t rounding,uint32_t out[64]);
/* Atomic refusal; distinct, nonaliasing input/frame/output is a precondition.
 * Caller FP status and pixels are deliberately not emulated by this model. */
int mnm_glyph_backend_state(const MnmGlyphStateInput *,MnmGlyphState *);
#ifdef __cplusplus
}
#endif
#endif
