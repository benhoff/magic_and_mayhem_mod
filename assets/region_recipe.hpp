#pragma once
#include "persistence.hpp"
namespace mnm::assets {
struct RegionSpecific {std::uint32_t section=0;std::int32_t rotation=0,column=0,row=0;};
struct RegionRandom {std::uint32_t section=0,occurrences=0;};
struct RegionRecipe {
 std::uint32_t id=0,columns=0,rows=0;
 std::string name,path,spritePath,prefix;
 std::vector<RegionSpecific> specific;
 std::vector<RegionRandom> random;
};
// Selective owned reader: unrelated campaign/balance keys stay outside scope.
// Decodes plain CFG payloads, preserving list order and wildcard -1.
PersistenceResult<std::vector<RegionRecipe>> decodeRegionRecipes(const Bytes&,const PersistenceLimits& = {});
PersistenceResult<std::vector<RegionRecipe>> loadRegionRecipes(AssetFile&,bool packed=false,const PersistenceLimits& = {});
std::string regionSectionPath(const RegionRecipe&,std::uint32_t section);
}
