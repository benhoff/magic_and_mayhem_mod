#pragma once
#include "../../reconstruction/animation/controller.hpp"
#include "terrain_entity_updates.hpp"
namespace mnm::reconstruction {
// 004650d0: previous display in forward mode; next display before a stop in
// reverse mode. Return raw -1 if none. Refuse original out-of-sequence reads.
std::int32_t
terrainLinkedTerminalFrame(const std::vector<assets::AnimationRecord> &,
                           const AnimationControllerState &);
// 004eb820 differs from004eb710: no creature barrier, and local surface refresh
// only for frame zero when the new classification is not16. Class16 returns
// immediately. One-past catalog indices are refused before mutation.
TerrainFrameWrite
setTerrainAnimationFrame(assets::MapAsset &, std::size_t,
                         const assets::TerrainCatalog &, std::uint32_t,
                         const std::vector<std::uint8_t> &occupied = {});
} // namespace mnm::reconstruction
