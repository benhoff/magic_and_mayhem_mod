#include "../renderer/canvas_sequence.hpp"
#include "../reconstruction/rendering/fade_span.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
using Bytes=std::vector<unsigned char>;
static unsigned u32(const Bytes &b,std::size_t at){if(at+4>b.size())throw std::runtime_error("Fade extent");unsigned v;std::memcpy(&v,b.data()+at,4);return v;}
int main(int argc,char **argv) try {
  if(argc!=3)throw std::runtime_error("Expected fade corpus, output");
  std::ifstream in(argv[1],std::ios::binary);const Bytes b{std::istreambuf_iterator<char>(in),{}};
  if(b.size()<12 || std::memcmp(b.data(),"MNMFADE1",8))throw std::runtime_error("Fade corpus header");
  std::ofstream out(argv[2],std::ios::binary);std::size_t at=12;
  for(unsigned c=0;c<u32(b,8);++c){
    const auto w=u32(b,at),h=u32(b,at+4),stride=u32(b,at+8),format=u32(b,at+12);at+=16;
    const auto count=mnm::reconstruction::rendering::fadeWordCount(w,h,stride);
    if(at+std::size_t(stride)*h*2>b.size())throw std::runtime_error("Fade plane extent");
    std::vector<std::uint16_t> plane(stride*h);std::memcpy(plane.data(),b.data()+at,plane.size()*2);at+=plane.size()*2;
    mnm::render::Image image{int(w),int(h),{}};for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)image.pixels.push_back(plane[y*stride+x]);
    mnm::render::CanvasSequence canvas;canvas.create(1,w,h);canvas.update(1,0,0,image);
    mnm::reconstruction::rendering::fadePhysicalWords(plane,w,h,stride,std::int32_t(format));
    canvas.fade(1,stride,count,format?0x7def:0x7bef);
    out.write(reinterpret_cast<const char *>(plane.data()),plane.size()*2);
    for(auto p:canvas.read(1).pixels){out.put(char(p));out.put(char(p>>8));}
  }
  if(at!=b.size() || !out)throw std::runtime_error("Fade output/input extent");
  std::cout<<"Native physical words and owned canvas projection pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
