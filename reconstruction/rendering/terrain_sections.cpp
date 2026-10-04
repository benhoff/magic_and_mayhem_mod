#include "terrain_sections.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void extent(const assets::MapAsset& m){
 if(!m.width || !m.height || !m.layers || m.width>128 || m.height>128 || m.layers>32 ||
 m.cells.size()!=std::size_t(m.width)*m.height*m.layers)throw std::invalid_argument("Section map dimensions");
}
std::size_t index(const assets::MapAsset& m,unsigned x,unsigned y,unsigned z){return (std::size_t(z)*m.height+y)*m.width+x;}
}
void copyTerrainSection(assets::MapAsset& dst,const assets::MapAsset& src,const assets::TerrainCatalog& t,
 unsigned side,unsigned sx,unsigned sy,unsigned dx,unsigned dy,unsigned rotation){
 extent(dst);extent(src);
 if(!side || rotation>3 || src.layers>dst.layers || sx>src.width || sy>src.height ||
 side>src.width-sx || side>src.height-sy || dx>dst.width || dy>dst.height ||
 side>dst.width-dx || side>dst.height-dy)throw std::out_of_range("Section copy extent/rotation");
 // Stage all cells before writing: validation is atomic and source/destination
 // aliases cannot change later inputs. Original only mutates source definition.
 std::vector<assets::MapCell> scratch; scratch.reserve(std::size_t(side)*side*src.layers);
 for(unsigned z=0;z<src.layers;++z)for(unsigned y=0;y<side;++y)for(unsigned x=0;x<side;++x){
  auto c=src.cells[index(src,sx+x,sy+y,z)];
  if(rotation){
   if(c.flags10&8)throw std::invalid_argument("Object-linked section rotation requires entity state");
   if(c.definition>=t.records.size())throw std::out_of_range("Section definition");
   const auto& r=t.records[c.definition];const auto at=0x94-4*rotation;
   c.definition=std::uint16_t(r[at])|(std::uint16_t(r[at+1])<<8);
   for(unsigned i=0;i<rotation;++i)c.flags8=std::uint16_t((c.flags8&0xc3ff)|((c.flags8&0x1c00)<<1)|((c.flags8&0x2000)>>3));
  }
  scratch.push_back(c);
 }
 std::size_t i=0;
 for(unsigned z=0;z<src.layers;++z)for(unsigned y=0;y<side;++y)for(unsigned x=0;x<side;++x){
  unsigned tx=x,ty=y;
  if(rotation==1){tx=side-1-y;ty=x;}
  if(rotation==2){tx=side-1-x;ty=side-1-y;}
  if(rotation==3){tx=y;ty=side-1-x;}
  dst.cells[index(dst,dx+tx,dy+ty,z)]=scratch[i++];
 }
}
assets::MapAsset assembleTerrainRegion(unsigned columns,unsigned rows,unsigned side,unsigned layers,
 const std::vector<TerrainSection>& sections,const assets::TerrainCatalog& t){
 if(!columns || !rows || !side || side>128 || columns>128/side || rows>128/side ||
 !layers || layers>32 || sections.size()!=std::size_t(columns)*rows)throw std::invalid_argument("Complete section grid dimensions");
 assets::MapAsset result;result.width=columns*side;result.height=rows*side;result.layers=layers;
 result.cells.resize(std::size_t(result.width)*result.height*layers);
 std::vector<bool> used(columns*rows);
 for(const auto& s:sections){
  if(!s.source || s.source->layers!=layers || s.column>=columns || s.row>=rows || used[s.row*columns+s.column])throw std::invalid_argument("Section grid coverage/layers");
  used[s.row*columns+s.column]=true;
  copyTerrainSection(result,*s.source,t,side,s.sourceX,s.sourceY,s.column*side,s.row*side,s.rotation);
 }
 return result;
}
}
