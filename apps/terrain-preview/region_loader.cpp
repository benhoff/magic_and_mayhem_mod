#include "region_loader.hpp"
#include <stdexcept>
namespace mnm::preview {
namespace {
std::unique_ptr<assets::AssetFile> open(const assets::AssetStore& store,const std::string& path){auto r=store.open(path);if(auto* e=std::get_if<assets::Error>(&r))throw std::runtime_error(path+": "+e->detail);return std::get<std::unique_ptr<assets::AssetFile>>(std::move(r));}
}
assets::RegionRecipe loadTerrainRecipe(const assets::AssetStore& store,const std::string& config,unsigned id){auto f=open(store,config);auto r=assets::loadRegionRecipes(*f);if(auto* e=std::get_if<assets::PersistenceError>(&r))throw std::runtime_error(e->detail);for(auto& recipe:std::get<std::vector<assets::RegionRecipe>>(r))if(recipe.id==id)return std::move(recipe);throw std::out_of_range("Region recipe not found");}
LoadedFixedRegion loadFixedTerrainRegion(const assets::AssetStore& store,const assets::RegionRecipe& recipe,const assets::TerrainCatalog& t){
 if(!recipe.random.empty())throw std::invalid_argument("Region requires random constraint selection");
 for(const auto& s:recipe.specific)if(s.rotation<0 || s.column<0 || s.row<0)throw std::invalid_argument("Region requires wildcard placement/rotation");
 LoadedFixedRegion result;std::vector<assets::MapAsset> maps;maps.reserve(recipe.specific.size());std::size_t cells=0;
 for(const auto& s:recipe.specific){result.paths.push_back(assets::regionSectionPath(recipe,s.section));auto f=open(store,result.paths.back());auto r=assets::loadMap(*f);if(auto* e=std::get_if<assets::PersistenceError>(&r))throw std::runtime_error(e->detail);maps.push_back(std::get<assets::MapAsset>(std::move(r)));cells+=maps.back().cells.size();if(cells>4u*128*128*32)throw std::out_of_range("Region source storage");}
 std::vector<const assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);
 result.plan=reconstruction::planFixedTerrainRegion(recipe,sources);result.assembly=reconstruction::assembleFixedTerrainRegion(result.plan,sources,t);return result;
}
}
