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
 // Every invalid definition, including the final top-layer cell, is refused
 // before any surface mutation. Projection likewise leaves its input intact.
 const auto same=[](const assets::MapAsset& a,const assets::MapAsset& b){
  if(a.width!=b.width || a.height!=b.height || a.layers!=b.layers || a.metadata!=b.metadata || a.cells.size()!=b.cells.size())return false;
  for(std::size_t i=0;i<a.cells.size();++i){const auto& x=a.cells[i];const auto& y=b.cells[i];
   if(x.definition!=y.definition || x.references!=y.references || x.flags8!=y.flags8 || x.flags10!=y.flags10)return false;}
  return true;
 };
 assets::MapAsset valid;valid.width=3;valid.height=2;valid.layers=3;valid.cells.resize(18);
 valid.metadata.fill(0xdeadbeef);for(auto& c:valid.cells){c.definition=1;c.references={1,0xffff,2};c.flags8=0x1235;c.flags10=0x600b;}
 for(std::size_t i=0;i<valid.cells.size();++i){auto bad=valid;bad.cells[i].definition=2;const auto before=bad;
  for(bool prepare:{false,true}){bool rejected=false;try{if(prepare)reconstruction::prepareTerrainGeometry(bad,t);else reconstruction::deriveTerrainSurfaces(bad,t);}catch(const std::out_of_range&){rejected=true;}check(rejected && same(bad,before));}
 }
 for(unsigned field=0;field<8;++field){auto bad=valid;auto catalog=t;
  switch(field){case 0:bad.width=0;break;case 1:bad.height=0;break;case 2:bad.layers=0;break;case 3:bad.width=129;break;case 4:bad.height=129;break;case 5:bad.layers=33;break;case 6:bad.cells.pop_back();break;default:catalog.records.clear();}
  const auto before=bad;
  for(bool prepare:{false,true}){bool rejected=false;try{if(prepare)reconstruction::prepareTerrainGeometry(bad,catalog);else reconstruction::deriveTerrainSurfaces(bad,catalog);}catch(const std::invalid_argument&){rejected=true;}check(rejected && same(bad,before));}
 }
 const auto before=valid;const auto projected=reconstruction::prepareTerrainGeometry(valid,t);
 check(same(valid,before) && projected.map.metadata==valid.metadata);
 check(projected.projectedObjects==18 && projected.projectedReferences==36 && projected.changedCells==18);
 for(const auto& c:projected.map.cells)check(c.references==std::array<std::uint16_t,3>{0xffff,0xffff,0xffff} && !(c.flags10&0x2008));
 // Isolated edge masks, all four owners, and the toroidal one-cell domain.
 for(unsigned owner=0;owner<4;++owner)for(bool disagreement:{false,true})for(unsigned mask=0;mask<16;++mask){
  assets::MapAsset map;map.width=map.height=1;map.layers=2;map.cells.resize(2);
  for(auto& c:map.cells){c.definition=1;c.references.fill(0xffff);}
  map.cells[0].flags8=std::uint16_t(owner);map.cells[1].flags8=std::uint16_t(owner^unsigned(disagreement));
  auto catalog=t;catalog.records[1][0xa8]=std::uint8_t(0x20|mask);
  const auto top=map.cells[1];reconstruction::deriveTerrainSurfaces(map,catalog);
  const unsigned edges=((mask&8)&&(mask&1)?8:0)|((mask&1)&&(mask&2)?16:0)|((mask&4)&&(mask&2)?32:0)|((mask&4)&&(mask&8)?64:0);
  check((map.cells[0].flags8&0x78)==edges);
  check(bool(map.cells[0].flags10&(0x200u<<owner))==disagreement);
  check(map.cells[0].definition==unsigned(disagreement || edges!=0x78));
  check(map.cells[1].definition==top.definition && map.cells[1].flags8==top.flags8 && map.cells[1].flags10==top.flags10 && map.cells[1].references==top.references);
 }
 std::cout<<"Terrain geometry boundaries passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
