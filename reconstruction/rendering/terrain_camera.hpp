#pragma once
#include "terrain_traversal.hpp"
#include <optional>
namespace mnm::reconstruction {
// Selected setters only; allocation/resource acquisition and lifecycle are external.
void bindTerrainCamera(TerrainCamera&,std::uint32_t width,std::uint32_t height,std::uint32_t layers);
void setTerrainCameraViewport(TerrainCamera&,TerrainViewport);
void setTerrainCameraPosition(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::uint32_t layers,std::uint32_t x,std::uint32_t y,std::uint32_t z);
struct TerrainCameraFocus {std::int32_t x=0,y=0,z=0;};
// 004f7a00: raw cell words shift with 32-bit wrap before the fine setter.
void setTerrainCameraCellPosition(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::uint32_t layers,std::int32_t x,std::int32_t y,std::int32_t z);
// 004f7ae0: null focus preserves every field, even without a bound map.
// Signed XY wraps to positive fine coordinates; Z retains its raw word bits.
void followTerrainCamera(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::uint32_t layers,std::optional<TerrainCameraFocus>);
void scrollTerrainCamera(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::int32_t horizontal,std::int32_t vertical);
// 004f79b0 clamps whole cells to [-5, extent+5], preserving fractions unless
// the original clamp branch fires (including equality at the upper bound).
void clampTerrainCamera(TerrainCamera&,std::uint32_t width,std::uint32_t height);
// 004f7b30 moves whole map cells with 32-bit wrap, then applies that clamp.
void moveTerrainCameraCells(TerrainCamera&,std::uint32_t width,std::uint32_t height,
    std::int32_t columns,std::int32_t rows);
}
