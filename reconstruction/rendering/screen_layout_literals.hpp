#pragma once
#include <cstdint>
namespace mnm::reconstruction {
// Immutable No-CD layout-parser literals, checked against the pinned image by
// the original comparison driver. These are build data, not observed outputs.
struct ScreenLayoutLiteral {
  std::uint32_t address;
  const char *text;
};
inline constexpr ScreenLayoutLiteral ScreenLayoutLiterals[] = {
    {0x5ec200, "TEXTBUTTON_"},
    {0x5ec1f4, "EDITBOX_"},
    {0x5ec1e8, "SLIDERBAR_"},
    {0x5ec1d8, "STANDARDBUTTON_"},
    {0x5ec1c8, "RADIOBUTTON_"},
    {0x5ec1bc, "LISTBOX_"},
    {0x5ec1b4, "TEXT_"},
    {0x5ec1a8, "STATBAR_"},
    {0x5ec19c, "SPRITEBAR_"},
    {0x5ec18c, "MESSAGELISTBOX_"},
    {0x5ec180, "PICTUREBOX_"},
    {0x5ec178, "Font"},
    {0x5ec170, "LARGE"},
    {0x5ec168, "TINY"},
    {0x5ec160, "Rect2"},
    {0x5ec158, "Rect1"},
    {0x5ec14c, "%i,%i,%i,%i"},
    {0x5ec144, "Text"},
    {0x5ec134, "SpriteIndexes"},
    {0x5ec128, "minValue"},
    {0x5ec11c, "maxValue"},
    {0x5ec110, "TextFlags"},
    {0x5eb674, "LEFT"},
    {0x5eb66c, "RIGHT"},
    {0x5ec108, "Colours"},
    {0x5ec100, "step"},
    {0x5da820, ","},
    {0x5d9058, "\\"}};
} // namespace mnm::reconstruction
