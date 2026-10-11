#include "font_layout.hpp"
#include <utility>

namespace mnm::compat {
FontLayout::FontLayout(assets::SftFont font):font_(std::move(font)) {
  contours_=FontText(font_).contours();
}
reconstruction::rendering::FontLayoutResult FontLayout::draw(
    render::CanvasSequence &canvas,std::uint32_t id,
    const reconstruction::rendering::FontLayoutInput &in,render::Rect clip,
    std::array<unsigned,3> tint,const std::array<float,64> &table) const {
  auto input=in;input.ascent=font_.ascent;input.descent=font_.descent;
  auto result=reconstruction::rendering::fontLineLayout(contours_,input);
  if (!result.glyphs.empty()) {
    auto staged=canvas;
    for (const auto &glyph:result.glyphs)
      staged.glyph(id,font_.glyphs.frames.at(*glyph.frame),glyph.x,glyph.y,clip,tint,table);
    canvas=std::move(staged);
  }
  return result;
}
}
