#include "../reconstruction/casting/descriptor.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_CAST_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::casting;
int main(int argc, char **argv) try {
#ifdef MNM_CAST_REFERENCE
  if (argc != 2) throw std::runtime_error("pinned PE required");
  map_image(read(argv[1]));
  std::array<unsigned char, 144> body{};
  std::memcpy(body.data(), reinterpret_cast<void *>(0x534000), body.size());
#else
  (void)argc; (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](std::uint32_t word) { digest = (digest ^ word) * 1099511628211ull; };
  for (unsigned n = 0; n < 65536; ++n) {
    std::array<unsigned char, 49> guarded{};
    guarded.fill(0xa7);
    PackedCastCommand input{};
    std::uint32_t random = n ^ 0x971369abu;
    for (auto &v : input) { random = random * 1664525u + 1013904223u; v = random >> 24; }
    input[25] = n & 255u; input[26] = n >> 8;
    //Every WORD field independently includes both extreme values.
    for (unsigned j = 0; j < 6; ++j)
      if ((n + j) % 13 == 0 || (n + j) % 13 == 1)
        input[13 + j * 2] = input[14 + j * 2] = (n + j) % 13 == 0 ? 0 : 255;
    std::memcpy(guarded.data() + 8, input.data(), input.size());
    const auto inputBefore = guarded;
    const auto packedBefore = input;
    CastDescriptor output{};
    for (auto &v : output) { random = random * 1664525u + 1013904223u; v = random; }
    output[9] = n % 3 == 0 ? 0 : n % 3 == 1 ? UINT32_MAX : random;
    const auto outputBefore = output;
    expandCastDescriptor(input, output);
    if (output[9] != outputBefore[9] || input != packedBefore)
      throw std::runtime_error("caller/input preservation");
    if (output[7] != (input[25] == 255 ? UINT32_MAX : input[25]) ||
        output[8] != (input[26] == 255 ? UINT32_MAX : input[26]))
      throw std::runtime_error("independent sentinel");
#ifdef MNM_CAST_REFERENCE
    std::array<unsigned char, 64> actual{}, expected{};
    actual.fill(0xd3); expected.fill(0xd3);
    std::memcpy(actual.data() + 8, outputBefore.data(), 48);
    std::memcpy(expected.data() + 8, output.data(), 48);
    using Decode = void *(__attribute__((thiscall)) *)(void *, void *);
    if (reinterpret_cast<Decode>(0x534000)(guarded.data() + 8, actual.data() + 8) != actual.data() + 8 || actual != expected)
      throw std::runtime_error("original descriptor/return/guard mismatch case=" + std::to_string(n));
    if (guarded != inputBefore || std::memcmp(body.data(), reinterpret_cast<void *>(0x534000), body.size()))
      throw std::runtime_error("original input/body changed");
#else
    if (guarded != inputBefore) throw std::runtime_error("input guard");
#endif
    for (auto v : output) mix(v);
  }
  std::cout << "{\"success\":true,\"cases\":65536,\"functions\":1,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
