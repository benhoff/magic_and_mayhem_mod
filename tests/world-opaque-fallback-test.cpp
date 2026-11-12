// Whole original wrapper versus the production clipped pixel kernel. Only the
// selected callee entry is redirected in the private mapped fixture image.
#define main original_reference_unused_main
#define u32 original_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "../reconstruction/rendering/raster_pixels.h"
#include <array>
using u32=std::uint32_t;using u8=std::uint8_t;
#include "../runtime/scene/world_raster_callers.h"
struct Probe {u32 eax,ebx,esi,edi,stack;};
extern "C" void opaque_probe(void*,int,int,int,unsigned,Probe*);
asm(".text\n.globl opaque_probe\nopaque_probe:\n"
    "push %ebp\nmov %esp,%ebp\npush %ebx\npush %esi\npush %edi\n"
    "push 24(%ebp)\npush 20(%ebp)\npush 16(%ebp)\n"
    "mov 8(%ebp),%ecx\nmov 12(%ebp),%edx\n"
    "mov $0x12345678,%ebx\nmov $0x87654321,%esi\nmov $0x5a5a5a5a,%edi\n"
    "mov $0x57e8e0,%eax\ncall *%eax\n"
    "mov 28(%ebp),%edx\nmov %eax,0(%edx)\nmov %ebx,4(%edx)\n"
    "mov %esi,8(%edx)\nmov %edi,12(%edx)\n"
    "lea -12(%ebp),%eax\nsub %esp,%eax\nmov %eax,16(%edx)\n"
    "lea -12(%ebp),%esp\npop %edi\npop %esi\npop %ebx\npop %ebp\nret\n");
static u32 load(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(Bytes& b,unsigned at,u32 n){std::memcpy(b.data()+at,&n,4);}
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static unsigned rejected,executed,wrong_args;
static std::string current_case;
static void* expected_frame;static int expected_x,expected_y,expected_shade;static unsigned expected_palette;
extern "C" unsigned native_clipped(void* raw,int x,int y,int shade,unsigned ordinal){
 const auto pc=u32(reinterpret_cast<std::uintptr_t>(__builtin_return_address(0)));
 if(pc!=0x57e957){++wrong_args;return 0xdead0001;}
 if(!mnm_world_raster_caller(pc,17,reinterpret_cast<const u8*>(pc))){++rejected;return 0xdead0001;}
 ++executed;
 if(raw!=expected_frame||x!=expected_x||y!=expected_y||shade!=expected_shade||ordinal!=expected_palette){++wrong_args;return 0xdead0001;}
 auto* f=static_cast<const u8*>(raw);auto node=load(f+28);
 for(unsigned i=0;i<ordinal;++i)node=load(reinterpret_cast<void*>(node));
 const auto shift=load(reinterpret_cast<void*>(node+4)),neutral=load(reinterpret_cast<void*>(node+8));
 int q=shade>>shift;if(q<0&&shift)++q;
 const auto* table=reinterpret_cast<const u8*>(node+12+(neutral+q)*512);
 MnmPixelPlane p{reinterpret_cast<std::uint16_t*>(load(reinterpret_cast<void*>(0x658174))),16,12,20,0};
 MnmRasterPixels r{};r.frame=f;r.bytes=load(f);r.indexed=1;r.palette_step=2;r.palette=table;r.contiguous=1;r.period=16;
 r.left=x-int(load(f+12));r.top=y-int(load(f+16));
 r.clip_left=int(load(reinterpret_cast<void*>(0x6e0008)));r.clip_top=int(load(reinterpret_cast<void*>(0x6cbb6c)));
 r.clip_right=int(load(reinterpret_cast<void*>(0x6a49b8)));r.clip_bottom=int(load(reinterpret_cast<void*>(0x656618)));
 if(!mnm_raster_pixels(&p,&r)){++wrong_args;return 0xdead0001;}
 // The caller must mask these deliberately nonzero high bits.
 return 0xa5a50000;
}
int main(int argc,char** argv)try{
 check(argc==3,"Expected original image and expect-rejected/expect-fixed");
 const bool baseline=std::string(argv[2])=="expect-rejected";
 check(baseline||std::string(argv[2])=="expect-fixed","Mode");
 map_image(read(argv[1]));
 const std::array<u8,6> entry{0x55,0x8b,0xec,0x83,0xec,0x18};
 auto* target=reinterpret_cast<u8*>(0x57de00);check(!std::memcmp(target,entry.data(),6),"Original clipped entry");
 const std::array<u8,10> continuation{0x83,0xc4,0x14,0x8b,0x75,0x00,0x25,0xff,0xff,0x00};
 check(!std::memcmp(reinterpret_cast<void*>(0x57e957),continuation.data(),10),"Exact primary continuation");
 const Bytes wrapper(reinterpret_cast<u8*>(0x57e8e0),reinterpret_cast<u8*>(0x57ea58));
 const auto original_jump=load(reinterpret_cast<void*>(0x57e953));check(0x57e957+original_jump==0x57de00,"Exact direct target");
 std::array<std::vector<u32>,2> palettes;
 for(unsigned bank=0;bank<2;++bank){palettes[bank].resize(3+255*128);palettes[bank][1]=0;palettes[bank][2]=127;
  for(unsigned t=0;t<255;++t)for(unsigned i=0;i<256;++i){const std::uint16_t word=t&&i?std::uint16_t(t*257+i*97+bank*13):0;
   std::memcpy(reinterpret_cast<u8*>(palettes[bank].data()+3)+t*512+i*2,&word,2);}}
 palettes[0][0]=u32(reinterpret_cast<std::uintptr_t>(palettes[1].data()));
 const int xs[]={-9,-2,0,1,2,5,9,13,15,16,22},ys[]={-6,-1,0,1,4,9,11,12,19};
 const int origins[]={-3,0,2},shades[]={-127,0,127};
 const int clips[][4]={{0,0,16,12},{2,1,14,10}};
 unsigned cases=0,refused_cases=0,fallback_cases=0,inside_cases=0;std::uint64_t pixels=0;
 for(u32 fast:{0x5947b2u,0x59521au})for(int ox:origins)for(int oy:origins)for(const auto& clip:clips)
 for(int x:xs)for(int y:ys)for(int shade:shades)for(unsigned ordinal=0;ordinal<2;++ordinal){
  current_case="fast="+std::to_string(fast)+" origin="+std::to_string(ox)+","+std::to_string(oy)+" xy="+std::to_string(x)+","+std::to_string(y)+" shade="+std::to_string(shade)+" palette="+std::to_string(ordinal)+" clip="+std::to_string(clip[0])+","+std::to_string(clip[1])+","+std::to_string(clip[2])+","+std::to_string(clip[3]);
  Bytes frames(108+3*40,0);put(frames,0,108);put(frames,4,7);put(frames,8,4);put(frames,12,u32(ox));put(frames,16,u32(oy));
  const u32 ptr=u32(reinterpret_cast<std::uintptr_t>(palettes[0].data()));put(frames,28,ptr);
  for(unsigned row=0;row<4;++row){put(frames,40+row*8,72+row*4);put(frames,44+row*8,88+row*5);
   const u8 control[]={1,3,1,2};std::memcpy(frames.data()+72+row*4,control,4);
   for(unsigned i=0;i<5;++i)frames[88+row*5+i]=u8((row*5+i)*7);}
  for(unsigned aux=0;aux<3;++aux){put(frames,108+aux*40,40);put(frames,108+aux*40+28,ptr);}
  const auto frame_before=frames;
  std::vector<std::uint16_t> original(32+20*12+32),native;
  for(unsigned i=0;i<original.size();++i)original[i]=std::uint16_t(i*39+0x1234);native=original;
  auto setup=[&](std::vector<std::uint16_t>& dest){
   global(0x658174,reinterpret_cast<std::uintptr_t>(dest.data()+32));global(0x6a2dc8,20);global(0x6e1f68,0);global(0x6e1f88,0);global(0x6de6e1,1);
   global(0x6c4e3c,fast);for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(clip[i]));};
  Probe a{},b{};setup(original);std::memcpy(target,entry.data(),entry.size());
  opaque_probe(frames.data(),x,y,shade,ordinal,&a);
  check(frames==frame_before,"Original modified borrowed source");
  setup(native);target[0]=0xe9;const auto displacement=u32(reinterpret_cast<std::uintptr_t>(&native_clipped))-0x57de05;
  std::memcpy(target+1,&displacement,4);target[5]=0xcc;
  expected_frame=frames.data();expected_x=x;expected_y=y;expected_shade=shade;expected_palette=ordinal;
  const auto old_rejected=rejected,old_executed=executed;
  opaque_probe(frames.data(),x,y,shade,ordinal,&b);
  check(a.stack==0&&b.stack==0,"Wrapper RET12/cdecl20 cleanup");
  for(const auto& q:{a,b})check(q.ebx==0x12345678&&q.esi==0x87654321&&q.edi==0x5a5a5a5a,"Wrapper nonvolatile state");
  check(!wrong_args,"Caller argument forwarding or native kernel admission");
  if(rejected!=old_rejected){check(baseline,"Missing producer continuation");++refused_cases;}
  else {check(original==native,"Whole destination/guard/stride mismatch");check((a.eax&65535)==(b.eax&65535),"Wrapper low16AX mismatch");pixels+=original.size();}
  if(executed!=old_executed)++fallback_cases;else if(rejected==old_rejected)++inside_cases;
  check(frames==frame_before,"Native modified borrowed source");
  check(!std::memcmp(wrapper.data(),reinterpret_cast<void*>(0x57e8e0),wrapper.size()),"Original wrapper changed");
  ++cases;
 }
 check(cases==21384&&inside_cases&&((baseline&&refused_cases&&!executed)||(!baseline&&!rejected&&fallback_cases)),"Incomplete guard reproduction/domain");
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"guard_rejected_cases\":"<<refused_cases<<",\"native_fallback_cases\":"<<fallback_cases<<",\"original_fast_or_hidden_cases\":"<<inside_cases<<",\"compared_words_including_canaries\":"<<pixels<<",\"wrapper_bytes_unchanged\":true,\"callee_entry_redirected\":true,\"original_game_executed\":false,\"scope\":\"Primary indexed/black fallback only, blend-tail enabled with empty auxiliary frames; no secondary caller or full volatile ABI claim\"}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n'<<current_case<<'\n';return 1;}
