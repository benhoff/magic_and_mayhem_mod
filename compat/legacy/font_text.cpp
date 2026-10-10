#include "font_text.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::compat {
FontText::FontText(assets::SftFont font):font_(std::move(font)) {
  if (font_.glyphs.frames.size()>223 || font_.rowCount>128 ||
      font_.rowMetrics.size()!=std::uint64_t(font_.metricGlyphCount)*font_.rowCount ||
      font_.metricGlyphCount<font_.glyphs.frames.size())
    throw std::invalid_argument("Font outside bounded byte consumer domain");
  contours_.glyphCount=font_.glyphs.frames.size();contours_.rows=font_.rowCount;
  for (const auto &r:font_.rowMetrics) contours_.contours.push_back({r.leading,r.trailing});
}
void FontText::draw(render::CanvasSequence &canvas,std::uint32_t id,
    reconstruction::rendering::FontCursor &cursor,std::uint8_t byte,std::int32_t mode,
    render::Rect clip,std::array<unsigned,3> tint,const std::array<float,64> &table) const {
  auto next=cursor;
  const auto request=reconstruction::rendering::fontByteGlyph(contours_,next,byte,mode);
  if (request.frame) canvas.glyph(id,font_.glyphs.frames.at(*request.frame),request.x,request.y,clip,tint,table);
  cursor=std::move(next); // Commit state only after the atomic raster admission.
}
}
