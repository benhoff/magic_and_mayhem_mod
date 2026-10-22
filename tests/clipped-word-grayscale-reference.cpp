// Execute unmodified No-CD 589c90 against authored inputs and an independent
// unpacked-channel oracle. The runner pins the PE hash before execution.
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <sys/mman.h>
#include <vector>
#include "clipped-word-grayscale-cases.hpp"
using Bytes=std::vector<std::uint8_t>;
static unsigned word(const Bytes& b,unsigned p){
  if(p>b.size()||b.size()-p<4)throw std::runtime_error("PE extent");
  unsigned value;std::memcpy(&value,b.data()+p,4);return value;
}
static void put(Bytes& b,unsigned p,unsigned value){std::memcpy(b.data()+p,&value,4);}
static void global(std::uintptr_t p,unsigned value){std::memcpy(reinterpret_cast<void*>(p),&value,4);}
static void map(const char* path){
  std::ifstream file(path,std::ios::binary);if(!file)throw std::runtime_error("PE input");
  Bytes b(std::istreambuf_iterator<char>(file),{});
  const auto pe=word(b,60),opt=pe+24,length=word(b,opt+56);
  if(word(b,opt+28)!=0x400000)throw std::runtime_error("PE base");
  auto* image=static_cast<std::uint8_t*>(mmap(reinterpret_cast<void*>(0x400000),length,7,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
  if(image==MAP_FAILED)throw std::runtime_error("private PE map");
  const auto count=unsigned(b.at(pe+6)|b.at(pe+7)<<8),table=opt+unsigned(b.at(pe+20)|b.at(pe+21)<<8);
  for(unsigned i=0;i<count;++i){const auto p=table+i*40,n=word(b,p+16),raw=word(b,p+20),rva=word(b,p+12);
    if(raw>b.size()||n>b.size()-raw||rva>length||n>length-rva)throw std::runtime_error("PE section");
    std::memcpy(image+rva,b.data()+raw,n);
  }
  const std::uint8_t signature[]={0x83,0xec,0x1c,0xa1,0x88,0x1f,0x6e,0};
  if(std::memcmp(reinterpret_cast<void*>(0x589c90),signature,sizeof(signature)))throw std::runtime_error("original entry");
}
int main(int argc,char** argv)try{
  if(argc!=3)throw std::runtime_error("expected pinned PE and retained output prefix");
  map(argv[1]);
  // Zero format selects RGB565 shifts. These private data pages are fixture
  // globals; executable bytes and the input PE remain unchanged.
  global(0x6e1f88,0);
  const std::uint16_t masks[]={0xf800,0x07e0,0x001f};
  std::memcpy(reinterpret_cast<void*>(0x6e1f7c),masks,sizeof(masks));
  const auto& cases=grayscale_fixture::cases;
  unsigned compared=0;
  for(unsigned index=0;index<sizeof(cases)/sizeof(cases[0]);++index){
    const auto c=cases[index];const unsigned height=256,stride=259;
    const unsigned samples=c.width*c.height;
    std::vector<std::uint16_t> source(samples);std::vector<unsigned char> mask(samples);
    for(unsigned i=0;i<samples;++i){source[i]=index?std::uint16_t(i*739+31):std::uint16_t(i);mask[i]=!c.transparent||(i%7!=0&&i%11!=0);}
    Bytes frame(40+c.height*8,0);put(frame,4,c.width);put(frame,8,c.height);put(frame,12,c.originX);put(frame,16,c.originY);put(frame,28,0xffffffff);
    for(int y=0;y<c.height;++y){
      Bytes controls,pixels;int x=0;
      while(x<c.width){unsigned skip=0,draw=0;
        while(x<c.width&&!mask[y*c.width+x]&&skip<255){++skip;++x;}
        controls.push_back(skip);
        if(x==c.width)break;
        while(x<c.width&&mask[y*c.width+x]&&draw<255){const auto v=source[y*c.width+x];pixels.push_back(v);pixels.push_back(v>>8);++x;++draw;}
        controls.push_back(draw);
      }
      put(frame,40+y*8,frame.size());frame.insert(frame.end(),controls.begin(),controls.end());
      put(frame,44+y*8,frame.size());frame.insert(frame.end(),pixels.begin(),pixels.end());
    }
    put(frame,0,frame.size());
    std::vector<std::uint16_t> guarded(stride*height+64,0x1234),expected(stride*height,0x1234);
    auto* output=guarded.data()+32;
    for(int y=0;y<c.height;++y)for(int x=0;x<c.width;++x){
      const auto at=y*c.width+x;const int dx=c.x-c.originX+x,dy=c.y-c.originY+y;
      if(!mask[at]||dx<c.left||dx>=c.right||dy<c.top||dy>=c.bottom)continue;
      const auto v=source[at];const unsigned red=v/2048,green=(v/32)%64,blue=v%32,mean=(red+green/2+blue)/3;
      const bool horizontal=c.x-c.originX<c.left||c.x-c.originX+c.width>c.right;
      expected[dy*stride+dx]=horizontal?v:mean*2113;
    }
    global(0x658174,reinterpret_cast<std::uintptr_t>(output));global(0x6a2dc8,stride);
    global(0x6e0008,c.left);global(0x6cbb6c,c.top);global(0x6a49b8,c.right);global(0x656618,c.bottom);
    using Draw=unsigned(*)(void*,int,int);reinterpret_cast<Draw>(0x589c90)(frame.data(),c.x,c.y);
    if(!std::equal(expected.begin(),expected.end(),output)){
      const auto prefix=std::string(argv[2])+"-case-"+std::to_string(index);
      std::ofstream original(prefix+".original.565",std::ios::binary),oracle(prefix+".expected.565",std::ios::binary);
      original.write(reinterpret_cast<const char*>(output),expected.size()*2);
      oracle.write(reinterpret_cast<const char*>(expected.data()),expected.size()*2);
      unsigned differences=0;
      for(unsigned i=0;i<expected.size();++i)if(output[i]!=expected[i]){
        if(differences<8)std::cerr<<"sample "<<i%stride<<","<<i/stride<<" original "<<output[i]<<" expected "<<expected[i]<<'\n';
        ++differences;
      }
      std::cerr<<"different words "<<differences<<'\n';
      throw std::runtime_error("original grayscale pixels differ in case "+std::to_string(index));
    }
    if(!std::all_of(guarded.begin(),guarded.begin()+32,[](auto v){return v==0x1234;})||!std::all_of(guarded.end()-32,guarded.end(),[](auto v){return v==0x1234;}))throw std::runtime_error("destination guards changed");
    compared+=stride*height;
    std::ofstream retained(std::string(argv[2])+"-case-"+std::to_string(index)+".original.565",std::ios::binary);
    retained.write(reinterpret_cast<const char*>(output),expected.size()*2);
    if(!retained)throw std::runtime_error("Cannot retain original grayscale words");
  }
  std::cout<<"{\"success\":true,\"cases\":"<<grayscale_fixture::count<<",\"compared_words\":"<<compared<<",\"full_source_words\":65536,\"different_words\":0}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
