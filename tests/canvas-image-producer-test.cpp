#include "../compat/legacy/canvas_producers.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
using Bytes=std::vector<std::uint8_t>;
std::uint32_t word(const Bytes &b,std::size_t at){
  if(at>b.size() || b.size()-at<4)throw std::runtime_error("Image fixture extent");
  return b[at]|std::uint32_t(b[at+1])<<8|std::uint32_t(b[at+2])<<16|std::uint32_t(b[at+3])<<24;
}
int main(int argc,char **argv) try {
  if(argc!=3)throw std::runtime_error("Expected image corpus and output");
  std::ifstream input(argv[1],std::ios::binary);const Bytes b{std::istreambuf_iterator<char>(input),{}};
  if(b.size()<12 || std::memcmp(b.data(),"MNMIMG01",8))throw std::runtime_error("Image fixture header");
  std::ofstream output(argv[2],std::ios::binary);unsigned refusals=0;
  std::size_t at=12;
  for(unsigned n=0;n<word(b,8);++n){
    const auto size=word(b,at),kind=word(b,at+4),w=word(b,at+8),h=word(b,at+12),rawSize=word(b,at+24);
    const Bytes raw(b.begin()+at+32,b.begin()+at+32+rawSize);
    mnm::legacy::CanvasProducerReplay canvas([&](const std::string &name){
      if(name!="owned.bmp")throw std::runtime_error("Unexpected source path");
      return raw;
    });
    mnm::legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=1;create.fields[5]=w;create.fields[6]=h;
    canvas.apply(create);auto fill=create;fill.fields[2]=5;fill.fields[12]=w;fill.fields[13]=h;fill.fields[14]=0x2bab;canvas.apply(fill);
    auto image=create;image.fields[2]=kind;image.fields[8]=word(b,at+16);image.fields[9]=word(b,at+20);
    if(kind==19){
      image.fields[19]=word(b,at+28);image.fields[20]=raw.size()-image.fields[19];
      image.fields[14]=(raw[14]|unsigned(raw[15])<<8)==24 ? 1 : 0;
      image.payload=raw;
    }else{image.payload={'o','w','n','e','d','.','b','m','p',0};}
    canvas.apply(image);
    const auto native=canvas.read(1);
    output.write(reinterpret_cast<const char *>(native.pixels.data()),native.pixels.size()*4);
    // Malformed source must refuse before touching previously composed pixels.
    const auto before=native.pixels;bool failed=false;
    auto malformed=image;
    if(kind==19){malformed.payload[16]=1;}
    else {malformed.fields[14]=1;}
    try{canvas.apply(malformed);}catch(const std::exception &){failed=true;}
    if(!failed || canvas.read(1).pixels!=before)throw std::runtime_error("Non-atomic image refusal");
    ++refusals;at+=size;
  }
  if(at!=b.size() || !output)throw std::runtime_error("Image input/output extent");
  std::cout<<"{\"success\":true,\"atomic_refusals\":"<<refusals<<"}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
