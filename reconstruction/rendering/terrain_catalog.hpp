#pragma once
#include "terrain_generation.hpp"
#include "../../assets/region_recipe.hpp"
namespace mnm::reconstruction {
struct TerrainRegionCatalog {
    unsigned columns=0,rows=0,side=0,layers=0;
    RegionDescriptorBank bank;
    RegionConnectorState connectors; // Final counters after expansion.
    std::vector<RegionSpecificRequest> requests;
    std::vector<unsigned> firstDescriptors,sourceForDescriptor;
};
// Sources correspond to Specific entries followed by Random entries, including
// repeated section IDs. Consumes header fields only, retaining no source pointers.
TerrainRegionCatalog buildTerrainRegionCatalog(const assets::RegionRecipe&,
    const std::vector<const assets::MapAsset*>&);
}
