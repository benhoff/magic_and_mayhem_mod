#pragma once
#include "resource_manager.hpp"
#include "sprite.hpp"
#include <list>

namespace mnm::render {
struct ResourceCacheLimits {
    std::size_t frames=24;
    // Colour + coverage texels, charged separately from renderer-wide budgets.
    std::uint64_t pixels=8ULL*1024*1024;
    // Async hosts must explicitly adopt CPU dependencies before drawing.
    bool residentOnly=false;
    bool preparedOnly=false; // cache misses require an owned CPU completion
    bool indexedAtlas=false; // bounded pages; words and projected masks share storage
    std::size_t atlasFrames=4096; // metadata bound independent of surface handles
};
struct ShadowRows {
    int first=0,last=0;
    bool operator==(const ShadowRows& other) const {return first==other.first&&last==other.last;}
};
struct ResourceUploadRequest {
    assets::ResourceId id;
    std::uint64_t revision=0;
    std::size_t frame=0;
    std::optional<SpriteColourTable> colours;
    std::optional<ShadowRows> shadow;
    bool indexed=false;
    bool operator==(const ResourceUploadRequest& other) const {
        return id==other.id&&revision==other.revision&&frame==other.frame&&colours==other.colours&&shadow==other.shadow&&indexed==other.indexed;
    }
};
PreparedSpriteFrame prepareResourceUpload(const assets::VisualResource&,const ResourceUploadRequest&,const std::function<void()>& checkpoint={});
struct ResourceCacheStats {
    std::size_t frames=0, surfaces=0;
    std::uint64_t pixels=0, hits=0, uploads=0, evictions=0;
};
// GUI-thread owner of uploads only; manager and renderer must outlive this cache.
// draw() lends no cache references to callers. LRU eviction respects local and
// renderer-wide budgets. Palette keys contain values, never pointer identities.
class SpriteAtlas;
class ResourceCache final {
public:
    ResourceCache(GlBlitter&,assets::ResourceManager&,ResourceCacheLimits limits={});
    ~ResourceCache();
    ResourceCache(const ResourceCache&)=delete;
    ResourceCache& operator=(const ResourceCache&)=delete;
    void draw(const assets::ResourceId&,std::size_t frame,SurfaceId destination,
              int anchorX,int anchorY,std::optional<Rect> viewport=std::nullopt,
              const SpriteColourTable* colours=nullptr,const SpriteComposite& composite={});
    // Constant-size cache query; no pixel expansion or file loading in prepared mode.
    ResourceUploadRequest request(const assets::ResourceId&,std::size_t,const SpriteColourTable*,std::optional<ShadowRows> = {});
    bool ready(const ResourceUploadRequest&) const;
    void supply(const ResourceUploadRequest&,PreparedSpriteFrame);
    bool advance(const ResourceUploadRequest&,std::size_t& bytes);
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
        std::optional<ShadowRows> shadow;
        std::uint64_t pixels=0;
        std::unique_ptr<UploadedSpriteFrame> upload;
    };
    std::unique_ptr<SpriteAtlas> atlas_;
    GlBlitter& renderer_;
    assets::ResourceManager& resources_;
    ResourceCacheLimits limits_;
    ResourceCacheStats stats_;
    std::thread::id thread_;
    std::list<Entry> entries_; // oldest first
    void checkThread() const;
    void drawUpload(const ResourceUploadRequest&,SurfaceId,int,int,std::optional<Rect>,const SpriteComposite&);
    void erase(std::list<Entry>::iterator);
    UploadedSpriteFrame& upload(const ResourceUploadRequest&);
    Entry& admit(const ResourceUploadRequest&,PreparedSpriteFrame);
};
}
