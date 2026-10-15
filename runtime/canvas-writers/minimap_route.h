#pragma once
#include "../../reconstruction/rendering/minimap_pixels.h"
#ifdef __cplusplus
extern "C" {
#endif
enum { MNM_MAP_TERRAIN, MNM_MAP_CELL, MNM_MAP_CREATURE, MNM_MAP_CAMERA0, MNM_MAP_SITES=7 };
typedef int (*MnmMapResolve)(uint32_t *,unsigned,struct MnmPixelPlane *,struct MnmPixelPlane *,struct MnmMinimapTerrain *,struct MnmMinimapView *,struct MnmMinimapCamera *,uint32_t *);
typedef void (*MnmMapRelease)(const struct MnmMinimapTerrain *);
extern MnmMapResolve mnm_map_resolve;
extern MnmMapRelease mnm_map_release;
extern uintptr_t mnm_map_original[MNM_MAP_SITES],mnm_map_resume[MNM_MAP_SITES];
extern uint32_t mnm_map_native[MNM_MAP_SITES],mnm_map_forwarded[MNM_MAP_SITES];
int mnm_map_route(uint32_t *,unsigned);
#ifdef __cplusplus
}
#endif
