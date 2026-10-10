#pragma once
#include "../../assets/sft.hpp"
#include "../../renderer/canvas_sequence.hpp"
#include "../../reconstruction/rendering/font_byte.hpp"

namespace mnm::compat {
// Owned byte text producer for the recovered NoCD font consumer. Widget-free;
// callers supply the original spacing mode and initialized coverage explicitly.
class FontText {
public:
  explicit FontText(assets::SftFont);
  void draw(render::CanvasSequence &, std::uint32_t canvas,
            reconstruction::rendering::FontCursor &, std::uint8_t byte,
            std::int32_t punctuationMode, render::Rect,
            std::array<unsigned,3> tint, const std::array<float,64> &) const;
  const reconstruction::rendering::ByteFont &contours() const { return contours_; }
private:
  assets::SftFont font_;
  reconstruction::rendering::ByteFont contours_;
};
}
