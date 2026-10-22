#include "../reconstruction/rendering/alternate_canvas_copy.hpp"
#include <iostream>
#ifdef MNM_ORIGINAL_REFERENCE
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction;
using U = std::uint32_t;
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  if (argc != 2)
    throw std::runtime_error("Expected original executable");
  map_image(read(argv[1]));
  const auto *code = reinterpret_cast<const unsigned char *>(0x594508);
  const Bytes unchanged(code, code + 84);
#else
  (void)argv;
  if (argc != 1)
    throw std::runtime_error("Unexpected native input");
#endif
  U id = 0;
  for (U width : {4u, 8u, 16u, 32u})
    for (U height : {1u, 2u, 5u})
      for (int padding : {-4, -1, 0, 1, 4})
        for (int delta : {-5, -3, -1, 0, 1, 2, 3, 4, 5, 64, 256})
          for (U pattern = 0; pattern < 3; ++pattern) {
            std::vector<std::uint16_t> storage(1024);
            for (U i = 0; i < 1024; ++i)
              storage[i] = std::uint16_t(pattern == 0   ? i * 7919 + 3
                                         : pattern == 1 ? 0
                                                        : i ^ (i << 7));
#ifdef MNM_ORIGINAL_REFERENCE
            global(0x658174, U(reinterpret_cast<std::uintptr_t>(storage.data() +
                                                                64 + delta)));
            global(0x6e1f6c,
                   U(reinterpret_cast<std::uintptr_t>(storage.data() + 64)));
            global(0x6a2dc8, width);
            global(0x6def50, height);
            global(0x6e1f70, U(padding));
            reinterpret_cast<void (*)()>(0x594508)();
#else
            copyAlternateCanvas(storage, 64, 64 + delta, width, height,
                                padding);
#endif
            const U header[]{id++,       width,    height,
                             U(padding), U(delta), pattern};
            std::cout.write(reinterpret_cast<const char *>(header),
                            sizeof(header));
            std::cout.write(reinterpret_cast<const char *>(storage.data()),
                            2048);
          }
#ifdef MNM_ORIGINAL_REFERENCE
  if (std::memcmp(code, unchanged.data(), unchanged.size()))
    throw std::runtime_error("Original alternate copy changed");
#else
  unsigned refusals = 0;
  for (U kind = 0; kind < 4; ++kind) {
    std::vector<std::uint16_t> words(64, 0x1234);
    const auto before = words;
    try {
      copyAlternateCanvas(words, kind == 3 ? 60 : 0, 0,
                          kind == 0   ? 0
                          : kind == 1 ? 3
                                      : 8,
                          kind == 2 ? 0 : 2, 0);
    } catch (const std::invalid_argument &) {
      ++refusals;
    }
    if (words != before)
      throw std::runtime_error("Alternate refusal mutated storage");
  }
  if (refusals != 4)
    throw std::runtime_error("Alternate extent refusal count");
#endif
  if (id != 1980)
    throw std::runtime_error("Alternate copy denominator");
  return std::cout ? 0 : 1;
} catch (const std::exception &error) {
  std::cerr << error.what() << '\n';
  return 1;
}
