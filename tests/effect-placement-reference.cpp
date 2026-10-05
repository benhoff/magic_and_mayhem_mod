#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif
#include "effect_placement.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <stdexcept>
using namespace mnm::reconstruction;
static std::uint32_t randomWord(){static std::uint32_t state=0x493f20;state=state*1664525u+1013904223u;return state;}
static unsigned dword(const unsigned char* p,unsigned offset){unsigned n;std::memcpy(&n,p+offset,4);return n;}
static void putWord(unsigned char* p,unsigned offset,unsigned n){std::memcpy(p+offset,&n,4);}
#define require(v) do{if(!(v))throw std::runtime_error("Effect placement mismatch at line "+std::to_string(__LINE__));}while(false)
#ifdef ORIGINAL_REFERENCE
class PrivateSeh {
 unsigned short saved_=0;std::uint32_t head_=0xffffffffu;
public:
 PrivateSeh(){
  asm volatile("mov %%fs,%0":"=r"(saved_));user_desc segment{};segment.entry_number=unsigned(-1);
  segment.base_addr=reinterpret_cast<std::uintptr_t>(&head_);segment.limit=3;segment.seg_32bit=1;segment.useable=1;
  if(syscall(SYS_set_thread_area,&segment))throw std::runtime_error("Private exception head failed");
  const unsigned short selector=(segment.entry_number<<3)|3;asm volatile("mov %0,%%fs"::"r"(selector):"memory");
 }
 void check(){require(head_==0xffffffffu);}
 ~PrivateSeh(){asm volatile("mov %0,%%fs"::"r"(saved_):"memory");}
};
static unsigned dispatches=0;
static void __attribute__((thiscall)) deferredType(void* object,unsigned type){
 auto* p=static_cast<unsigned char*>(object);require(dword(p,4)==1);
 putWord(p,0x28,type);++dispatches;
}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
 if(argc!=3)throw std::runtime_error("Expected PE output");map_image(read(argv[1]));
 const unsigned char expected[]={0x64,0xa1,0,0,0,0,0x6a,0xff};
 require(!std::memcmp(reinterpret_cast<void*>(0x493f20),expected,sizeof(expected)));
 const unsigned char dispatchBytes[]={0x8b,0x44,0x24,0x04,0x81};
 require(!std::memcmp(reinterpret_cast<void*>(0x494ab0),dispatchBytes,sizeof(dispatchBytes)));
 // Only the explicitly deferred type setup is redirected, in private memory.
 auto* entry=reinterpret_cast<unsigned char*>(0x494ab0);entry[0]=0xe9;
 const std::uint32_t relative=reinterpret_cast<std::uintptr_t>(deferredType)-0x494ab5;std::memcpy(entry+1,&relative,4);
 global(0x5fed7c,0);PrivateSeh seh;
 using Place=void(__attribute__((thiscall)) *)(void*,int,const std::uint32_t*);
 const int outArg=2;
#else
 if(argc!=2)throw std::runtime_error("Expected output");const int outArg=1;
#endif
 std::ofstream output(argv[outArg],std::ios::binary);unsigned placements=0,linked=0,skipped=0,cellChecks=0;
 const std::array<unsigned,12> sentinelOffsets{0x1aa,0x1ae,0x1b2,0x1b6,0x1ba,0x1be,0x1c2,0x1c6,0x1ca,0x212,0x216,0x21a};
 for(unsigned fixture=0;fixture<128;++fixture){
  const unsigned w=fixture==127?128:8u<<(fixture%3),h=fixture==127?128:8u<<((fixture/3)%3),layers=fixture==127?32:std::array<unsigned,4>{1,3,8,32}[(fixture/9)%4];
  EffectPlacementPool pool(w,h,layers,16);pool.setScan(fixture%17);
  for(unsigned i=0;i<pool.cells().size();++i){auto& c=pool.cells()[i];c.flags=0x12380u+((i+fixture)%5==0?0x40000000u:0u);c.terrain=i%4;}
#ifdef ORIGINAL_REFERENCE
  Bytes records(16+16*0x22e + 16,0xa5),cells(16+pool.cells().size()*12+16,0xa5),catalog(4*356,0);
  auto* recordBase=records.data()+16;auto* cellBase=cells.data()+16;
  for(unsigned i=0;i<16;++i){auto* p=recordBase+i*0x22e;putWord(p,0,i);putWord(p,4,0);putWord(p,0x24,0x689498);const std::uint16_t next=NoEffect;std::memcpy(p+0x1a0,&next,2);}
  for(unsigned i=0;i<pool.cells().size();++i){const auto& c=pool.cells()[i];std::memcpy(cellBase+i*12,&c.terrain,2);std::memcpy(cellBase+i*12+2,&c.head,2);putWord(cellBase+i*12,8,c.flags);}
  global(0x689498,reinterpret_cast<std::uintptr_t>(recordBase));global(0x68949c,16);global(0x6898dc,pool.scan());
  global(0x6c5494,w);global(0x6c5498,h);global(0x6c549c,layers);global(0x6c54dc,reinterpret_cast<std::uintptr_t>(cellBase));global(0x65660c,reinterpret_cast<std::uintptr_t>(catalog.data()));
  for(unsigned y=0;y<h;++y)global(0x6cb942+y*4,y*w);
  for(unsigned z=0;z<layers;++z)global(0x6cb8c2+z*4,z*w*h);
#endif
  for(unsigned step=0;step<16;++step){
   const unsigned slot=(step*7)%16,type=std::array<unsigned,5>{3,13,22,24,36}[(step+fixture)%5];
   std::array<std::uint32_t,63> params;for(auto& n:params)n=randomWord();params[6]=0xffffffffu;
   // Shared start cell exercises append chains, wrapping and subcell precision.
   const int x=int(w*16+step),y=int(h*16+step),periodX=int(w*32),periodY=int(h*32);
   params[0]=std::uint32_t(x+(int(step%3)-1)*periodX);params[1]=std::uint32_t(y+(int(step%3)-1)*periodY);
   params[2]=step%4==0?0x7fffffffu:layers*16-1;
   params[3]=randomWord()%(3*w*32)-w*32;params[4]=randomWord()%(3*h*32)-h*32;params[5]=randomWord()&0x7fffffff;
   pool.place(slot,type,params);const auto& r=pool.records()[slot];const auto& cell=pool.cells()[r.cell];
   if(cell.flags&0x40000000u)++skipped;else ++linked;
#ifdef ORIGINAL_REFERENCE
   auto* p=recordBase+slot*0x22e;reinterpret_cast<Place>(0x493f20)(p,type,params.data());seh.check();
   require(dispatches==placements+1 && dword(p,4)==1 && dword(p,0x28)==type && dword(p,0x22a)==1);
   require(!std::memcmp(p+0x2c,r.parameters.data(),252));
   for(unsigned k=0;k<3;++k){require(dword(p,8+4*k)==r.position[k]);require(dword(p,0x14+4*k)==r.units[k]);require(dword(p,0x1f6+4*k)==r.initialPosition[k] && dword(p,0x202+4*k)==r.initialPosition[k]);}
   for(unsigned k=0;k<12;++k)require(dword(p,sentinelOffsets[k])==r.sentinels[k]);
   for(unsigned offset:{0x198u,0x19cu,0x20eu,0x21eu,0x222u})require(dword(p,offset)==0);
   require(dword(p,0x194)==reinterpret_cast<std::uintptr_t>(cellBase+r.cell*12));
   require(dword(p,0x190)==reinterpret_cast<std::uintptr_t>(catalog.data()+cell.terrain*356));
   require(*reinterpret_cast<unsigned*>(0x6898dc)==pool.scan());
   for(unsigned i=0;i<16;++i){std::uint16_t next;std::memcpy(&next,recordBase+i*0x22e + 0x1a0,2);require(next==pool.records()[i].next);}
   for(unsigned i=0;i<pool.cells().size();++i){std::uint16_t head,terrain;std::memcpy(&head,cellBase+i*12+2,2);std::memcpy(&terrain,cellBase+i*12,2);require(head==pool.cells()[i].head && terrain==pool.cells()[i].terrain && dword(cellBase+i*12,8)==pool.cells()[i].flags);}
   for(const auto* b:{&records,&cells})for(unsigned i=0;i<16;++i)require((*b)[i]==0xa5 && (*b)[b->size()-16+i]==0xa5);
#endif
   for(const auto* words:{&r.parameters})output.write(reinterpret_cast<const char*>(words->data()),words->size()*4);
   for(const auto* words:{&r.position,&r.units,&r.initialPosition})output.write(reinterpret_cast<const char*>(words->data()),words->size()*4);
   output.write(reinterpret_cast<const char*>(r.sentinels.data()),r.sentinels.size()*4);
   for(unsigned n:{r.type,unsigned(r.active),unsigned(r.initialized),r.cell,pool.scan()})output.write(reinterpret_cast<const char*>(&n),4);
   cellChecks+=pool.cells().size();++placements;
  }
  for(const auto& r:pool.records())output.write(reinterpret_cast<const char*>(&r.next),2);
  for(const auto& c:pool.cells()){output.write(reinterpret_cast<const char*>(&c.head),2);output.write(reinterpret_cast<const char*>(&c.flags),4);}
 }
 if(!output)throw std::runtime_error("Output failure");
 std::cout<<"{\"fixtures\":128,\"placements\":"<<placements<<",\"linked\":"<<linked<<",\"skipped\":"<<skipped<<",\"cell_checks\":"<<cellChecks<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
