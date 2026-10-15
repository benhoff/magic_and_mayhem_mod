#define main sprite_reference_unused_main
#define u32 reference_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "../runtime/canvas-writers/pixel_route.h"
#include "../runtime/canvas-writers/fade_route.h"
extern "C" {
#include "../runtime/shadow/win32_min.h"
}
#include <algorithm>
#include <array>
#include <sstream>
#include <unistd.h>
extern "C" {
alignas(16) unsigned char pixel_host_fx[512],pixel_seed_fx[512],pixel_after_fx[512];
std::uint32_t pixel_saved_sp,pixel_seed_flags,pixel_after_flags,pixel_argc,pixel_function,pixel_object,pixel_args[6];
std::uint32_t pixel_seed_stack;
std::uint32_t pixel_gpr[8],pixel_unlock_gpr[3],pixel_unlock_args[2],pixel_unlocks;
void mnm_pixel_probe();void mnm_pixel_test_unlock();
int WIN PixelWritersInstallForTest();
static unsigned lastError=1234,threadId=1;
struct Region {std::uintptr_t begin,end;unsigned protect;};
static Region region(const void* p) {
 std::ifstream maps("/proc/self/maps");std::string line;const auto at=reinterpret_cast<std::uintptr_t>(p);
 while(std::getline(maps,line)){std::istringstream row(line);std::string range,permissions;row>>range>>permissions;
  const auto dash=range.find('-');const auto begin=std::stoul(range.substr(0,dash),nullptr,16),end=std::stoul(range.substr(dash+1),nullptr,16);
  if(at>=begin && at<end)return {begin,end,permissions[0]!='r'?1u:permissions[2]=='x'?(permissions[1]=='w'?0x40u:0x20u):permissions[1]=='w'?4u:2u};}return {};
}
u32 WIN VirtualQuery(const void* p,void* out,u32 size){if(size!=28)return 0;const auto v=region(p);if(!v.end)return 0;auto* m=static_cast<u32*>(out);std::fill_n(m,7,0);m[0]=v.begin;m[3]=v.end-v.begin;m[4]=0x1000;m[5]=v.protect;lastError=999;return 28;}
u32 WIN GetLastError(){return lastError;}
void WIN SetLastError(u32 v){lastError=v;}
u32 WIN GetCurrentThreadId(){lastError=998;return threadId;}
void* WIN GetModuleHandleA(const char*){return reinterpret_cast<void*>(0x400000);}
[[maybe_unused]] static int WIN nativeDib(void*,int,int,int,int,int,int,int,int,const unsigned char*,const unsigned char*,unsigned,unsigned){return INT32_MIN;}
[[maybe_unused]] static int WIN pixelCurrentHistory(const void*,u32,u32,u32,u32,u32){return 0;}
[[maybe_unused]] static void WIN pixelCountProducer(u32,u32,u32){}
void* WIN GetProcAddress(void* module,const char* name){
#ifdef MNM_PIXEL_TEST_GETPROC
 return MNM_PIXEL_TEST_GETPROC(module,name);
#else
 (void)module;if(!std::strcmp(name,"NativeDrawCpuHistoryV1"))return reinterpret_cast<void*>(pixelCurrentHistory);if(!std::strcmp(name,"NativeDrawProducerCountV1"))return reinterpret_cast<void*>(pixelCountProducer);return reinterpret_cast<void*>(nativeDib);
#endif
}
u32 WIN GetEnvironmentVariableA(const char*,char* out,u32 size){if(size<2)return 2;out[0]='1';out[1]=0;return 1;}
void* WIN VirtualAlloc(void*,u32 n,u32,u32){auto* p=mmap(nullptr,n,7,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);return p==MAP_FAILED?nullptr:p;}
int WIN VirtualProtect(void* p,u32 n,u32 protect,u32* old){*old=region(p).protect;const auto begin=reinterpret_cast<std::uintptr_t>(p)&~4095u,end=(reinterpret_cast<std::uintptr_t>(p)+n+4095u)&~4095u;return mprotect(reinterpret_cast<void*>(begin),end-begin,protect==0x40?7:protect==0x20?5:protect==4?3:1)==0;}
int WIN FlushInstructionCache(HANDLE,const void*,u32){return 1;}
HANDLE WIN GetCurrentProcess(){return nullptr;}
void WIN ExitProcess(u32 code){_exit(code);}
[[maybe_unused]] static unsigned __attribute__((thiscall)) lock(void* object){return static_cast<unsigned*>(object)[9];}
}
struct State {std::array<unsigned,8> gpr;std::array<unsigned,3> unlockGpr;std::array<unsigned,2> args;std::array<unsigned char,512> fx;unsigned flags,unlocks,error;};
static State call(){pixel_unlocks=0;lastError=1234;mnm_pixel_probe();State s{};std::copy_n(pixel_gpr,8,s.gpr.begin());std::copy_n(pixel_unlock_gpr,3,s.unlockGpr.begin());std::copy_n(pixel_unlock_args,2,s.args.begin());std::copy_n(pixel_after_fx,512,s.fx.begin());s.flags=pixel_after_flags;s.unlocks=pixel_unlocks;s.error=lastError;return s;}
#ifndef MNM_PIXEL_TEST_UTILITIES
int main(int argc,char** argv) try {
 if(argc!=3)throw std::runtime_error("Expected original PE and result JSON");map_image(read(argv[1]));
 const unsigned sites[7]={0x54b31d,0x545ce9,0x54b53d,0x54b618,0x58d7e7,0x58d430,0x58edb0};
 const unsigned lengths[7]={8,6,8,8,8,5,6},functions[7]={0x54b2d0,0x545c10,0x54b510,0x54b5f0,0x58d770,0x58d3b0,0x58ed80};
 unsigned char original[7][8]={},patched[7][8]={};for(unsigned i=0;i<7;++i)std::memcpy(original[i],reinterpret_cast<void*>(sites[i]),lengths[i]);
 if(!PixelWritersInstallForTest())throw std::runtime_error("Production installer refused pinned source");
 for(unsigned i=0;i<7;++i)std::memcpy(patched[i],reinterpret_cast<void*>(sites[i]),lengths[i]);
 unsigned char* l=reinterpret_cast<unsigned char*>(0x58b660);l[0]=0xe9;*reinterpret_cast<unsigned*>(l+1)=reinterpret_cast<unsigned>(lock)-0x58b665;
 /* Wave preparation is retained, but its x87-to-integer helper is external to
  * the selected pixel contract. Both routes execute this same supplied stub. */
 reinterpret_cast<unsigned char*>(0x59bee0)[0]=0xdd;reinterpret_cast<unsigned char*>(0x59bee0)[1]=0xd8;
 reinterpret_cast<unsigned char*>(0x59bee0)[2]=0x31;reinterpret_cast<unsigned char*>(0x59bee0)[3]=0xc0;reinterpret_cast<unsigned char*>(0x59bee0)[4]=0xc3;
 asm volatile("fninit; fxsave %0":"=m"(pixel_seed_fx));
 unsigned cases=0,words=0,forwards=0;
 for(unsigned kind=0;kind<7;++kind)for(unsigned format=0;format<2;++format)for(unsigned cell=0;cell<384;++cell){
  const unsigned w=1+cell%63,h=1+(cell/9)%11,stride=w+1+cell%7,physical=stride*(h+2)*2+512;
  std::vector<std::uint16_t> seed(physical);for(unsigned i=0;i<physical;++i)seed[i]=static_cast<std::uint16_t>(i*317+cell*7919);
  auto pixels=seed;auto* target=pixels.data()+256;std::vector<std::uint16_t> source(physical);for(unsigned i=0;i<physical;++i)source[i]=i*593+cell;
  Bytes encoded(128,0),palette(768);for(unsigned i=0;i<768;++i)palette[i]=i*43+cell;
  for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;){const unsigned n=std::min(w-x,1+(x+y+cell)%17),v=(x*11+y*29+cell)%256;if(n==1 && v<192)encoded.push_back(v);else{encoded.push_back(192+n);encoded.push_back(v);}x+=n;}
  unsigned pcx[5]={reinterpret_cast<unsigned>(encoded.data()),unsigned(encoded.size()),reinterpret_cast<unsigned>(palette.data()),w,h};
  unsigned table[40]={};table[32]=reinterpret_cast<unsigned>(mnm_pixel_test_unlock);unsigned iface=reinterpret_cast<unsigned>(table);
  unsigned surface[10]={0,reinterpret_cast<unsigned>(&iface),0,0,w,h,stride*2,0,0,reinterpret_cast<unsigned>(target)};
  unsigned src[10]={0,reinterpret_cast<unsigned>(&iface),0,0,w,h,stride,0,0,reinterpret_cast<unsigned>(source.data()+256)};
  unsigned dst[10]={0,reinterpret_cast<unsigned>(&iface),0,0,w,h,stride,0,0,reinterpret_cast<unsigned>(target)};
  unsigned rect[4]={0,0,w,h};unsigned shape[5]={w,h,cell%3?65535u:cell,cell%3?65534u:cell+3,cell%3?65520u:cell+11};
  global(0x6e1f88,format);global(0x658174,reinterpret_cast<unsigned>(target));global(0x6a2dc8,stride);global(0x6e0008,0);global(0x6cbb6c,0);global(0x6a49b8,w);global(0x656618,h);
  pixel_function=functions[kind];pixel_object=kind==1?reinterpret_cast<unsigned>(shape):kind==4||kind==6?reinterpret_cast<unsigned>(surface):kind==5?reinterpret_cast<unsigned>(src):reinterpret_cast<unsigned>(pcx);
  pixel_argc=kind==4||kind==6?0:kind==0?1:kind==5?3:2;
  pixel_args[0]=kind==0?reinterpret_cast<unsigned>(surface):kind==1?unsigned(-int(cell%5)):kind==5?reinterpret_cast<unsigned>(dst):reinterpret_cast<unsigned>(target);
  pixel_args[1]=kind==1?unsigned(-int(cell%3)):kind==5?reinterpret_cast<unsigned>(rect):stride;pixel_args[2]=0;
  pixel_seed_flags=0x202|(cell&1?0x8d5:0);
  const bool other=cell && cell%19==0;threadId=1;
  std::memcpy(reinterpret_cast<void*>(sites[kind]),original[kind],lengths[kind]);const auto before=call();const auto expected=pixels;
  pixels=seed;threadId=other?2:1;
  const auto countBefore=kind<6?mnm_pixel_native[kind]:mnm_fade_native;
  const auto forwardBefore=kind<6?mnm_pixel_forwarded[kind]:mnm_fade_forwarded;
  std::memcpy(reinterpret_cast<void*>(sites[kind]),patched[kind],lengths[kind]);const auto after=call();
  if(expected!=pixels)throw std::runtime_error("Pixel mismatch kind="+std::to_string(kind)+" cell="+std::to_string(cell));
  if(before.gpr!=after.gpr || ((before.flags^after.flags)&0xcd5) || before.unlocks!=after.unlocks || before.args!=after.args || before.unlockGpr!=after.unlockGpr || before.error!=after.error)
   {for(unsigned i=0;i<8;++i)if(before.gpr[i]!=after.gpr[i])std::cerr<<"gpr"<<i<<" "<<std::hex<<before.gpr[i]<<" != "<<after.gpr[i]<<"\n";
    for(unsigned i=0;i<3;++i)if(before.unlockGpr[i]!=after.unlockGpr[i])std::cerr<<"unlock"<<i<<" "<<std::hex<<before.unlockGpr[i]<<" != "<<after.unlockGpr[i]<<"\n";
    throw std::runtime_error("Caller mismatch kind="+std::to_string(kind)+" cell="+std::to_string(cell));}
  /* x87 code pointer differs only when the original preparation/helper itself
   * executes; its arithmetic/status/physical register and SSE payload agree. */
  for(unsigned i=0;i<512;++i)if(!(i>=6 && i<24) && before.fx[i]!=after.fx[i])throw std::runtime_error("FX mismatch");
  const auto countAfter=kind<6?mnm_pixel_native[kind]:mnm_fade_native,forwardAfter=kind<6?mnm_pixel_forwarded[kind]:mnm_fade_forwarded;
  if(other && forwardAfter!=forwardBefore+1 && !(kind==1 && expected==seed && forwardAfter==forwardBefore))
   throw std::runtime_error("Other-thread call did not forward exactly once kind="+std::to_string(kind)+" cell="+std::to_string(cell));
  if(countAfter==countBefore && !other && kind!=1)throw std::runtime_error("Native route unexpectedly refused");
  if(countAfter!=countBefore)++cases;else ++forwards;words+=physical;
 }
 std::ofstream out(argv[2]);out<<"{\"success\":true,\"native_cases\":"<<cases<<",\"forward_cases\":"<<forwards<<",\"physical_words\":"<<words<<",\"producer_routes\":7,\"live_replacement\":false}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
#endif
