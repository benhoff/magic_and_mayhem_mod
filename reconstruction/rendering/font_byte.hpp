#pragma once
#include <cstdint>
#include <optional>
#include <vector>

namespace mnm::reconstruction::rendering {
struct FontContour { std::int32_t leading, trailing; };
struct ByteFont {
  std::uint32_t glyphCount = 0, rows = 0;
  std::vector<FontContour> contours; // Glyph-major, byte 33 first.
};
struct FontCursor {
  std::vector<std::int32_t> trailing;
  std::int32_t x = 0, y = 0, tracking = 0, tabAdvance = 0;
};
struct ByteGlyph {
  std::optional<std::uint32_t> frame;
  std::int32_t x, y;
};
// NoCD 4a58e0: byte advances and optional contour updates, modulo-32-bit math.
std::int32_t fontByteAdvance(const ByteFont &, FontCursor &, std::uint8_t,
                            bool update, std::int32_t punctuationMode);
// Admitted subset of 4a59e0: unsupported frame lookups refuse before mutation.
// Newline is a suppressed byte here, not a line-layout instruction.
ByteGlyph fontByteGlyph(const ByteFont &, FontCursor &, std::uint8_t,
                       std::int32_t punctuationMode);
}
