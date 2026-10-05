#pragma once
#include "terrain_catalog.hpp"
#include "terrain_region.hpp"
namespace mnm::reconstruction {
// Converts complete owned assignments into concrete ordinary section-copy
// requests. Source identity is catalog entry identity, not just section number.
FixedRegionPlan planGeneratedTerrainRegion(const TerrainRegionCatalog&,const RegionGenerationResult&);
struct GeneratedTerrainRegionPlan {
    TerrainRegionCatalog catalog;
    RegionGenerationResult generation;
    std::optional<FixedRegionPlan> plan; // Ten failed attempts retain state without a plan.
};
GeneratedTerrainRegionPlan generateTerrainRegionPlan(const assets::RegionRecipe&,
    const std::vector<const assets::MapAsset*>&,std::uint32_t seed);
}
