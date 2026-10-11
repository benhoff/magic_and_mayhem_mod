#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "../runtime/canvas-writers/fade_route.h"
#include <array>
extern "C" {
alignas(16) unsigned char mnm_fade_host_fx[512],mnm_fade_seed_fx[512],mnm_fade_after_fx[512],dirty_fx[512];
std::uint32_t mnm_fade_saved_sp,mnm_fade_seed_flags,mnm_fade_after_flags,mnm_fade_function=0x58ed80;
std::uint32_t mnm_fade_gpr[8],mnm_fade_unlock_gpr[3],mnm_fade_unlock_args[2],mnm_fade_unlock_flags;
std::uint32_t mnm_fade_unlocks,mnm_fade_original_calls,mnm_fade_trampoline;
std::uintptr_t mnm_fade_tail;
static unsigned finishes;
void mnm_fade_probe(void *);
void mnm_fade_test_unlock();
void mnm_fade_test_original();
void mnm_fade_enter();
void mnm_fade_finish_entry();
void mnm_fade_finish() { ++finishes;asm volatile("fxrstor %0"::"m"(dirty_fx)); }
}
static unsigned mode;
static MnmFadePlane lease;
static int resolve(const std::uint32_t *,MnmFadePlane *p) {
  *p=lease;
  // Simulate arbitrary resolver FP/SSE scratch use. The entry must restore it.
  asm volatile("fxrstor %0"::"m"(dirty_fx));
  if(mode==2)p->width=2049;
  if(mode==3)p->format=!p->format;
  return mode!=1;
}
static void jump(unsigned char *at,const void *target,unsigned length) {
  at[0]=0xe9;auto delta=std::uint32_t(reinterpret_cast<std::uintptr_t>(target)-reinterpret_cast<std::uintptr_t>(at)-5);
  std::memcpy(at+1,&delta,4);for(unsigned i=5;i<length;++i)at[i]=0x90;
}
struct State {
  std::array<std::uint32_t,8> gpr;
  std::array<std::uint32_t,3> unlockGpr;
  std::array<std::uint32_t,2> args;
  std::array<unsigned char,512> fx;
  unsigned flags,unlockFlags,unlocks;
};
static State call(void *object) {
  mnm_fade_unlocks=0;mnm_fade_probe(object);State s;
  std::memcpy(s.gpr.data(),mnm_fade_gpr,sizeof(mnm_fade_gpr));
  std::memcpy(s.unlockGpr.data(),mnm_fade_unlock_gpr,sizeof(mnm_fade_unlock_gpr));
  std::memcpy(s.args.data(),mnm_fade_unlock_args,sizeof(mnm_fade_unlock_args));
  std::memcpy(s.fx.data(),mnm_fade_after_fx,512);
  s.flags=mnm_fade_after_flags;s.unlockFlags=mnm_fade_unlock_flags;s.unlocks=mnm_fade_unlocks;return s;
}
int main(int argc,char **argv) try {
  if(argc!=4)throw std::runtime_error("Expected pinned PE, fade corpus, output");
  map_image(read(argv[1]));const auto b=read(argv[2]);
  if(b.size()<12 || std::memcmp(b.data(),"MNMFADE1",8))throw std::runtime_error("Fade corpus header");
  auto *entry=reinterpret_cast<unsigned char *>(0x58edb0),*lock=reinterpret_cast<unsigned char *>(0x58b660);
  const unsigned char expected[]={0x53,0x8b,0x47,0x10,0x33,0xdb};
  if(std::memcmp(entry,expected,6) || std::memcmp(lock,"\x83\xec\x08\x33\xc0",5))throw std::runtime_error("Entry anchors changed");
  auto *trampoline=static_cast<unsigned char *>(mmap(nullptr,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
  if(trampoline==MAP_FAILED)throw std::runtime_error("Trampoline allocation");
  // The outer back-edge targets 58edb1, inside the entry patch. Copy the whole
  // self-contained loop so refusal never jumps into an overwritten operand.
  std::memcpy(trampoline,entry,53);jump(trampoline+53,entry+53,5);
  mnm_fade_trampoline=reinterpret_cast<std::uintptr_t>(trampoline);mnm_fade_original=reinterpret_cast<std::uintptr_t>(mnm_fade_test_original);mnm_fade_resume=0x58ede5;
  lock[0]=0xb8;lock[5]=0xc3;
  std::uint32_t table[33]={},iface[1]={},surface[12]={};
  table[32]=reinterpret_cast<std::uintptr_t>(mnm_fade_test_unlock);iface[0]=reinterpret_cast<std::uintptr_t>(table);surface[1]=reinterpret_cast<std::uintptr_t>(iface);
  asm volatile("fxsave %0":"=m"(mnm_fade_host_fx));
  asm volatile("fninit; fld1; fldpi; fxsave %0":"=m"(mnm_fade_seed_fx));
  asm volatile("fninit; fldz; fxsave %0":"=m"(dirty_fx));
  asm volatile("fxrstor %0"::"m"(mnm_fade_host_fx));
  // XMM registers are defined architectural state even for an integer loop.
  for(unsigned i=160;i<288;++i){mnm_fade_seed_fx[i]=static_cast<unsigned char>(i*37);dirty_fx[i]=static_cast<unsigned char>(i*13);}
  std::ofstream out(argv[3],std::ios::binary);out.write("MNMFDLP1",8);std::size_t at=12;
  for(unsigned c=0;c<u32(b,8);++c){
    const auto w=u32(b,at),h=u32(b,at+4),stride=u32(b,at+8),format=u32(b,at+12);at+=16;
    if(!w||!h||w>2048||h>2048||stride<w||stride>4096||at+stride*h*2>b.size())throw std::runtime_error("Fade extent");
    std::vector<std::uint16_t> plane(stride*h+64,0xb37d);const auto words=stride*h;
    auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(plane.data()+32));std::memcpy(lock+1,&pointer,4);
    surface[4]=w;surface[5]=h;surface[6]=stride*2;global(0x6e1f88,format);lease={plane.data()+32,w,h,stride,format};
    for(unsigned control=0;control<4;++control)for(mode=0;mode<5;++mode){
      mnm_fade_seed_flags=0x202u|((control&1)?0x400:0)|0x895u;
      mnm_fade_tail=0x58ede5;mnm_fade_resume=control&2?reinterpret_cast<std::uintptr_t>(mnm_fade_finish_entry):0x58ede5;
      std::memcpy(plane.data()+32,b.data()+at,words*2);std::memcpy(entry,expected,6);const auto original=call(surface);const auto originalWords=plane;
      std::memcpy(plane.data()+32,b.data()+at,words*2);mnm_fade_native=mnm_fade_forwarded=mnm_fade_original_calls=finishes=0;mnm_fade_resolve=mode==4?nullptr:resolve;
      jump(entry,reinterpret_cast<void *>(mnm_fade_enter),6);const auto native=call(surface);
      unsigned mask=0;
      if(plane==originalWords)mask|=1;
      if(original.gpr==native.gpr)mask|=2;
      if(((original.flags^native.flags)&0xcd5u)==0)mask|=4;
      if(original.fx==native.fx)mask|=8;
      if(original.unlockGpr==native.unlockGpr && original.args==native.args && ((original.unlockFlags^native.unlockFlags)&0xcd5u)==0)mask|=16;
      if(original.unlocks==1 && native.unlocks==1)mask|=32;
      bool guards=true;for(unsigned i=0;i<32;++i)guards&=plane[i]==0xb37d&&plane[words+32+i]==0xb37d;
      if(guards)mask|=64;
      if(mnm_fade_native==(mode==0) && mnm_fade_forwarded==(mode!=0) && mnm_fade_original_calls==(mode!=0) && finishes==unsigned(mode==0 && (control&2)))mask|=128;
      const std::uint32_t row[]={c,mode,control,mask,words,mnm_fade_native,mnm_fade_forwarded,mnm_fade_original_calls};out.write(reinterpret_cast<const char *>(row),sizeof(row));
      if(mask!=255)std::cerr<<"Mismatch "<<c<<' '<<mode<<' '<<control<<" mask "<<mask<<'\n';
    }
    at+=words*2;
  }
  if(at!=b.size() || !out)throw std::runtime_error("Input/output extent");
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
