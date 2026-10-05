#pragma once
#include "terrain_region.hpp"
#include "terrain_generated.hpp"
#include "asset_file.hpp"
namespace mnm::preview {
assets::RegionRecipe loadTerrainRecipe(const assets::AssetStore&,const std::string& config,unsigned id);
struct LoadedFixedRegion {reconstruction::FixedRegionAssembly assembly;reconstruction::FixedRegionPlan plan;std::vector<std::string> paths;};
LoadedFixedRegion loadFixedTerrainRegion(const assets::AssetStore&,const assets::RegionRecipe&,const assets::TerrainCatalog&);
struct LoadedGeneratedRegion {
 reconstruction::GeneratedTerrainRegionPlan generated;
 std::optional<reconstruction::FixedRegionAssembly> assembly;
 std::vector<std::string> paths;
};
// Complete source payloads are temporary; returned plans, state and map own storage.
LoadedGeneratedRegion loadGeneratedTerrainRegion(const assets::AssetStore&,const assets::RegionRecipe&,const assets::TerrainCatalog&,std::uint32_t seed);
}
