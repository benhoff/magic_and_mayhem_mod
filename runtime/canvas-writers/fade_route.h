#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
struct MnmFadePlane { uint16_t *pixels;uint32_t width,height,stride,format; };
/* Resolve a live lease without altering captured registers; no partial writes. */
typedef int (*MnmFadeResolve)(const uint32_t*,struct MnmFadePlane*);
extern MnmFadeResolve mnm_fade_resolve;
extern uintptr_t mnm_fade_original,mnm_fade_resume;
extern uint32_t mnm_fade_native,mnm_fade_forwarded;
int mnm_fade_route(uint32_t *registers);
#ifdef __cplusplus
}
#endif
