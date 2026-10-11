#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Resolved admitted span; caller owns initialized storage and its lease. */
void mnm_fade_words(uint16_t *words,uint32_t count,uint32_t format);
#ifdef __cplusplus
}
#endif
