// Whole bounded callers and an explicitly delimited scheduler region in a
// private PE32 mapping. All external services below are checked recorders.
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
#include <string>
#include <vector>
using Bytes=std::vector<unsigned char>;
#endif
#include "terrain_light_refresh.hpp"
#include "terrain_light_suppliers.hpp"
#include "terrain_lighting_cycle.hpp"
#include <algorithm>
#include <memory>
using namespace mnm::reconstruction;
using U=std::uint32_t;
constexpr U Arena=0x10000000,Source=Arena+0x10000,Created=Source+0xe4b;
constexpr U Owner=Arena+0x20000,Creator=Arena+0x30000,Parent=Arena+0x100000,Map=0x6c5490;
struct Memory {
#ifndef ORIGINAL_REFERENCE
 Bytes image=Bytes(0x400000),arena=Bytes(0x400000);
#endif
 std::vector<U> events;
 U collision=0,ownerFound=1,allocation=1,placement=1,deactivate=0,timer=1,deadlineService=1,reentrant=0;
 bool full=false;
 std::unique_ptr<TerrainLightingCycle> cycle;
 unsigned char* ptr(U p) {
#ifdef ORIGINAL_REFERENCE
  return reinterpret_cast<unsigned char*>(std::uintptr_t(p));
#else
  if(p>=0x400000 && p<0x800000)return image.data()+p-0x400000;
  if(p>=Arena && p<Arena+arena.size())return arena.data()+p-Arena;
  throw std::runtime_error("Guest address outside fixture");
#endif
 }
 U get32(U p){U v;std::memcpy(&v,ptr(p),4);return v;}
 void set32(U p,U v){std::memcpy(ptr(p),&v,4);}
 void set8(U p,std::uint8_t v){*ptr(p)=v;}
 void event(U address,U receiver,std::initializer_list<U> args={}) {
  events.insert(events.end(),{address,receiver,U(args.size())});events.insert(events.end(),args);
 }
 U call(U address,U receiver,std::initializer_list<U> args={}) {
  event(address,receiver,args);std::vector<U> a(args);
  switch(address) {
   case 0x4f13f0:return collision;
   case 0x466870:return ownerFound?Owner:0;
   case 0x52a640:return allocation?Created:0;
   case 0x4f5230:return placement;
   case 0x536f00:return timer;
   case 0x4f7540:return deadlineService;
   case 0x5070e0:
    for(U i=0;i<3;++i)set32(receiver+8+i*4,a.at(i));
    set32(receiver+0xa8,a.at(3));set32(receiver+4,1);set32(receiver+0xe4,1);break;
   case 0x506290:set32(receiver+0x174,a.at(1));break;
   case 0x51aa30:if(deactivate)set32(receiver+4,0);break;
  }
  return 0;
 }
 U convertColumn(U v){event(0x40e650,0,{v});return (v+2)%19;}
 U convertRow(U v){event(0x4132e0,0,{v});return (v+3)%21;}
 U resolvePosition(U p,std::array<U,3>& xyz){event(0x44d970,p,{xyz[0],xyz[1],xyz[2]});return 0x55;}
 U submitCommand(std::array<U,3>,const std::array<std::uint8_t,22>&){throw std::runtime_error("Command continuation outside this older fixture");}
 void requestRefresh(){set32(0x5e41a8,1);}
 void subcycle(bool objects) {
  event(objects?0x4f2ad0:0x4f27d0,Map,{get32(0x5e41a8)});
  if(reentrant==(objects?2u:1u))set32(0x5e41a8,0x80);
 }
 void updateLighting() {
  if(!full){subcycle(false);subcycle(true);}else{
   TerrainLightingSnapshot s;s.player=int(get32(0x644520));s.changed=int(get32(0x5e41a8));
   s.creaturesEnabled=get32(Map+0x808)!=0;s.objectsEnabled=get32(Map+0x80c)!=0;
   s.creatureScan=2;s.objectScan=1;s.view={9,10,9};s.relations=initialTerrainLightRelations();
   for(U p:{Source,Created})s.creatures.push_back({get32(p+4)!=0,get32(p+0xe4)!=0,int(get32(p+0x174)),get32(p+8),get32(p+12),get32(p+16)});
   const U o=Arena+0x40000;s.objects.push_back({get32(o+4)!=0,2,get32(o+8),get32(o+12),get32(o+16),9,10});
   cycle->step(s);
  }
  set32(0x5e41a8,0);
 }
 void schedulerExit(bool aborted){event(0xf0000001,Parent,{aborted?1u:0u});}
} memory;
#ifdef ORIGINAL_REFERENCE
extern "C" U __attribute__((fastcall)) svc_collision(void* p,void*,U a,U b,U c,U d,U e,U f){return memory.call(0x4f13f0,U(std::uintptr_t(p)),{a,b,c,d,e,f});}
extern "C" U __attribute__((fastcall)) svc_owner(void* p,void*,U a){return memory.call(0x466870,U(std::uintptr_t(p)),{a});}
extern "C" U __attribute__((fastcall)) svc_allocate(void* p,void*){return memory.call(0x52a640,U(std::uintptr_t(p)));}
extern "C" U __attribute__((fastcall)) svc_init(void* p,void*,U a,U b,U c,U d){return memory.call(0x5070e0,U(std::uintptr_t(p)),{a,b,c,d});}
extern "C" U __attribute__((fastcall)) svc_bind(void* p,void*,U a,U b,U c){return memory.call(0x506290,U(std::uintptr_t(p)),{a,b,c});}
extern "C" U __attribute__((fastcall)) svc_state(void* p,void*,U a){return memory.call(0x5183e0,U(std::uintptr_t(p)),{a});}
extern "C" U __attribute__((fastcall)) svc_x(void*,void*,U v){return memory.convertColumn(v);}
extern "C" U __attribute__((fastcall)) svc_y(void*,void*,U v){return memory.convertRow(v);}
extern "C" U __attribute__((fastcall)) svc_resolve(void* p,void*,U*,U* x,U* y,U* z){std::array<U,3> a{*x,*y,*z};return memory.resolvePosition(U(std::uintptr_t(p)),a);}
extern "C" U __attribute__((fastcall)) svc_place(void* p,void*,U a,U b){return memory.call(0x4f5230,U(std::uintptr_t(p)),{a,b});}
#define SERVICE0(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*){return memory.call(address,U(std::uintptr_t(p)));}
#define SERVICE1(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*,U a){return memory.call(address,U(std::uintptr_t(p)),{a});}
#define SERVICE3(name,address) extern "C" U __attribute__((fastcall)) name(void* p,void*,U a,U b,U c){return memory.call(address,U(std::uintptr_t(p)),{a,b,c});}
SERVICE3(svc_remove,0x5093e0) SERVICE3(svc_add,0x509230)
SERVICE0(svc_a8f0,0x51a8f0) SERVICE0(svc_f850,0x50f850) SERVICE0(svc_aa30,0x51aa30)
SERVICE0(svc_a880,0x51a880) SERVICE0(svc_78c0,0x5178c0) SERVICE1(svc_release_state,0x54dce0)
SERVICE0(svc_0e70,0x520e70) SERVICE0(svc_ab30,0x50ab30) SERVICE1(svc_dispatch,0x54dad0)
SERVICE0(svc_4380,0x534380) SERVICE0(svc_2e20,0x4f2e20) SERVICE0(svc_3900,0x4f3900)
SERVICE0(svc_4d10,0x544d10) SERVICE0(svc_5910,0x545910) SERVICE0(svc_0f00,0x4e0f00)
SERVICE0(svc_f5d0,0x46f5d0) SERVICE0(svc_1300,0x571300) SERVICE0(svc_7540,0x4f7540)
extern "C" U __attribute__((fastcall)) svc_timer(void* p,void*,U a,U b){return memory.call(0x536f00,U(std::uintptr_t(p)),{a,b});}
extern "C" void svc_exit(){memory.schedulerExit(false);}
extern "C" void svc_abort(){memory.schedulerExit(true);}
extern "C" void __attribute__((fastcall)) svc_creatures(void*,void*){memory.subcycle(false);}
extern "C" void __attribute__((fastcall)) svc_objects(void*,void*){memory.subcycle(true);}
#include "refresh-adapter-bytes.hpp"
static void redirect(U address,std::uintptr_t target) {
 const auto& expected=adapterBytes.at(address);auto* p=memory.ptr(address);
 if(std::memcmp(p,expected.data(),expected.size()))throw std::runtime_error("Adapter original bytes mismatch: "+std::to_string(address));
 p[0]=0xe9;const U d=U(target)-address-5;std::memcpy(p+1,&d,4);
}
static void executeScheduler() {
 U receiver=Parent,entry=0x46b836;
 asm volatile("push %%ebp\n\tpush %%ebx\n\tpush %%esi\n\tpush %%edi\n\tmov %%eax,%%esi\n\txor %%ebx,%%ebx\n\tcall *%%edx\n\tpop %%edi\n\tpop %%esi\n\tpop %%ebx\n\tpop %%ebp"
  : "+a"(receiver),"+d"(entry) : : "ecx","memory","cc");
}
static void protect(const Bytes& pe,bool writable) {
 U head=u32(pe,0x3c),opt=head+24;unsigned n=pe[head+6]|(pe[head+7]<<8),os=pe[head+20]|(pe[head+21]<<8);
 for(unsigned i=0;i<n;++i){U s=opt+os+i*40;if(u32(pe,s+36)&0x20000000){U rva=u32(pe,s+12),size=(std::max(u32(pe,s+8),u32(pe,s+16))+4095)&~4095u;
 if(mprotect(memory.ptr(0x400000+rva),size,PROT_READ|PROT_EXEC|(writable?PROT_WRITE:0)))throw std::runtime_error("Code protection failed");}}
}
#endif
int main(int argc,char** argv)try {
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected pinned PE and output");const auto pe=read(argv[1]);map_image(pe);
 if(mmap(reinterpret_cast<void*>(Arena),0x400000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Guest arena map failed");
#define REDIRECT(addr,name) redirect(addr,reinterpret_cast<std::uintptr_t>(&name))
 REDIRECT(0x4f13f0,svc_collision);REDIRECT(0x466870,svc_owner);REDIRECT(0x52a640,svc_allocate);
 REDIRECT(0x5070e0,svc_init);REDIRECT(0x506290,svc_bind);REDIRECT(0x5183e0,svc_state);
 REDIRECT(0x40e650,svc_x);REDIRECT(0x4132e0,svc_y);REDIRECT(0x44d970,svc_resolve);REDIRECT(0x4f5230,svc_place);
 REDIRECT(0x5093e0,svc_remove);REDIRECT(0x509230,svc_add);REDIRECT(0x51a8f0,svc_a8f0);REDIRECT(0x50f850,svc_f850);REDIRECT(0x51aa30,svc_aa30);
 REDIRECT(0x51a880,svc_a880);REDIRECT(0x5178c0,svc_78c0);REDIRECT(0x54dce0,svc_release_state);REDIRECT(0x520e70,svc_0e70);REDIRECT(0x50ab30,svc_ab30);REDIRECT(0x54dad0,svc_dispatch);
 REDIRECT(0x534380,svc_4380);REDIRECT(0x4f2e20,svc_2e20);REDIRECT(0x4f3900,svc_3900);REDIRECT(0x544d10,svc_4d10);REDIRECT(0x545910,svc_5910);REDIRECT(0x4e0f00,svc_0f00);REDIRECT(0x46f5d0,svc_f5d0);REDIRECT(0x571300,svc_1300);REDIRECT(0x536f00,svc_timer);REDIRECT(0x4f7540,svc_7540);
 REDIRECT(0x46b91a,svc_exit);REDIRECT(0x46bca3,svc_abort);
 const Bytes savedCreature(memory.ptr(0x4f27d0),memory.ptr(0x4f27d0)+8),savedObjects(memory.ptr(0x4f2ad0),memory.ptr(0x4f2ad0)+8);
 REDIRECT(0x4f27d0,svc_creatures);REDIRECT(0x4f2ad0,svc_objects);protect(pe,false);
 const char* output=argv[2];
#else
 if(argc!=2)throw std::runtime_error("Expected output");const char* output=argv[1];
#endif
 TerrainLightRefreshCallers<Memory> native(memory);std::ofstream out(output,std::ios::binary);
 std::uint64_t bytes=0;U creationCases=0,positionCases=0,schedulerCases=0,sceneTicks=0,sceneCheckpoints=0;
 const auto emit=[&](const void* p,std::size_t n){out.write(static_cast<const char*>(p),n);bytes+=n;};
 const auto word=[&](U v){emit(&v,4);};
 const auto events=[&](){word(U(memory.events.size()));emit(memory.events.data(),memory.events.size()*4);};
 const auto reset=[&](){memory.events.clear();std::memset(memory.ptr(Source),0,0xe4b*2);std::memset(memory.ptr(Creator),0x69,0x200);std::memset(memory.ptr(Owner),0,0x1000);
  memory.set32(0x6def58,Source);memory.set32(0x6def5c,2);memory.set32(0x644520,0);
  memory.set32(Creator+8,7);memory.set32(Creator+12,8);memory.set32(Creator+16,2);memory.set32(Creator+0x44,0);memory.set32(Creator+0x198,0);
  memory.set32(Source,123);memory.set32(Created,456);memory.set32(Source+0x174,0);memory.set32(Source+0x104,37);
  memory.collision=0;memory.ownerFound=1;memory.allocation=1;memory.placement=1;memory.deactivate=0;
 };
 const auto create=[&](U kind,U inherit){
#ifdef ORIGINAL_REFERENCE
  using Call=U(__attribute__((thiscall))*)(void*,U,U);return reinterpret_cast<Call>(0x48f690)(memory.ptr(Creator),kind,inherit);
#else
  return native.createFromLinkedCreature(Creator,kind,inherit);
#endif
 };
 const auto position=[&](U p,U event){
#ifdef ORIGINAL_REFERENCE
  using Call=U(__attribute__((thiscall))*)(void*,U);return reinterpret_cast<Call>(0x51e890)(memory.ptr(p),event);
#else
  return native.positionEvent(p,event);
#endif
 };
 const auto schedule=[&](){
#ifdef ORIGINAL_REFERENCE
  executeScheduler();
#else
  native.schedulerRegion(Parent);
#endif
 };
 for(U trial=0;trial<384;++trial){reset();U a=trial;const U kind=a%3;a/=3;U inherit=a%2;a/=2;memory.collision=a%2;a/=2;memory.ownerFound=a%2;a/=2;memory.allocation=a%2;a/=2;U equal=a%2;a/=2;U cap=a%2;a/=2;U old=a%2?0x80000000:0;
  memory.set32(0x5e41a8,old);memory.set32(0x644520,equal?0:1);memory.set32(Owner+0x8f1,3);memory.set32(Owner+0x8f5,cap?3:2);
  memory.set32(0x6a5f88+kind*0x5c9,11+kind);memory.set32(0x6a5f8c+kind*0x5c9,21+kind);
  word(create(kind,inherit));emit(memory.ptr(Creator),0x200);emit(memory.ptr(Source),0xe4b*2);word(memory.get32(0x5e41a8));events();++creationCases;
 }
 for(U trial=0;trial<640;++trial){reset();U a=trial;const U event=std::array<U,5>{0,6,9,15,0xffffffff}[a%5];a/=5;memory.placement=a%2;a/=2;memory.deactivate=a%2;a/=2;U state=a%4;a/=4;U optional=a%2;a/=2;U alternate=a%2;a/=2;
  memory.set32(Source+0x70c,4);memory.set32(Source+0x710,6);memory.set32(Source+0x714,2);memory.set32(Source+0x718,0);memory.set32(Source+4,1);
  memory.set32(Source+0xa0,state);memory.set32(Source+0x618,optional);memory.set32(Source+0x977,0);memory.set32(Source+0x798,alternate);
  memory.set32(Source+0x97f,2);memory.set32(Source+0x983,3);memory.set32(Source+0x987,1);memory.set32(0x5e41a8,a%2?2:0);
  word(position(Source,event));emit(memory.ptr(Source),0xe4b);word(memory.get32(0x5e41a8));events();++positionCases;
 }
 for(U trial=0;trial<1152;++trial){memory.events.clear();std::memset(memory.ptr(Parent),0,0x10400);U a=trial;
  memory.set32(Parent+0xfd5a,a%2);a/=2;memory.timer=a%2;a/=2;memory.set32(0x5e1404,a%2);a/=2;memory.set32(0x6deed0,a%2);a/=2;memory.set32(0x6f2aa8,a%2);a/=2;memory.deadlineService=a%2;a/=2;memory.reentrant=a%3;a/=3;
  memory.set32(0x5e41a8,std::array<U,3>{0,1,0x80000000}[a%3]);memory.set32(Parent+0xfd62,77);memory.set32(Parent+0xfdea,100);
  memory.set32(0x6c4830,a%2?101:100);memory.set32(0x657cdc,0xffffffff);
  schedule();word(memory.get32(0x5e41a8));word(memory.get32(Parent+0xfdea));events();++schedulerCases;
 }
#ifdef ORIGINAL_REFERENCE
 // Restore exact original subcycle bytes; the scheduler, outer updater, both
 // subcycles, stamps, request helper and initializer now execute unchanged.
 protect(pe,true);std::memcpy(memory.ptr(0x4f27d0),savedCreature.data(),8);std::memcpy(memory.ptr(0x4f2ad0),savedObjects.data(),8);protect(pe,false);
#endif
 memory.full=true;
 for(U trial=0;trial<12;++trial){reset();memory.cycle=std::make_unique<TerrainLightingCycle>(19,21,5,TerrainLightingConfig{-40,9});
  std::memset(memory.ptr(Map),0,0x66c0);memory.set32(Map+4,19);memory.set32(Map+8,21);memory.set32(Map+12,5);memory.set32(Map+16,19*21);const U count=19*21*3;memory.set32(Map+0x7e0,count);
  for(U i=0;i<5;++i){const U at=Arena+0x200010+i*0x10000;std::memset(memory.ptr(at-16),0xa5,count+32);std::memset(memory.ptr(at),0x81,count);memory.set32(Map+0x7cc+i*4,at);}
  for(U y=0;y<21;++y)memory.set32(Map+0x64b2+y*4,y*19);
  for(U z=0;z<3;++z)memory.set32(Map+0x6432+z*4,z*19*21);
  memory.set32(0x5e18e8,U(-40));memory.set32(0x5e18ec,9);memory.set8(0x5e18f0,0x81);
  memory.set32(Map+0x808,1);memory.set32(Map+0x80c,1);memory.set32(Map+0x642a,1);memory.set32(Map+0x6426,1);memory.set32(Map+0x642e,0);
#ifdef ORIGINAL_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall))*)(void*)>(0x4ecdd0)(memory.ptr(Map));
#endif
  for(U at=0x6e8d90;at<=0x6e8dac;at+=4)memory.set32(at,0);
  memory.set32(0x6df180,2);const U object=Arena+0x40000;std::memset(memory.ptr(object),0,0x22e);memory.set32(object+4,1);memory.set32(object+8,9);memory.set32(object+12,10);memory.set32(object+16,2);memory.set32(object+0x28,0);memory.set32(object+0x1c2,9);memory.set32(object+0x1c6,10);
  memory.set32(0x689498,object);memory.set32(0x68949c,1);memory.set32(0x6898dc,1);memory.set32(0x689498+12,2);
  memory.set32(0x689930+0x39,9);memory.set32(0x689930+0x3d,10);memory.set32(0x689930+0x59,9);
  std::memset(memory.ptr(Parent),0,0x10400);memory.set32(Parent+0xfd62,1);memory.set32(0x6deed0,0);memory.set32(0x6f2aa8,0);memory.set32(0x657cdc,3);memory.set32(0x5e41a8,1);memory.timer=1;memory.deadlineService=0;
  for(U tick=0;tick<32;++tick){memory.events.clear();memory.set32(0x6c4830,tick);
   if(tick==1 || tick==9){memory.set32(Creator+8,(tick+trial)%19);memory.set32(Creator+12,(tick+trial)%21);word(create(0,tick%2));}
   if(tick==3 || tick==18){memory.set32(Created+0x70c,(tick+trial)%19);memory.set32(Created+0x710,(tick+trial)%21);memory.set32(Created+0x714,2);memory.set32(Created+0x718,0);word(position(Created,9));}
   memory.set32(0x5e1404,(tick%7)!=1);memory.timer=tick%11!=3;memory.set32(Parent+0xfd5a,(tick%13)==5);
   schedule();word(memory.get32(0x5e41a8));events();
#ifdef ORIGINAL_REFERENCE
   std::array<U,11> state{memory.get32(0x6e8d90),memory.get32(0x6e8d94),memory.get32(0x6e8d98),memory.get32(0x6e8d9c),memory.get32(Map+0x642e),memory.get32(0x6e8da0),memory.get32(0x6e8da4),memory.get32(0x6e8da8),memory.get32(0x6e8dac),memory.get32(Map+0x642a),memory.get32(Map+0x6426)};
#else
   const auto& c=memory.cycle->creatures();const auto& o=memory.cycle->objects();std::array<U,11> state{c.phase,c.remaining,c.quota,c.cursor,c.published,o.phase,o.remaining,o.quota,o.cursor,o.active,o.requested};
#endif
   emit(state.data(),state.size()*4);
   for(U i=0;i<5;++i){U at=Arena+0x200010+i*0x10000;
#ifdef ORIGINAL_REFERENCE
    emit(memory.ptr(at),count);
#else
    emit(memory.cycle->field().buffers()[i].data(),count);
#endif
    for(U g=0;g<16;++g)if(*memory.ptr(at-16+g)!=0xa5 || *memory.ptr(at+count+g)!=0xa5)throw std::runtime_error("Light buffer guard changed");
   }
   ++sceneTicks;++sceneCheckpoints;
  }
 }
 #ifdef ORIGINAL_REFERENCE
 for(const auto& item:protectedBytes)if(std::memcmp(memory.ptr(item.first),item.second.data(),item.second.size()))throw std::runtime_error("Protected original body changed");
#endif
 if(!out)throw std::runtime_error("Output failed");
 std::cout<<"{\"success\":true,\"creation_cases\":"<<creationCases<<",\"position_cases\":"<<positionCases<<",\"scheduler_cases\":"<<schedulerCases<<",\"scene_ticks\":"<<sceneTicks<<",\"scene_checkpoints\":"<<sceneCheckpoints<<",\"compared_bytes\":"<<bytes<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
