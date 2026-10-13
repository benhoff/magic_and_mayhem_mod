#include "descriptor.hpp"
namespace mnm::reconstruction::casting {
void expandCastDescriptor(const PackedCastCommand &input, CastDescriptor &output) {
  output[0] = input[12];
  for (unsigned i = 0; i < 6; ++i) {
    const unsigned offset = 13 + 2 * i;
    output[i + 1] = std::uint32_t(input[offset]) |
                    (std::uint32_t(input[offset + 1]) << 8);
  }
  for (unsigned i = 0; i < 2; ++i)
    output[i + 7] = input[i + 25] == 255 ? UINT32_MAX : input[i + 25];
  output[10] = std::uint32_t(input[28]) | (std::uint32_t(input[29]) << 8) |
               (std::uint32_t(input[30]) << 16) | (std::uint32_t(input[31]) << 24);
  output[11] = input[32];
}
}
