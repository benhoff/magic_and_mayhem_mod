#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#else
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
using Bytes=std::vector<unsigned char>;
#endif
#include "terrain_light_script_relations.hpp"
#include <algorithm>
using namespace mnm::reconstruction;using U=std::uint32_t;
constexpr U Arena=0x10000000,Script=Arena+0x100,Characters=Arena+0x1000,Creatures=Arena+0x10000,Manager=Arena+0x30000;
struct Memory {
#ifndef ORIGINAL_REFERENCE
 Bytes image=Bytes(0x400000),arena=Bytes(0x400000);
#endif
 std::vector<U> events;
 unsigned char* ptr(U p){
#ifdef ORIGINAL_REFERENCE
  return reinterpret_cast<unsigned char*>(std::uintptr_t(p));
#else
  if(p>=0x400000&&p<0x800000)return image.data()+p-0x400000;if(p>=Arena&&p<Arena+arena.size())return arena.data()+p-Arena;throw std::runtime_error("Guest address");
#endif
 }
 U get32(U p){U n;std::memcpy(&n,ptr(p),4);return n;}void set32(U p,U n){std::memcpy(ptr(p),&n,4);}unsigned char get8(U p){return *ptr(p);}void set8(U p,unsigned char v){*ptr(p)=v;}
 U call(U address,U receiver,std::initializer_list<U> args={}){
  events.insert(events.end(),{address,receiver,U(args.size())});events.insert(events.end(),args);std::vector<U> a(args);
  if(address==0x466900){U i=get8(a.at(0))-'A';if(i>=8)throw std::runtime_error("Owner name");return 0x643a80+i*0x154;}
  if(address==0x465500||address==0x4655e0){U other=0x643a80+a.at(0)*0x154,which=address==0x465500?0x18:0x20;U mine=(receiver-0x643a80)/0x154;if(mine!=a.at(0)){set8(receiver+which+a.at(0),a.at(1));set8(other+which+mine,a.at(1));}}
  if(address==0x51ec80)set32(receiver+0x174,a.at(0));
  if(address==0x50ab30)set32(receiver+0x5ec,0);
  if(address==0x463cb0)return Arena+0x40000;
  return address==0x564d60?0x77:0;
 }
} memory;
#ifdef ORIGINAL_REFERENCE
#define SERVICE0(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*){return memory.call(address,U(std::uintptr_t(p)));}
#define SERVICE1(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*,U a){return memory.call(address,U(std::uintptr_t(p)),{a});}
#define SERVICE2(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*,U a,U b){return memory.call(address,U(std::uintptr_t(p)),{a,b});}
#define SERVICE3(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*,U a,U b,U c){return memory.call(address,U(std::uintptr_t(p)),{a,b,c});}
SERVICE1(svc_owner,0x466900) SERVICE1(svc_change,0x51ec80) SERVICE2(svc_alliance,0x465500) SERVICE2(svc_share,0x4655e0)
SERVICE0(svc_cleanup,0x51edf0) SERVICE0(svc_reset,0x50ab30) SERVICE2(svc_publish,0x463eb0) SERVICE1(svc_finish,0x564d60) SERVICE3(svc_reveal,0x508ca0)
extern "C" U svc_visibility(){return memory.call(0x463cb0,0);}
extern "C" U __attribute__((fastcall)) svc_notification(void* p,void*,U a,U b,U c,U d,U e,U f,U g){return memory.call(0x571310,U(std::uintptr_t(p)),{a,b,c,d,e,f,g});}
#include "refresh-adapter-bytes.hpp"
static void redirect(U at,std::uintptr_t target){auto* p=memory.ptr(at);const auto& bytes=adapterBytes.at(at);if(std::memcmp(p,bytes.data(),bytes.size()))throw std::runtime_error("Adapter bytes");p[0]=0xe9;U delta=U(target)-at-5;std::memcpy(p+1,&delta,4);}
static void protect(const Bytes& pe){U head=u32(pe,60),opt=head+24;unsigned n=pe[head+6]|pe[head+7]<<8,os=pe[head+20]|pe[head+21]<<8;for(unsigned i=0;i<n;++i){U s=opt+os+i*40;if(u32(pe,s+36)&0x20000000){U rva=u32(pe,s+12),size=(std::max(u32(pe,s+8),u32(pe,s+16))+4095)&~4095u;if(mprotect(memory.ptr(0x400000+rva),size,PROT_READ|PROT_EXEC))throw std::runtime_error("RX protection");}}}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE output");auto pe=read(argv[1]);map_image(pe);if(mmap(reinterpret_cast<void*>(Arena),0x400000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Arena");
#define REDIRECT(at,name) redirect(at,reinterpret_cast<std::uintptr_t>(&name))
 REDIRECT(0x466900,svc_owner);REDIRECT(0x51ec80,svc_change);REDIRECT(0x465500,svc_alliance);REDIRECT(0x4655e0,svc_share);REDIRECT(0x51edf0,svc_cleanup);REDIRECT(0x50ab30,svc_reset);REDIRECT(0x508ca0,svc_reveal);REDIRECT(0x463cb0,svc_visibility);REDIRECT(0x463eb0,svc_publish);REDIRECT(0x564d60,svc_finish);REDIRECT(0x571310,svc_notification);protect(pe);const char* output=argv[2];
#else
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 std::ofstream out(output,std::ios::binary);std::uint64_t bytes=0;U totalEvents=0;
 const auto emit=[&](const void* p,std::size_t n){out.write(static_cast<const char*>(p),n);bytes+=n;};const auto word=[&](U v){emit(&v,4);};
 const auto name=[&](U p,U i){memory.set8(p,'A'+i%8);memory.set8(p+1,0);};
 for(U trial=0;trial<2048;++trial){
  memory.events.clear();std::memset(memory.ptr(Script),0,0x400);std::memset(memory.ptr(0x643a80),0,8*0x154);std::memset(memory.ptr(Characters),0,8*0x40);std::memset(memory.ptr(Creatures),0,12*0xe4b);
  std::memcpy(memory.ptr(0x5fc874),"NONE",5);memory.set32(0x6f37f8,Characters);memory.set32(0x6f37fc,trial%11==0?0:8);memory.set32(0x6def58,Creatures);memory.set32(0x6def5c,12);memory.set32(0x6df180,trial%13==0?0:12);memory.set32(0x644520,(trial/7)%8);
  for(U i=0;i<8;++i){U p=0x643a80+i*0x154;memory.set32(p+4,(trial+i)%3==0);name(p+0x50,i);for(U j=0;j<8;++j){memory.set8(p+0x18+j,(trial+i+j)%3?1:0);memory.set8(p+0x20+j,(trial+i+j)%2?0x80:0);}U c=Characters+i*0x40;memory.set32(c,(trial+i)%5!=0);name(c+4,i);memory.set32(c+0x28,(trial+i)%7==0?~U(0):i);memory.set32(c+0x3c,(trial+i)%4!=0);}
  for(U i=0;i<12;++i){U c=Creatures+i*0xe4b;memory.set32(c+4,(trial+i)%5!=0);memory.set32(c+0xe4,(trial+i)%3!=0);memory.set32(c+0x174,(trial+i)%8);for(U j=0;j<3;++j)memory.set32(c+8+j*4,3+i+j);const U states[5]={3,9,10,11,12};memory.set32(c+0x5ec,states[(trial+i)%5]);memory.set32(c+0x5f0,(trial+i)%2?3:0);}
  const U offsets[6]={0x2cc,0x2ec,0x30c,0x32c,0x34c,0x36c};
  for(U j=0;j<6;++j){if(trial&(1u<<j))name(Script+offsets[j],(trial/64+j/2)%8);else std::memcpy(memory.ptr(Script+offsets[j]),"NONE",5);}
  // The original missing-partner sharing path reads uninitialized owner
  // locals (retained negative evidence); valid actions supply both names.
  if(trial&32)name(Script+0x30c,(trial/64+1)%8);
  memory.set32(Script+0x2c,trial%3?15:~U(0));memory.set32(Script+0x30,trial%2);memory.set32(Script+0x38c,(trial/3)%2);
  U result;
#ifdef ORIGINAL_REFERENCE
  using Call=U(__attribute__((thiscall))*)(void*,U);result=reinterpret_cast<Call>(0x56a8d0)(memory.ptr(Manager),Script);
#else
  result=TerrainLightScriptRelations<Memory>(memory).execute(Manager,Script);
#endif
  word(result);emit(memory.ptr(0x643a80),8*0x154);emit(memory.ptr(Creatures),12*0xe4b);emit(memory.ptr(Characters),8*0x40);emit(memory.ptr(Script),0x400);word(memory.events.size());emit(memory.events.data(),memory.events.size()*4);totalEvents+=memory.events.size();
 }
#ifdef ORIGINAL_REFERENCE
 for(const auto& pair:protectedBytes)if(std::memcmp(memory.ptr(pair.first),pair.second.data(),pair.second.size()))throw std::runtime_error("Original body changed");
#else
 memory.events.clear();std::memcpy(memory.ptr(Script+0x30c),"NONE",5);name(Script+0x36c,0);
 Bytes beforeOwners(memory.ptr(0x643a80),memory.ptr(0x643a80)+8*0x154),beforeCreatures(memory.ptr(Creatures),memory.ptr(Creatures)+12*0xe4b);
 bool refused=false;try{TerrainLightScriptRelations<Memory>(memory).execute(Manager,Script);}catch(const std::invalid_argument&){refused=true;}
 if(!refused||!memory.events.empty()||std::memcmp(beforeOwners.data(),memory.ptr(0x643a80),beforeOwners.size())||std::memcmp(beforeCreatures.data(),memory.ptr(Creatures),beforeCreatures.size()))throw std::runtime_error("Malformed sharing action did not refuse atomically");
#endif
 if(!out)throw std::runtime_error("Output");std::cout<<"{\"success\":true,\"script_cases\":2048,\"event_words\":"<<totalEvents<<",\"compared_bytes\":"<<bytes<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
