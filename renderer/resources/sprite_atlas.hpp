#pragma once
#include "resource_cache.hpp"
#include <map>
#include <tuple>

namespace mnm::render {
// GUI-thread cache. Pages own storage; entries own metadata and pending CPU rows.
// Eviction retires a whole least-recently-drawn page, avoiding unbounded holes.
class SpriteAtlas final {
public:
    SpriteAtlas(GlBlitter&,std::uint64_t pixels,std::size_t frames);
    ~SpriteAtlas();
    bool ready(const ResourceUploadRequest&) const;
    void supply(const ResourceUploadRequest&,PreparedSpriteFrame);
    bool advance(const ResourceUploadRequest&,std::size_t&);
    void draw(const ResourceUploadRequest&,SurfaceId,int,int,std::optional<Rect>,const SpriteComposite&);
    void release(const assets::ResourceId&);
    void clear();
    ResourceCacheStats stats() const {return stats_;}
private:
    using Key=std::tuple<int,std::string,std::uint64_t,std::size_t,int,int,bool,std::optional<SpriteColourTable>>;
    struct Page {SurfaceId pixels=0,mask=0;int size=0,x=0,y=0,rowHeight=0;std::uint64_t used=0;std::size_t entries=0;};
    struct Entry {Page* page=nullptr;Rect region{};int originX=0,originY=0;std::optional<SpriteColourTable> palette;std::optional<PreparedSpriteFrame> pending;int plane=0,row=0;};
    GlBlitter& renderer_;
    std::uint64_t pixelLimit_,clock_=0;
    std::size_t frameLimit_;
    ResourceCacheStats stats_;
    std::list<Page> pages_;
    std::map<Key,Entry> entries_;
    static Key key(const ResourceUploadRequest&);
    void erasePage(Page*);
    void evict();
};
}
