#pragma once
#include "resource_manager.hpp"
#include "sprite.hpp"
#include <list>

namespace mnm::render {
struct ResourceCacheLimits {
    std::size_t frames=24;
    // Colour + coverage texels, charged separately from renderer-wide budgets.
    std::uint64_t pixels=8ULL*1024*1024;
};
struct ResourceCacheStats {
    std::size_t frames=0, surfaces=0;
    std::uint64_t pixels=0, hits=0, uploads=0, evictions=0;
};
// GUI-thread owner of uploads only; manager and renderer must outlive this cache.
// draw() lends no cache references to callers. LRU eviction respects local and
// renderer-wide budgets. Palette keys contain values, never pointer identities.
class ResourceCache final {
public:
    ResourceCache(GlBlitter&,assets::ResourceManager&,ResourceCacheLimits limits={});
    ResourceCache(const ResourceCache&)=delete;
    ResourceCache& operator=(const ResourceCache&)=delete;
    void draw(const assets::ResourceId&,std::size_t frame,SurfaceId destination,
              int anchorX,int anchorY,std::optional<Rect> viewport=std::nullopt,
              const SpriteColourTable* colours=nullptr);
    void release(const assets::ResourceId&); // GPU only
    void retire(const assets::ResourceId&); // GPU + manager decoded data
    void clear(); // GPU only; bindings/decoded resources remain
    ResourceCacheStats stats() const;
private:
    struct Entry {
        assets::ResourceId id;
        std::uint64_t revision=0;
        std::size_t frame=0;
        std::optional<SpriteColourTable> colours;
        std::uint64_t pixels=0;
        std::unique_ptr<UploadedSpriteFrame> upload;
    };
    GlBlitter& renderer_;
    assets::ResourceManager& resources_;
    ResourceCacheLimits limits_;
    ResourceCacheStats stats_;
    std::thread::id thread_;
    std::list<Entry> entries_; // oldest first
    void checkThread() const;
    void erase(std::list<Entry>::iterator);
    UploadedSpriteFrame& upload(const assets::ResourceId&,std::size_t,const SpriteColourTable*);
};
}
