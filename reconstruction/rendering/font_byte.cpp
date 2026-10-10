#include "font_byte.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction::rendering {
namespace {
std::int32_t signedWord(std::uint32_t n) {
  return static_cast<std::int32_t>(n < 0x80000000u ? std::int64_t(n)
                                                : std::int64_t(n)-0x100000000LL);
}
std::int32_t add(std::int32_t a, std::int32_t b) {
  return signedWord(std::uint32_t(a)+std::uint32_t(b));
}
bool suppressed(std::uint8_t b) { return b==32 || b==95 || b==9 || b==10; }
void validate(const ByteFont &font, const FontCursor &cursor) {
  if (font.rows>128 || cursor.trailing.size()!=font.rows ||
      std::uint64_t(font.rows)*font.glyphCount>font.contours.size())
    throw std::invalid_argument("Incomplete bounded font contours/state");
}
}
std::int32_t fontByteAdvance(const ByteFont &font, FontCursor &cursor,
                           std::uint8_t byte, bool update, std::int32_t mode) {
  validate(font,cursor);
  if (byte==9) return cursor.tabAdvance;
  if (byte<32) return 0;
  const unsigned mapped=(byte==32 || byte==95) ? 97 : byte;
  if (mapped<33 || mapped-33>=font.glyphCount) return 0;
  std::int32_t advance=0;
  for (unsigned row=0;row<font.rows;++row) {
    const auto &c=font.contours[(mapped-33)*font.rows+row];
    const auto distance=signedWord(std::uint32_t(cursor.trailing[row])-std::uint32_t(c.leading));
    advance=std::max(advance,distance);
    if (update) cursor.trailing[row]=c.trailing;
  }
  std::int32_t extra=0;
  if (mode==1 && (mapped=='!' || mapped=='?' || mapped==':' || mapped==';'))
    extra=add(cursor.tracking,1);
  if (byte==0x60 || byte==0x91 || byte==0x92 || byte==0x27 || byte==0xb4) extra=2;
  return add(add(cursor.tracking,extra),advance);
}
ByteGlyph fontByteGlyph(const ByteFont &font, FontCursor &cursor,
                       std::uint8_t byte, std::int32_t mode) {
  validate(font,cursor);
  if (!suppressed(byte) && (byte<33 || unsigned(byte-33)>=font.glyphCount))
    throw std::invalid_argument("Byte draw would address an unavailable glyph");
  cursor.x=add(cursor.x,fontByteAdvance(font,cursor,byte,true,mode));
  return {suppressed(byte) ? std::nullopt : std::optional<std::uint32_t>(byte-33),cursor.x,cursor.y};
}
}
