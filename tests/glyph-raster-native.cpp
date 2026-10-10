#include "../renderer/canvas_sequence.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
namespace {
using Bytes=std::vector<std::uint8_t>;
std::uint32_t word(const Bytes &b, std::size_t at) {
  if (at>b.size() || b.size()-at<4) throw std::runtime_error("Glyph input extent");
  return b[at]|std::uint32_t(b[at+1])<<8|std::uint32_t(b[at+2])<<16|std::uint32_t(b[at+3])<<24;
}
void put(Bytes &b, unsigned at, unsigned value) {
  for (unsigned i=0;i<4;++i) b.at(at+i)=std::uint8_t(value>>(8*i));
}
}
int main(int argc,char **argv) try {
  if (argc!=3) throw std::runtime_error("Expected glyph cases, output");
  std::ifstream in(argv[1],std::ios::binary);
  const Bytes input{std::istreambuf_iterator<char>(in),{}};
  if (input.size()<12 || std::memcmp(input.data(),"MNMGLY01",8)) throw std::runtime_error("Glyph corpus header");
  std::ofstream output(argv[2],std::ios::binary);
  std::size_t at=12;
  for (unsigned n=0;n<word(input,8);++n) {
    const auto bytes=word(input,at),size=word(input,at+4),w=word(input,at+8),h=word(input,at+12);
    if (bytes!=316+size || bytes>input.size()-at) throw std::runtime_error("Glyph record extent");
    // Native loader decodes the independently encoded source RLE and mask.
    Bytes spr(796+size);
    put(spr,0,0x00525053);put(spr,4,spr.size());put(spr,8,4);put(spr,12,1);put(spr,16,1);
    std::copy(input.begin()+at+316,input.begin()+at+bytes,spr.begin()+796);put(spr,796+28,0);
    auto decoded=mnm::assets::decodeSprite(spr);
    if (auto *e=std::get_if<mnm::assets::SpriteError>(&decoded)) throw std::runtime_error(e->detail);
    const auto &frame=std::get<mnm::assets::Sprite>(decoded).frames.at(0);
    mnm::render::Image image{int(w),int(h),{}};
    for (unsigned i=0;i<w*h;++i) image.pixels.push_back(std::uint16_t(i*1273+word(input,at+56)));
    mnm::render::CanvasSequence canvas;canvas.create(1,w,h);canvas.update(1,0,0,image);
    std::array<float,64> coverage{};std::memcpy(coverage.data(),input.data()+at+60,256);
    canvas.glyph(1,frame,std::int32_t(word(input,at+20)),std::int32_t(word(input,at+24)),
      {std::int32_t(word(input,at+28)),std::int32_t(word(input,at+32)),std::int32_t(word(input,at+36)),std::int32_t(word(input,at+40))},
      {word(input,at+44),word(input,at+48),word(input,at+52)},coverage);
    for (auto p:canvas.read(1).pixels) {output.put(char(p));output.put(char(p>>8));}
    at+=bytes;
  }
  if (at!=input.size() || !output) throw std::runtime_error("Glyph corpus/output extent");
  std::cout << "Native glyph canvases pass\n";
  return 0;
} catch (const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
