#include "terrain_entity_rotation.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool ok){if(!ok)throw std::runtime_error("Linked rotation unit mismatch");}
template<class F> static void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
static void word(std::array<std::uint8_t,356>& bytes,unsigned at,std::uint32_t n){for(unsigned i=0;i<4;++i)bytes[at+i]=std::uint8_t(n>>(8*i));}
int main()try{
 assets::Animation animation;animation.starts={0,3,5,6};animation.records={{4,-100,{}},{0,0,{}},{6,0,{}},{0,1,{}},{6,0,{}},{6,0,{}}};assets::TerrainCatalog catalog;catalog.records.resize(2);word(catalog.records[0],0x90,1);
 assets::MapCell cell;cell.definition=0;cell.flags10=8;auto result=reconstruction::rotateTerrainLinkedDefinition(cell,animation,catalog,1);check(result.action==reconstruction::TerrainLinkedRotationAction::replace&&result.definition==1&&cell.definition==0&&cell.flags10==8);check(reconstruction::terrainEntityFirstFrame(animation,0)==0&&reconstruction::terrainEntityFirstFrame(animation,2)==-1);
 word(catalog.records[0],0x90,0xffffffffu);result=reconstruction::rotateTerrainLinkedDefinition(cell,animation,catalog,1);check(result.action==reconstruction::TerrainLinkedRotationAction::replace&&result.definition==2);
 cell.definition=2;result=reconstruction::rotateTerrainLinkedDefinition(cell,animation,catalog,1);check(result.action==reconstruction::TerrainLinkedRotationAction::reset&&cell.flags10==8);
 cell.definition=99;check(reconstruction::rotateTerrainLinkedDefinition(cell,animation,catalog,1).action==reconstruction::TerrainLinkedRotationAction::reset);cell.flags10=0;check(reconstruction::rotateTerrainLinkedDefinition(cell,{}, {},99).action==reconstruction::TerrainLinkedRotationAction::unchanged);
 cell.flags10=8;cell.definition=0;refused([&]{reconstruction::rotateTerrainLinkedDefinition(cell,animation,catalog,0);});refused([&]{reconstruction::rotateTerrainLinkedDefinition(cell,{},catalog,1);});
 auto shortAnimation=animation;shortAnimation.starts={0,3,5};word(catalog.records[0],0x90,99);refused([&]{reconstruction::rotateTerrainLinkedDefinition(cell,shortAnimation,catalog,1);});word(catalog.records[0],0x90,1);check(reconstruction::rotateTerrainLinkedDefinition(cell,shortAnimation,catalog,1).definition==1);
 auto malformed=animation;malformed.starts[1]=99;refused([&]{reconstruction::terrainEntityFirstFrame(malformed,0);});
 std::cout<<"Linked rotation selection, stop/control scan and safe bound policies passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
