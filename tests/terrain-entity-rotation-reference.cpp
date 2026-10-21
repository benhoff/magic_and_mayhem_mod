// Whole unchanged004efb90 and464bd0; checked private reset request endpoint.
#define main previous_camera_reference_unused_main
#include "terrain-camera-reference.cpp"
#undef main
#include "terrain_entity_rotation.hpp"
#include <cstdlib>
static_assert(sizeof(mnm::assets::MapCell)==12 && sizeof(mnm::assets::AnimationRecord)==44,"Private original fixture layouts");
using namespace mnm::reconstruction;
static unsigned resets=0;
static void __attribute__((fastcall)) resetRequested(void* p){if(get(p,8)&0x00080000u)std::abort();++resets;}
int main(int argc,char** argv)try{
 require(argc==2);mapImage(read(argv[1]));const unsigned char original[]={0x83,0xec,0x30,0x8d,0x4c,0x24,0};require(!std::memcmp(reinterpret_cast<void*>(0x4efb90),original,sizeof(original)));
 const auto* reset=reinterpret_cast<const unsigned char*>(0x4eb8a0);const unsigned char resetEntry[]={0x83,0xec,0x2c};require(!std::memcmp(reset,resetEntry,sizeof(resetEntry)));
 unsigned char adapter[]={0x68,0,0,0,0,0xc3};const auto callback=reinterpret_cast<std::uintptr_t>(&resetRequested);std::memcpy(adapter+1,&callback,4);std::memcpy(reinterpret_cast<void*>(0x4eb8a0),adapter,6);
 using Rotate=void(__attribute__((stdcall)) *)(void*,unsigned);std::mt19937 rng(0x4efb90);unsigned calls=0,replacements=0,resetCases=0,unchanged=0;
 for(unsigned trial=0;trial<4096;++trial){
  mnm::assets::Animation animation;for(unsigned i=0;i<64;++i){animation.starts.push_back(animation.records.size());animation.records.push_back({1,int(i),{}});if((i+trial)%11)animation.records.push_back({0,int(rng()%32),{}});animation.records.push_back({6,0,{}});}animation.starts.push_back(animation.records.size());
  mnm::assets::TerrainCatalog catalog;catalog.records.resize(32);for(auto& record:catalog.records)for(unsigned r=1;r<=3;++r){const auto target=trial%7?std::uint32_t(rng()%35):0xffffffffu;put(record.data(),0x94-4*r,target);}
  std::array<unsigned,6> aniHeader{{0,0,0,0,0,unsigned(animation.starts.size())}},ttdHeader{{0,0,0,32,0,0}};put(reinterpret_cast<void*>(0x6c4518),0,reinterpret_cast<std::uintptr_t>(aniHeader.data()));put(reinterpret_cast<void*>(0x6c4518),4,reinterpret_cast<std::uintptr_t>(animation.starts.data()));put(reinterpret_cast<void*>(0x6c4518),8,reinterpret_cast<std::uintptr_t>(animation.records.data()));put(reinterpret_cast<void*>(0x656608),0,reinterpret_cast<std::uintptr_t>(ttdHeader.data()));put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(catalog.records.data()));
  for(unsigned rotation=1;rotation<=3;++rotation){
   mnm::assets::MapCell cell;cell.definition=(trial+rotation)%68;cell.references={std::uint16_t(rng()),std::uint16_t(rng()),std::uint16_t(rng())};cell.flags8=std::uint16_t(rng());cell.flags10=std::uint16_t(rng()|8);
   std::array<unsigned char,44> raw;raw.fill(0xa5);auto* pointer=raw.data()+16;std::memcpy(pointer,&cell,12);auto expected=raw;const auto decision=rotateTerrainLinkedDefinition(cell,animation,catalog,rotation);auto result=cell;
   if(decision.action==TerrainLinkedRotationAction::replace){result.definition=decision.definition;++replacements;}
   else if(decision.action==TerrainLinkedRotationAction::reset){result.flags10&=0xfff7;++resetCases;}else ++unchanged;
   std::memcpy(expected.data()+16,&result,12);resets=0;reinterpret_cast<Rotate>(0x4efb90)(pointer,rotation);require(resets==unsigned(decision.action==TerrainLinkedRotationAction::reset)&&raw==expected);++calls;
  }
 }
 std::cout<<"{\"success\":true,\"calls\":"<<calls<<",\"replacements\":"<<replacements<<",\"reset_requests\":"<<resetCases<<",\"unchanged\":"<<unchanged<<",\"original_reset_body_executed\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
