#include "terrain_entity_updates.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool ok){if(!ok)throw std::runtime_error("Linked terrain update unit mismatch");}
template<class F> static void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
static void word(std::array<std::uint8_t,356>& r,unsigned at,std::uint32_t v){for(unsigned i=0;i<4;++i)r[at+i]=std::uint8_t(v>>(8*i));}
static bool same(const assets::MapAsset& a,const assets::MapAsset& b){if(a.cells.size()!=b.cells.size())return false;for(unsigned i=0;i<a.cells.size();++i){const auto& x=a.cells[i];const auto& y=b.cells[i];if(x.definition!=y.definition||x.references!=y.references||x.flags8!=y.flags8||x.flags10!=y.flags10)return false;}return true;}
int main()try{
 assets::TerrainCatalog catalog;catalog.records.resize(3);catalog.records[1][0xa8]=0x2f;word(catalog.records[1],0x94,7);word(catalog.records[2],0x94,16);
 assets::MapAsset map;map.width=map.height=3;map.layers=3;map.cells.resize(27);for(auto& c:map.cells){c.definition=1;c.references={0xffff,0xffff,0xffff};c.flags8=0x1234;}const auto before=map;
 check(reconstruction::refreshTerrainCellNeighborhood(map,13,catalog));for(unsigned i=0;i<18;++i)check((map.cells[i].flags8&0x78)==0x78);for(unsigned i=18;i<27;++i)check(map.cells[i].flags8==before.cells[i].flags8);
 map.cells[17].flags10|=8;check(!reconstruction::refreshTerrainCellNeighborhood(map,13,catalog));
 map=before;map.cells[13].references[1]=7;auto blocked=map;auto result=reconstruction::setTerrainLinkedFrame(map,13,catalog,2);check(!result.accepted&&same(map,blocked));word(catalog.records[2],0x94,7);result=reconstruction::setTerrainLinkedFrame(map,13,catalog,2);check(!result.accepted&&same(map,blocked));catalog.records[1][0xb0]=catalog.records[2][0xb0]=8;result=reconstruction::setTerrainLinkedFrame(map,13,catalog,2);check(result.accepted&&map.cells[13].definition==2);
 map=blocked;map.cells[13].flags10|=0x80;word(catalog.records[2],0x94,16);result=reconstruction::setTerrainLinkedFrame(map,13,catalog,2);check(result.accepted&&(map.cells[13].flags10&1));map.cells[13].flags10|=3;result=reconstruction::setTerrainLinkedFrame(map,13,catalog,0);check(result.accepted&&!(map.cells[13].flags10&3));
 auto saved=map;refused([&]{reconstruction::setTerrainLinkedFrame(map,13,catalog,3);});check(same(map,saved));check(!reconstruction::setTerrainLinkedFrame(map,13,catalog,4).accepted&&same(map,saved));
 map=before;map.cells[0].definition=999;saved=map;refused([&]{reconstruction::setTerrainLinkedFrame(map,13,catalog,2);});check(same(map,saved));refused([&]{reconstruction::refreshTerrainCellNeighborhood(map,13,catalog,{1});});
 assets::MapAsset tiny;tiny.width=tiny.height=1;tiny.layers=2;tiny.cells.resize(2);for(auto& c:tiny.cells)c.references={0xffff,0xffff,0xffff};reconstruction::refreshTerrainCellNeighborhood(tiny,0,catalog);check(tiny.cells[0].flags8&0x80);reconstruction::refreshTerrainCellNeighborhood(tiny,0,catalog,{1,0});check(!(tiny.cells[0].flags8&0x80));
 std::cout<<"Linked frame admission, local aliases, final-neighbor invalidation and atomic bounds passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
