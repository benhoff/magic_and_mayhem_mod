#pragma once
#include "../sprites/sprite.hpp"
#include <list>
namespace mnm::render {
struct FontGlyph {std::size_t frame=0;std::int32_t x=0,y=0;};
struct FontDrawPlan {
 std::shared_ptr<const assets::Sprite> font;
 std::vector<FontGlyph> glyphs;
};
// Native opaque RGB565 policy, with separate coverage and signed glyph origins.
// Preflight all glyphs and viewport before modifying caller-owned pixels.
void drawFontPixels(Image&,const FontDrawPlan&,Rect,const SpriteColourTable* = nullptr);
struct FontCacheLimits {std::size_t frames=16;std::uint64_t pixels=1024*1024;};
struct FontCacheStats {std::size_t frames=0;std::uint64_t pixels=0,hits=0,uploads=0,evictions=0;};
// GUI-thread owner. Renderer outlives this cache; plans retain immutable font data.
// The supplied source must remain immutable; plans bind that same owned source.
class FontGpuCache {
 struct Entry {std::size_t frame;std::optional<SpriteColourTable> colours;std::uint64_t pixels;std::unique_ptr<UploadedSpriteFrame> upload;};
 GlBlitter& renderer_;std::shared_ptr<const assets::Sprite> font_;
 FontCacheLimits limits_;FontCacheStats stats_;std::list<Entry> entries_;
 UploadedSpriteFrame& glyph(std::size_t,const SpriteColourTable*);
public:
 FontGpuCache(GlBlitter&,std::shared_ptr<const assets::Sprite>,FontCacheLimits = {});
 FontGpuCache(const FontGpuCache&)=delete;
 FontGpuCache& operator=(const FontGpuCache&)=delete;
 // Complete logical preflight; physical GL/allocation failure may follow a draw.
 void draw(SurfaceId,const FontDrawPlan&,Rect,const SpriteColourTable* = nullptr);
 void clear();
 FontCacheStats stats() const;
};
}
