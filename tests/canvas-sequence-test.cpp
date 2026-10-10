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
mnm::legacy::CanvasProducer encodedFrame(bool indexed = false) {
  mnm::legacy::CanvasProducer c{};
  c.payload.resize(indexed ? 52 : 54);
  auto put = [&](unsigned at, unsigned value) {
    for (unsigned k = 0; k < 4; ++k) c.payload[at+k] = std::uint8_t(value >> (k*8));
  };
  c.fields[19] = c.payload.size();
  put(0, c.payload.size()); put(4, 2); put(8, 1);
  put(40, 48); put(44, 50); c.payload[48] = 0; c.payload[49] = 2;
  if (indexed) { c.payload[50] = 1; c.payload[51] = 2; }
  else { c.payload[50] = 0; c.payload[51] = 0xf8; c.payload[52] = 0x1f; c.payload[53] = 0; }
  return c;
}
void decodedFrameCache() {
  using namespace mnm;
  legacy::CanvasFrameCache cache({2, 4096});
  auto a = encodedFrame(); const auto first = cache.frame(a, false);
  expect(cache.frame(a, false) == first && cache.stats().decodes == 1 && cache.stats().hits == 1);
  legacy::CanvasFrameCache callerOwned;
  const auto retained = callerOwned.frame(a, false);
  a.payload[50] = 0x99;
  expect(callerOwned.frame(a, false) != retained);
  a.payload[50] = 0;
  expect(callerOwned.frame(a, false) == retained && callerOwned.stats().decodes == 2);
  // Caller mutation changes the exact key; both key and decoded output own data.
  auto b = a; b.payload[50] = 0x34;
  auto second = cache.frame(b, false);
  expect(second != first && std::get<std::vector<std::uint16_t>>(first->pixels)[0] == 0xf800);
  cache.frame(a, false); // A becomes most recent: C must evict B, not A.
  auto c = a; c.payload[50] = 0x56; cache.frame(c, false);
  expect(cache.stats().evictions == 1 && cache.stats().frames == 2);
  expect(cache.frame(a, false) == first);
  expect(cache.frame(b, false) != second && cache.stats().decodes == 4);
  expect(std::get<std::vector<std::uint16_t>>(second->pixels)[0] == 0xf834); // survives eviction
  auto malformed = a; malformed.payload[40] = 39;
  const auto before = cache.stats();
  refuses([&] { cache.frame(malformed, false); });
  refuses([&] { cache.frame(malformed, false); });
  expect(cache.stats().decodes == before.decodes && cache.stats().evictions == before.evictions);
  malformed = a; malformed.payload.pop_back(); refuses([&] { cache.frame(malformed, false); });
  malformed = a; malformed.fields[19]++; refuses([&] { cache.frame(malformed, false); });
  malformed = a; malformed.payload[28] = 1; refuses([&] { cache.frame(malformed, false); });
  legacy::CanvasFrameCache measure;
  measure.frame(a, false); const auto bytes = measure.stats().bytes;
  legacy::CanvasFrameCache bounded({4, bytes}); bounded.frame(a, false); bounded.frame(b, false);
  expect(bounded.stats().frames == 1 && bounded.stats().bytes == bytes && bounded.stats().evictions == 1);
  legacy::CanvasFrameCache bypass({1, bytes-1}); bypass.frame(a, false); bypass.frame(a, false);
  expect(bypass.stats().decodes == 2 && bypass.stats().bypasses == 2 && !bypass.stats().frames && !bypass.stats().bytes);
  // One zero-sized encoded frame is valid with either storage tag.
  legacy::CanvasProducer empty{}; empty.payload.resize(40); empty.payload[0] = 40; empty.fields[19] = 40;
  legacy::CanvasFrameCache tagged;
  const auto direct = tagged.frame(empty, false), indexed = tagged.frame(empty, true);
  expect(direct != indexed && tagged.stats().decodes == 2);
  expect(std::holds_alternative<std::vector<std::uint16_t>>(direct->pixels));
  expect(std::holds_alternative<std::vector<std::uint8_t>>(indexed->pixels));
  refuses([] { legacy::CanvasFrameCache invalid({0, 1}); });
  refuses([] { legacy::CanvasFrameCache invalid({4097, 1}); });
  refuses([] { legacy::CanvasFrameCache invalid({1, 64 * 1024 * 1024 + 1}); });
  // Effect palettes are draw state, not part of the decoded-source cache.
  legacy::CanvasProducerReplay replay([](const auto &) -> std::vector<std::uint8_t> { throw std::runtime_error("Unexpected asset"); });
  legacy::CanvasProducer create{}; create.fields[2]=1; create.fields[3]=7; create.fields[5]=2; create.fields[6]=1; replay.apply(create);
  auto draw=encodedFrame(true); auto &r=draw.fields;
  r[2]=9; r[3]=7; r[12]=2; r[13]=1; r[15]=1; r[17]=1; r[20]=512; draw.payload.resize(r[19]+512);
  draw.payload[r[19]+2]=0x34; draw.payload[r[19]+3]=0x12; replay.apply(draw);
  expect(replay.read(7).pixels[0]==0x1234);
  draw.payload[r[19]+2]=0x78; draw.payload[r[19]+3]=0x56; replay.apply(draw);
  expect(replay.read(7).pixels[0]==0x5678 && replay.frameCacheStats().decodes==1 && replay.frameCacheStats().hits==1);
}
void rasterSampling() {
  using namespace mnm;
  const render::Image initial{6, 2, {0xffff,0xf800,0x07e0,0x001f,0x1234,0x5678,
                                    0x0101,0x0202,0x0303,0x0404,0x0505,0x0606}};
  assets::SpriteFrame sprite; sprite.width=4; sprite.height=2;
  sprite.opaqueMask={1,1,0,1,1,0,1,1}; sprite.pixels=std::vector<std::uint16_t>(8,0x4210);
  std::array<std::uint16_t,256> palette{}; std::array<int,16> shifts{}; shifts[0]=1; shifts[1]=0;
  for (unsigned mode : {0u,1u,2u,3u,4u}) {
    render::CanvasSequence canvas; canvas.create(1,6,2); canvas.update(1,0,0,initial);
    // Independent pre-draw snapshot oracle, including transparent and clipped samples.
    auto expected=initial;
    for (unsigned draw=0; draw<2; ++draw) {
    const auto snapshot=expected;
    for (unsigned y=0;y<2;++y) for(unsigned x=0;x<4;++x) {
      const unsigned dx=x+1, at=y*6+dx;
      if(dx>=4 || !sprite.opaqueMask[y*4+x]) continue;
      const auto source=0x4210u, destination=snapshot.pixels[at], mask=0x7befu;
      auto value=source;
      if(mode==1)value=((source>>1)&mask)+((destination>>1)&mask);
      if(mode==2)value=((destination>>1)&mask)+((destination>>2)&(mask>>1)&mask)+((source>>2)&(mask>>1)&mask);
      if(mode==4)value=((source>>1)&mask)+((source>>2)&(mask>>1)&mask)+((destination>>2)&(mask>>1)&mask);
      if(mode==3)value=snapshot.pixels[at+shifts[y]];
      expected.pixels[at]=value;
    }
    canvas.raster(1,sprite,1,0,{1,0,4,2},mode,1,palette,shifts,2);
    expect(canvas.read(1).pixels==expected.pixels);
    }
  }
  render::CanvasSequence canvas; canvas.create(1,6,2); canvas.update(1,0,0,initial);
  shifts[0]=-1; refuses([&] { canvas.raster(1,sprite,1,0,{0,0,6,2},3,1,palette,shifts,2); });
  expect(canvas.read(1).pixels==initial.pixels);
  shifts[0]=16; refuses([&] { canvas.raster(1,sprite,1,0,{0,0,6,2},3,1,palette,shifts,2); });
  // An undefined destination remains refused even when its displacement source is defined.
  render::CanvasSequence undefined; undefined.create(1,6,2); undefined.fill(1,{2,0,6,2},123);
  shifts[0]=1; refuses([&] { undefined.raster(1,sprite,1,0,{0,0,6,2},3,1,palette,shifts,2); });
  render::CanvasSequence missingSample; missingSample.create(1,6,2); missingSample.fill(1,{1,0,2,1},123);
  refuses([&] { missingSample.raster(1,sprite,1,0,{0,0,6,2},3,1,palette,shifts,2); });
  // Largest admitted offset still reads untouched pixels in the same row.
  render::CanvasSequence wide; wide.create(1,20,1);
  render::Image row{20,1,{}}; for(unsigned i=0;i<20;++i)row.pixels.push_back(i);
  wide.update(1,0,0,row); sprite.width=2; sprite.height=1; sprite.opaqueMask={1,1}; sprite.pixels=std::vector<std::uint16_t>{0,0};
  shifts.fill(16); wide.raster(1,sprite,1,0,{0,0,20,1},3,1,palette,shifts,1);
  row.pixels[1]=17; row.pixels[2]=18; expect(wide.read(1).pixels==row.pixels);
}
} // namespace
int main() {
  try {
    using namespace mnm;
    decodedFrameCache(); rasterSampling();
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
    std::cout << "Bounded decoded-frame reuse, raster sample ordering, undefined input, retained self-copy, colour key, font "
                 "clipping/truncation and malformed stream checks passed\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
