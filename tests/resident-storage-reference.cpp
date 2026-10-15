// Independent private execution of pinned original raster bodies. Diagnostic
// native entry pixels are private oracle inputs and never feed native rendering.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "world-raster-reference.hpp"

int main(int argc,char** argv)try{
 if(argc!=5)throw std::runtime_error("Expected pinned PE, closed draws, private baseline, output");
 map_image(read(argv[1]));const auto input=read(argv[2]),baseline=read(argv[3]);
 if(input.size()<20||std::memcmp(input.data(),"MNMSCHK1",8))throw std::runtime_error("Checkpoint header");
 const unsigned width=u32(input,8),height=u32(input,12),count=u32(input,16);
 if(width!=800||height!=600||count>12320||baseline.size()!=width*height*2)throw std::runtime_error("Checkpoint bounds");
 std::vector<std::uint16_t> pixels(width*height);std::memcpy(pixels.data(),baseline.data(),baseline.size());
 unsigned at=20,rasters=0,primitives=0;
 for(unsigned i=0;i<count;++i){
  if(input.size()-at<100)throw std::runtime_error("Truncated checkpoint draw");
  const auto entry=u32(input,at);at+=4;const auto size=u32(input,at);
  if(size<96||size>input.size()-at)throw std::runtime_error("Checkpoint draw extent");
  Bytes record(input.begin()+at,input.begin()+at+size);at+=size;
  const auto kind=u32(record,8);
  if(kind==9){
   if(u32(record,60)==8){
    if((entry!=0x597086&&entry!=0x596cb8)||u32(record,68)||u32(record,80)||u32(record,76)<40||size!=96+u32(record,76))throw std::runtime_error("WORD raster bounds");
    Bytes frame(record.begin()+96,record.end());const std::uint32_t marker=UINT32_MAX;std::memcpy(frame.data()+28,&marker,4);
    global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f68,0);global(0x6e1f88,0);
    for(unsigned j=0;j<4;++j)global(j==0?0x6e0008:j==1?0x6cbb6c:j==2?0x6a49b8:0x656618,u32(record,40+j*4));
    using Draw=unsigned (*)(void*,int,int);
    const auto ax=reinterpret_cast<Draw>(entry)(frame.data(),std::int32_t(u32(record,32)),std::int32_t(u32(record,36)))&65535u;
    if(ax!=u32(record,84))throw std::runtime_error("Original WORD caller AX");
   }else if((originalRaster(record,entry,pixels)&65535u)!=u32(record,84))throw std::runtime_error("Original raster caller AX");
   ++rasters;continue;
  }
  if(entry||u32(record,72)!=size-96||(kind!=14&&kind!=26))throw std::runtime_error("Checkpoint primitive");
  global(0x658174,reinterpret_cast<std::uintptr_t>(pixels.data()));global(0x6a2dc8,width);global(0x6e1f68,0);global(0x6e1f88,0);
  for(unsigned j=0;j<4;++j)global(j==0?0x6e0008:j==1?0x6cbb6c:j==2?0x6a49b8:0x656618,u32(record,40+j*4));
  const int x=std::int32_t(u32(record,32)),y=std::int32_t(u32(record,36));
  if(kind==14){
   if(size!=116)throw std::runtime_error("Additive primitive extent");
   std::uint32_t object[18]={};object[0]=0x5c7420;for(unsigned j=0;j<5;++j)object[12+j]=u32(record,96+j*4);
   using Draw=unsigned (__attribute__((thiscall)) *)(void*,int,int);reinterpret_cast<Draw>(0x54ab00)(object,x,y);
   if(reinterpret_cast<unsigned char*>(object)[0x44]!=1)throw std::runtime_error("Original additive side effect");
  }else{
   const auto w=u32(record,76),h=u32(record,80),mode=u32(record,60);
   if(!w||!h||w>64||h>64||mode>3||size!=96+w*h*2)throw std::runtime_error("Colour primitive extent");
   std::vector<std::uint16_t> rgb(w*h*3);for(unsigned j=0;j<w*h;++j){const auto p=unsigned(record[96+j*2])|unsigned(record[97+j*2])<<8;rgb[j*3]=p>>11;rgb[j*3+1]=(p>>5)&63;rgb[j*3+2]=p&31;}
   unsigned char object[80]={};const std::uint32_t fields[]={mode,w,h};std::memcpy(object+28,fields,12);const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(rgb.data()));std::memcpy(object+46,&pointer,4);
   using Draw=unsigned (__attribute__((thiscall)) *)(void*,int,int);reinterpret_cast<Draw>(0x54ab60)(object,x,y);
  }++primitives;
 }
 if(at!=input.size()||!rasters)throw std::runtime_error("Checkpoint incomplete");
 save(argv[4],pixels);std::cout<<rasters<<" original rasters, "<<primitives<<" original primitives and caller AX checked\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
