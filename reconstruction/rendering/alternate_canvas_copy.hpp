#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace mnm::reconstruction {
// 594508 consumes positive dimensions and a width divisible by four WORDs.
// Padding is in DWORDs, unlike the pitch's WORD units. It snapshots four
// source WORDs before each store and advances forwards even for overlapping
// buffers. This contract supplies no source allocation or writer provenance.
inline void copyAlternateCanvas(std::vector<std::uint16_t> &storage,
                                std::int64_t source, std::int64_t destination,
                                std::uint32_t width, std::uint32_t height,
                                std::int32_t padding) {
  if (!width || width % 4 || !height || height > 4096 || width > 4096)
    throw std::invalid_argument("Original alternate copy dimensions");
  const auto stride = std::int64_t(width) + 2 * std::int64_t(padding);
  for (std::uint32_t row = 0; row < height; ++row) {
    const auto from = source + row * stride,
               to = destination + std::int64_t(row) * width;
    if (from < 0 || to < 0 || from + width > std::int64_t(storage.size()) ||
        to + width > std::int64_t(storage.size()))
      throw std::invalid_argument(
          "Alternate copy requires owned physical extents");
  }
  for (std::uint32_t row = 0; row < height; ++row) {
    for (std::uint32_t x = 0; x < width; x += 4) {
      std::array<std::uint16_t, 4> words;
      for (unsigned i = 0; i < 4; ++i)
        words[i] = storage[std::size_t(source + x + i)];
      for (unsigned i = 0; i < 4; ++i)
        storage[std::size_t(destination + x + i)] = words[i];
    }
    source += stride;
    destination += width;
  }
}
} // namespace mnm::reconstruction
