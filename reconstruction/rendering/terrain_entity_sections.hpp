#pragma once
#include "terrain_entity_rotation.hpp"
#include "terrain_sections.hpp"
namespace mnm::reconstruction {
struct TerrainEntitySectionResult {
  std::vector<std::size_t> resetCells;
};
// Full selected square copy, including linked sequence selection. Resets are
// explicit asset-stage empty cells: no foreign controller/reference is
// borrowed. Returned physical destination indices let runtime initialization
// retain reset provenance. Source assets stay immutable, including aliased
// square copies.
TerrainEntitySectionResult copyTerrainEntitySection(
    assets::MapAsset &, const assets::MapAsset &,
    const assets::TerrainCatalog &, const assets::Animation &, unsigned side,
    unsigned sourceX, unsigned sourceY, unsigned destinationX,
    unsigned destinationY, unsigned rotation);
assets::MapAsset assembleTerrainEntityRegion(
    unsigned columns, unsigned rows, unsigned side, unsigned layers,
    const std::vector<TerrainSection> &, const assets::TerrainCatalog &,
    const assets::Animation &);
} // namespace mnm::reconstruction
