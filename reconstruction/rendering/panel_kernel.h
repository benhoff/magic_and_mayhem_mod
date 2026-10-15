#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Original RGB565/RGB555 panel passes on an admitted, initialized locked plane.
 * Bounds are native admission policy; refusal performs no destination writes. */
struct MnmPanelPlane {
    uint16_t *pixels;
    uint32_t width, height, stride, format;
};
struct MnmPanelRequest {
    int32_t left, top, right, bottom;
    uint32_t mode, percent, pressed, reverse;
};
int mnm_panel_admit(const struct MnmPanelPlane *, const struct MnmPanelRequest *);
int mnm_panel_draw(const struct MnmPanelPlane *, const struct MnmPanelRequest *);
#ifdef __cplusplus
}
#endif
