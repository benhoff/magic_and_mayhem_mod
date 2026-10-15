#include "pixel_kernel.h"
#include <stddef.h>
#include <limits.h>
static uint32_t u16(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8; }
static uint32_t u32(const uint8_t *p) { return u16(p) | u16(p + 2) << 16; }
static int span(const void *p, uint32_t n) { return p && n && (uintptr_t)p <= UINTPTR_MAX - n; }
static int overlaps(const void *a, uint32_t n, const void *b, uint32_t m) {
    return (uintptr_t)a < (uintptr_t)b + m && (uintptr_t)b < (uintptr_t)a + n;
}
int mnm_pixel_plane(const struct MnmPixelPlane *p) {
    return p && p->width && p->width <= 2048 && p->height && p->height <= 2048 &&
        p->stride >= p->width && p->stride <= 4096 && p->format <= 1 &&
        span(p->pixels, p->stride * p->height * 2);
}
static uint16_t colour(const uint8_t *rgb, uint32_t format) {
    return (uint16_t)((uint32_t)(rgb[0] >> 3) << (format ? 10 : 11) |
        (uint32_t)(rgb[1] >> (format ? 3 : 2)) << 5 | rgb[2] >> 3);
}
int mnm_pcx_admit(const struct MnmPcxInput *s) {
    if (!s || !s->width || s->width > 2048 || !s->height || s->height > 2048 ||
        s->bytes < 128 || s->bytes > 16u * 1024u * 1024u || !span(s->encoded, s->bytes)) return 0;
    uint32_t at = 128;
    for (uint32_t y = 0; y < s->height; ++y) {
        uint32_t x = 0;
        while (x < s->width) {
            if (at >= s->bytes) return 0;
            uint32_t n = 1, v = s->encoded[at++];
            if ((v & 192) == 192) {
                n = v & 63;
                if (!n || at >= s->bytes) return 0;
                ++at;
            }
            if (n > s->width - x) return 0;
            x += n;
        }
    }
    return 1;
}
static int pcx_target(const struct MnmPcxInput *s, const void *pixels, uint32_t bytes) {
    return mnm_pcx_admit(s) && span(pixels, bytes) &&
        !overlaps(pixels, bytes, s->encoded, s->bytes) &&
        (!s->palette || (span(s->palette, 768) && !overlaps(pixels, bytes, s->palette, 768)));
}
int mnm_pcx_words(const struct MnmPixelPlane *p, const struct MnmPcxInput *s) {
    if (!mnm_pixel_plane(p) || !s || !s->palette || s->width > p->width || s->height > p->height ||
        !pcx_target(s, p->pixels, p->stride * p->height * 2)) return 0;
    uint32_t at = 128;
    for (uint32_t y = 0; y < s->height; ++y) for (uint32_t x = 0; x < s->width;) {
        uint32_t n = 1, v = s->encoded[at++];
        if ((v & 192) == 192) { n = v & 63; v = s->encoded[at++]; }
        const uint16_t c = colour(s->palette + v * 3, p->format);
        for (uint32_t i = 0; i < n; ++i) p->pixels[y * p->stride + x++] = c;
    }
    return 1;
}
int mnm_pcx_indices(uint8_t *p, uint32_t bytes, uint32_t stride, const struct MnmPcxInput *s) {
    if (!s || stride < s->width || stride > 4096 || s->height > 2048 ||
        bytes < stride * s->height || !pcx_target(s, p, bytes)) return 0;
    uint32_t at = 128;
    for (uint32_t y = 0; y < s->height; ++y) for (uint32_t x = 0; x < s->width;) {
        uint32_t n = 1, v = s->encoded[at++];
        if ((v & 192) == 192) { n = v & 63; v = s->encoded[at++]; }
        for (uint32_t i = 0; i < n; ++i) p[y * stride + x++] = (uint8_t)v;
    }
    return 1;
}
int mnm_dib_layout(const struct MnmDibInput *s, uint32_t *w, uint32_t *h, uint32_t *stride) {
    if (!s || !span(s->info, s->info_bytes) || s->info_bytes < 40) return 0;
    const uint8_t *b = s->info;
    const uint32_t width = u32(b + 4), height = u32(b + 8), bits = u16(b + 14);
    if (u32(b) != 40 || !width || width > 2048 || !height || height > 2048 || u16(b + 12) != 1 ||
        u32(b + 16) || (bits != 1 && bits != 4 && bits != 8 && bits != 24) || u32(b + 32) || u32(b + 36)) return 0;
    const uint32_t palette = bits == 24 ? 0 : (1u << bits) * 4;
    const uint32_t pitch = ((width * bits + 31) / 32) * 4, size = pitch * height;
    if (s->usage != (bits == 24 ? 1u : 0u) || s->info_bytes < 40 + palette || s->pixel_bytes < size ||
        (u32(b + 20) && u32(b + 20) != size) || !span(s->pixels, s->pixel_bytes)) return 0;
    if (w) *w = width;
    if (h) *h = height;
    if (stride) *stride = pitch;
    return 1;
}
int mnm_dib_words(const struct MnmPixelPlane *p, const struct MnmDibInput *s, int32_t x, int32_t y) {
    uint32_t w, h, stride;
    if (!mnm_pixel_plane(p) || !mnm_dib_layout(s, &w, &h, &stride) ||
        overlaps(p->pixels, p->stride * p->height * 2, s->info, s->info_bytes) ||
        overlaps(p->pixels, p->stride * p->height * 2, s->pixels, s->pixel_bytes)) return 0;
    const uint32_t bits = u16(s->info + 14);
    for (uint32_t j = 0; j < h; ++j) {
        const int64_t dy = (int64_t)y + j;
        if (dy < 0 || dy >= p->height) continue;
        const uint8_t *row = s->pixels + (h - 1 - j) * stride;
        for (uint32_t i = 0; i < w; ++i) {
            const int64_t dx = (int64_t)x + i;
            if (dx < 0 || dx >= p->width) continue;
            uint32_t index = bits == 8 ? row[i] : bits == 4 ? (row[i / 2] >> (i % 2 ? 0 : 4)) & 15 : (row[i / 8] >> (7 - i % 8)) & 1;
            const uint8_t *bgr = bits == 24 ? row + i * 3 : s->info + 40 + index * 4;
            const uint8_t rgb[3] = {bgr[2], bgr[1], bgr[0]};
            p->pixels[(uint32_t)dy * p->stride + (uint32_t)dx] = colour(rgb, p->format);
        }
    }
    return 1;
}
int mnm_rgb_add(const struct MnmPixelPlane *p, int32_t l, int32_t t, int32_t r, int32_t b,
                uint16_t red, uint16_t green, uint16_t blue) {
    if (!mnm_pixel_plane(p) || l < 0 || t < 0 || r <= l || b <= t || (uint32_t)r > p->width || (uint32_t)b > p->height) return 0;
    const uint32_t shift = p->format ? 10 : 11, gm = p->format ? 31 : 63;
    for (int32_t y = t; y < b; ++y) for (int32_t x = l; x < r; ++x) {
        const uint16_t old = p->pixels[(uint32_t)y * p->stride + (uint32_t)x];
        uint32_t a = (uint16_t)(((old >> shift) & 31) + red);
        uint32_t c = (uint16_t)(((old >> 5) & gm) + green), d = (uint16_t)((old & 31) + blue);
        if (a > 31) a = 31;
        if (c > gm) c = gm;
        if (d > 31) d = 31;
        p->pixels[(uint32_t)y * p->stride + (uint32_t)x] = (uint16_t)(a << shift | c << 5 | d);
    }
    return 1;
}
int mnm_pixel_whitewash(const struct MnmPixelPlane *p) {
    if (!mnm_pixel_plane(p)) return 0;
    const uint32_t shift = p->format ? 10 : 11, gm = p->format ? 31 : 63, add = p->format ? 16 : 32;
    /* The original consumes a contiguous width*height prefix, including padding. */
    for (uint32_t i = 0; i < p->width * p->height; ++i) {
        uint32_t old = p->pixels[i];
        p->pixels[i] = (uint16_t)((((old >> shift) & 31) / 2 + 16) << shift |
            ((((old >> 5) & gm) / 2 + add) << 5) | ((old & 31) / 2 + 16));
    }
    return 1;
}
int mnm_pixel_copy(const struct MnmPixelPlane *d, const struct MnmPixelPlane *s, uint32_t w, uint32_t h) {
    if (!mnm_pixel_plane(d) || !mnm_pixel_plane(s) || !w || !h ||
        w > d->width || w > s->width || h > d->height || h > s->height) return 0;
    for (uint32_t y = 0; y < h; ++y) for (uint32_t x = 0; x < w; ++x)
        d->pixels[y * d->stride + x] = s->pixels[y * s->stride + x];
    return 1;
}
