static void* rasterProc(void*,const char*);
#define MNM_PIXEL_TEST_UTILITIES
#define MNM_PIXEL_TEST_GETPROC rasterProc
#include "pixel-splice-reference.cpp"
extern "C" {int WIN RasterWritersInstallForTest();u32 raster_cdecl;extern u32 mnm_raster_native[],mnm_raster_forwarded[];}
extern "C" {
#include "../runtime/canvas-writers/raster_sites.h"
}
static unsigned short* ownedPixels;
static unsigned ownedWidth,ownedStride,ownedHeight;
static int WIN rasterPlane(const void* base,u32* d,u32 bytes){
 if(base!=ownedPixels||bytes!=24)return -1;d[0]=1;d[1]=ownedWidth;d[2]=ownedHeight;d[3]=ownedStride;d[4]=1;d[5]=*reinterpret_cast<unsigned*>(0x6e1f88);return 0;
}
static int WIN currentHistory(const void*,u32,u32,u32,u32,u32){return 0;}
static void WIN countProducer(u32,u32,u32){}
static void* rasterProc(void*,const char* name){if(!std::strcmp(name,"NativeDrawCpuHistoryV1"))return reinterpret_cast<void*>(currentHistory);if(!std::strcmp(name,"NativeDrawProducerCountV1"))return reinterpret_cast<void*>(countProducer);if(std::strcmp(name,"NativeDrawPlaneV1"))throw std::runtime_error("Unexpected plane lookup");return reinterpret_cast<void*>(rasterPlane);}
static Bytes encoded(unsigned w,unsigned h,unsigned seed,bool indexed){
 Bytes controls,colours;std::vector<unsigned> rows,pixels;
 for(unsigned y=0;y<h;++y){rows.push_back(controls.size());pixels.push_back(colours.size());unsigned x=0;
  while(x<w){const unsigned skip=std::min(w-x,(x+y+seed)%4);controls.push_back(skip);x+=skip;if(x==w)break;
   const unsigned run=std::min(w-x,1+(x*3+y+seed)%13);controls.push_back(run);
   for(unsigned i=0;i<run;++i){const unsigned value=(x+i+y*43+seed*71);colours.push_back(value);if(!indexed)colours.push_back(value>>8);}x+=run;
  }
 }
 Bytes frame(40+h*8);auto put=[&](unsigned at,unsigned n){std::memcpy(frame.data()+at,&n,4);};
 put(4,w);put(8,h);put(12,unsigned(-3));put(16,2);put(28,UINT32_MAX);
 for(unsigned y=0;y<h;++y){put(40+y*8,frame.size()+rows[y]);put(44+y*8,frame.size()+controls.size()+pixels[y]);}
 frame.insert(frame.end(),controls.begin(),controls.end());frame.insert(frame.end(),colours.begin(),colours.end());put(0,frame.size());return frame;
}
int main(int argc,char** argv)try{
 if(argc!=3&&argc!=4)throw std::runtime_error("Expected pinned original PE/result");map_image(read(argv[1]));
 unsigned char original[MNM_RASTER_SITES][9]={},patched[MNM_RASTER_SITES][9]={};
 for(unsigned i=0;i<MNM_RASTER_SITES;++i)std::memcpy(original[i],reinterpret_cast<void*>(sites[i].at),sites[i].length);
 if(!RasterWritersInstallForTest())throw std::runtime_error("Production installer refused");
 for(unsigned i=0;i<MNM_RASTER_SITES;++i)std::memcpy(patched[i],reinterpret_cast<void*>(sites[i].at),sites[i].length);
 asm volatile("fninit; fxsave %0":"=m"(pixel_seed_fx));pixel_seed_stack=1;
 const unsigned functions[]={0x5947b2,0x59521a,0x595b47,0x59603e,0x595677,0x596cb8,0x597086,0x596490,0x5968a4,0x57e1b0,0x57ec90,0x57f0f0,0x57f5f0,0x5806f0,0x57e540,0x589c90,0x57de00};
 unsigned admitted=0,forwarded=0,workspaceCases=0,capturedAdmitted=0;std::uint64_t words=0;std::array<unsigned,17> byKind{};
 for(unsigned kind=0;kind<17;++kind)for(unsigned cell=0;cell<640;++cell){
  const bool captured=argc==4&&(kind==5||kind==15)&&cell<32;
  const unsigned format=cell%2,w=captured?40:cell<512?1+cell/2%39:cell%2?129:255,h=captured?40:1+cell/78%7,
      width=captured?596:cell<512?96:320,height=captured?140:64,stride=width+(captured?0:7);
  const bool indexed=kind<=3||kind==9||kind==10||kind==11||kind==12||kind==13||kind==14||kind==16;
  Bytes frame=captured?read(argv[3]):encoded(w,h,cell,indexed);
  std::vector<unsigned char> palette(12+256*4,0);const unsigned step=kind==2||kind==3?4:2;
  for(unsigned i=0;i<256;++i){const unsigned value=((i*7919+cell*107)&(format?32767:65535))|
      (step==4&&cell/17%2?((i*5939+cell*123)&65535)<<16:0);std::memcpy(palette.data()+12+i*step,&value,step);}
  if(indexed&&kind!=13&&kind!=14){const unsigned ptr=reinterpret_cast<unsigned>(palette.data());std::memcpy(frame.data()+28,&ptr,4);}
  else if(indexed){const unsigned zero=0;std::memcpy(frame.data()+28,&zero,4);}
  const auto frameBefore=frame,paletteBefore=palette;
  std::vector<unsigned short> seed(stride*(height+2)+512);for(unsigned i=0;i<seed.size();++i)seed[i]=i*7919+cell*107;
  auto pixels=seed;ownedPixels=pixels.data()+256;ownedWidth=width;ownedStride=stride;ownedHeight=height;
  global(0x658174,reinterpret_cast<unsigned>(ownedPixels));global(0x6a2dc8,stride);global(0x6e1f88,format);global(0x6e1f68,0);
  global(0x6e0008,captured?548:cell/23%5);global(0x6cbb6c,captured?90:cell/31%4);global(0x6a49b8,captured?597:width-cell/37%5);global(0x656618,captured?139:height-cell/41%5);
  const unsigned short masks[]={static_cast<unsigned short>(format?0x7c00:0xf800),static_cast<unsigned short>(format?0x3e0:0x7e0),31};std::memcpy(reinterpret_cast<void*>(0x6e1f7c),masks,6);
  global(0x6c4830,cell/11%16);global(0x5e15f8,2);global(0x5f14d0,0);
  for(unsigned i=0;i<16;++i){global(0x656640+i*4,i%9);for(unsigned a=1;a<=8;++a)global(0x5f14d0+(a*16+i)*4,i%(a*2));}
  const int left=captured?552:int(cell/7%(width+7))-7,top=captured?94:int(cell/19%70)-5;
  pixel_function=functions[kind];pixel_object=reinterpret_cast<unsigned>(frame.data());raster_cdecl=kind<=8||kind==15||kind==16;
  pixel_argc=raster_cdecl?5:kind==13?2:kind==14?1:3;
  if(raster_cdecl){pixel_args[0]=reinterpret_cast<unsigned>(frame.data());pixel_args[1]=captured?unsigned(left):unsigned(left-3);pixel_args[2]=captured?unsigned(top-4):unsigned(top+2);pixel_args[3]=0;pixel_args[4]=0;}
  else{pixel_args[0]=unsigned(top+2);pixel_args[1]=kind==13?2:0;pixel_args[2]=0;}
  /* Fastcall X is in EDX; the test probe reads this additional seed. */
  extern unsigned raster_x;raster_x=unsigned(left-3);
  pixel_seed_flags=0x202|(cell&1?0x8d5:0)|(cell&2?0x400:0);
  unsigned workspace[16];for(unsigned i=0;i<16;++i)workspace[i]=0xabc00000+i*7919+cell;
  for(unsigned i=0;i<MNM_RASTER_SITES;++i)std::memcpy(reinterpret_cast<void*>(sites[i].at),original[i],sites[i].length);
  std::memcpy(reinterpret_cast<void*>(0x5f1e50),workspace,64);threadId=1;const auto before=call();const auto expected=pixels;unsigned expectedWorkspace[16];std::memcpy(expectedWorkspace,reinterpret_cast<void*>(0x5f1e50),64);
  pixels=seed;std::memcpy(reinterpret_cast<void*>(0x5f1e50),workspace,64);threadId=cell&&cell%19==0?2:1;
  unsigned count=0;for(unsigned i=0;i<MNM_RASTER_SITES;++i)count+=mnm_raster_native[i];
  for(unsigned i=0;i<MNM_RASTER_SITES;++i)std::memcpy(reinterpret_cast<void*>(sites[i].at),patched[i],sites[i].length);
  const auto after=call();unsigned next=0;for(unsigned i=0;i<MNM_RASTER_SITES;++i)next+=mnm_raster_native[i];
  if(pixels!=expected){for(unsigned i=0;i<pixels.size();++i)if(pixels[i]!=expected[i]){std::cerr<<"pixel "<<i<<" "<<std::hex<<expected[i]<<" != "<<pixels[i]<<"\n";break;}throw std::runtime_error("Pixel mismatch kind="+std::to_string(kind)+" cell="+std::to_string(cell));}
  if(frame!=frameBefore||palette!=paletteBefore)throw std::runtime_error("Source mutation");
  bool abi=(before.gpr[0]&65535)==(after.gpr[0]&65535)&&before.error==after.error;
  for(unsigned i=3;i<8;++i)abi=abi&&before.gpr[i]==after.gpr[i];
  if(kind<=8)abi=abi&&before.gpr==after.gpr;
  if(!abi||((before.flags^after.flags)&0xcd5))throw std::runtime_error("Caller ABI mismatch kind="+std::to_string(kind)+" cell="+std::to_string(cell));
  for(unsigned i=0;i<512;++i)if(!(i>=6&&i<24)&&before.fx[i]!=after.fx[i])throw std::runtime_error("FX mismatch");
  if(kind<=8&&std::memcmp(expectedWorkspace,reinterpret_cast<void*>(0x5f1e50),64)){
   for(unsigned i=0;i<16;++i)if(expectedWorkspace[i]!=*reinterpret_cast<unsigned*>(0x5f1e50+i*4))std::cerr<<"workspace "<<i<<" "<<std::hex<<expectedWorkspace[i]<<" != "<<*reinterpret_cast<unsigned*>(0x5f1e50+i*4)<<"\n";
   throw std::runtime_error("Workspace mismatch kind="+std::to_string(kind)+" cell="+std::to_string(cell));}
  if(next!=count){if(captured)++capturedAdmitted;++admitted;++byKind[kind];if(kind<=8)++workspaceCases;}else ++forwarded;words+=pixels.size();
 }
 if(argc==4&&!capturedAdmitted)throw std::runtime_error("Captured frame never admitted");
 for(unsigned i=0;i<17;++i)if(!byKind[i])throw std::runtime_error("No native execution for entry="+std::to_string(i));
 std::ofstream out(argv[2]);out<<"{\"success\":true,\"native_cases\":"<<admitted<<",\"captured_native_cases\":"<<capturedAdmitted<<",\"forward_or_hidden_cases\":"<<forwarded<<",\"workspace_cases\":"<<workspaceCases<<",\"physical_words\":"<<words<<",\"formats\":2,\"original_entries\":17,\"pixel_splices\":"<<MNM_RASTER_SITES<<",\"by_entry\":[";
 for(unsigned i=0;i<17;++i){if(i)out<<',';out<<byKind[i];}out<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
unsigned raster_x;
