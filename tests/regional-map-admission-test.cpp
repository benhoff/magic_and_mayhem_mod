#include "../apps/world-scene/map_navigation.hpp"
#include <iostream>
#include <stdexcept>
int main()try{
  using namespace mnm;assets::MapAsset map;map.width=map.height=64;map.layers=16;map.cells.resize(65536);
  for(auto &v:map.cells)v.references.fill(0xffff);
  for(unsigned i=0;i<4096;++i)map.cells[i].definition=1;
  assets::TerrainCatalog catalog;catalog.records.resize(2);catalog.records[1][0x94]=16;catalog.records[1][0xb0]=8;
  auto projected=scene::projectMapNavigation(map,catalog,{0,0,64,64});
  if(projected.geometry.cells.size()!=65536||projected.standing.size()!=62*62)throw std::runtime_error("expanded navigation admission differs");
  const auto payload=scene::mapPayload(projected.geometry);auto decoded=assets::decodeMapPayload(payload);
  if(!std::holds_alternative<assets::MapAsset>(decoded))throw std::runtime_error("expanded navigation MAP roundtrip");
  scene::validateMapGeometry(std::get<assets::MapAsset>(decoded),catalog,projected.frozen);
  auto refuses=[](auto fn){bool rejected=false;try{fn();}catch(const std::exception &){rejected=true;}if(!rejected)throw std::runtime_error("expanded navigation exceeded bound");};
  refuses([&]{scene::projectMapNavigation(map,catalog,{0,0,65,64});});
  map.layers=17;map.cells.resize(64*64*17);for(auto &v:map.cells)v.references.fill(0xffff);
  refuses([&]{scene::projectMapNavigation(map,catalog,{0,0,64,64});});
  std::cout<<"{\"success\":true,\"cells\":65536,\"over_budget_rejected\":true}\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
