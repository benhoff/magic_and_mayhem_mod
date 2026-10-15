#include "pixel_route.h"
MnmPixelResolve mnm_pixel_resolve;
uintptr_t mnm_pixel_original[MNM_PIXEL_SITES], mnm_pixel_resume[MNM_PIXEL_SITES];
uint32_t mnm_pixel_native[MNM_PIXEL_SITES], mnm_pixel_forwarded[MNM_PIXEL_SITES];
int mnm_pixel_route(uint32_t *r, unsigned kind) {
    if (kind >= MNM_PIXEL_SITES) return 0;
    struct MnmPixelPlane p = {0};
    struct MnmPcxInput s = {0};
    uint32_t args[12] = {0};
    if (!mnm_pixel_resolve || !mnm_pixel_resolve(r, kind, &p, &s, args)) goto refuse;
    if (kind == MNM_PCX_SURFACE || kind == MNM_PCX_RAW_WORDS) {
        if (!mnm_pcx_words(&p, &s)) goto refuse;
        r[7] = s.height;
        r[6] = kind == MNM_PCX_SURFACE ? (uint32_t)(uintptr_t)p.pixels : s.height;
        if (kind == MNM_PCX_RAW_WORDS) r[5] = p.stride * 2;
    } else if (kind == MNM_PCX_INDICES) {
        if (!mnm_pcx_indices((uint8_t *)p.pixels, args[0], p.stride, &s)) goto refuse;
        /* Original consumes all runs, leaving source cursor after final row. */
        uint32_t at = 128;
        for (uint32_t y = 0; y < s.height; ++y) for (uint32_t x = 0; x < s.width;) {
            uint32_t v = s.encoded[at++], n = 1;
            if ((v & 192) == 192) { n = v & 63; ++at; }
            x += n;
        }
        r[7] = r[6] = s.height;
        r[5] = (uint32_t)(uintptr_t)s.encoded + at;
    } else if (kind == MNM_RGB_ADD) {
        if (!mnm_rgb_add(&p, (int32_t)args[0], (int32_t)args[1], (int32_t)args[2],
            (int32_t)args[3], (uint16_t)args[4], (uint16_t)args[5], (uint16_t)args[6])) goto refuse;
        r[7] = args[7]; r[5] = args[8]; r[6] = 0;
    } else if (kind == MNM_BRIGHTEN) {
        if (!mnm_pixel_whitewash(&p)) goto refuse;
        r[7] = (uint32_t)(uintptr_t)p.pixels; r[5] = r[0];
    } else {
        struct MnmPixelPlane source = {(uint16_t *)(uintptr_t)args[0], args[1], args[2], args[3], p.format};
        if (!mnm_pixel_copy(&p, &source, args[4], args[5])) goto refuse;
    }
    ++mnm_pixel_native[kind];
    return 1;
refuse:
    ++mnm_pixel_forwarded[kind];
    return 0;
}
