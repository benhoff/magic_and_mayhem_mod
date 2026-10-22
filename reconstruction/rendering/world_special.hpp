#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace mnm::reconstruction {
struct WorldSpecialRun {
  unsigned row{}, left{};
  std::vector<unsigned char> indices;
};
struct WorldSpecialFrame {
  unsigned width{}, height{};
  std::int32_t originX{}, originY{};
  std::vector<WorldSpecialRun> runs;
};
inline WorldSpecialFrame
decodeWorldSpecial(const std::vector<unsigned char> &bytes) {
  auto word = [&](unsigned at) {
    if (at > bytes.size() || bytes.size() - at < 4)
      throw std::invalid_argument("World special frame extent");
    std::uint32_t value;
    std::memcpy(&value, bytes.data() + at, 4);
    return value;
  };
  if (bytes.size() < 40 || word(0) != bytes.size())
    throw std::invalid_argument("World special frame size");
  WorldSpecialFrame frame{
      word(4), word(8), std::int32_t(word(12)), std::int32_t(word(16)), {}};
  if (!frame.width && !frame.height)
    return frame;
  if (!frame.width || !frame.height || frame.width > 2048 ||
      frame.height > 2048 || 40 + frame.height * 8 > bytes.size())
    throw std::invalid_argument("World special dimensions");
  const auto table = 40 + frame.height * 8, firstPixels = word(44);
  for (unsigned y = 0; y < frame.height; ++y) {
    auto control = word(40 + y * 8), pixel = word(44 + y * 8);
    unsigned x = 0;
    if (control < table || control >= firstPixels || pixel < firstPixels ||
        pixel > bytes.size())
      throw std::invalid_argument("World special planes");
    while (x < frame.width) {
      if (control >= firstPixels)
        throw std::invalid_argument("World special skip extent");
      const auto skip = bytes[control++];
      if (skip > frame.width - x)
        throw std::invalid_argument("World special skip");
      x += skip;
      if (x == frame.width)
        break;
      if (control >= firstPixels)
        throw std::invalid_argument("World special run extent");
      const auto count = bytes[control++];
      if (!count || count > frame.width - x || count > bytes.size() - pixel)
        throw std::invalid_argument("World special run");
      frame.runs.push_back(
          {y, x, {bytes.begin() + pixel, bytes.begin() + pixel + count}});
      pixel += count;
      x += count;
    }
  }
  return frame;
}
struct WorldSpecialCanvas {
  std::vector<std::uint16_t> &words;
  unsigned width, height, stride;
  bool rgb555;
  std::size_t origin{};
  std::array<std::int32_t, 4> clip;
  std::size_t at(std::int64_t x, std::int64_t y) const {
    const auto linear = std::int64_t(origin) + y * stride + x;
    if (!width || !height || stride < width || origin > words.size() ||
        words.size() - origin < std::size_t(stride) * height || linear < 0 ||
        std::uint64_t(linear) >= words.size())
      throw std::invalid_argument(
          "World special write escaped physical ownership");
    return std::size_t(linear);
  }
};
struct WorldWriteDisplacement {
  unsigned amplitude{3};
  std::uint32_t phase{};
  std::array<std::int32_t, 16> table{};
  void prepare() {
    if (amplitude > 8)
      throw std::invalid_argument("World write displacement period");
    if (!amplitude)
      return;
    if (table[0] != 32000)
      return;
    const std::uint64_t bits = 0x3fd921fb52713c02ull;
    double angle;
    std::memcpy(&angle, &bits, 8);
    for (unsigned i = 0; i < 2 * amplitude; ++i) {
      long double value = static_cast<long double>(i) * angle;
#if defined(__i386__) || defined(__x86_64__)
      __asm__("fsin" : "+t"(value));
#else
#error Exact original write-displacement sine requires the x87 execution contract.
#endif
      table[i] = std::int32_t(value * amplitude) + std::int32_t(amplitude);
    }
  }
};
enum class WorldSpecialOperation { outline, tint, writeDisplacement };
template <class Colour>
std::uint16_t
drawWorldSpecial(WorldSpecialOperation operation,
                 const WorldSpecialFrame &frame, WorldSpecialCanvas canvas,
                 std::int32_t x, std::int32_t y, std::int32_t shade,
                 unsigned palette, std::array<unsigned, 3> tint,
                 WorldWriteDisplacement &displacement, Colour &&colour) {
  if (operation == WorldSpecialOperation::writeDisplacement)
    displacement.prepare();
  if (!frame.width && !frame.height)
    return 0;
  if (operation == WorldSpecialOperation::writeDisplacement &&
      !displacement.amplitude)
    throw std::invalid_argument("Original write displacement divides by zero");
  const auto left = std::int64_t(x) - frame.originX,
             top = std::int64_t(y) - frame.originY;
  const bool horizontal =
      left >= canvas.clip[0] && left + frame.width <= canvas.clip[2];
  const auto mask = canvas.rgb555 ? 0x7defu : 0x7befu;
  std::uint16_t fixed = 0;
  if (operation == WorldSpecialOperation::tint) {
    std::array<unsigned, 3> channels{};
    for (unsigned k = 0; k < 3; ++k) {
      const auto product =
          std::uint32_t(tint[k] & 255u) * (std::uint32_t(shade) + 127u);
      const auto signedProduct = product <= 0x7fffffffu
                                     ? std::int32_t(product)
                                     : -1 - std::int32_t(0xffffffffu - product);
      auto value = std::uint32_t(signedProduct / 127);
      if (std::uint16_t(value) > 255)
        value = 255;
      channels[k] = value;
    }
    fixed = std::uint16_t(canvas.rgb555 ? ((channels[0] & 248) << 7) |
                                              ((channels[1] & 248) << 2) |
                                              (channels[2] >> 3 & 31)
                                        : ((channels[0] & 0xfff8) << 8) |
                                              ((channels[1] & 252) << 3) |
                                              (channels[2] >> 3 & 31));
  }
  struct Write {
    std::size_t at;
    std::uint16_t value;
    bool half;
  };
  std::vector<Write> writes;
  auto plan = [&](std::int64_t dx, std::int64_t dy, std::uint16_t value,
                  bool half = false) {
    writes.push_back({canvas.at(dx, dy), value, half});
  };
  for (const auto &run : frame.runs) {
    const auto dy = top + run.row;
    if (dy < canvas.clip[1] || dy >= canvas.clip[3])
      continue;
    const auto begin = left + run.left;
    const auto end = begin + std::int64_t(run.indices.size());
    if (horizontal && operation == WorldSpecialOperation::outline) {
      plan(begin, dy, colour(palette, shade, run.indices.front()));
      if (run.indices.size() > 1)
        plan(end - 1, dy, colour(palette, shade, run.indices.back()));
    } else if (horizontal && operation == WorldSpecialOperation::tint) {
      // The original deliberately touches the word BEFORE an opaque
      // run; that write can precede the logical clip but must be owned.
      plan(begin - 1, dy, fixed);
      plan(begin, dy, fixed);
      if (run.indices.size() > 1) {
        plan(end - 2, dy, fixed);
        plan(end - 1, dy, fixed);
      }
    } else
      for (unsigned i = 0; i < run.indices.size(); ++i) {
        const auto dx = begin + i;
        if (dx < canvas.clip[0] || dx >= canvas.clip[2])
          continue;
        if (operation == WorldSpecialOperation::writeDisplacement) {
          const auto phase = std::uint32_t(dy) + displacement.phase;
          const auto period = 2 * displacement.amplitude;
          const auto offset =
              horizontal
                  ? displacement.table[phase % period]
                  : std::abs(int(displacement.amplitude) - int(phase % period));
          plan(dx + offset, dy, colour(palette, shade, run.indices[i]));
        } else
          plan(dx, dy,
               colour(operation == WorldSpecialOperation::tint ? 0u : palette,
                      shade, run.indices[i]),
               true);
      }
  }
  for (const auto &write : writes) {
    auto value = write.value;
    if (write.half)
      value = std::uint16_t(((value >> 1) & mask) +
                            ((canvas.words[write.at] >> 1) & mask));
    canvas.words[write.at] = value;
  }
  return 0;
}
enum class WorldOpaqueEntry {
  word,
  indexed,
  black,
  fallback,
  blend75,
  blend50,
  blend25
};
struct WorldOpaqueSource {
  unsigned token{};
  bool words{};
};
template <class Backend>
std::uint16_t drawWorldOpaque(const std::array<WorldOpaqueSource, 4> &sources,
                              bool blend, std::int32_t x, std::int32_t y,
                              std::int32_t shade, unsigned palette,
                              Backend &&backend) {
  auto copy = [&](const WorldOpaqueSource &source) {
    if (source.words)
      return std::uint16_t(
          backend(WorldOpaqueEntry::word, source.token, x, y, shade, palette));
    const bool black = shade == -127;
    auto result = std::uint16_t(
        backend(black ? WorldOpaqueEntry::black : WorldOpaqueEntry::indexed,
                source.token, x, y, shade, palette));
    if (black)
      result = result == 1;
    if (result == 1)
      result = std::uint16_t(backend(WorldOpaqueEntry::fallback, source.token,
                                     x, y, shade, palette));
    return result;
  };
  auto result = copy(sources[0]);
  if (!blend)
    return result | copy(sources[1]);
  const WorldOpaqueEntry entries[]{WorldOpaqueEntry::blend75,
                                   WorldOpaqueEntry::blend50,
                                   WorldOpaqueEntry::blend25};
  for (unsigned i = 0; i < 3; ++i) {
    if (sources[i + 1].words)
      throw std::invalid_argument(
          "Original packed blends require indexed frames");
    result |= std::uint16_t(
        backend(entries[i], sources[i + 1].token, x, y, shade, palette));
  }
  return result;
}
} // namespace mnm::reconstruction
