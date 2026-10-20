#include "terrain_entity_rotation.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::size_t count(const assets::Animation& animation){
 if(animation.starts.size()<2||animation.starts.size()>65537||animation.records.size()>65536)throw std::invalid_argument("Linked terrain ANI bounds");
 return animation.starts.size()-1;
}
std::uint32_t word(const std::array<std::uint8_t,356>& record,unsigned at){return std::uint32_t(record[at])|(std::uint32_t(record[at+1])<<8)|(std::uint32_t(record[at+2])<<16)|(std::uint32_t(record[at+3])<<24);}
}
std::int32_t terrainEntityFirstFrame(const assets::Animation& animation,std::uint32_t sequence){
 if(sequence>=count(animation))throw std::out_of_range("Linked terrain sequence ordinal");
 const auto first=animation.starts[sequence],end=animation.starts[sequence+1];
 if(first>=end||end>animation.records.size())throw std::out_of_range("Linked terrain sequence extent");
 for(auto at=first;at<end;++at){const auto& record=animation.records[at];if(record.opcode==0)return record.argument;if(record.opcode==6)return -1;}
 throw std::invalid_argument("Linked terrain sequence has no display or stop");
}
TerrainLinkedRotation rotateTerrainLinkedDefinition(const assets::MapCell& cell,
 const assets::Animation& animation,const assets::TerrainCatalog& catalog,unsigned rotation){
 TerrainLinkedRotation result;result.definition=cell.definition;
 if(!(cell.flags10&8))return result;
 if(rotation<1||rotation>3)throw std::invalid_argument("Linked terrain rotation");
 const auto sequences=count(animation);
 if(cell.definition>=sequences){result.action=TerrainLinkedRotationAction::reset;return result;}
 result.sourceFrame=terrainEntityFirstFrame(animation,cell.definition);
 if(result.sourceFrame==-1){result.action=TerrainLinkedRotationAction::reset;return result;}
 if(std::uint32_t(result.sourceFrame)>=catalog.records.size())throw std::out_of_range("Linked terrain first frame outside TTD");
 result.targetFrame=word(catalog.records[std::uint32_t(result.sourceFrame)],0x94-4*rotation);
 const unsigned first=cell.definition<3?0:cell.definition-3;
 const auto last=std::min<std::size_t>(unsigned(cell.definition)+3,catalog.records.size());
 for(std::size_t candidate=first;candidate<=last;++candidate){
  // Original upper bound is TTD count, inclusive, rather than ANI count. A
  // match before that invalid read remains admitted; an actual overrun refuses.
  if(candidate>=sequences)throw std::out_of_range("Original linked rotation search exceeds owned ANI");
  if(std::uint32_t(terrainEntityFirstFrame(animation,unsigned(candidate)))==result.targetFrame){
   if(candidate!=cell.definition){result.action=TerrainLinkedRotationAction::replace;result.definition=std::uint16_t(candidate);}return result;
  }
 }
 return result;
}
}
