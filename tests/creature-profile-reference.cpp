#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "creature_profile.hpp"
#include "creature_movement.hpp"
#include "route_scalar.hpp"
#include "creature_motion.hpp"
#include "no_cd.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>
using namespace mnm;
static const char* keys[]={"TileHeight","TileSizeXY","Acceleration","SwimmingAbility","CanFly","GroundSpeed","FlyingSpeed"};
static std::array<std::string,7> fields;
#ifdef ORIGINAL_REFERENCE
static unsigned __attribute__((stdcall)) cwd(unsigned capacity,char* output){if(capacity!=256)throw std::runtime_error("Directory capacity");std::strcpy(output,"C:\\game");return 7;}
static unsigned __attribute__((stdcall)) profile(const char* section,const char* key,const char* fallback,char* output,unsigned capacity,const char* path){
 if(!std::strcmp(path,"C:\\game\\cfg\\HTH.cfg") && !*fallback && capacity==256){*output=0;return 0;}
 if(std::string(section).rfind("CREATURE_",0)!=0 || *fallback || capacity!=256 || std::strcmp(path,"C:\\game\\cfg\\creature.cfg")){std::cerr<<"Request "<<section<<"/"<<key<<" fallback="<<fallback<<" capacity="<<capacity<<" path="<<path<<"\n";std::abort();}
 for(unsigned k=0;k<7;++k)if(!std::strcmp(key,keys[k])){std::strcpy(output,fields[k].c_str());return fields[k].size();}
 *output=0;return 0; // Other fields intentionally absent; no full type-load claim.
}
#endif
static std::vector<unsigned char> input(const char* p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("Input open");return {std::istreambuf_iterator<char>(f),{}};}
#ifdef ORIGINAL_REFERENCE
extern "C" void __attribute__((thiscall)) finishMotion(void* object){std::uint32_t zero=0;std::memcpy(static_cast<char*>(object)+4,&zero,4);}
static unsigned compareMotion(const assets::Animation& ani,const std::array<std::uint32_t,48>& samples){
 const unsigned char anchor[]={0x83,0xec,0x40,0x53,0x55,0x8b,0xe9};
 auto* code=reinterpret_cast<unsigned char*>(0x512460);if(std::memcmp(code,anchor,sizeof(anchor)))throw std::runtime_error("Completion anchor");
 const auto relative=std::uint32_t(reinterpret_cast<std::uintptr_t>(finishMotion))-0x512460-5;code[0]=0xe9;std::memcpy(code+1,&relative,4);
 unsigned transitions=0;
 for(unsigned direction=0;direction<8;++direction)for(int category:{0,4})for(bool vertical:{false,true})for(int height:{-32,0,32})for(int rate:{48,135,400}){
  const auto a=ani.starts[direction],z=ani.starts[direction+1];
  std::vector<assets::AnimationRecord> records(ani.records.begin()+a,ani.records.begin()+z);
  reconstruction::NoCdAnimationPlayer player(records);player.start();
  const unsigned bank=direction&1;
  reconstruction::MotionInputs p;p.separateCursor=true;p.rate=rate;p.duration=135;p.gridX=3;p.gridY=2;p.heightOrigin=48;p.heightDelta=height;p.vertical=vertical;p.direction=direction;
  for(unsigned i=0;i<48;++i)p.samples[i]=samples[i];
  reconstruction::MotionState state;state.frame=state.initialFrame=12*bank;state.fineX=96;state.fineY=64;state.fineZ=48;
  std::array<std::uint32_t,11> header{};header[5]=9;std::array<std::uint32_t,9> offsets{};offsets[8]=records.size();
  std::uint32_t asset[3]={reinterpret_cast<std::uintptr_t>(header.data()),reinterpret_cast<std::uintptr_t>(offsets.data()),reinterpret_cast<std::uintptr_t>(records.data())};
  std::vector<unsigned char> object(0xe4b,0);
  const auto put=[&](unsigned at,std::int32_t value){std::memcpy(object.data()+at,&value,4);};
  const auto get=[&](unsigned at){std::int32_t v;std::memcpy(&v,object.data()+at,4);return v;};
  put(0xb0+28,reinterpret_cast<std::uintptr_t>(asset));
  using Start=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned);reinterpret_cast<Start>(0x464cb0)(object.data()+0xb0,direction,0);
  put(0x14,96);put(0x18,64);put(0x1c,48);put(0xb8f,reinterpret_cast<std::uintptr_t>(p.samples.data()+bank*12));put(0xb93,reinterpret_cast<std::uintptr_t>(p.samples.data()+bank*12));
  put(4,1);put(8,3);put(12,2);put(0x608,direction);put(0x98f,vertical?1:0);put(0x10c,category);put(0xb5f,48);put(0xb57,height);put(0xb63,rate);put(0xb67,135);
  for(unsigned tick=0;tick<120;++tick){
   auto staged=player;reconstruction::MotionAnimation events{[&]{return staged.tick();},[&]{staged.start();}};
   const auto done=reconstruction::advance_creature_motion(state,p,&events);player=std::move(staged);
   using Tick=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Tick>(0x5104b0)(object.data());
   if((get(4)==0)!=done || state.accumulator!=get(0xb6b) || state.progress!=get(0xb3f) || state.travelX!=get(0xb47) || state.travelY!=get(0xb4b) || state.fineX!=get(0x14) || state.fineY!=get(0x18) || state.fineZ!=get(0x1c) || state.residualX!=get(0xb7b) || state.residualY!=get(0xb7f) || state.animationFrame!=unsigned(get(0xb77)) || state.frame!=(std::uint32_t(get(0xb8f))-reinterpret_cast<std::uintptr_t>(p.samples.data()))/4)
    throw std::runtime_error("Installed profile original motion differs");
   const auto& controller=player.state();const auto begin=reinterpret_cast<std::uintptr_t>(records.data());
   if(controller.pc!=(std::uint32_t(get(0xb0+24))-begin)/44 || controller.active!=bool(get(0xb0+8)) || controller.delay!=unsigned(get(0xbc)) || controller.elapsed!=unsigned(get(0xc0)) || controller.repeats!=unsigned(get(0xc4)) || !controller.displayedRecord || *controller.displayedRecord!=(std::uint32_t(get(0xb4))-begin)/44)
    throw std::runtime_error("Installed profile original ANI differs");
   ++transitions;if(done)break;if(tick==119)throw std::runtime_error("Installed profile segment did not finish");
  }
 }
 return transitions;
}
#endif
int main(int argc,char** argv)try{
 if(argc!=4)throw std::runtime_error("Expected PE selected-field fixtures and ANI");
#ifdef ORIGINAL_REFERENCE
 map_image(read(argv[1]));global(0x5c50bc,reinterpret_cast<std::uintptr_t>(cwd));global(0x5c507c,reinterpret_cast<std::uintptr_t>(profile));
 const unsigned char anchor[]={0xb8,0x24,0x10,0,0};if(std::memcmp(reinterpret_cast<void*>(0x502b20),anchor,5))throw std::runtime_error("Loader anchor");
 if(*reinterpret_cast<unsigned char*>(0x505220)!=0x83 || *reinterpret_cast<unsigned char*>(0x505160)!=0x83 || *reinterpret_cast<unsigned char*>(0x5057b0)!=0x85)throw std::runtime_error("Profile anchors");
 using Select=const char*(__attribute__((fastcall)) *)(unsigned,int);
 *reinterpret_cast<unsigned char*>(0x6dbe9d)=0;
 if(std::strcmp(reinterpret_cast<Select>(0x5057b0)(10,0),"creatures\\redcap.ani"))throw std::runtime_error("Redcap original asset binding");
#endif
 auto aniBytes=input(argv[3]);auto decoded=assets::decodeAnimation(aniBytes);if(auto* e=std::get_if<assets::AnimationError>(&decoded))throw std::runtime_error(e->detail);
 const auto& ani=std::get<assets::Animation>(decoded);const auto samples=reconstruction::groundMovementSamples(ani);const auto maximum=reconstruction::groundMovementMaximum(samples);
 std::ifstream fixtures(argv[2]);unsigned count=0;while(std::getline(fixtures,fields[0])){
  for(unsigned k=1;k<7;++k)if(!std::getline(fixtures,fields[k]))throw std::runtime_error("Fixture extent");
  assets::Config cfg;for(unsigned k=0;k<7;++k){auto name=std::string(keys[k]);for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');cfg.sections["creature_10"][name]=fields[k];}
  const auto c=reconstruction::normalizeCreatureMovementConfig(assets::creatureMovementConfig(cfg,10));
#ifdef ORIGINAL_REFERENCE
  std::vector<unsigned char> table(16+28*0x5c9+16,0xa5);auto* receiver=table.data()+16;std::memset(receiver,0,28*0x5c9);
  using Load=unsigned(__attribute__((fastcall)) *)(const char*,const char*,void*);
  if(reinterpret_cast<Load>(0x502b20)("cfg\\creature.cfg","cfg\\HTH.cfg",receiver)!=1)throw std::runtime_error("Original loader failed");
  const auto get=[](const unsigned char* p,unsigned at){std::int32_t n;std::memcpy(&n,p+at,4);return n;};
  for(unsigned i=0;i<28;++i){const auto* p=receiver+i*0x5c9;
   if(get(p,0)!=int(i) || get(p,8)!=c.width || get(p,12)!=c.height || get(p,16)!=c.acceleration || get(p,0x3c)!=int(c.canFly) || get(p,0x44)!=c.swimming || get(p,0x5bd)!=c.groundSpeed || get(p,0x5c1)!=c.flyingSpeed)throw std::runtime_error("Original selected CFG fields differ");
  }
  for(unsigned i=0;i<16;++i)if(table[i]!=0xa5 || table[table.size()-1-i]!=0xa5)throw std::runtime_error("Type-table guard changed");
  if(!c.canFly){
   auto* p=receiver+10*0x5c9;
   std::uint32_t asset[3]={reinterpret_cast<std::uintptr_t>(aniBytes.data()),reinterpret_cast<std::uintptr_t>(aniBytes.data()+44),reinterpret_cast<std::uintptr_t>(aniBytes.data()+44+ani.starts.size()*4)};
   std::memcpy(reinterpret_cast<void*>(0x6dbf00+10*276),asset,sizeof(asset));
   std::uint32_t fallback=720;std::memcpy(reinterpret_cast<void*>(0x6c007d),&fallback,4);*reinterpret_cast<float*>(0x5e15f0)=0.969F;
   using Build=void(__attribute__((thiscall)) *)(void*,unsigned);reinterpret_cast<Build>(0x505220)(p,10);
   if(std::memcmp(p+0xd8,samples.data(),samples.size()*4) || get(p,0x589)!=int(maximum)){std::cerr<<"Maximum "<<get(p,0x589)<<" vs "<<maximum<<" samples ";for(unsigned i=0;i<48;++i)std::cerr<<get(p,0xd8+4*i)<<":"<<samples[i]<<",";std::cerr<<"\n";throw std::runtime_error("Original derived ANI sample/max mismatch");}
  }
#endif
  ++count;
 }
 unsigned motionComparisons=0;
#ifdef ORIGINAL_REFERENCE
 motionComparisons=compareMotion(ani,samples);
#endif
 std::cout<<"{\"motion_comparisons\":"<<motionComparisons<<",\"all_match\":true,\"profiles\":"<<count<<",\"selected_field_comparisons\":"<<count*28*7<<",\"maximum\":"<<maximum<<",\"samples\":[";
 for(unsigned i=0;i<48;++i){if(i)std::cout<<',';std::cout<<samples[i];}std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
