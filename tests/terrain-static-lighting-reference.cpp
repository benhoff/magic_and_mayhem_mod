#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "terrain_static_lighting.hpp"
#include "terrain_creature_lighting.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static std::uint32_t rng=0x4f2ad0;
static unsigned next(unsigned n){rng=rng*1664525u+1013904223u;return (rng>>8)%n;}
#ifdef ORIGINAL_REFERENCE
static void put(Bytes& b,unsigned offset,unsigned value){std::memcpy(b.data()+offset,&value,4);}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE output");map_image(read(argv[1]));const char* output=argv[2];
 const unsigned char entry[]={0x83,0xec,0x10,0x53,0x55,0x8b,0xe9};
 if(std::memcmp(reinterpret_cast<void*>(0x4f2ad0),entry,sizeof(entry)))throw std::runtime_error("Static updater entry mismatch");
 using Call=void(__attribute__((thiscall)) *)(void*);
 using Gate=int(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned);
 using Stamp=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);
#endif
#ifndef ORIGINAL_REFERENCE
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 std::ofstream file(output,std::ios::binary);std::uint64_t compared=0;unsigned ticks=0,gates=0,stamps=0;
 // Admission checks are independent of updater enumeration and stamp values.
 for(unsigned i=0;i<32768;++i){
  const unsigned w=17+next(112),h=17+next(112);TerrainLightView view{next(w),next(h),next(std::min(w,h)+1)};
  const unsigned x=next(w),y=next(h),index=next(34);
  const std::uint32_t admitted=admitsTerrainStaticLight(w,h,view,x,y,index);
#ifdef ORIGINAL_REFERENCE
  global(0x6c5494,w);global(0x6c5498,h);global(0x689930+0x39,view.column);global(0x689930+0x3d,view.row);global(0x689930+0x59,view.extent);
  if(unsigned(reinterpret_cast<Gate>(0x4ff700)(reinterpret_cast<void*>(0x689930),x,y,index))!=admitted){std::cerr<<"gate "<<w<<","<<h<<" view "<<view.column<<","<<view.row<<","<<view.extent<<" source "<<x<<","<<y<<","<<index<<" native "<<admitted<<"\n";throw std::runtime_error("Gate mismatch fixture "+std::to_string(i));}
#endif
  file.write(reinterpret_cast<const char*>(&admitted),4);++gates;
 }
 for(unsigned trial=0;trial<128;++trial){
  const unsigned w=17+next(32),h=17+next(32),layers=1+next(9),capacity=next(33),scanCount=next(capacity+1);
  const TerrainLightingConfig config{-int(next(128)),int(next(255))-127};
  TerrainLightField field(w,h,layers,config);TerrainStaticLightCycle cycle;TerrainCreatureLightCycle creaturesCycle;
  std::vector<TerrainLightObject> objects(capacity);std::vector<TerrainLightCreature> creatures(8);
  TerrainLightRelations relations;
  for(auto& c:creatures)c={true,true,0,next(w),next(h),next(layers)};
  for(auto& o:objects)o={next(4)!=0,next(5)?2+next(32):0,next(w),next(h),next(layers),next(w),next(h)};
#ifdef ORIGINAL_REFERENCE
  const unsigned objectAddress=0x6c5490;Bytes object(0x66c0);std::array<Bytes,5> buffers;
  const auto count=field.buffers()[0].size();put(object,4,w);put(object,8,h);put(object,12,layers);put(object,16,w*h);put(object,0x7e0,count);
  for(unsigned i=0;i<5;++i){buffers[i].assign(count+32,0xa5);put(object,0x7cc+4*i,reinterpret_cast<std::uintptr_t>(buffers[i].data()+16));}
  for(unsigned y=0;y<h;++y)put(object,0x64b2+4*y,y*w);
  for(unsigned z=0;z<(layers+1)/2;++z)put(object,0x6432+4*z,z*w*h);
  std::memcpy(reinterpret_cast<void*>(objectAddress),object.data(),object.size());
#endif
#ifdef ORIGINAL_REFERENCE
  global(0x5e18e8,unsigned(config.ambient));global(0x5e18ec,unsigned(config.ramp));reinterpret_cast<Call>(0x4ecdd0)(reinterpret_cast<void*>(objectAddress));
  for(unsigned a:{0x6e8d90u,0x6e8d94u,0x6e8d98u,0x6e8d9cu,0x6e8da0u,0x6e8da4u,0x6e8da8u,0x6e8dacu})global(a,0);
  Bytes records(capacity*0x22e + 32,0xa5),creatureRecords(8*0xe4b+32,0xa5);
  global(0x689498,reinterpret_cast<std::uintptr_t>(records.data()+16));global(0x68949c,capacity);global(0x6898dc,scanCount);
  for(unsigned kind=0;kind<33;++kind)global(0x689498+12+kind*12,kind==32?0:kind+2);
  global(0x6def58,reinterpret_cast<std::uintptr_t>(creatureRecords.data()+16));global(0x6def5c,8);global(0x6df180,8);global(0x644520,0);
  for(unsigned i=0;i<8;++i){const auto& c=creatures[i];const unsigned at=16+i*0xe4b;put(creatureRecords,at+4,1);put(creatureRecords,at+0xe4,1);put(creatureRecords,at+0x174,0);put(creatureRecords,at+8,c.column);put(creatureRecords,at+12,c.row);put(creatureRecords,at+16,c.layer);}
#endif
  const auto compare=[&](unsigned tick){
   for(unsigned i=0;i<5;++i){const auto& bytes=field.buffers()[i];
#ifdef ORIGINAL_REFERENCE
    if(std::memcmp(buffers[i].data()+16,bytes.data(),bytes.size())){
     for(unsigned j=0;j<bytes.size();++j)if(std::int8_t(buffers[i][j+16])!=bytes[j]){std::cerr<<"first cell "<<j<<" original "<<int(std::int8_t(buffers[i][j+16]))<<" native "<<int(bytes[j])<<'\n';break;}
     throw std::runtime_error("Buffer mismatch trial/tick/buffer "+std::to_string(trial)+"/"+std::to_string(tick)+"/"+std::to_string(i));}
    for(unsigned j=0;j<16;++j)if(buffers[i][j]!=0xa5 || buffers[i][bytes.size()+16+j]!=0xa5)throw std::runtime_error("Buffer guard changed");
#endif
    file.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());compared+=bytes.size();
   }
  };
  // Compare additive stamping separately: all extents and aliases, unaffected by gate.
  for(unsigned index=2;index<=33;++index){const unsigned x=next(w),y=next(h),z=next(layers);
   field.stampStatic({x,y,z,index/2+1});
#ifdef ORIGINAL_REFERENCE
   reinterpret_cast<Stamp>(0x4f1c60)(reinterpret_cast<void*>(objectAddress),x,y,z,index);
#endif
   compare(100+index);++stamps;
  }
  for(unsigned tick=0;tick<24;++tick){
   TerrainLightView view{next(w),next(h),next(std::min(w,h)+1)};
   for(auto& o:objects)if(next(5)==0){o.column=next(w);o.row=next(h);o.layer=next(layers);o.admissionColumn=next(w);o.admissionRow=next(h);o.active=next(2);o.lightIndex=next(4)?2+next(32):0;}
   const int changed=tick==4?1:tick==9?2:tick==19?-1:0;const bool enabled=tick!=6 && tick!=15;
   bool published=tick%3!=0;
#ifdef ORIGINAL_REFERENCE
   global(0x689930+0x39,view.column);global(0x689930+0x3d,view.row);global(0x689930+0x59,view.extent);
   for(unsigned i=0;i<capacity;++i){const auto& o=objects[i];const unsigned at=16+i*0x22e;put(records,at+4,o.active);put(records,at+0x28,o.lightIndex?o.lightIndex-2:32);put(records,at+8,o.column);put(records,at+12,o.row);put(records,at+16,o.layer);put(records,at+0x1c2,o.admissionColumn);put(records,at+0x1c6,o.admissionRow);}
   global(0x5e41a8,changed);global(objectAddress+0x80c,enabled);
#endif
   if(trial%2){const auto& s=cycle.state();creaturesCycle.step(field,creatures,8,0,relations,{true,s.active,s.requested,changed});published=creaturesCycle.state().published;
#ifdef ORIGINAL_REFERENCE
    reinterpret_cast<Call>(0x4f27a0)(reinterpret_cast<void*>(objectAddress));if(*reinterpret_cast<unsigned*>(0x5e41a8))throw std::runtime_error("Outer updater failed to clear changed");
#endif
   }else{
#ifdef ORIGINAL_REFERENCE
    global(objectAddress+0x642e,published);reinterpret_cast<Call>(0x4f2ad0)(reinterpret_cast<void*>(objectAddress));
#endif
   }
   cycle.step(field,objects,scanCount,view,published,changed,enabled);const auto& s=cycle.state();
   const std::array<std::uint32_t,6> values{s.phase,s.remaining,s.quota,s.cursor,s.active,s.requested};
#ifdef ORIGINAL_REFERENCE
   const std::array<unsigned,6> addresses{0x6e8da0,0x6e8da4,0x6e8da8,0x6e8dac,objectAddress+0x642a,objectAddress+0x6426};
   for(unsigned i=0;i<6;++i)if(*reinterpret_cast<unsigned*>(addresses[i])!=values[i])throw std::runtime_error("Static state mismatch trial/tick/field "+std::to_string(trial)+"/"+std::to_string(tick)+"/"+std::to_string(i));
   for(unsigned j=0;j<16;++j)if(records[j]!=0xa5 || records[records.size()-16+j]!=0xa5)throw std::runtime_error("Object guard changed");
#endif
   file.write(reinterpret_cast<const char*>(values.data()),sizeof(values));compare(tick);++ticks;
  }
 }
 if(!file)throw std::runtime_error("Output failed");
 std::cout<<"{\"trials\":128,\"ticks\":"<<ticks<<",\"combined_ticks\":1536,\"gates\":"<<gates<<",\"stamps\":"<<stamps<<",\"buffer_bytes\":"<<compared<<",\"state_values\":"<<ticks*6<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
