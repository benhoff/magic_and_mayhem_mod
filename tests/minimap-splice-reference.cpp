static void* mapProc(void*,const char*);
#define MNM_PIXEL_TEST_UTILITIES
#define MNM_PIXEL_TEST_GETPROC mapProc
#include "pixel-splice-reference.cpp"
#include "../runtime/canvas-writers/minimap_route.h"
#include "minimap-fixture.hpp"
#include "minimap-overlays-fixture.hpp"
#include <cstdlib>
extern "C" {
int WIN MinimapWritersInstallForTest();
HANDLE WIN GetProcessHeap(){return nullptr;}
void* WIN HeapAlloc(HANDLE,u32,u32 n){return std::malloc(n);}
int WIN HeapFree(HANDLE,u32,void* p){std::free(p);lastError=987;return 1;}
}
struct Owned {const void* pixels;unsigned width,height,stride;};
static std::vector<Owned> owned;
static int WIN mapPlane(const void* pixels,u32* d,u32 bytes){
 if(bytes!=24)return -1;for(const auto& p:owned)if(p.pixels==pixels){d[0]=1;d[1]=p.width;d[2]=p.height;d[3]=p.stride;d[4]=1;d[5]=*reinterpret_cast<unsigned*>(0x6e1f88);return 0;}return -1;
}
static int WIN currentHistory(const void*,u32,u32,u32,u32,u32){return 0;}
static void WIN countProducer(u32,u32,u32){}
static void* mapProc(void*,const char* name){if(!std::strcmp(name,"NativeDrawCpuHistoryV1"))return reinterpret_cast<void*>(currentHistory);if(!std::strcmp(name,"NativeDrawProducerCountV1"))return reinterpret_cast<void*>(countProducer);if(std::strcmp(name,"NativeDrawPlaneV1"))throw std::runtime_error("Unexpected storage lookup");return reinterpret_cast<void*>(mapPlane);}
static void equal(const State& a,const State& b,const std::string& label){
 if(a.gpr!=b.gpr || ((a.flags^b.flags)&0xcd5) || a.unlocks!=b.unlocks || a.args!=b.args || a.unlockGpr!=b.unlockGpr || a.error!=b.error){
  for(unsigned i=0;i<8;++i)if(a.gpr[i]!=b.gpr[i])std::cerr<<label<<" gpr"<<i<<" "<<std::hex<<a.gpr[i]<<" != "<<b.gpr[i]<<"\n";
  std::cerr<<label<<" flags "<<std::hex<<a.flags<<" != "<<b.flags<<"\n";throw std::runtime_error("Caller mismatch "+label);}
 for(unsigned i=0;i<512;++i)if(!(i>=6 && i<24) && a.fx[i]!=b.fx[i])throw std::runtime_error("FX mismatch "+label);
}
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected original PE and result JSON");map_image(read(argv[1]));
 const unsigned sites[7]={0x553a6f,0x553799,0x553986,0x55280b,0x552bdc,0x552fac,0x55337c},lengths[7]={5,8,6,6,6,6,6};
 unsigned char original[7][8]={},patched[7][8]={};for(unsigned i=0;i<7;++i)std::memcpy(original[i],reinterpret_cast<void*>(sites[i]),lengths[i]);
 if(!MinimapWritersInstallForTest())throw std::runtime_error("Production minimap installer refused");
 for(unsigned i=0;i<7;++i)std::memcpy(patched[i],reinterpret_cast<void*>(sites[i]),lengths[i]);
 auto* l=reinterpret_cast<unsigned char*>(0x58b660);l[0]=0xe9;*reinterpret_cast<unsigned*>(l+1)=reinterpret_cast<unsigned>(lock)-0x58b665;
 asm volatile("fninit; fxsave %0":"=m"(pixel_seed_fx));
 unsigned terrainCases=0,overlayCases=0,borderCases=0,forwardCases=0;std::uint64_t words=0;
 for(unsigned cell=0;cell<512;++cell){
  minimap_fixture::Case c{2+int(cell%16)*2,1+int(cell/7%19),0,0,cell%4,cell/4%4,cell};c.h=std::min(c.h,c.w);c.cx=cell%c.w;c.cy=cell%c.h;
  auto seed=minimap_fixture::background(c,false),pixels=seed,history=minimap_fixture::background(c,true);const auto oldHistory=history;
  const unsigned stride=minimap_fixture::stride(c),width=minimap_fixture::width(c),height=minimap_fixture::height(c);
  owned={{pixels.data(),width,height,stride},{history.data(),width,height,stride}};
  std::vector<unsigned char> object(0x6220),cells(c.w*c.h*16);std::vector<signed char> states(c.w*c.h);
  auto field=[&](unsigned at,unsigned v){std::memcpy(object.data()+at,&v,4);};
  unsigned table[33]={};table[32]=reinterpret_cast<unsigned>(mnm_pixel_test_unlock);unsigned iface=reinterpret_cast<unsigned>(table);
  for(unsigned i=0;i<states.size();++i){states[i]=minimap_fixture::hidden(c,i)?-1:1;const unsigned ptr=reinterpret_cast<unsigned>(states.data()+i);const auto colour=minimap_fixture::colour(c,i);
   std::memcpy(cells.data()+i*16+8,&ptr,4);std::memcpy(cells.data()+i*16+12,&colour,2);}
  field(0x29,reinterpret_cast<unsigned>(cells.data()));field(0xc5,c.view);field(0xdf,reinterpret_cast<unsigned>(pixels.data()));field(0xff,0x13572468);
  field(0xe7,reinterpret_cast<unsigned>(pixels.data()+minimap_fixture::rootY(c)*stride+minimap_fixture::rootX(c)));field(0xeb,stride);
  field(0x6199,c.w);field(0x619d,c.h);field(0x61a1,c.cx);field(0x61a5,c.cy);
  field(0x61a9+4,reinterpret_cast<unsigned>(&iface));field(0x61c1,stride*2);field(0x61cd,reinterpret_cast<unsigned>(history.data()));
  const auto seedObject=object,oldCells=cells;const auto oldStates=states;global(0x5e1404,c.fog!=0);global(0x5e18f0,unsigned(-1));global(0x6e1f88,cell&1);
  pixel_function=0x553a40;pixel_object=reinterpret_cast<unsigned>(object.data());pixel_argc=0;pixel_seed_flags=0x202|(cell&1?0xcd5:0);
  std::memcpy(reinterpret_cast<void*>(sites[0]),original[0],lengths[0]);const auto before=call();const auto expected=pixels;const auto expectedObject=object;
  pixels=seed;object=seedObject;threadId=cell && cell%19==0?2:1;const auto count=mnm_map_native[0];
  std::memcpy(reinterpret_cast<void*>(sites[0]),patched[0],lengths[0]);const auto after=call();equal(before,after,"terrain "+std::to_string(cell));
  if(pixels!=expected || object!=expectedObject || history!=oldHistory || cells!=oldCells || states!=oldStates)throw std::runtime_error("Terrain pixel/source/state mismatch");
  if(threadId==1 && mnm_map_native[0]!=count+1)throw std::runtime_error("Valid terrain refused cell="+std::to_string(cell));
  if(mnm_map_native[0]!=count)++terrainCases;else ++forwardCases;words+=pixels.size();
 }
 /* Camera cases isolate their pixel passes while retaining their original
  * origin preparation, colour helper and subsequent marker-list control. */
 *reinterpret_cast<unsigned char*>(0x553a40)=0xc3;
 const unsigned functions[7]={0,0x5536c0,0x553850,0x5527a0,0x552b50,0x552f20,0x5532f0};
 for(unsigned kind=1;kind<7;++kind)for(unsigned cell=0;cell<384;++cell){
  const int view=kind<3?cell%4:kind-3;
  minimap_overlays_fixture::Case c{int(kind<3?kind-1:2),32+int(cell%3)*16,32+int(cell/3%3)*16,0,0,96+int(cell%2)*32,96+int(cell/2%2)*32,view,int(cell%2),0,0,int(cell%9),int(cell/5%2),int(cell/7%2),int(cell/11%2),int(cell%4),int(cell/13%5)*128,int(cell/17%4)*64,int(cell)};
  c.cx=cell%c.w;c.cy=cell/c.w%c.h;c.x=cell*7%c.w;c.y=cell*11%c.h;
  auto seed=minimap_overlays_fixture::background(c),pixels=seed;const unsigned stride=minimap_overlays_fixture::stride(c),width=minimap_overlays_fixture::width(c),height=minimap_overlays_fixture::height(c);
  owned={{pixels.data(),width,height,stride}};std::vector<unsigned char> object(0x6220);auto field=[&](unsigned at,unsigned v){std::memcpy(object.data()+at,&v,4);};
  const auto palette=minimap_overlays_fixture::palette(c);std::memcpy(object.data()+0xcb,palette.data(),18);
  field(0xc5,c.view);field(0xdf,reinterpret_cast<unsigned>(pixels.data()));field(0xeb,stride);field(0xf7,minimap_overlays_fixture::originX(c));field(0xfb,minimap_overlays_fixture::originY(c));
  field(0x103,c.rw);field(0x107,c.rh);field(0x61a1,c.cx);field(0x61a5,c.cy);field(0x6141,c.flash);field(0x6207,cell/23%2);
  field(0x6199,0x13572468);field(0x619d,0x24681357);
  global(0x6c5494,c.w);global(0x6c5498,c.h);global(0x6e1f88,c.rgb555);global(0x5e1404,c.fog);global(0x5e18f0,unsigned(-1));
  const auto markers=minimap_overlays_fixture::markers(c);std::vector<unsigned> raw;std::vector<signed char> states(4*128*c.w,1);
  for(unsigned i=0;i<128;++i)global(0x6cb942+i*4,i*c.w);for(unsigned i=0;i<32;++i)global(0x6cb8c2+i*4,i*128*c.w);
  for(unsigned i=0;i<markers.size();++i){const auto& m=markers[i];raw.insert(raw.end(),{unsigned(m.x),unsigned(m.y),i*2});states[i*128*c.w+m.y*c.w+m.x]=m.hidden?-1:1;}
  field(0x614d,raw.empty()?0:reinterpret_cast<unsigned>(raw.data()));field(0x6151,raw.empty()?0:reinterpret_cast<unsigned>(raw.data()+raw.size()));global(0x6c5c5c,reinterpret_cast<unsigned>(states.data()));
  std::vector<unsigned> rows(height);for(unsigned y=0;y<height;++y)rows[y]=reinterpret_cast<unsigned>(pixels.data())+((minimap_overlays_fixture::originY(c)+y)*stride+minimap_overlays_fixture::originX(c))*2;
  field(0xf3,reinterpret_cast<unsigned>(rows.data()));const auto corners=minimap_overlays_fixture::corners(c);for(unsigned i=0;i<4;++i){field(0x6121+i*8,corners[i][0]);field(0x6125+i*8,corners[i][1]);}
  int rectangle[]={5,7,5+c.viewportW,7+c.viewportH};global(0x6c482c,reinterpret_cast<unsigned>(rectangle));
  const auto seedObject=object;const auto oldStates=states;const auto oldRows=rows,oldRaw=raw;
  pixel_function=functions[kind];pixel_object=reinterpret_cast<unsigned>(object.data());pixel_argc=kind==1?4:0;
  pixel_args[0]=c.x;pixel_args[1]=c.y;pixel_args[2]=c.index;pixel_args[3]=c.emphasized;pixel_seed_flags=0x202|(cell&1?0xcd5:0);
  std::memcpy(reinterpret_cast<void*>(sites[kind]),original[kind],lengths[kind]);threadId=1;const auto before=call();const auto expected=pixels;const auto expectedObject=object;
  pixels=seed;object=seedObject;threadId=cell && cell%19==0?2:1;const auto count=mnm_map_native[kind];
  std::memcpy(reinterpret_cast<void*>(sites[kind]),patched[kind],lengths[kind]);const auto after=call();equal(before,after,"overlay "+std::to_string(kind)+" cell "+std::to_string(cell));
  if(pixels!=expected || object!=expectedObject || states!=oldStates || rows!=oldRows || raw!=oldRaw)throw std::runtime_error("Overlay pixel/source/state mismatch kind="+std::to_string(kind)+" cell="+std::to_string(cell));
  const bool encountered=kind!=2 || std::any_of(markers.begin(),markers.end(),[&](const auto& m){return !c.fog || !m.hidden;});
  if(threadId==1 && encountered && mnm_map_native[kind]==count)throw std::runtime_error("Valid overlay refused kind="+std::to_string(kind)+" cell="+std::to_string(cell));
  if(mnm_map_native[kind]!=count){++overlayCases;if(kind>=3 && !(cell/23%2))++borderCases;}else ++forwardCases;words+=pixels.size();
 }
 std::ofstream out(argv[2]);out<<"{\"success\":true,\"terrain_native_cases\":"<<terrainCases<<",\"overlay_native_cases\":"<<overlayCases<<",\"outer_border_native_cases\":"<<borderCases<<",\"forward_or_no_marker_cases\":"<<forwardCases<<",\"physical_words\":"<<words<<",\"pixel_pass_sites\":7,\"caller_state_equal\":true}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
