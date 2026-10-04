#pragma once
#include "terrain_traversal.hpp"
namespace mnm::reconstruction {
// Selected setters only; allocation/resource acquisition and lifecycle are external.
void bindTerrainCamera(TerrainCamera&,std::uint32_t width,std::uint32_t height,std::uint32_t layers);
void setTerrainCameraViewport(TerrainCamera&,TerrainViewport);
void setTerrainCameraPosition(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::uint32_t layers,std::uint32_t x,std::uint32_t y,std::uint32_t z);
void scrollTerrainCamera(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::int32_t horizontal,std::int32_t vertical);
}
