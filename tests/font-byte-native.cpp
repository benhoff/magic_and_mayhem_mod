#include "../compat/legacy/font_text.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
using Bytes=std::vector<std::uint8_t>;
std::uint32_t word(const Bytes &b,std::size_t at) {
  if (at>b.size() || b.size()-at<4) throw std::runtime_error("Text extent");
  return b[at]|std::uint32_t(b[at+1])<<8|std::uint32_t(b[at+2])<<16|std::uint32_t(b[at+3])<<24;
}
int main(int argc,char **argv) try {
  if (argc!=3) throw std::runtime_error("Expected corpus, output");
  std::ifstream in(argv[1],std::ios::binary);const Bytes b{std::istreambuf_iterator<char>(in),{}};
  if (b.size()<12 || std::memcmp(b.data(),"MNMTXT01",8)) throw std::runtime_error("Text header");
  std::ofstream out(argv[2],std::ios::binary);
  auto put=[&](std::uint32_t v){for(unsigned k=0;k<4;++k)out.put(char(v>>(8*k)));};
  std::size_t at=12;
  for (unsigned n=0;n<word(b,8);++n) {
    const auto size=word(b,at),fontSize=word(b,at+4),count=word(b,at+8),draw=word(b,at+12);
    Bytes raw(b.begin()+at+56,b.begin()+at+56+fontSize);
    auto decoded=mnm::assets::decodeSft(raw);
    if (auto *e=std::get_if<mnm::assets::SftError>(&decoded)) throw std::runtime_error(e->detail);
    const auto rows=word(raw,16);mnm::compat::FontText text(std::get<mnm::assets::SftFont>(std::move(decoded)));
    mnm::reconstruction::rendering::FontCursor cursor;
    cursor.x=std::int32_t(word(b,at+32));cursor.y=std::int32_t(word(b,at+36));
    cursor.tracking=std::int32_t(word(b,at+24));cursor.tabAdvance=std::int32_t(word(b,at+28));
    for(unsigned r=0;r<rows;++r)cursor.trailing.push_back(std::int32_t(word(b,at+56+fontSize+r*4)));
    mnm::render::CanvasSequence canvas;const auto w=word(b,at+48),h=word(b,at+52);
    if (draw) {
      canvas.create(1,w,h);mnm::render::Image image{int(w),int(h),{}};
      for(unsigned i=0;i<w*h;++i)image.pixels.push_back(std::uint16_t(i*1273+113));
      canvas.update(1,0,0,image);
    }
    std::array<float,64> coverage{};for(unsigned k=0;k<64;++k)coverage[k]=k/64.f;
    for(unsigned i=0;i<count;++i) {
      const auto byte=std::uint8_t(word(b,at+56+fontSize+rows*4+i*4));std::int32_t result=0;
      if(draw) text.draw(canvas,1,cursor,byte,std::int32_t(word(b,at+20)),
          {0,int(word(b,at+44)),int(w),int(h)},{173,91,237},coverage);
      else result=mnm::reconstruction::rendering::fontByteAdvance(text.contours(),cursor,byte,
          word(b,at+16)!=0,std::int32_t(word(b,at+20)));
      put(result);put(cursor.x);put(cursor.y);for(auto r:cursor.trailing)put(r);
    }
    if(draw)for(auto pixel:canvas.read(1).pixels){out.put(char(pixel));out.put(char(pixel>>8));}
    at+=size;
  }
  if(at!=b.size() || !out)throw std::runtime_error("Text output/input extent");
  std::cout<<"Native owned byte text producer pass\n";return 0;
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
