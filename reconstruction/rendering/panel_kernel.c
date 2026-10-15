#include "panel_kernel.h"
#include <stddef.h>

int mnm_panel_admit(const struct MnmPanelPlane *p, const struct MnmPanelRequest *r) {
    if (!p || !r || !p->pixels || !p->width || p->width > 2048 ||
        !p->height || p->height > 2048 || p->stride < p->width ||
        p->stride > 4096 || p->format > 1 || r->mode < 15 || r->mode > 17 ||
        r->percent > 1000 || r->left < 0 || r->top < 0 ||
        r->right < r->left || r->bottom < r->top)
        return 0;
    const uint32_t extra = r->mode == 15 ? 2 : r->mode == 16 ? 1 : 0;
    if (p->width < extra || p->height < extra ||
        (uint32_t)r->right > p->width - extra ||
        (uint32_t)r->bottom > p->height - extra)
        return 0;
    const uintptr_t address = (uintptr_t)p->pixels;
    return address <= UINTPTR_MAX - (uintptr_t)p->stride * p->height * 2;
}

static void pixel(const struct MnmPanelPlane *p, const struct MnmPanelRequest *r,
                  int32_t x, int32_t y, int sign) {
    uint16_t *at = p->pixels + (uint32_t)y * p->stride + (uint32_t)x;
    const uint32_t old = *at, red_shift = p->format ? 10 : 11;
    const uint32_t green_bits = p->format ? 31 : 63;
    const uint32_t green_shift = p->format ? 3 : 2;
    uint32_t c[3] = {((old >> red_shift) & 31) << 3,
                     ((old >> 5) & green_bits) << green_shift, (old & 31) << 3};
    for (unsigned i = 0; i < 3; ++i) {
        if (r->mode == 15) {
            int32_t v = (int32_t)c[i] + sign * (int32_t)(c[i] * r->percent / 100);
            c[i] = v < 0 ? 0 : v > 255 ? 255 : (uint32_t)v;
        } else {
            c[i] = (c[i] >> 1) + 128;
        }
    }
    *at = (uint16_t)(((c[0] >> 3) << red_shift) |
                     ((c[1] >> green_shift) << 5) | (c[2] >> 3));
}

int mnm_panel_draw(const struct MnmPanelPlane *p, const struct MnmPanelRequest *r) {
    if (!mnm_panel_admit(p, r)) return 0;
    if (r->mode == 17) {
        for (int32_t y = r->top; y < r->bottom; ++y)
            for (int32_t x = r->left; x < r->right; ++x) pixel(p, r, x, y, 0);
    } else if (r->mode == 16) {
        for (int32_t x = r->left; x < r->right; ++x) pixel(p, r, x, r->top, 0);
        for (int32_t x = r->left + 1; x <= r->right; ++x) pixel(p, r, x, r->bottom, 0);
        for (int32_t y = r->top + 1; y <= r->bottom; ++y) pixel(p, r, r->left, y, 0);
        for (int32_t y = r->top; y < r->bottom; ++y) pixel(p, r, r->right, y, 0);
    } else {
        const int32_t right = r->right + 1, bottom = r->bottom + 1;
        const int sign = r->pressed ? 1 : -1;
        const int32_t rows[4] = {r->top, r->top + 1, bottom, bottom - 1};
        const int32_t cols[4] = {r->left, r->left + 1, right, right - 1};
        for (unsigned j = 0; j < 4; ++j) {
            const int inset = j >= 2 ? 2 : 0;
            const int direction = j >= 2 && r->reverse ? -sign : sign;
            for (int32_t x = r->left + inset; x < right; ++x)
                pixel(p, r, x, rows[j], direction);
        }
        for (unsigned j = 0; j < 4; ++j) {
            const int inset = j < 2 ? 2 : 0;
            const int direction = j >= 2 && r->reverse ? -sign : sign;
            for (int32_t y = r->top + inset; y < bottom; ++y)
                pixel(p, r, cols[j], y, direction);
        }
    }
    return 1;
}
