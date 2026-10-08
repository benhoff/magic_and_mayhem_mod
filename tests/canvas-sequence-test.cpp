#include "../compat/legacy/canvas_producers.hpp"
#include "../renderer/canvas_sequence.hpp"
#include <iostream>
#include <algorithm>
#include <stdexcept>
namespace {
void expect(bool b) {
  if (!b)
    throw std::runtime_error("Canvas sequence assertion failed");
}
template <class F> void refuses(F f) {
  bool failed = false;
  try {
    f();
  } catch (const std::exception &) {
    failed = true;
  }
  expect(failed);
}
} // namespace
int main() {
  try {
    using namespace mnm;
    render::CanvasSequence canvases;
    canvases.create(1, 4, 2);
    refuses([&] { canvases.read(1); });
    canvases.fill(1, {0, 0, 4, 2}, 0xffff);
    canvases.update(1, 0, 0, {2, 1, {1, 2}});
    canvases.copy(1, 1, {0, 0, 3, 1}, 1, 0);
    expect(canvases.read(1).pixels ==
           std::vector<std::uint32_t>(
               {1, 1, 2, 65535, 65535, 65535, 65535, 65535}));
    canvases.create(2, 4, 2);
    canvases.fill(2, {0, 0, 4, 2}, 123);
    canvases.copy(1, 2, {0, 0, 4, 2}, 0, 0, 65535);
    expect(canvases.read(2).pixels ==
           std::vector<std::uint32_t>({1, 1, 2, 123, 123, 123, 123, 123}));
    assets::SpriteFrame glyph;
    glyph.width = 2;
    glyph.height = 1;
    glyph.opaqueMask = {1, 1};
    glyph.pixels = std::vector<std::uint8_t>{0, 32};
    std::array<float, 64> coverage{};
    for (unsigned i = 0; i < 64; ++i)
      coverage[i] = float(i) / 63;
    canvases.fill(1, {0, 0, 4, 2}, 0xffff);
    canvases.glyph(1, glyph, -1, 0, {0, 0, 4, 2}, {0, 0, 0}, coverage);
    expect(canvases.read(1).pixels[0] == 65535);
    canvases.glyph(1, glyph, 0, 0, {0, 0, 4, 2}, {0, 0, 0}, coverage);
    expect(canvases.read(1).pixels[0] == 65535);
    expect(canvases.read(1).pixels[1] == ((15u << 11) | (30u << 5) | 15));
    canvases.create(3, 4, 2);
    refuses([&] { canvases.copy(3, 1, {0, 0, 4, 2}, 0, 0); });
    refuses([&] {
      canvases.glyph(3, glyph, 0, 0, {0, 0, 4, 2}, {0, 0, 0}, coverage);
    });
    canvases.release(3);
    refuses([&] { canvases.read(3); });
    refuses(
        [&] { legacy::decodeCanvasProducers(std::vector<std::uint8_t>(64)); });
    // Incremental queue boundaries must match full decoding across arbitrary
    // batches, and duplicate sequence/changed envelope must be rejected.
    std::vector<std::uint8_t> wire(64+192,0);
    auto put=[&](unsigned p,unsigned v){for(unsigned k=0;k<4;++k)wire[p+k]=std::uint8_t(v>>(k*8));};
    const char magic[]="MNMPRO01";std::copy_n(magic,8,wire.begin());put(8,1);put(12,64);put(16,0x40209ca7);put(20,65536);put(24,1);
    for(unsigned i=0;i<2;++i){put(64+i*96,96);put(68+i*96,i+1);put(72+i*96,i?12:11);put(120+i*96,1);}
    auto full=legacy::decodeCanvasProducers(wire);legacy::CanvasProducerStream incremental{0,{}};
    std::vector<std::uint8_t> first(wire.begin(),wire.begin()+160),last(wire.begin(),wire.begin()+64);last.insert(last.end(),wire.begin()+160,wire.end());
    legacy::appendCanvasProducers(incremental,first);legacy::appendCanvasProducers(incremental,last,true);
    expect(incremental.operations.size()==full.operations.size());expect(incremental.operations.back().fields==full.operations.back().fields);
    refuses([&]{legacy::appendCanvasProducers(incremental,last);});
    first[24]=2;refuses([&]{legacy::appendCanvasProducers(incremental,first);});
    std::cout << "Undefined input, retained self-copy, colour key, font "
                 "clipping/truncation and malformed stream checks passed\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
