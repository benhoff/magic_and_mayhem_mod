#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace mnm::reconstruction {
// No-CD 59455c. Owned WORD storage, positive rows and width divisible by 32.
// Models masked binary64 x87 loads/stores with four free stack slots. Returns
// the raised invalid/denormal flags; control/status/stack ownership is external.
inline unsigned copyAlternateX87Canvas(std::vector<std::uint16_t>& words,
    std::size_t source, std::size_t destination, unsigned width,
    unsigned height, int padding) {
  if (!width || width % 32 || !height)
    throw std::invalid_argument("Original x87 alternate dimensions");
  if (source > words.size() || destination > words.size() ||
      width > words.size() - source ||
      std::uint64_t(width) * height > words.size() - destination)
    throw std::invalid_argument("Owned x87 alternate extent");
  const auto step = std::int64_t(width) + 2 * std::int64_t(padding);
  const auto rows = height - 1;
  // Check division bounds before multiplication, including extreme authored
  // dimensions/padding. Every source row lies between the two endpoints.
  if (rows && (step >= 0
      ? std::uint64_t(step) > (words.size() - source - width) / rows
      : std::uint64_t(-step) > source / rows))
    throw std::invalid_argument("Owned x87 alternate row extent");
  auto* bytes = reinterpret_cast<unsigned char*>(words.data());
  unsigned flags = 0;
  auto read64 = [&](std::int64_t at) {
    std::uint64_t value; std::memcpy(&value, bytes + at, 8); return value;
  };
  auto write64 = [&](std::int64_t at, std::uint64_t value) {
    std::memcpy(bytes + at, &value, 8);
  };
  auto load = [&](std::int64_t at) {
    auto value = read64(at);
    const auto exponent = (value >> 52) & 0x7ff;
    const auto fraction = value & 0xfffffffffffffULL;
    if (!exponent && fraction) flags |= 2; // x87 denormal operand
    if (exponent == 0x7ff && fraction && !(fraction & (1ULL << 51))) {
      flags |= 1; value |= 1ULL << 51; // quiet signaling NaN
    }
    return value;
  };
  std::int64_t src = std::int64_t(source) * 2;
  std::int64_t dst = std::int64_t(destination) * 2;
  for (unsigned row = 0; row < height; ++row) {
    for (unsigned col = 0; col < width; col += 32) {
      // Preserve instruction ordering under aliasing. Integer pairs are read
      // before each corresponding x87 load, then written before the next pair.
      auto pair = read64(src + 32); const auto f3 = load(src + 24);
      write64(dst + 32, pair);
      pair = read64(src + 40); const auto f2 = load(src + 16);
      write64(dst + 40, pair);
      pair = read64(src + 48); const auto f1 = load(src + 8);
      write64(dst + 48, pair);
      pair = read64(src + 56); const auto f0 = load(src);
      write64(dst + 56, pair);
      write64(dst, f0); write64(dst + 8, f1);
      write64(dst + 16, f2); write64(dst + 24, f3);
      src += 64; dst += 64;
    }
    src += 4 * std::int64_t(padding);
  }
  return flags;
}
} // namespace mnm::reconstruction
