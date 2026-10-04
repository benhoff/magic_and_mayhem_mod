#pragma once
#include "terrain_region.hpp"
#include "asset_file.hpp"
namespace mnm::preview {
assets::RegionRecipe loadTerrainRecipe(const assets::AssetStore&,const std::string& config,unsigned id);
struct LoadedFixedRegion {reconstruction::FixedRegionAssembly assembly;reconstruction::FixedRegionPlan plan;std::vector<std::string> paths;};
LoadedFixedRegion loadFixedTerrainRegion(const assets::AssetStore&,const assets::RegionRecipe&,const assets::TerrainCatalog&);
}
