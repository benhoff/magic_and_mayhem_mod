// Execute the unchanged original two-frame wrapper. Redirect only its existing
// pixel callees in a private mapped PE image for the native comparison.
#define main original_reference_unused_main
#define u32 original_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "../reconstruction/rendering/raster_pixels.h"
#include "../reconstruction/rendering/world_special.hpp"
#include <array>
#include <map>
using u32=std::uint32_t;using u8=std::uint8_t;
#include "../runtime/scene/world_raster_callers.h"
struct Probe {u32 eax,ebx,esi,edi,stack;};
extern "C" void secondary_probe(void*,int,int,int,unsigned,Probe*);
asm(".text\n.globl secondary_probe\nsecondary_probe:\n"
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
static std::string current_case;
static u32 raw_tag,indexed_tag;
static void* frames_expected[2];static int x_expected,y_expected,shade_expected;static unsigned ordinal_expected;
static unsigned refusals,wrong_args;static std::map<std::pair<u32,u32>,unsigned> attempted,admitted;
static bool scripted;static u32 scripted_values[2][4];static std::vector<unsigned> trace;
static unsigned call_native(u32 pc,u32 tag,void* raw,int x,int y,int shade,unsigned ordinal){
 unsigned which=raw==frames_expected[0]?0:1;
 if((raw!=frames_expected[0]&&raw!=frames_expected[1])||x!=x_expected||y!=y_expected||
    ((tag==13||tag==14||tag==17)&&(shade!=shade_expected||ordinal!=ordinal_expected))){++wrong_args;return 0;}
 const u32 expected=which?(tag==12?0x57ea02:tag==17?0x57ea40:tag==22||tag==36?0x57e9ec:0x57ea29):
                           (tag==12?0x57e915:tag==17?0x57e957:tag==22||tag==36?0x57e8fb:0x57e940);
 if(pc!=expected){++wrong_args;return 0;}
 ++attempted[{pc,tag}];
 if(!mnm_world_raster_caller(pc,tag,reinterpret_cast<const u8*>(pc))){++refusals;return 0xdead0001;}
 ++admitted[{pc,tag}];const unsigned operation=tag==22||tag==36?0:tag==12?1:tag==17?3:2;
 if(scripted){trace.push_back(which*4+operation);return scripted_values[which][operation];}
 const auto* f=static_cast<const u8*>(raw);const auto width=load(f+4),height=load(f+8);
 if(!width||!height)return 0xa5a50000;
 const int left=x-int(load(f+12)),top=y-int(load(f+16));
 const int cl=int(load(reinterpret_cast<void*>(0x6e0008))),cr=int(load(reinterpret_cast<void*>(0x6a49b8)));
 if((tag==12||tag==13||tag==14)&&(left<cl||std::int64_t(left)+width>=cr))return 0xa5a50001;
 MnmPixelPlane p{reinterpret_cast<std::uint16_t*>(load(reinterpret_cast<void*>(0x658174))),16,12,20,0};
 MnmRasterPixels r{};r.frame=f;r.bytes=load(f);r.indexed=load(f+28)!=0xffffffffu;r.palette_step=2;r.period=16;
 r.left=left;r.top=top;r.clip_left=cl;r.clip_right=cr;
 r.clip_top=int(load(reinterpret_cast<void*>(0x6cbb6c)));r.clip_bottom=int(load(reinterpret_cast<void*>(0x656618)));
 r.contiguous=tag==12?0:1;r.mode=tag==12?7:0;
 if(r.indexed&&tag!=12){auto node=load(f+28);for(unsigned i=0;i<ordinal;++i)node=load(reinterpret_cast<void*>(node));
  const auto shift=load(reinterpret_cast<void*>(node+4)),neutral=load(reinterpret_cast<void*>(node+8));
  int q=shade>>shift;if(q<0&&shift)++q;r.palette=reinterpret_cast<const u8*>(node+12+(neutral+q)*512);}
 if(!mnm_raster_pixels(&p,&r)){++wrong_args;return 0;}
 return 0xa5a50000;
}
extern "C" unsigned native_raw(void* f,int x,int y){return call_native(u32(reinterpret_cast<std::uintptr_t>(__builtin_return_address(0))),raw_tag,f,x,y,0,0);}
extern "C" unsigned native_black(void* f,int x,int y){return call_native(u32(reinterpret_cast<std::uintptr_t>(__builtin_return_address(0))),12,f,x,y,0,0);}
extern "C" unsigned native_indexed(void* f,int x,int y,int s,unsigned o){return call_native(u32(reinterpret_cast<std::uintptr_t>(__builtin_return_address(0))),indexed_tag,f,x,y,s,o);}
extern "C" unsigned native_clipped(void* f,int x,int y,int s,unsigned o){return call_native(u32(reinterpret_cast<std::uintptr_t>(__builtin_return_address(0))),17,f,x,y,s,o);}
struct Redirect {u32 entry;std::array<u8,6> bytes;std::uintptr_t native;};
static void redirect(std::vector<Redirect>& sites,bool native){for(const auto& s:sites){auto* p=reinterpret_cast<u8*>(s.entry);
 if(!native)std::memcpy(p,s.bytes.data(),6);else {p[0]=0xe9;const u32 d=u32(s.native)-s.entry-5;std::memcpy(p+1,&d,4);p[5]=0xcc;}}}
static void probe_contract(const Probe& q){check(q.stack==0,"Wrapper RET12/cdecl cleanup");check(q.ebx==0x12345678&&q.esi==0x87654321&&q.edi==0x5a5a5a5a,"Nonvolatile registers");}
static Bytes frame(bool words,int ox,int oy,bool empty,u32 palette,unsigned variant){
 Bytes f(words?128:108,0);put(f,0,f.size());put(f,4,empty?0:7);put(f,8,4);put(f,12,u32(ox));put(f,16,u32(oy));put(f,28,words?0xffffffffu:palette);
 for(unsigned y=0;y<4;++y){put(f,40+y*8,72+y*4);put(f,44+y*8,88+y*5*(words?2:1));
  const u8 controls[]={1,3,1,2};std::memcpy(f.data()+72+y*4,controls,4);
  for(unsigned i=0;i<5;++i){if(words){const std::uint16_t v=std::uint16_t((y*5+i)*997+variant*173);std::memcpy(f.data()+88+(y*5+i)*2,&v,2);}else f[88+y*5+i]=u8((y*5+i)*7+variant);}}
 return f;
}
int main(int argc,char** argv)try{
 check(argc==3,"Expected original image and mode");const bool baseline=std::string(argv[2])=="expect-rejected";
 check(baseline||std::string(argv[2])=="expect-fixed","Mode");map_image(read(argv[1]));
 const Bytes wrapper(reinterpret_cast<u8*>(0x57e8e0),reinterpret_cast<u8*>(0x57ea58));
 std::vector<Redirect> redirects;
 for(auto binding:std::initializer_list<std::pair<u32,std::uintptr_t>>{{0x597086,(std::uintptr_t)&native_raw},{0x596cb8,(std::uintptr_t)&native_raw},{0x595677,(std::uintptr_t)&native_black},{0x5947b2,(std::uintptr_t)&native_indexed},{0x59521a,(std::uintptr_t)&native_indexed},{0x57de00,(std::uintptr_t)&native_clipped}}){Redirect r{binding.first,{},binding.second};std::memcpy(r.bytes.data(),reinterpret_cast<void*>(r.entry),6);redirects.push_back(r);}
 std::array<std::vector<u32>,2> palettes;
 for(unsigned bank=0;bank<2;++bank){palettes[bank].resize(3+255*128);palettes[bank][1]=0;palettes[bank][2]=127;
  for(unsigned t=0;t<255;++t)for(unsigned i=0;i<256;++i){const std::uint16_t word=t&&i?std::uint16_t(t*257+i*97+bank*13):0;std::memcpy(reinterpret_cast<u8*>(palettes[bank].data()+3)+t*512+i*2,&word,2);}}
 palettes[0][0]=u32(reinterpret_cast<std::uintptr_t>(palettes[1].data()));const u32 palette=u32(reinterpret_cast<std::uintptr_t>(palettes[0].data()));
 const int xs[]={-9,-2,0,2,5,9,13,16,22},ys[]={-6,-1,0,4,9,12,19},origins[]={-3,0,2},shades[]={-127,0,127};
 const int clips[][4]={{0,0,16,12},{2,1,14,10}};
 unsigned cases=0,rejected_cases=0;std::uint64_t words=0;
 for(u32 raw:{0x597086u,0x596cb8u})for(u32 fast:{0x5947b2u,0x59521au})for(unsigned kind=0;kind<4;++kind)
 for(int ox:origins)for(int oy:origins)for(const auto& clip:clips)for(int x:xs)for(int y:ys)for(int shade:shades)for(unsigned ordinal=0;ordinal<2;++ordinal){
  current_case="kind="+std::to_string(kind)+" raw="+std::to_string(raw)+" indexed="+std::to_string(fast)+" xy="+std::to_string(x)+","+std::to_string(y)+" origin="+std::to_string(ox)+","+std::to_string(oy)+" shade="+std::to_string(shade)+" ordinal="+std::to_string(ordinal);
  Bytes frames=frame(kind&1,0,0,false,palette,0),second=frame(kind&2,ox,oy,false,palette,1);const auto delta=frames.size();frames.insert(frames.end(),second.begin(),second.end());
  const auto before=frames;frames_expected[0]=frames.data();frames_expected[1]=frames.data()+delta;x_expected=x;y_expected=y;shade_expected=shade;ordinal_expected=ordinal;
  raw_tag=raw==0x597086?22:36;indexed_tag=fast==0x5947b2?13:14;
  std::vector<std::uint16_t> original(32+20*12+32),native;for(unsigned i=0;i<original.size();++i)original[i]=std::uint16_t(i*39+0x1234);native=original;
  auto setup=[&](std::vector<std::uint16_t>& dest){global(0x658174,reinterpret_cast<std::uintptr_t>(dest.data()+32));global(0x6a2dc8,20);global(0x6e1f68,0);global(0x6e1f88,0);global(0x6de6e1,0);global(0x6903dc,raw);global(0x6c4e3c,fast);for(unsigned i=0;i<4;++i)global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(clip[i]));};
  Probe a{},b{};setup(original);redirect(redirects,false);secondary_probe(frames.data(),x,y,shade,ordinal,&a);
  setup(native);redirect(redirects,true);const auto prior=refusals;secondary_probe(frames.data(),x,y,shade,ordinal,&b);
  probe_contract(a);probe_contract(b);check(!wrong_args,"Arguments or native kernel admission");
  if(refusals!=prior){check(baseline,"Missing second-frame continuation");++rejected_cases;}
  else {check(original==native,"Destination/guard/stride mismatch");check((a.eax&65535)==(b.eax&65535),"Combined low16 AX mismatch");words+=original.size();}
  check(frames==before,"Borrowed source changed");check(!std::memcmp(wrapper.data(),reinterpret_cast<void*>(0x57e8e0),wrapper.size()),"Original wrapper changed");++cases;
 }
 check(cases==108864,"Incomplete pixel domain");
 for(auto pair:std::initializer_list<std::pair<u32,u32>>{{0x57e9ec,22},{0x57e9ec,36},{0x57ea02,12},{0x57ea29,13},{0x57ea29,14},{0x57ea40,17}}){check(attempted[pair]>0,"Untested second-frame pair");check(baseline?admitted[pair]==0:admitted[pair]==attempted[pair],"Incomplete admission result");}
 check(baseline?rejected_cases==cases:!refusals,"Incomplete old/new guard reproduction");
 unsigned ax_cases=0;
 if(!baseline){
  // Execute every second-frame black low16 result with two different high-word
  // poisons. The original wrapper remains unchanged; the recovered model is an
  // independent oracle for call order, AX1 fallbacks and low16 OR.
  scripted=true;redirect(redirects,true);
  for(unsigned kind=0;kind<4;++kind)for(int shade:{-127,0})for(unsigned low=0;low<65536;++low){
   Bytes frames=frame(kind&1,0,0,false,palette,0),second=frame(kind&2,0,0,false,palette,1);const auto delta=frames.size();frames.insert(frames.end(),second.begin(),second.end());
   frames_expected[0]=frames.data();frames_expected[1]=frames.data()+delta;x_expected=5;y_expected=4;shade_expected=shade;ordinal_expected=1;
   for(unsigned j=0;j<4;++j){scripted_values[0][j]=0x5a5a0000u|((low*31337+j*17)&65535);scripted_values[1][j]=0xa5a50000u|((low+j*3)&65535);}
   // Ensure black and indexed AX1 branches each occur as well as all non-one values.
   scripted_values[1][1]=0xa5a50000u|low;scripted_values[1][2]=0xa5a50000u|low;
   std::array<mnm::reconstruction::WorldOpaqueSource,4> sources{{{0,bool(kind&1)},{1,bool(kind&2)},{2,false},{3,false}}};std::vector<unsigned> expected_trace;
   auto oracle=[&](mnm::reconstruction::WorldOpaqueEntry e,unsigned token,int x,int y,int s,unsigned o){check(x==5&&y==4&&s==shade&&o==1,"Model forwarding");const unsigned op=e==mnm::reconstruction::WorldOpaqueEntry::word?0:e==mnm::reconstruction::WorldOpaqueEntry::black?1:e==mnm::reconstruction::WorldOpaqueEntry::indexed?2:3;expected_trace.push_back(token*4+op);return scripted_values[token][op];};
   const auto expected=mnm::reconstruction::drawWorldOpaque(sources,false,5,4,shade,1,oracle);trace.clear();Probe q{};secondary_probe(frames.data(),5,4,shade,1,&q);probe_contract(q);
   check((q.eax&65535)==expected&&trace==expected_trace,"Original/model AX routing, masking or OR");check(!wrong_args&&!refusals,"Scripted guard/forwarding");++ax_cases;
  }
 }
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"guard_rejected_cases\":"<<rejected_cases<<",\"compared_words_including_canaries\":"<<words<<",\"ax_routing_cases\":"<<ax_cases<<",\"wrapper_bytes_unchanged\":true,\"original_game_executed\":false,\"secondary_pairs\":[";
 bool comma=false;for(const auto& [pair,count]:attempted)if(pair.first>=0x57e9d8){if(comma)std::cout<<',';comma=true;std::cout<<"{\"pc\":"<<pair.first<<",\"tag\":"<<pair.second<<",\"attempted\":"<<count<<",\"admitted\":"<<admitted[pair]<<'}';}std::cout<<"]}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n'<<current_case<<'\n';return 1;}
