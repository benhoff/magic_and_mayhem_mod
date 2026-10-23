#pragma once
#include "pixel_kernel.h"
#ifdef __cplusplus
extern "C" {
#endif
struct MnmMinimapView {
    int32_t grid_width, grid_height, center_x, center_y, realm_width, realm_height, origin_x, origin_y;
    uint32_t orientation;
};
struct MnmMinimapTerrain {
    uint32_t width, height, center_x, center_y, orientation;
    int32_t root_x, root_y;
    /* Three source bytes per cell: little-endian colour WORD, hidden0/1. */
    const uint8_t *cells;
};
struct MnmMinimapCamera {
    int32_t viewport_width, viewport_height, directions[8];
    uint32_t border;
    uint16_t border_colour;
};
int mnm_minimap_terrain(const struct MnmPixelPlane *, const struct MnmPixelPlane *history, const struct MnmMinimapTerrain *);
int mnm_minimap_cell(const struct MnmPixelPlane *, const struct MnmMinimapView *, int32_t x, int32_t y,
                      uint16_t colour, uint32_t ring, uint32_t *returned_word);
int mnm_minimap_cell_at(const struct MnmPixelPlane *, int32_t x, int32_t y, uint16_t colour, uint32_t ring, uint32_t *returned_word);
int mnm_minimap_creature(const struct MnmPixelPlane *, int32_t x, int32_t y, uint16_t colour);
int mnm_minimap_camera(const struct MnmPixelPlane *, const struct MnmMinimapView *, const struct MnmMinimapCamera *);
int mnm_minimap_terrain_with_store(const struct MnmPixelPlane *,const struct MnmPixelPlane *,const struct MnmMinimapTerrain *,MnmPixelStore16);
int mnm_minimap_cell_with_store(const struct MnmPixelPlane *,const struct MnmMinimapView *,int32_t,int32_t,uint16_t,uint32_t,uint32_t *,MnmPixelStore16);
int mnm_minimap_cell_at_with_store(const struct MnmPixelPlane *,int32_t,int32_t,uint16_t,uint32_t,uint32_t *,MnmPixelStore16);
int mnm_minimap_creature_with_store(const struct MnmPixelPlane *,int32_t,int32_t,uint16_t,MnmPixelStore16);
int mnm_minimap_camera_with_store(const struct MnmPixelPlane *,const struct MnmMinimapView *,const struct MnmMinimapCamera *,MnmPixelStore16);
#ifdef __cplusplus
}
#endif
