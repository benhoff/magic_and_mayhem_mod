#pragma once
#include "sprite.hpp"

namespace mnm::render {
struct SpritePlacement {
    std::size_t frame=0;
    std::int32_t x=0,y=0;
    std::optional<SpriteColourTable> colours;
};
struct SpriteDrawPlan {
    std::shared_ptr<const assets::Sprite> source;
    std::vector<SpritePlacement> placements;
};
// Opaque native RGB565 batches. Callers keep shared sources immutable.
// CPU publication is atomic. GPU prepares and uploads the complete batch before
// the first draw; physical draw failure can still leave an accepted prefix.
void drawSpritePixels(Image&,const SpriteDrawPlan&,Rect);
void drawSpriteBatch(GlBlitter&,SurfaceId,const SpriteDrawPlan&,Rect);
}
