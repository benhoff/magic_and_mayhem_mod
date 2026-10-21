#include "../reconstruction/rendering/alternate_canvas_x87.hpp"
#include <array>
#include <cstring>
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using U = std::uint32_t;
int main(int argc, char** argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2) throw std::runtime_error("Expected original image");
  map_image(read(argv[1]));
  const auto* code = reinterpret_cast<const unsigned char*>(0x59455c);
  const Bytes unchanged(code, code + 144);
#else
  (void)argv;
  if (argc != 1) throw std::runtime_error("Unexpected native argument");
#endif
  const std::array<std::uint64_t, 12> patterns{{
    0, 0x8000000000000000ULL, 0x7ff0000000000000ULL,
    0xfff0000000000000ULL, 0x7ff8000000001234ULL,
    0x7ff0000000001234ULL, 0xfff0000000004321ULL,
    1, 0x800fffffffffffffULL, 0x0010000000000000ULL,
    0x7fefffffffffffffULL, 0xffffffffffffffffULL}};
  U id = 0;
  for (U width : {32u, 64u, 96u})
    for (U height : {1u, 2u, 5u})
      for (int padding : {-32, -4, -1, 0, 1, 4, 32})
        for (int delta : {-33, -16, -4, -1, 0, 1, 4, 16, 31, 32, 33, 256})
          for (U pattern = 0; pattern < patterns.size() + 2; ++pattern) {
            std::vector<std::uint16_t> words(2048);
            for (unsigned i = 0; i < words.size(); ++i)
              words[i] = std::uint16_t(i * 7919 + (i >> 3) * 13);
            if (pattern < patterns.size())
              for (unsigned i = 0; i < words.size(); i += 4)
                std::memcpy(words.data() + i, &patterns[pattern], 8);
            else if (pattern == patterns.size() + 1)
              for (unsigned i = 0; i < words.size(); i += 4)
                std::memcpy(words.data() + i, &patterns[(i / 4) % patterns.size()], 8);
            unsigned status = 0;
#ifdef MNM_ORIGINAL_REFERENCE
            global(0x6e1f6c, U(reinterpret_cast<std::uintptr_t>(words.data() + 512)));
            global(0x658174, U(reinterpret_cast<std::uintptr_t>(words.data() + 512 + delta)));
            global(0x6a2dc8, width); global(0x6def50, height);
            global(0x6e1f70, U(padding));
            asm volatile("fninit");
            reinterpret_cast<void(*)()>(0x59455c)();
            unsigned short sw, cw;
            asm volatile("fnstsw %0; fnstcw %1" : "=m"(sw), "=m"(cw));
            if ((sw & 0x3800) || cw != 0x037f)
              throw std::runtime_error("Original x87 stack/control changed");
            status = sw & 0x3f;
#else
            status = mnm::reconstruction::copyAlternateX87Canvas(
                words, 512, 512 + delta, width, height, padding);
#endif
            const U header[]{id++, width, height, U(padding), U(delta), pattern, status};
            std::cout.write(reinterpret_cast<const char*>(header), sizeof(header));
            std::cout.write(reinterpret_cast<const char*>(words.data()), words.size() * 2);
          }
#ifdef MNM_ORIGINAL_REFERENCE
  if (std::memcmp(code, unchanged.data(), unchanged.size()))
    throw std::runtime_error("Original x87 copy body changed");
  asm volatile("fninit");
#else
  unsigned refusals = 0;
  for (unsigned kind = 0; kind < 8; ++kind) {
    std::vector<std::uint16_t> words(128, 0x1234);
    const auto before = words;
    try {
      mnm::reconstruction::copyAlternateX87Canvas(words,
          kind == 3 ? 120 : 0, kind == 4 ? 120 : 0,
          kind == 0 ? 0 : kind == 1 ? 31 : 32,
          kind == 2 ? 0 : kind == 6 ? 0xffffffffu : 2,
          kind == 5 ? -32 : kind == 7 ? 0x7fffffff : 0);
    } catch (const std::invalid_argument&) { ++refusals; }
    if (words != before) throw std::runtime_error("Refusal mutated storage");
  }
  if (refusals != 8) throw std::runtime_error("Refusal denominator");
#endif
  if (id != 10584) throw std::runtime_error("x87 case denominator");
  return std::cout ? 0 : 1;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n'; return 1;
}
