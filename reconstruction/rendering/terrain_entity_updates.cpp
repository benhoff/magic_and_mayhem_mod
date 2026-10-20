#include "terrain_entity_updates.hpp"
#include <optional>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void dimensions(const assets::MapAsset& map,std::size_t cell,const std::vector<std::uint8_t>& occupied){
 if(!map.width||map.width>128||!map.height||map.height>128||!map.layers||map.layers>32||map.cells.size()!=std::size_t(map.width)*map.height*map.layers||cell>=map.cells.size()||(!occupied.empty()&&occupied.size()!=map.cells.size()))throw std::invalid_argument("Linked terrain neighborhood bounds");
}
std::uint32_t word(const assets::TerrainCatalog& catalog,std::uint32_t definition,unsigned offset){
 const auto& r=catalog.records.at(definition);return std::uint32_t(r[offset])|(std::uint32_t(r[offset+1])<<8)|(std::uint32_t(r[offset+2])<<16)|(std::uint32_t(r[offset+3])<<24);
}
struct Patch {std::size_t index;std::uint16_t flags;};
struct Neighborhood {std::vector<Patch> patches;bool dirty=false;};
Neighborhood plan(const assets::MapAsset& map,std::size_t changed,const assets::TerrainCatalog& catalog,
 const std::vector<std::uint8_t>& occupied,std::optional<assets::MapCell> replacement={}){
 dimensions(map,changed,occupied);const auto area=map.width*map.height;const int z=int(changed/area),y=int((changed%area)/map.width),x=int(changed%map.width);
 const auto cell=[&](std::size_t index)->const assets::MapCell& {return replacement&&index==changed?*replacement:map.cells[index];};
 const auto wrap=[](int n,unsigned size){auto v=n%int(size);if(v<0)v+=int(size);return unsigned(v);};
 const auto index=[&](int cx,int cy,unsigned cz){return (std::size_t(cz)*map.height+wrap(cy,map.height))*map.width+wrap(cx,map.width);};
 const auto flags=[&](int cx,int cy,unsigned cz){return catalog.records.at(cell(index(cx,cy,cz)).definition)[0xa8];};
 Neighborhood result;result.patches.reserve(18);auto last=changed;
 for(int layer=z-1;layer<=z;++layer){if(layer<0||unsigned(layer)+1>=map.layers)continue;
  for(int row=y-1;row<=y+1;++row)for(int column=x-1;column<=x+1;++column){
   last=index(column,row,unsigned(layer));const auto& value=cell(last);auto output=std::uint16_t(value.flags8&0xff87);const auto above=flags(column,row,unsigned(layer)+1);
   const auto north=flags(column,row-1,unsigned(layer)),east=flags(column+1,row,unsigned(layer)),south=flags(column,row+1,unsigned(layer)),west=flags(column-1,row,unsigned(layer));
   if((east&8)&&(south&1)&&(above&0x20))output|=8;
   if((south&1)&&(west&2)&&(above&0x20))output|=0x10;
   if((north&4)&&(west&2)&&(above&0x20))output|=0x20;
   if((north&4)&&(east&8)&&(above&0x20))output|=0x40;
   if(!value.definition&&value.references[0]==0xffff&&value.references[1]==0xffff&&value.references[2]==0xffff&&!(value.flags10&0x2000)&&(occupied.empty()||!occupied[last]))output|=0x80;
   else output&=0xff7f;
   result.patches.push_back({last,output});
  }
 }
 result.dirty=!(cell(last).flags10&8);return result;
}
void publish(assets::MapAsset& map,const Neighborhood& result){for(const auto& patch:result.patches)map.cells[patch.index].flags8=patch.flags;}
}
bool refreshTerrainCellNeighborhood(assets::MapAsset& map,std::size_t cell,const assets::TerrainCatalog& catalog,const std::vector<std::uint8_t>& occupied){
 const auto result=plan(map,cell,catalog,occupied);publish(map,result);return result.dirty;
}
TerrainFrameWrite setTerrainLinkedFrame(assets::MapAsset& map,std::size_t index,const assets::TerrainCatalog& catalog,std::uint32_t frame,const std::vector<std::uint8_t>& occupied){
 dimensions(map,index,occupied);if(frame>catalog.records.size())return {};
 if(frame>=catalog.records.size())throw std::out_of_range("Original terrain frame equals one-past TTD count");
 auto candidate=map.cells[index];const auto oldClass=word(catalog,candidate.definition,0x94),newClass=word(catalog,frame,0x94);
 if(candidate.references[1]!=0xffff&&!(candidate.flags10&0x80)&&frame){
  if(oldClass!=newClass||!(catalog.records.at(candidate.definition)[0xb0]&8)||!(catalog.records.at(frame)[0xb0]&8))return {};
 }
 if(oldClass==0x10)candidate.flags10&=0xfffe;
 candidate.definition=std::uint16_t(frame);
 if(newClass==0x10)candidate.flags10|=1;
 else if(!candidate.definition)candidate.flags10&=0xfffc;
 const auto result=plan(map,index,catalog,occupied,candidate);
 map.cells[index]=candidate;publish(map,result);return {true,result.dirty};
}
}
