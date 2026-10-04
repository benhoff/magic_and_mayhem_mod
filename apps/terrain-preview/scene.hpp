#pragma once
#include "sprite.hpp"
#include "terrain_catalog.hpp"
#include "terrain_submission.hpp"
#include "sprite_visibility.hpp"
#include "map.hpp"
#include "terrain_traversal.hpp"
namespace mnm::preview {
struct TerrainPreviewTile {std::uint32_t definition=0;reconstruction::TerrainTile state;std::size_t cell=0;};
struct TerrainMapRegion {std::uint32_t x=0,y=0,layer=0,width=3,height=3;};
// Explicit one-layer slice. Anchors are preview layout, while depth coordinates,
// definition IDs and initial flag words come from the MAP grid.
std::vector<TerrainPreviewTile> mapRegionTiles(const assets::MapAsset&,TerrainMapRegion,bool overlap=false);
std::vector<TerrainPreviewTile> worldTerrainTiles(const assets::MapAsset&,const reconstruction::TerrainCamera&);
struct TerrainPreviewDraw {std::size_t tile=0;reconstruction::TerrainDraw draw;};
struct TerrainPreviewResult {render::Image pixels;QImage image;std::vector<TerrainPreviewDraw> queue;std::vector<reconstruction::VisibilityOwner> owners;};
// Application orchestration: explicit bounded tiles, no simulation or widgets.
TerrainPreviewResult renderTerrain(render::GlBlitter&,const assets::TerrainCatalog&,
    const assets::Sprite&,std::vector<TerrainPreviewTile>,reconstruction::TerrainAdmission,bool visibility,bool world=false);
}
