#include "panel_route.h"
MnmPanelResolve mnm_panel_resolve;
uintptr_t mnm_panel_original[3], mnm_panel_resume[3];
uint32_t mnm_panel_native[3], mnm_panel_forwarded[3];
void (*mnm_panel_notice)(unsigned,int);
int mnm_panel_route(uint32_t *r, unsigned kind) {
    if (kind >= 3) return 0;
    struct MnmPanelPlane plane = {0};
    struct MnmPanelRequest request = {0};
    if (!mnm_panel_resolve || !mnm_panel_resolve(r, kind, &plane, &request) ||
        request.mode != 15 + kind || !mnm_panel_draw(&plane, &request)) {
        ++mnm_panel_forwarded[kind];if(mnm_panel_notice)mnm_panel_notice(kind,0);
        return 0;
    }
    /* Skip only the pixel passes. Rematerialize operands used by the original
     * Unlock tail. Private loop scratch is not exposed as a host contract. */
    uint32_t *stack = r + 9;
    if (kind == 0) {
        stack[0x28 / 4] = (uint32_t)(uintptr_t)plane.pixels;
        r[7] = 4;
        r[8] = (r[8] & ~0x8d5u) | 0x44u; /* CMP(4,4) */
    } else if (kind == 1) {
        r[7] = (uint32_t)(uintptr_t)plane.pixels;
        r[0] = stack[0x4c / 4];
        r[6] = 0;
        r[8] = (r[8] & ~0x8d5u) | 0x44u; /* final DEC(1) */
    } else {
        r[4] = (uint32_t)(uintptr_t)plane.pixels;
        r[0] = stack[0x14 / 4];
        r[5] = (uint32_t)request.bottom;
        r[8] = (r[8] & ~0x8d5u) | 0x44u; /* final CMP(bottom,bottom) */
    }
    ++mnm_panel_native[kind];if(mnm_panel_notice)mnm_panel_notice(kind,1);
    return 1;
}
