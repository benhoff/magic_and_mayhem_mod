#include "resource_cache.hpp"
#include <stdexcept>

namespace mnm::render {
namespace {
// Bitmap conversion is an explicit unshaded RGB565/opaque native policy.
// SPR retains its own origin, palette selection and separate coverage mask.
assets::Sprite bitmapSprite(const assets::ResourceImage& image){
    assets::Sprite result;assets::SpriteFrame frame;
    std::visit([&](const auto& input){
        using T=std::decay_t<decltype(input)>;
        if constexpr(!std::is_same_v<T,assets::Sprite>){
            if(input.width>2048 || input.height>2048)throw std::runtime_error("Bitmap exceeds renderer dimensions");
            frame.width=input.width;frame.height=input.height;
            const auto count=std::size_t(input.width)*input.height;
            frame.opaqueMask.assign(count,1);
            if constexpr(std::is_same_v<T,assets::PcxImage>){
                result.storage=assets::SpriteStorage::indexed8;result.palettes.resize(1);frame.paletteIndex=0;
                for(std::size_t i=0;i<256;++i){const auto c=input.palette[i];result.palettes[0][i]={c[0],c[1],c[2]};}
                frame.pixels=input.indices;
            }else{
                result.storage=assets::SpriteStorage::rgb565;
                std::vector<std::uint16_t> words;words.reserve(count);
                for(std::size_t i=0;i<count;++i)
                    words.push_back((std::uint16_t(input.rgb[i*3]>>3)<<11)|(std::uint16_t(input.rgb[i*3+1]>>2)<<5)|(input.rgb[i*3+2]>>3));
                frame.pixels=std::move(words);
            }
        }
    },image);
    result.frames.push_back(std::move(frame));return result;
}
}
ResourceCache::ResourceCache(GlBlitter& renderer,assets::ResourceManager& resources,ResourceCacheLimits limits)
    :renderer_(renderer),resources_(resources),limits_(limits),thread_(std::this_thread::get_id()){
    if(!limits.frames || limits.frames>32 || !limits.pixels || limits.pixels>16777216)
        throw std::invalid_argument("Resource upload budgets exceed renderer bounds or are zero");
}
void ResourceCache::checkThread() const{
    if(std::this_thread::get_id()!=thread_)throw std::runtime_error("ResourceCache used from another thread");
}
void ResourceCache::erase(std::list<Entry>::iterator it){
    stats_.pixels-=it->pixels;stats_.surfaces-=it->pixels?2:0;--stats_.frames;
    entries_.erase(it);++stats_.evictions;
}
UploadedSpriteFrame& ResourceCache::upload(const assets::ResourceId& id,std::size_t index,const SpriteColourTable* colours){
    const auto& resource=resources_.load(id);
    if(index>=resource.frameCount())throw std::out_of_range("Resource frame outside image");
    const std::optional<SpriteColourTable> table=colours?std::make_optional(*colours):std::nullopt;
    for(auto it=entries_.begin();it!=entries_.end();++it){
        if(it->id==id && it->revision==resource.revision && it->frame==index && it->colours==table){
            entries_.splice(entries_.end(),entries_,it);++stats_.hits;return *it->upload;
        }
    }
    // Validate/convert before evicting any valid upload. Decoded sources can only
    // be created through the owned, checked loaders in ResourceManager.
    assets::Sprite converted;
    const auto* sprite=std::get_if<assets::Sprite>(&resource.image);
    if(!sprite){converted=bitmapSprite(resource.image);sprite=&converted;}
    if(colours && sprite->storage!=assets::SpriteStorage::indexed8)
        throw std::invalid_argument("Colour table requires indexed image");
    const auto& frame=sprite->frames.at(index);
    const auto pixels=std::uint64_t(frame.width)*frame.height*2;
    const std::size_t surfaces=pixels?2:0;
    if(frame.width>2048 || frame.height>2048 || pixels>limits_.pixels)
        throw std::runtime_error("Resource frame exceeds upload budget");
    const auto global=renderer_.stats();
    if(global.surfaces-stats_.surfaces+surfaces>64 || global.pixels-stats_.pixels+pixels>16777216)
        throw std::runtime_error("External renderer surfaces leave insufficient upload space");
    for(auto it=entries_.begin();it!=entries_.end();){
        if(it->id==id && it->revision!=resource.revision){const auto old=it++;erase(old);}else ++it;
    }
    while(stats_.frames>=limits_.frames || stats_.pixels+pixels>limits_.pixels ||
          renderer_.stats().surfaces+surfaces>64 || renderer_.stats().pixels+pixels>16777216)
        erase(entries_.begin());
    auto uploaded=std::make_unique<UploadedSpriteFrame>(renderer_,*sprite,index,colours);
    entries_.push_back({id,resource.revision,index,table,pixels,std::move(uploaded)});
    ++stats_.frames;stats_.pixels+=pixels;stats_.surfaces+=surfaces;++stats_.uploads;
    return *entries_.back().upload;
}
void ResourceCache::draw(const assets::ResourceId& id,std::size_t frame,SurfaceId destination,int x,int y,
                         std::optional<Rect> viewport,const SpriteColourTable* colours){
    checkThread();auto& uploaded=upload(id,frame,colours);
    if(viewport)uploaded.drawClipped(destination,x,y,*viewport);else uploaded.draw(destination,x,y);
}
void ResourceCache::release(const assets::ResourceId& id){
    checkThread();resources_.recipe(id); // reject invalid/unknown IDs before mutation
    for(auto it=entries_.begin();it!=entries_.end();){if(it->id==id){const auto old=it++;erase(old);}else ++it;}
}
void ResourceCache::retire(const assets::ResourceId& id){release(id);resources_.unload(id);}
void ResourceCache::clear(){checkThread();while(!entries_.empty())erase(entries_.begin());}
ResourceCacheStats ResourceCache::stats() const{checkThread();return stats_;}
}
