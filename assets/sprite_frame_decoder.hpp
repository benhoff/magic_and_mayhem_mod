#pragma once
#include "sprite_loader.hpp"

namespace mnm::assets::detail {
// Private native codec boundary. Envelope callers validate their own headers.
struct SpriteFrameLayout {
    std::uint32_t version=0,count=0,palettes=0,flags=0;
    std::uint64_t paletteOffset=0,table=0,base=0;
};
SpriteResult decodeSpriteFrames(const std::vector<std::uint8_t>&,const SpriteFrameLayout&,const SpriteLimits&);
}
