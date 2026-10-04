#pragma once
#include "terrain_sections.hpp"
#include "../../assets/region_recipe.hpp"
namespace mnm::reconstruction {
struct RegionBlock {unsigned source=0,sourceX=0,sourceY=0,column=0,row=0,rotation=0;};
struct FixedRegionPlan {unsigned columns=0,rows=0,side=0,layers=0;std::vector<RegionBlock> blocks;};
// Authored placement only: no random candidates, wildcard/fallback rotation or
// edge matching. Sources correspond in order to the recipe's Specific entries.
// Original section anchor is source block (0,0), with toroidal block placement.
FixedRegionPlan planFixedTerrainRegion(const assets::RegionRecipe&,const std::vector<const assets::MapAsset*>&);
// Owned ordinary projection + mixed-height copy; geometry must run afterwards.
struct FixedRegionAssembly {assets::MapAsset map;unsigned projectedObjects=0,projectedReferences=0;};
FixedRegionAssembly assembleFixedTerrainRegion(const FixedRegionPlan&,const std::vector<const assets::MapAsset*>&,const assets::TerrainCatalog&);
}
