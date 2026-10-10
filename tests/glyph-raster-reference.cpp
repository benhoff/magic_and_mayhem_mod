// Unmodified pinned PE32 glyph entry; private source/destination storage only.
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include <algorithm>
int main(int argc, char **argv) try {
  if (argc != 4) throw std::runtime_error("Expected PE, glyph cases, output");
  map_image(read(argv[1]));
  if (std::memcmp(reinterpret_cast<void *>(0x581ec0), "\x83\xec\x28\x8b\x44\x24\x30", 7))
    throw std::runtime_error("Original glyph entry changed");
  const auto input = read(argv[2]);
  if (input.size() < 12 || std::memcmp(input.data(), "MNMGLY01", 8))
    throw std::runtime_error("Glyph corpus header");
  std::ofstream output(argv[3], std::ios::binary);
  std::size_t at = 12;
  for (unsigned n = 0; n < u32(input, 8); ++n) {
    const auto bytes = u32(input, at), size = u32(input, at+4);
    const auto width = u32(input, at+8), height = u32(input, at+12), stride = u32(input, at+16);
    if (!width || width > 256 || !height || height > 256 || stride < width || stride > 260 ||
        size < 40 || bytes != 60+256+size || bytes > input.size()-at)
      throw std::runtime_error("Glyph corpus extent");
    Bytes frame(input.begin()+at+316, input.begin()+at+bytes);
    const auto source = frame;
    std::vector<std::uint16_t> pixels(64+stride*height, 0xa55a);
    for (unsigned y=0; y<height; ++y) for (unsigned x=0; x<width; ++x)
      pixels[32+y*stride+x] = std::uint16_t((y*width+x)*1273+u32(input,at+56));
    const auto before = pixels;
    global(0x658174, reinterpret_cast<std::uintptr_t>(pixels.data()+32));
    global(0x6a2dc8, stride); global(0x6e1f88, 0);
    *reinterpret_cast<std::uint16_t *>(0x6e1f7c)=0xf800;
    *reinterpret_cast<std::uint16_t *>(0x6e1f7e)=0x07e0;
    *reinterpret_cast<std::uint16_t *>(0x6e1f80)=0x001f;
    for (unsigned i=0; i<4; ++i)
      global(i==0?0x6e0008:i==1?0x6cbb6c:i==2?0x6a49b8:0x656618,u32(input,at+28+i*4));
    std::memcpy(reinterpret_cast<void *>(0x6f5dac),input.data()+at+60,256);
    const float initialized=1;
    std::memcpy(reinterpret_cast<void *>(0x6f5eac),&initialized,4);
    using Draw = unsigned(__attribute__((fastcall)) *)(void *, int, int, int, int, int);
    reinterpret_cast<Draw>(0x581ec0)(frame.data(),std::int32_t(u32(input,at+20)),
        std::int32_t(u32(input,at+24)),u32(input,at+44),u32(input,at+48),u32(input,at+52));
    if (frame != source) throw std::runtime_error("Original glyph modified source");
    for (unsigned i=0; i<pixels.size(); ++i) {
      const bool guard=i<32 || i>=32+stride*height || (i-32)%stride>=width;
      if (guard && pixels[i]!=before[i]) throw std::runtime_error("Original glyph wrote guard/padding");
    }
    for (unsigned y=0; y<height; ++y)
      output.write(reinterpret_cast<const char *>(pixels.data()+32+y*stride),width*2);
    at+=bytes;
  }
  if (at!=input.size() || !output) throw std::runtime_error("Glyph corpus/output extent");
  std::cout << "Original glyph canvases, source guards and row padding pass\n";
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
