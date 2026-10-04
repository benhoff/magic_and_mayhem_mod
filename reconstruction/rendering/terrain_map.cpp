#include "terrain_map.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::uint32_t word(const std::array<std::uint8_t,356>& r,unsigned at){
 return std::uint32_t(r[at])|(std::uint32_t(r[at+1])<<8)|(std::uint32_t(r[at+2])<<16)|(std::uint32_t(r[at+3])<<24);
}
void validate(const assets::MapAsset& m,const assets::TerrainCatalog& t){
 if(!m.width || !m.height || !m.layers || m.width>128 || m.height>128 || m.layers>32 ||
    m.cells.size()!=std::size_t(m.width)*m.height*m.layers || t.records.empty())
  throw std::invalid_argument("Terrain geometry dimensions/catalog");
 for(const auto& c:m.cells)if(c.definition>=t.records.size())throw std::out_of_range("Terrain geometry definition");
}
}
void deriveTerrainSurfaces(assets::MapAsset& m,const assets::TerrainCatalog& t){
 validate(m,t);
 const auto index=[&](unsigned x,unsigned y,unsigned z){return (std::size_t(z)*m.height+y)*m.width+x;};
 const auto flags=[&](unsigned x,unsigned y,unsigned z){return t.records[m.cells[index(x,y,z)].definition][0xa8];};
 // First complete pass precedes culling: all TTD lookups see original definitions.
 for(unsigned z=0;z+1<m.layers;++z)for(unsigned y=0;y<m.height;++y)for(unsigned x=0;x<m.width;++x){
  auto& c=m.cells[index(x,y,z)];const auto above=flags(x,y,z+1);
  const auto north=flags(x,(y+m.height-1)%m.height,z),east=flags((x+1)%m.width,y,z);
  const auto south=flags(x,(y+1)%m.height,z),west=flags((x+m.width-1)%m.width,y,z);
  c.flags8&=0xff87;
  if((east&8) && (south&1) && (above&0x20))c.flags8|=8;
  if((south&1) && (west&2) && (above&0x20))c.flags8|=0x10;
  if((north&4) && (east&8) && (above&0x20))c.flags8|=0x40;
  if((north&4) && (west&2) && (above&0x20))c.flags8|=0x20;
  const auto boundary=std::uint16_t(0x200u<<(c.flags8&3));
  if((c.flags8^m.cells[index(x,y,z+1)].flags8)&3)c.flags10|=boundary;
  else c.flags10&=std::uint16_t(~boundary & 0xbfff);
  if(!c.definition && c.references[0]==0xffff && c.references[1]==0xffff && c.references[2]==0xffff && !(c.flags10&0x2000))c.flags8|=0x80;
  else c.flags8&=0xff7f;
 }
 for(unsigned z=0;z+1<m.layers;++z)for(unsigned y=0;y<m.height;++y)for(unsigned x=0;x<m.width;++x){
  auto& c=m.cells[index(x,y,z)];
  if(!(c.flags10&(0x200u<<(c.flags8&3))) && (c.flags8&0x78)==0x78){
   c.definition=0;c.flags10=std::uint16_t((c.flags10&0xfffc)|0x4000);
  }
 }
}
TerrainGeometry prepareTerrainGeometry(const assets::MapAsset& source,const assets::TerrainCatalog& t){
 validate(source,t);TerrainGeometry result;result.map=source;
 for(auto& c:result.map.cells){
  result.projectedObjects+=bool(c.flags10&8);
  for(auto& ref:c.references){result.projectedReferences+=ref!=0xffff;ref=0xffff;}
  // Explicit ordinary-terrain projection precedes the selected initializer fields.
  c.flags10&=0xdff7;
  if(c.definition && word(t.records[c.definition],0x94)==0x10)c.flags10|=1;
  else c.flags10&=0xfffe;
 }
 deriveTerrainSurfaces(result.map,t);
 for(std::size_t i=0;i<source.cells.size();++i){const auto& a=source.cells[i];const auto& b=result.map.cells[i];
  result.changedCells+=a.definition!=b.definition || a.references!=b.references || a.flags8!=b.flags8 || a.flags10!=b.flags10;
  result.removedDefinitions+=a.definition!=0 && b.definition==0;
 }
 return result;
}
}
