static void* rectProc(void*,const char*);
#define MNM_PIXEL_TEST_UTILITIES
#define MNM_PIXEL_TEST_GETPROC rectProc
#include "pixel-splice-reference.cpp"
extern "C" {int WIN ColourRectInstallForTest();extern u32 mnm_colour_rect_native,mnm_colour_rect_forwarded;}
static unsigned short* ownedPixels;
static unsigned ownedStride,ownedHeight;
static int WIN rectPlane(const void* base,u32* d,u32 bytes){
 if(base!=ownedPixels||bytes!=24)return -1;d[0]=1;d[1]=ownedStride-7;d[2]=ownedHeight;d[3]=ownedStride;d[4]=1;d[5]=*reinterpret_cast<unsigned*>(0x6e1f88);return 0;
}
static unsigned historyQueries;
static int WIN currentHistory(const void*,u32,u32,u32,u32,u32){++historyQueries;return 0;}
static void WIN countProducer(u32,u32,u32){}
static void* rectProc(void*,const char* name){if(!std::strcmp(name,"NativeDrawCpuHistoryV1"))return reinterpret_cast<void*>(currentHistory);if(!std::strcmp(name,"NativeDrawProducerCountV1"))return reinterpret_cast<void*>(countProducer);if(std::strcmp(name,"NativeDrawPlaneV1"))throw std::runtime_error("Unexpected plane lookup");return reinterpret_cast<void*>(rectPlane);}
int main(int argc,char** argv)try{
 if(argc!=3)throw std::runtime_error("Expected pinned original PE/result");map_image(read(argv[1]));
 unsigned char original[5],patched[5];std::memcpy(original,reinterpret_cast<void*>(0x547e53),5);
 if(!ColourRectInstallForTest())throw std::runtime_error("Production installer refused");std::memcpy(patched,reinterpret_cast<void*>(0x547e53),5);
 asm volatile("fninit; fxsave %0":"=m"(pixel_seed_fx));
 pixel_seed_stack=1;
 unsigned admitted=0,forwarded=0,emptyAdmitted=0;std::uint64_t words=0;
 PixelRouteCoverage<4> modes;
 const std::array<std::string,4> modeNames={"colour_rect/mode0","colour_rect/mode1","colour_rect/mode2","colour_rect/mode3"};
 for(unsigned cell=0;cell<4096+256;++cell){
  const bool edge=cell>=4096;const unsigned e=edge?cell-4096:cell;
  const unsigned format=e%2,mode=e/2%4,w=edge?1+e/128:1+cell/8%41,h=edge?w:1+cell/328%13,visibleWidth=64,visibleHeight=32,stride=visibleWidth+7;
  std::vector<unsigned short> seed(stride*(visibleHeight+2)+512);for(unsigned i=0;i<seed.size();++i)seed[i]=i*7919+cell*107;
  auto pixels=seed;ownedPixels=pixels.data()+256;ownedStride=stride;ownedHeight=visibleHeight;
  unsigned char shape[22]={};std::memcpy(shape,&mode,4);std::memcpy(shape+4,&w,4);std::memcpy(shape+8,&h,4);
  std::vector<unsigned short> triples(w*h*3);for(unsigned i=0;i<w*h;++i){triples[i*3]=(i+cell)%32;triples[i*3+1]=(i*13+cell*7)%(format?32:64);triples[i*3+2]=(i*29+cell*3)%32;}
  std::memcpy(shape+12,triples.data(),6);const unsigned ptr=(edge?e/8%2:cell/16%2)?reinterpret_cast<unsigned>(triples.data()):0;std::memcpy(shape+18,&ptr,4);
  const auto source=triples;
  global(0x658174,reinterpret_cast<unsigned>(ownedPixels));global(0x6a2dc8,stride);global(0x6e1f88,format);
  global(0x6e0008,edge?0:cell/23%7);global(0x6cbb6c,edge?0:cell/31%5);global(0x6a49b8,visibleWidth-(edge?0:cell/37%9));global(0x656618,visibleHeight-(edge?0:cell/41%7));
  pixel_function=0x547d80;pixel_object=reinterpret_cast<unsigned>(shape);pixel_argc=2;
  pixel_args[0]=unsigned(int(cell/47%75)-11);pixel_args[1]=unsigned(int(cell/53%40)-7);pixel_seed_flags=0x202|(cell&1?0x8d5:0)|(cell/8%2?0x400:0);
  if(edge){pixel_args[0]=e/16%4==0?unsigned(-int(w)):e/16%4==2?visibleWidth:1;pixel_args[1]=e/16%4==1?unsigned(-int(h)):e/16%4==3?visibleHeight:1;pixel_seed_flags=0x202|(e&1?0x8d5:0)|(e/64%2?0x400:0);}
  std::memcpy(reinterpret_cast<void*>(0x547e53),original,5);threadId=1;const auto before=call();const auto expected=pixels;
  pixels=seed;threadId=!edge&&cell&&cell%19==0?2:1;const auto count=mnm_colour_rect_native,queries=historyQueries;
  std::memcpy(reinterpret_cast<void*>(0x547e53),patched,5);const auto after=call();
  if(pixels!=expected||triples!=source)throw std::runtime_error("Pixel/source mismatch cell="+std::to_string(cell)+" mode="+std::to_string(mode));
  if(before.gpr!=after.gpr||((before.flags^after.flags)&0xcd5)||before.error!=after.error){
   for(unsigned i=0;i<8;++i)if(before.gpr[i]!=after.gpr[i])std::cerr<<"gpr"<<i<<" "<<std::hex<<before.gpr[i]<<" != "<<after.gpr[i]<<"\n";
   throw std::runtime_error("Caller mismatch cell="+std::to_string(cell));}
  if(before.fx!=after.fx)throw std::runtime_error("Full FX mismatch");
  if(edge&&(pixels!=seed||mnm_colour_rect_native!=count+1))throw std::runtime_error("Empty clipped rectangle did not execute natively cell="+std::to_string(cell));
  if(edge&&historyQueries!=queries)throw std::runtime_error("Empty rectangle requested CPU history");
  if(mode&&pixels!=seed&&historyQueries==queries)throw std::runtime_error("Destination blend bypassed CPU history guard");
  if(mnm_colour_rect_native!=count){if(edge)++emptyAdmitted;modes.matched(mode,format,pixels!=seed);++admitted;}else ++forwarded;words+=pixels.size();
 }
 modes.require(modeNames);
 PixelRouteCoverage<1> coverage;const std::array<std::string,1> routeNames={"colour_rect/rectangle"};
 for(const auto& mode:modes.routes)for(unsigned f=0;f<2;++f){coverage.routes[0].native[f]+=mode.native[f];coverage.routes[0].changed[f]+=mode.changed[f];}
 std::ofstream out(argv[2]);out<<"{\"success\":true,\"native_cases\":"<<admitted<<",\"empty_native_cases\":"<<emptyAdmitted<<",\"forward_or_hidden_cases\":"<<forwarded<<",\"physical_words\":"<<words<<",\"formats\":2,\"modes\":4,\"caller_state_equal\":true,";
 coverage.write(out,routeNames);out<<",\"mode_coverage\":{";modes.write(out,modeNames);out<<"}}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
