#define main sprite_reference_unused_main
#define u32 sprite_original_word
#include "sprite-binary-reference.cpp"
#undef main
#undef u32
#include "../reconstruction/rendering/particle_sprite.hpp"
#include <cstdio>
using u32=std::uint32_t;using u8=unsigned char;
namespace {
struct Span{u32 at,n;};std::vector<Span> spans;u32 imageBytes,stackAt,stackBytes;
struct Limit{u32 at=0,n=0,visible=0;} limit;
u32 addr(const void* p){return u32(reinterpret_cast<std::uintptr_t>(p));}
u32 get(const void* p){u32 v;std::memcpy(&v,p,4);return v;}
int readable(u32 p,u32 n){
 if(!p||!n||p+n<p)return 0;
 if(limit.n&&p<limit.at+limit.n&&limit.at<p+n)return p>=limit.at&&n<=limit.visible&&p-limit.at<=limit.visible-n;
 if(p>=0x400000&&n<=imageBytes&&p-0x400000<=imageBytes-n)return 1;
 if(p>=stackAt&&n<=stackBytes&&p-stackAt<=stackBytes-n)return 1;
 for(const auto& s:spans)if(p>=s.at&&n<=s.n&&p-s.at<=s.n-n)return 1;
 return 0;
}
#include "../runtime/scene/world_raster_callers.h"
#include "../runtime/scene/kind8_sprite_admission.h"
#include "../runtime/scene/fading_particle_admission.h"
u32 batch_guard_open=1,batch_owner=123,currentThread=123;
u32 GetCurrentThreadId(){return currentThread;}
#include "sprite-batch-caller.inc"
#include "../reconstruction/rendering/kind8_class.h"
#include "sprite-observer-caller.inc"
u32 controls=0,actual=0,variant=0,backendReturn=0,frameAddress=0;
std::vector<std::array<u32,6>> events;
void check(bool value){++controls;if(!value)throw std::runtime_error("Fading control "+std::to_string(controls));}
void observe(u32 tag,const u32* stack,u32 source,int x,int y,u32 shade,u32 palette){
 stackAt=addr(stack);stackBytes=44;check(raster_batch_caller(stack[0],tag,stack));
 check(!raster_caller(stack[0],tag,stack));check(source==frameAddress);++actual;
 events.push_back({tag,source,u32(x),u32(y),shade,palette});stackAt=stackBytes=0;
}
__attribute__((noinline)) u32 word(void* source,int x,int y){observe(variant?36:22,static_cast<u32*>(__builtin_frame_address(0))+1,addr(source),x,y,0,0);return backendReturn;}
__attribute__((noinline)) u32 generic(void* source,int x,int y,u32 shade,u32 palette){observe(variant?14:13,static_cast<u32*>(__builtin_frame_address(0))+1,addr(source),x,y,shade,palette);return backendReturn;}
__attribute__((noinline)) u32 black(void* source,int x,int y){observe(12,static_cast<u32*>(__builtin_frame_address(0))+1,addr(source),x,y,0,0);return backendReturn;}
__attribute__((noinline)) u32 clipped(void* source,int x,int y,u32 shade,u32 palette){observe(17,static_cast<u32*>(__builtin_frame_address(0))+1,addr(source),x,y,shade,palette);return backendReturn;}
#define BLEND(name,tag) __attribute__((noinline,fastcall)) u32 name(void* source,int x,int y,u32 shade,u32 palette){observe(tag,static_cast<u32*>(__builtin_frame_address(0))+1,addr(source),x,y,shade,palette);return backendReturn;}
BLEND(blend50,18) BLEND(blend75,19) BLEND(blend25,20)
void bind(u32 at,std::uintptr_t to){auto* p=reinterpret_cast<u8*>(at);p[0]=0xe9;const u32 rel=u32(to-at-5);std::memcpy(p+1,&rel,4);}
__attribute__((naked,noinline)) u32 invoke(const u32*){
 __asm__ volatile("push %ebp\nmov %esp,%ebp\npush %ebx\npush %esi\npush %edi\nmov 8(%ebp),%esi\npush $1f\nmov $0x5006a2,%eax\njmp *%eax\n1:\npop %edi\npop %esi\npop %ebx\nleave\nret");
}
}
int main(int argc,char** argv)try{
 check(argc==2);const auto image=read(argv[1]);map_image(image);imageBytes=sprite_original_word(image,sprite_original_word(image,60)+24+56);
 std::array<u8,160> object{};u32 frame[8]{},row[9]{},stack[11]{};frameAddress=addr(frame);
 auto put=[&](u32 offset,u32 value){std::memcpy(object.data()+offset,&value,4);};
 auto reset=[&](const FadingParticleSite& site,u32 tag){
  limit={};std::fill(object.begin(),object.end(),0);std::fill(std::begin(row),std::end(row),0);std::fill(std::begin(stack),std::end(stack),0);
  put(0,0x5c752c);put(0x5a,frameAddress);frame[7]=tag==22||tag==36?0xffffffffu:0;
  row[1]=addr(object.data());row[6]=8;row[7]=0x8ad08ad0;stack[0]=site.address;stack[1]=frameAddress;stack[site.parent_word]=0x5006b2;stack[site.parent_word-3]=addr(row);
  spans={{addr(object.data()),160},{frameAddress,32},{addr(row),36},{addr(stack),44}};
  global(0x5e1404,1);global(0x6e1f88,0);batch_guard_open=1;currentThread=123;
 };
 unsigned pairs=0;auto* pages=static_cast<u8*>(mmap(nullptr,8192,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));check(pages!=MAP_FAILED);check(mprotect(pages+4096,4096,PROT_NONE)==0);spans.push_back({addr(pages),4096});
 for(const auto& site:fading_particle_sites)for(u32 tag=0;tag<64;++tag){
  reset(site,tag);const bool allowed=(tag<32&&(site.tag_mask&(1u<<tag)))||(site.address==0x54a509&&tag==36);check(bool(raster_batch_caller(site.address,tag,stack))==allowed);if(!allowed)continue;++pairs;
  auto valid=[&]{return raster_batch_caller(site.address,tag,stack)!=0;};check(kind8_supported(addr(row)));check(!raster_caller(site.address,tag,stack));check(!raster_batch_caller(site.address,64,stack));check(!raster_batch_caller(site.address,tag,nullptr));
  batch_guard_open=0;check(!valid());batch_guard_open=1;currentThread=124;check(!valid());currentThread=123;
  for(u32 off=0;off<334;++off){auto* b=reinterpret_cast<u8*>(0x54a4a0+off);const u8 saved=*b;*b^=1;check(!valid());*b=saved;}check(valid());
  for(auto span:std::initializer_list<Span>{{0x5006a2,16},{0x5006b2,10}})for(u32 off=0;off<span.n;++off){auto* b=reinterpret_cast<u8*>(span.at+off);const u8 saved=*b;*b^=1;check(!valid());*b=saved;}
  for(auto span:std::initializer_list<Span>{{0x54a4a0,334},{0x5c752c,16},{addr(object.data()),122},{addr(row),36},{frameAddress,32},{addr(stack),(site.parent_word+1)*4}}){for(u32 n=0;n<span.n;++n){limit={span.at,span.n,n};check(!valid());}limit={};check(valid());}
  for(u32 n=0;n<(site.parent_word+1)*4;++n)check(!raster_batch_caller(site.address,tag,reinterpret_cast<const u32*>(pages+4096-n)));
  for(u32 n=0;n<122;++n){auto* p=pages+4096-n;if(n)std::memset(p,0,n);if(n>=4){const u32 table=0x5c752c;std::memcpy(p,&table,4);}row[1]=addr(p);check(!valid());check(!kind8_supported(addr(row)));}row[1]=addr(object.data());
  stack[site.parent_word]=0x5006e0;check(!valid());stack[site.parent_word]=0x5006b2;stack[site.parent_word-3]=0;check(!valid());stack[site.parent_word-3]=addr(row);
  row[6]=9;check(!valid());row[6]=8;row[7]=0;check(!valid());row[7]=0x8ad08ad0;put(0,0x5c7514);check(!valid());put(0,0x5c752c);
  const u32 old=get(reinterpret_cast<void*>(0x5c7538));global(0x5c7538,old^1);check(!valid());global(0x5c7538,old);global(0x5e1404,0);check(!valid());global(0x5e1404,1);global(0x6e1f88,1);check(!valid());global(0x6e1f88,0);
  frame[7]^=0xffffffffu;check(!valid());frame[7]^=0xffffffffu;check(valid());
 }
 check(pairs==10);reset(fading_particle_sites[0],22);
 bind(0x595677,reinterpret_cast<std::uintptr_t>(&black));bind(0x57de00,reinterpret_cast<std::uintptr_t>(&clipped));bind(0x57ec90,reinterpret_cast<std::uintptr_t>(&blend50));bind(0x57f0f0,reinterpret_cast<std::uintptr_t>(&blend75));bind(0x57f5f0,reinterpret_cast<std::uintptr_t>(&blend25));
 global(0x6903dc,addr(reinterpret_cast<void*>(&word)));global(0x6c4e3c,addr(reinterpret_cast<void*>(&generic)));*reinterpret_cast<u8*>(0x50084c)=0xc3;
 unsigned cases=0;for(u32 v:{0u,1u})for(u32 phase:{0u,25u,26u,50u,51u,75u,76u,100u})for(int shade:{-127,-8,127})for(u32 ret:{0u,1u,2u,0x10001u})for(bool words:{false,true}){
  if(words&&phase<=75)continue;variant=v;backendReturn=ret;put(0x76,4);put(0x56,phase);put(0x46,u32(shade));put(0x72,0x12345678);frame[7]=words?0xffffffffu:0;global(0x6f1d58+4*36,1);global(0x5e1428,100);row[2]=u32(-123);row[3]=456;row[7]=0x8ad08ad0;row[8]=0x12345678;events.clear();
  const auto before=object;check((invoke(row)&65535)==0);check(row[7]==0x8ad0&&row[8]==0x12345678&&object==before);
  std::vector<std::array<u32,6>> expected;using namespace mnm::reconstruction;drawFadingParticleSprite(true,{frameAddress,words},4,int(phase),100,shade,0x12345678,-123,456,[&](ParticleSpriteEntry entry,u32 source,int x,int y,int sh,u32 pal){
   u32 tag=entry==ParticleSpriteEntry::word?(variant?36:22):entry==ParticleSpriteEntry::indexed?(variant?14:13):entry==ParticleSpriteEntry::black?12:entry==ParticleSpriteEntry::fallback?17:entry==ParticleSpriteEntry::blend50?18:entry==ParticleSpriteEntry::blend75?19:20;
   const bool shortArgs=tag==12||tag==22||tag==36;expected.push_back({tag,source,u32(x),u32(y),shortArgs?0u:u32(sh),shortArgs?0u:pal});return backendReturn;
  });check(events==expected);++cases;
 }
 check(cases==240);check(munmap(pages,8192)==0);
 std::printf("{\"success\":true,\"guard_controls\":%u,\"context_pairs\":%u,\"original_primary_cases\":%u,\"actual_guarded_callbacks\":%u,\"ordinary_policy_unchanged\":true}\n",controls,pairs,cases,actual);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
