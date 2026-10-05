#include "region_loader.hpp"
#include <stdexcept>
namespace mnm::preview {
namespace {
std::unique_ptr<assets::AssetFile> open(const assets::AssetStore& store,const std::string& path){auto r=store.open(path);if(auto* e=std::get_if<assets::Error>(&r))throw std::runtime_error(path+": "+e->detail);return std::get<std::unique_ptr<assets::AssetFile>>(std::move(r));}
std::vector<assets::MapAsset> loadSources(const assets::AssetStore& store,const assets::RegionRecipe& recipe,std::vector<std::string>& paths,bool random){
 std::vector<assets::MapAsset> maps;maps.reserve(recipe.specific.size()+(random?recipe.random.size():0));std::size_t cells=0;
 const auto load=[&](unsigned section){paths.push_back(assets::regionSectionPath(recipe,section));auto f=open(store,paths.back());auto r=assets::loadMap(*f);if(auto* e=std::get_if<assets::PersistenceError>(&r))throw std::runtime_error(paths.back()+": "+e->detail);maps.push_back(std::get<assets::MapAsset>(std::move(r)));cells+=maps.back().cells.size();if(cells>4u*128*128*32)throw std::out_of_range("Region source storage");};
 for(const auto& s:recipe.specific)load(s.section);
 if(random)for(const auto& s:recipe.random)load(s.section);
 return maps;
}
}
assets::RegionRecipe loadTerrainRecipe(const assets::AssetStore& store,const std::string& config,unsigned id){auto f=open(store,config);auto r=assets::loadRegionRecipes(*f);if(auto* e=std::get_if<assets::PersistenceError>(&r))throw std::runtime_error(e->detail);for(auto& recipe:std::get<std::vector<assets::RegionRecipe>>(r))if(recipe.id==id)return std::move(recipe);throw std::out_of_range("Region recipe not found");}
LoadedFixedRegion loadFixedTerrainRegion(const assets::AssetStore& store,const assets::RegionRecipe& recipe,const assets::TerrainCatalog& t){
 if(!recipe.random.empty())throw std::invalid_argument("Region requires random constraint selection");
 for(const auto& s:recipe.specific)if(s.rotation<0 || s.column<0 || s.row<0)throw std::invalid_argument("Region requires wildcard placement/rotation");
 LoadedFixedRegion result;auto maps=loadSources(store,recipe,result.paths,false);
 std::vector<const assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);
 result.plan=reconstruction::planFixedTerrainRegion(recipe,sources);result.assembly=reconstruction::assembleFixedTerrainRegion(result.plan,sources,t);return result;
}
LoadedGeneratedRegion loadGeneratedTerrainRegion(const assets::AssetStore& store,const assets::RegionRecipe& recipe,const assets::TerrainCatalog& t,std::uint32_t seed){
 LoadedGeneratedRegion result;auto maps=loadSources(store,recipe,result.paths,true);
 std::vector<const assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);
 result.generated=reconstruction::generateTerrainRegionPlan(recipe,sources,seed);
 if(result.generated.plan)result.assembly=reconstruction::assembleFixedTerrainRegion(*result.generated.plan,sources,t);
 return result;
}
}
