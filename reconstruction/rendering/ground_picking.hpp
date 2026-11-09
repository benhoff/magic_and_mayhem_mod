#pragma once
#include "terrain_traversal.hpp"
#include <cstdint>
#include <optional>
#include <vector>

namespace mnm::reconstruction {
// Owned and validated opacity, independent of palette indices / pixel colours.
class PickingMask {
public:
    PickingMask(std::uint32_t width, std::uint32_t height,
                std::int32_t originX, std::int32_t originY,
                std::vector<std::uint8_t> opaque);
    bool hit(std::int32_t anchorX, std::int32_t anchorY,
             std::int32_t pointX, std::int32_t pointY) const noexcept;
private:
    std::uint32_t width_, height_;
    std::int32_t originX_, originY_;
    std::vector<std::uint8_t> opaque_;
};
struct GroundTile {
    std::uint16_t definition=0, flags8=0, flags10=0;
};
struct PickedGround {
    // Traversal annotations, deliberately distinct from physical grid indices.
    std::uint32_t column, row, layer;
};
struct GroundPickingResult {
    std::optional<PickedGround> ground;
    std::int32_t terrainDepth=0;
    std::uint32_t depthRow=0, submittedRows=0;
    std::vector<std::array<std::uint16_t,2>> flags;
};
// The viewport is the existing original traversal rectangle, not a new camera
// or screen-to-world approximation. All inputs/output are owned values.
GroundPickingResult pickWorldGround(std::uint32_t width, std::uint32_t height,
    std::uint32_t layers, const TerrainCamera&, TerrainViewport,
    std::uint32_t player, bool forceVisible,
    const std::vector<GroundTile>&, const std::vector<TerrainDefinition>&,
    const std::vector<PickingMask>&, std::int32_t pointX, std::int32_t pointY);

enum class WorldHitClass : std::uint32_t { none=0, ground=1, actor=2, artifact=3 };
struct WorldHitDepths {
    std::int32_t actor=0, artifact=0, terrain=0;
    bool actorPresent=false, artifactPresent=false, groundPresent=false;
};
WorldHitClass classifyWorldHit(const WorldHitDepths&) noexcept;
}
