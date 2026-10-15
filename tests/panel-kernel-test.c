#include "../reconstruction/rendering/panel_kernel.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    uint16_t words[65536];
    unsigned refusals = 0;
    for (unsigned format = 0; format < 2; ++format) {
        struct MnmPanelPlane p = {words, 256, 256, 256, format};
        struct MnmPanelRequest r = {0, 0, 256, 256, 17, 0, 0, 0};
        for (unsigned i = 0; i < 65536; ++i) words[i] = (uint16_t)i;
        assert(mnm_panel_draw(&p, &r));
        for (unsigned i = 0; i < 65536; ++i) {
            const unsigned shift = format ? 10 : 11, bits = format ? 31 : 63;
            const unsigned red = (((i >> shift) & 31) >> 1) + 16;
            const unsigned green = (((i >> 5) & bits) >> 1) + (format ? 16 : 32);
            const unsigned blue = ((i & 31) >> 1) + 16;
            assert(words[i] == (uint16_t)((red << shift) | (green << 5) | blue));
        }
    }
    struct MnmPanelPlane p = {words, 16, 13, 19, 0};
    for (unsigned i = 0; i < 65536; ++i) words[i] = 0x39e7;
    uint16_t prior[65536]; memcpy(prior, words, sizeof words);
    for (unsigned mode = 15; mode <= 17; ++mode) {
        const struct MnmPanelRequest bad[] = {
            {-1,0,4,4,mode,40,0,0}, {0,-1,4,4,mode,40,0,0},
            {4,0,3,4,mode,40,0,0}, {0,4,4,3,mode,40,0,0},
            {0,0,17,4,mode,40,0,0}, {0,0,4,14,mode,40,0,0},
            {0,0,4,4,mode,1001,0,0}, {0,0,INT32_MAX,INT32_MAX,mode,40,0,0}
        };
        for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; ++i) {
            assert(!mnm_panel_draw(&p, bad + i));
            assert(!memcmp(prior, words, sizeof words)); ++refusals;
        }
    }
    struct MnmPanelRequest r = {0,0,4,4,15,40,0,0};
    const struct MnmPanelPlane invalid[] = {
        {0,16,13,19,0}, {words,0,13,19,0}, {words,16,0,19,0},
        {words,2049,13,2049,0}, {words,16,2049,19,0}, {words,16,13,15,0},
        {words,16,13,4097,0}, {words,16,13,19,2}, {words,1,1,1,0},
        {(uint16_t *)(UINTPTR_MAX - 8),16,13,19,0}
    };
    for (unsigned i = 0; i < sizeof invalid / sizeof invalid[0]; ++i) {
        assert(!mnm_panel_draw(invalid+i, &r));assert(!memcmp(prior, words, sizeof words));++refusals;
    }
    printf("{\"success\":true,\"exhaustive_words\":131072,\"atomic_refusals\":%u}\n", refusals);
    return 0;
}
