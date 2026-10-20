#pragma once
#include "../../assets/map.hpp"
#include "../../assets/terrain_catalog.hpp"
namespace mnm::reconstruction {
struct TerrainFrameWrite {bool accepted=false,redrawDirty=false;};
// 004ed440: touched layer and the one below, clipped below the highest layer,
// each with a wrapped3x3 XY neighborhood. Occupancy is the owned result of the
// original artifact query; an empty vector explicitly describes no artifacts.
// Returns the original final-neighbor redraw dirty decision; never clears prior dirt.
bool refreshTerrainCellNeighborhood(assets::MapAsset&,std::size_t cell,
 const assets::TerrainCatalog&,const std::vector<std::uint8_t>& artifactOccupancy={});
// 004eb710. Out-of-range requests above original count are successful no-ops;
// the original one-past count and invalid old/neighbor definitions refuse safely.
// Frame/classification and every local flag patch publish together.
TerrainFrameWrite setTerrainLinkedFrame(assets::MapAsset&,std::size_t cell,
 const assets::TerrainCatalog&,std::uint32_t frame,
 const std::vector<std::uint8_t>& artifactOccupancy={});
}
