#pragma once
#include "font_byte.hpp"

namespace mnm::reconstruction::rendering {
enum class FontLayoutKind { rectangle, heightLimited, measure };
struct FontBounds { std::int32_t left, top, right, bottom; };
struct FontLine { std::int32_t x; std::uint32_t count, source; };
struct FontLayoutInput {
  FontLayoutKind kind = FontLayoutKind::rectangle;
  // Includes its terminating zero. Null source is admitted only for measure.
  std::vector<std::uint8_t> text;
  bool nullSource = false;
  FontBounds bounds{};
  FontCursor cursor;
  std::int32_t ascent = 0, descent = 0, lineHeight = 0, firstX = 0;
  std::int32_t punctuationMode = 0;
  std::uint8_t flags = 0;
};
struct FontLayoutResult {
  std::uint32_t width = 0;
  FontCursor cursor;
  std::vector<FontLine> lines;
  std::vector<ByteGlyph> glyphs;
  // Original optional output pointer remains untouched when no bytes render.
  std::optional<std::uint32_t> consumed;
};
// Selected bounded 4a5ce0 / 4a6190 / 4a6630 contracts. Transactional: no input
// state changes on refusal. Arithmetic follows modulo-32-bit x86 behavior.
FontLayoutResult fontLineLayout(const ByteFont &, const FontLayoutInput &);
}
