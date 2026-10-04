#pragma once
#include "../../assets/map.hpp"
#include "../../assets/terrain_catalog.hpp"
namespace mnm::reconstruction {
// Selected 004ecf20 geometry pass, preserving top-layer cells and runtime references.
void deriveTerrainSurfaces(assets::MapAsset&,const assets::TerrainCatalog&);
struct TerrainGeometry {
    assets::MapAsset map;
    std::uint32_t projectedObjects=0,projectedReferences=0,changedCells=0,removedDefinitions=0;
};
// Owned ordinary-terrain projection: omit object/creature references and flag paths,
// apply selected initial cell fields, then derive surface flags. No entities/lights.
TerrainGeometry prepareTerrainGeometry(const assets::MapAsset&,const assets::TerrainCatalog&);
}
