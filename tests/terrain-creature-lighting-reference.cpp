// Same owned fixture runs in 32-bit original/native and 64-bit native builds.
#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "terrain_creature_lighting.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace mnm::reconstruction;
static std::uint32_t rng=0x4f27d0;
static unsigned next(unsigned n){rng=rng*1664525u+1013904223u;return (rng>>8)%n;}
#ifdef ORIGINAL_REFERENCE
static void put(Bytes& b,unsigned offset,unsigned value){std::memcpy(b.data()+offset,&value,4);}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE output");map_image(read(argv[1]));
 const unsigned char expected[]={0x51,0x53,0x8b,0xd9,0x56,0x57};
 if(std::memcmp(reinterpret_cast<void*>(0x4f27d0),expected,sizeof(expected)))throw std::runtime_error("Updater entry mismatch");
 const char* output=argv[2];
#else
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 std::ofstream file(output,std::ios::binary);std::uint64_t compared=0;unsigned ticks=0;
 for(unsigned trial=0;trial<96;++trial){
  const unsigned w=17+next(16),h=17+next(16),layers=1+next(9),capacity=next(65),scanLimit=next(capacity+5);
  TerrainLightingConfig config{-int(next(128)),int(next(255))-127};
  TerrainLightField field(w,h,layers,config);TerrainCreatureLightCycle cycle;
  std::vector<TerrainLightCreature> creatures(capacity);TerrainLightRelations relations;
  for(auto& row:relations.first)for(auto& v:row)v=next(3)?std::uint8_t(1+next(255)):0;
  for(auto& row:relations.second)for(auto& v:row)v=next(3)?std::uint8_t(1+next(255)):0;
  for(auto& c:creatures)c={next(4)!=0,next(4)!=0,int(next(8)),next(w),next(h),next(layers)};
#ifdef ORIGINAL_REFERENCE
  const unsigned objectAddress=0x6c5490;Bytes object(0x66c0);std::array<Bytes,5> buffers;
  const auto count=field.buffers()[0].size();put(object,4,w);put(object,8,h);put(object,12,layers);put(object,16,w*h);put(object,0x7e0,count);
  for(unsigned i=0;i<5;++i){buffers[i].assign(count+32,0xa5);put(object,0x7cc+4*i,reinterpret_cast<std::uintptr_t>(buffers[i].data()+16));}
  for(unsigned y=0;y<h;++y)put(object,0x64b2+4*y,y*w);
  for(unsigned z=0;z<(layers+1)/2;++z)put(object,0x6432+4*z,z*w*h);
  std::memcpy(reinterpret_cast<void*>(objectAddress),object.data(),object.size());global(0x5e18e8,unsigned(config.ambient));global(0x5e18ec,unsigned(config.ramp));
  using Call=void(__attribute__((thiscall)) *)(void*);
  reinterpret_cast<Call>(0x4ecdd0)(reinterpret_cast<void*>(objectAddress));
  for(unsigned address:{0x6e8d90u,0x6e8d94u,0x6e8d98u,0x6e8d9cu})global(address,0);
  for(unsigned owner=0;owner<8;++owner)for(unsigned player=0;player<8;++player){
   *reinterpret_cast<unsigned char*>(0x643a80+owner*0x154+0x18+player)=relations.first[owner][player];
   *reinterpret_cast<unsigned char*>(0x643a80+owner*0x154+0x20+player)=relations.second[owner][player];
  }
  Bytes records(capacity*0xe4b+32,0xa5);global(0x6def58,reinterpret_cast<std::uintptr_t>(records.data()+16));global(0x6def5c,capacity);global(0x6df180,scanLimit);
#endif
  int player=int(next(8));
  for(unsigned tick=0;tick<24;++tick){
   // Exercise movement and changing admission between enumeration and visitation.
   if(tick==11)player=int(next(8));
   for(auto& c:creatures)if(next(7)==0){c.column=next(w);c.row=next(h);c.layer=next(layers);c.active=next(2);c.lightEnabled=next(2);c.owner=int(next(8));}
   TerrainCreatureLightControls controls{tick!=3 && tick!=17,trial%3==1,trial%3==2,tick==5?1:tick==13?2:tick==20?-1:0};
#ifdef ORIGINAL_REFERENCE
   for(unsigned i=0;i<capacity;++i){const auto& c=creatures[i];const auto at=16+i*0xe4b;put(records,at+4,c.active);put(records,at+0xe4,c.lightEnabled);put(records,at+0x174,c.owner);put(records,at+8,c.column);put(records,at+12,c.row);put(records,at+16,c.layer);}
   global(0x644520,player);global(0x5e41a8,controls.changed);global(objectAddress+0x808,controls.enabled);global(objectAddress+0x642a,controls.transitionActive);global(objectAddress+0x6426,controls.transitionRequested);
   reinterpret_cast<Call>(0x4f27d0)(reinterpret_cast<void*>(objectAddress));
#endif
   cycle.step(field,creatures,scanLimit,player,relations,controls);const auto& state=cycle.state();
   const std::array<std::uint32_t,5> values{state.phase,state.remaining,state.quota,state.cursor,state.published};
#ifdef ORIGINAL_REFERENCE
   const std::array<unsigned,5> addresses{0x6e8d90,0x6e8d94,0x6e8d98,0x6e8d9c,objectAddress+0x642e};
   for(unsigned i=0;i<5;++i)if(*reinterpret_cast<unsigned*>(addresses[i])!=values[i])throw std::runtime_error("Cycle state mismatch trial/tick/field "+std::to_string(trial)+"/"+std::to_string(tick)+"/"+std::to_string(i));
#endif
   file.write(reinterpret_cast<const char*>(values.data()),sizeof(values));
   for(unsigned i=0;i<5;++i){const auto& bytes=field.buffers()[i];
#ifdef ORIGINAL_REFERENCE
    if(std::memcmp(buffers[i].data()+16,bytes.data(),bytes.size()))throw std::runtime_error("Buffer mismatch trial/tick/field "+std::to_string(trial)+"/"+std::to_string(tick)+"/"+std::to_string(i));
    for(unsigned j=0;j<16;++j)if(buffers[i][j]!=0xa5 || buffers[i][bytes.size()+16+j]!=0xa5)throw std::runtime_error("Light field guard changed");
#endif
    file.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());compared+=bytes.size();
   }
#ifdef ORIGINAL_REFERENCE
   for(unsigned j=0;j<16;++j)if(records[j]!=0xa5 || records[records.size()-16+j]!=0xa5)throw std::runtime_error("Creature record guard changed");
#endif
   ++ticks;
  }
 }
 if(!file)throw std::runtime_error("Output failed");
 std::cout<<"{\"trials\":96,\"ticks\":"<<ticks<<",\"buffer_bytes\":"<<compared<<",\"state_values\":"<<ticks*5<<"}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
