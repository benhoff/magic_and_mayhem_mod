#pragma once
#include "../../reconstruction/rendering/pixel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
enum { MNM_PCX_SURFACE, MNM_RGB_ADD, MNM_PCX_INDICES, MNM_PCX_RAW_WORDS, MNM_BRIGHTEN, MNM_ROW_COPY, MNM_PIXEL_SITES };
/* Twelve adapter-local scratch words, not a wire contract. RGB uses 7=right,
 * 8=skipped-left and 9=proven empty horizontal work after original preparation. */
typedef int (*MnmPixelResolve)(uint32_t *, unsigned, struct MnmPixelPlane *, struct MnmPcxInput *, uint32_t *);
extern MnmPixelResolve mnm_pixel_resolve;
extern uintptr_t mnm_pixel_original[MNM_PIXEL_SITES], mnm_pixel_resume[MNM_PIXEL_SITES];
extern uint32_t mnm_pixel_native[MNM_PIXEL_SITES], mnm_pixel_forwarded[MNM_PIXEL_SITES];
extern void (*mnm_pixel_notice)(unsigned,int);
int mnm_pixel_route(uint32_t *, unsigned);
#ifdef __cplusplus
}
#endif
