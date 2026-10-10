#include "../renderer/canvas_sequence.hpp"
#include <cmath>
#include <iostream>
#include <limits>
namespace {
void expect(bool value) { if (!value) throw std::runtime_error("Glyph admission assertion"); }
template<class F> void refuses(F f) {
  bool refused=false;
  try {f();} catch(const std::exception &) {refused=true;}
  expect(refused);
}
}
int main() try {
  using namespace mnm;
  assets::SpriteFrame f;f.width=3;f.height=2;f.opaqueMask={1,1,1,1,1,1};
  f.pixels=std::vector<std::uint8_t>{63,32,1,2,3,4};
  std::array<float,64> coverage{};
  for(unsigned i=0;i<64;++i)coverage[i]=float(i)/63;
  render::CanvasSequence canvas;canvas.create(1,4,3);canvas.fill(1,{0,0,4,3},0xffff);
  const auto before=canvas.read(1).pixels;
  auto bad=f;std::get<std::vector<std::uint8_t>>(bad.pixels)[5]=64;
  refuses([&]{canvas.glyph(1,bad,0,0,{0,0,4,3},{0,0,0},coverage);});
  expect(canvas.read(1).pixels==before);
  // Malformed cropped rows still refuse atomically as an explicit native policy.
  refuses([&]{canvas.glyph(1,bad,0,0,{0,0,4,1},{0,0,0},coverage);});
  expect(canvas.read(1).pixels==before);
  for(float value:{-0.01f,std::nextafter(1.f,2.f),std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
    auto invalid=coverage;invalid[63]=value;
    refuses([&]{canvas.glyph(1,f,0,0,{0,0,4,3},{0,0,0},invalid);});
    expect(canvas.read(1).pixels==before);
  }
  refuses([&]{canvas.glyph(1,f,0,0,{0,0,4,3},{256,0,0},coverage);});
  expect(canvas.read(1).pixels==before);
  canvas.create(2,4,3);canvas.fill(2,{0,0,4,1},0xffff);
  refuses([&]{canvas.glyph(2,f,0,0,{0,0,4,3},{0,0,0},coverage);});
  // Define the previously missing rows to inspect the first row after refusal.
  canvas.fill(2,{0,1,4,3},0xffff);expect(canvas.read(2).pixels==before);
  canvas.glyph(1,f,-1,0,{0,0,4,3},{0,0,0},coverage);expect(canvas.read(1).pixels==before);
  // Transparent samples never read undefined storage, regardless of their index.
  canvas.create(3,4,3);bad.opaqueMask.assign(6,0);
  canvas.glyph(3,bad,0,0,{0,0,4,3},{0,0,0},coverage);
  canvas.fill(3,{0,0,4,3},123);expect(canvas.read(3).pixels==std::vector<std::uint32_t>(12,123));
  std::cout<<"8 atomic glyph refusals and transparent/horizontal no-ops pass\n";
  return 0;
} catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
