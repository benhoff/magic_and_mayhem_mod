#include "../renderer/canvas_sequence.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
using Bytes=std::vector<unsigned char>;
static Bytes read(const char *path) {std::ifstream in(path,std::ios::binary);if(!in)throw std::runtime_error("Input open");return Bytes(std::istreambuf_iterator<char>(in),{});}
static unsigned word(const Bytes &b,std::size_t at) {if(at+4>b.size())throw std::runtime_error("Input extent");unsigned result;std::memcpy(&result,b.data()+at,4);return result;}
static unsigned atomic() {
  unsigned count=0;
  for(unsigned mode=15;mode<=17;++mode){
    // Late visited pixel for each recovered order. Its failure must leave every
    // earlier defined pixel unchanged, including multiply visited corners.
    const int missingX=mode==15?10:mode==16?10:9,missingY=mode==15?10:mode==16?10:10;
    mnm::render::CanvasSequence canvas;canvas.create(1,16,16);
    for(int y=0;y<16;++y)for(int x=0;x<16;++x)
      if(x!=missingX || y!=missingY)canvas.fill(1,{x,y,x+1,y+1},0x39e7);
    bool refused=false;try{canvas.panel(1,{3,4,10,11},mode,40,true,true);}catch(const std::runtime_error &){refused=true;}
    if(!refused)throw std::runtime_error("Undefined panel destination admitted");
    canvas.fill(1,{missingX,missingY,missingX+1,missingY+1},0x39e7);
    for(auto p:canvas.read(1).pixels)if(p!=0x39e7)throw std::runtime_error("Partial panel mutation on refusal");
    ++count;
  }
  mnm::render::CanvasSequence canvas;canvas.create(1,16,16);
  canvas.fill(1,{0,0,16,16},0x39e7);const auto before=canvas.read(1).pixels;
  struct Bad {unsigned mode;mnm::render::Rect rect;unsigned percent;};
  std::vector<Bad> bad;
  for(unsigned mode=15;mode<=17;++mode){
    bad.push_back({mode,{-1,4,10,11},40});bad.push_back({mode,{3,4,17,11},40});
    bad.push_back({mode,{3,4,10,17},40});bad.push_back({mode,{3,4,10,11},1001});
  }
  bad.push_back({14,{3,4,10,11},40});bad.push_back({18,{3,4,10,11},40});
  bad.push_back({15,{3,4,15,15},40});bad.push_back({16,{3,4,16,16},40});
  for(const auto &b:bad){
    bool refused=false;
    try{canvas.panel(1,b.rect,b.mode,b.percent,true,true);}catch(const std::invalid_argument &){refused=true;}
    if(!refused || canvas.read(1).pixels!=before)throw std::runtime_error("Panel bounds/mode refusal mutated canvas");
    ++count;
  }
  return count;
}
int main(int argc,char **argv) try {
  if(argc!=3)throw std::runtime_error("Expected panel corpus, output");
  const auto b=read(argv[1]);
  if(b.size()<12 || std::memcmp(b.data(),"MNMPAN01",8))throw std::runtime_error("Panel corpus header");
  std::ofstream out(argv[2],std::ios::binary);std::size_t at=12;
  for(unsigned c=0;c<word(b,8);++c){
    const unsigned w=word(b,at),h=word(b,at+4),stride=word(b,at+8),mode=word(b,at+12);
    const mnm::render::Rect r{int(word(b,at+16)),int(word(b,at+20)),int(word(b,at+24)),int(word(b,at+28))};
    const auto percent=word(b,at+32),pressed=word(b,at+36),reverse=word(b,at+40);at+=44;
    if(!w||!h||w>2048||h>2048||stride<w||stride>4096||at+stride*h*2>b.size())throw std::runtime_error("Panel case extent");
    mnm::render::Image image{int(w),int(h),{}};std::vector<std::uint16_t> physical(stride*h);
    std::memcpy(physical.data(),b.data()+at,stride*h*2);at+=stride*h*2;
    for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)image.pixels.push_back(physical[y*stride+x]);
    mnm::render::CanvasSequence canvas;canvas.create(1,w,h);canvas.update(1,0,0,image);canvas.panel(1,r,mode,percent,pressed,reverse);const auto result=canvas.read(1);
    for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)physical[y*stride+x]=result.pixels[y*w+x];
    out.write(reinterpret_cast<const char *>(physical.data()),stride*h*2);
  }
  if(at!=b.size() || !out)throw std::runtime_error("Input/output extent");
  std::cout<<atomic()<<" atomic late-undefined panel refusals pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
