#include "terrain_map.hpp"
#include <stdexcept>
#include <iostream>
using namespace mnm;
void check(bool b){if(!b)throw std::runtime_error("Terrain geometry boundary mismatch");}
int main()try{
 assets::TerrainCatalog t;t.records.resize(2);t.records[1][0xa8]=0x2f;t.records[1][0x94]=0x10;
 assets::MapAsset m;m.width=m.height=1;m.layers=2;m.cells.resize(2);
 for(auto& c:m.cells){c.definition=1;c.references.fill(0xffff);}
 m.cells[0].flags10=0x4003;m.cells[1].flags8=0x1235;
 auto g=reconstruction::prepareTerrainGeometry(m,t);check(m.cells[0].definition==1 && m.cells[0].flags10==0x4003);
 // Owner low-bit disagreement keeps the lower cell admitted despite four edges.
 check(g.map.cells[0].definition==1 && (g.map.cells[0].flags8&0x78)==0x78 && (g.map.cells[0].flags10&0x200));
 m.cells[1].flags8=0;g=reconstruction::prepareTerrainGeometry(m,t);check(g.map.cells[0].definition==0 && (g.map.cells[0].flags10&0x4000) && !(g.map.cells[0].flags10&3));check(g.removedDefinitions==1);
 check(g.map.cells[1].definition==1 && g.map.cells[1].flags8==0 && g.map.cells[1].flags10==1);
 m.cells[0].definition=0;m.cells[0].flags8=0x78;m.cells[0].flags10=0;m.cells[1].definition=0;
 reconstruction::deriveTerrainSurfaces(m,t);check((m.cells[0].flags8&0xf8)==0x80);
 m.cells[0].references[2]=7;reconstruction::deriveTerrainSurfaces(m,t);check(!(m.cells[0].flags8&0x80));
 m.layers=1;m.cells.resize(1);auto copy=m.cells;reconstruction::deriveTerrainSurfaces(m,t);check(m.cells[0].flags8==copy[0].flags8);
 m.cells[0].definition=2;bool rejected=false;try{reconstruction::deriveTerrainSurfaces(m,t);}catch(const std::out_of_range&){rejected=true;}check(rejected && m.cells[0].definition==2);
 std::cout<<"Terrain geometry boundaries passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
