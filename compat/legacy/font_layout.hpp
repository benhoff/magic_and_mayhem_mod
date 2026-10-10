#pragma once
#include "font_text.hpp"
#include "../../reconstruction/rendering/font_layout.hpp"

namespace mnm::compat {
class FontLayout final {
public:
  explicit FontLayout(assets::SftFont);
  reconstruction::rendering::FontLayoutResult draw(render::CanvasSequence &,
      std::uint32_t canvas,const reconstruction::rendering::FontLayoutInput &,
      render::Rect clip,std::array<unsigned,3>,const std::array<float,64> &) const;
private:
  assets::SftFont font_;
  reconstruction::rendering::ByteFont contours_;
};
}
