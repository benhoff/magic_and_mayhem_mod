#pragma once
#include "../sprites/sprite.hpp"

namespace mnm::render {
enum class SpriteTransformKind {
    crop, trim, mirrorX, mirrorY, clockwise, halfTurn, counterclockwise,
    nearestResize, silhouette, outerOutline
};
struct SpriteTransform {
    SpriteTransformKind kind=SpriteTransformKind::trim;
    Rect region{}; // crop only; source-local, right/bottom exclusive
    int width=0,height=0; // nearestResize only
    std::uint16_t colour=0; // silhouette / outerOutline
};
// Native preparation policy: one selected frame becomes an owned direct RGB565
// sprite. No source mutation or encoded-run/auxiliary-plane semantics are carried
// into derived pixels. Pixel-grid origins transform in signed64-bit arithmetic.
// Input/output dimensions <=2048 and pixel area <=1Mi. Refuse origin overflow.
assets::Sprite transformSpriteFrame(const assets::Sprite&,std::size_t,
                                   const SpriteTransform&,
                                   const SpriteColourTable* colours=nullptr);
}
