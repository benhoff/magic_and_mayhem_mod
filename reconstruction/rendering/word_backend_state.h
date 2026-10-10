#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/* No-CD selected direct-word backends. Addresses are caller-supplied PE32
 * identities, never dereferenced. Predicts workspace and argument slots only.
 * Malformed input refusal is a model safety policy, not original behavior. */
typedef struct {
    const uint8_t *frame;
    size_t frame_bytes;
    uint32_t frame_address, canvas_address, right, bottom, stride_words, backend_rva;
    int32_t clip_left, clip_top, anchor_x, anchor_y;
} MnmWordBackendInput;
typedef struct {
    uint32_t words[16];
    int32_t argument_x, argument_y;
    uint32_t return_eax;
} MnmWordBackendState;
/* Successful prediction returns 0. Invalid/unsupported input returns 1 and
 * leaves the output untouched. Incoming workspace words are retained unless
 * the selected original branch writes them. */
int mnm_word_backend_state(const MnmWordBackendInput *, const uint32_t before[16],
                           MnmWordBackendState *);
#ifdef __cplusplus
}
#endif
