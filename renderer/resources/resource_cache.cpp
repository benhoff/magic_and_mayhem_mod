#include "resource_cache.hpp"
#include "sprite_atlas.hpp"
#include <stdexcept>
#include <limits>

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
PreparedSpriteFrame prepareResourceUpload(const assets::VisualResource& resource,const ResourceUploadRequest& request,const std::function<void()>& check){
    assets::Sprite converted;const auto* sprite=std::get_if<assets::Sprite>(&resource.image);
    if(!sprite){converted=bitmapSprite(resource.image);sprite=&converted;}
    if(!request.shadow)return prepareSpriteFrame(*sprite,request.frame,request.colours?&*request.colours:nullptr,check,request.indexed);
    const auto& f=sprite->frames.at(request.frame);const auto r=*request.shadow;
    if(r.first<0||r.first>=r.last||r.last>int(f.height))throw std::runtime_error("Invalid projected shadow rows");
    const int shift=r.last/2,rows=(r.last-r.first+1)/2;
    const auto width=int(f.width)+shift;
    if(width>2048||rows>2048)throw std::runtime_error("Projected shadow exceeds storage");
    const auto count=std::size_t(width)*rows;
    PreparedSpriteFrame result{{width,rows,std::vector<std::uint32_t>(count,0)},{width,rows,std::vector<std::uint32_t>(count,0)},0,0};
    for(int row=0;row<rows;++row){if(check)check();for(unsigned column=0;column<f.width;++column)
        result.mask.pixels[std::size_t(row)*width+column+shift-row]=f.opaqueMask.at(std::size_t(r.first+row*2)*f.width+column);}
    return result;
}
ResourceCache::ResourceCache(GlBlitter& renderer,assets::ResourceManager& resources,ResourceCacheLimits limits)
    :renderer_(renderer),resources_(resources),limits_(limits),thread_(std::this_thread::get_id()){
    if(limits.preparedOnly&&!limits.residentOnly)throw std::invalid_argument("Prepared uploads require resident-only resources");
    if(!limits.frames || limits.frames>32 || !limits.pixels || limits.pixels>16777216)
        throw std::invalid_argument("Resource upload budgets exceed renderer bounds or are zero");
    if(limits.indexedAtlas)atlas_=std::make_unique<SpriteAtlas>(renderer,limits.pixels,limits.atlasFrames);
}
ResourceCache::~ResourceCache()=default;
void ResourceCache::checkThread() const{
    if(std::this_thread::get_id()!=thread_)throw std::runtime_error("ResourceCache used from another thread");
}
void ResourceCache::erase(std::list<Entry>::iterator it){
    stats_.pixels-=it->pixels;stats_.surfaces-=it->pixels?2:0;--stats_.frames;
    entries_.erase(it);++stats_.evictions;
}
ResourceUploadRequest ResourceCache::request(const assets::ResourceId& id,std::size_t frame,const SpriteColourTable* colours,std::optional<ShadowRows> shadow){
    checkThread();const auto& resource=limits_.residentOnly?resources_.resident(id):resources_.load(id);
    if(frame>=resource.frameCount())throw std::out_of_range("Resource frame outside image");
    const auto* sprite=std::get_if<assets::Sprite>(&resource.image);
    const bool indexed=atlas_&&!shadow&&((sprite&&sprite->storage==assets::SpriteStorage::indexed8)||std::holds_alternative<assets::PcxImage>(resource.image));
    return {id,resource.revision,frame,colours?std::make_optional(*colours):std::nullopt,shadow,indexed};
}
bool ResourceCache::ready(const ResourceUploadRequest& request) const{
    checkThread();if(resources_.resident(request.id).revision!=request.revision)return false;
    if(atlas_)return atlas_->ready(request);
    for(const auto& e:entries_)if(e.id==request.id&&e.revision==request.revision&&e.frame==request.frame&&e.colours==request.colours&&e.shadow==request.shadow)return e.upload->ready();
    return false;
}
ResourceCache::Entry& ResourceCache::admit(const ResourceUploadRequest& request,PreparedSpriteFrame prepared){
    checkThread();const auto& resource=resources_.resident(request.id);
    if(resource.revision!=request.revision)throw std::runtime_error("Stale resource upload completion");
    for(const auto& e:entries_)if(e.id==request.id&&e.revision==request.revision&&e.frame==request.frame&&e.colours==request.colours&&e.shadow==request.shadow)
        throw std::runtime_error("Duplicate resource upload completion");
    const auto pixels=std::uint64_t(prepared.pixels.width)*prepared.pixels.height*2;
    const std::size_t surfaces=pixels?2:0;
    if(prepared.pixels.width<0||prepared.pixels.height<0||prepared.pixels.width>2048||prepared.pixels.height>2048||pixels>limits_.pixels)
        throw std::runtime_error("Resource frame exceeds upload budget");
    const auto global=renderer_.stats();
    if(global.surfaces-stats_.surfaces+surfaces>64||global.pixels-stats_.pixels+pixels>16777216)
        throw std::runtime_error("External renderer surfaces leave insufficient upload space");
    for(auto it=entries_.begin();it!=entries_.end();){if(it->id==request.id&&it->revision!=resource.revision){const auto old=it++;erase(old);}else ++it;}
    while(stats_.frames>=limits_.frames||stats_.pixels+pixels>limits_.pixels||renderer_.stats().surfaces+surfaces>64||renderer_.stats().pixels+pixels>16777216)erase(entries_.begin());
    auto uploaded=std::make_unique<UploadedSpriteFrame>(renderer_,std::move(prepared));
    entries_.push_back({request.id,request.revision,request.frame,request.colours,request.shadow,pixels,std::move(uploaded)});
    ++stats_.frames;stats_.pixels+=pixels;stats_.surfaces+=surfaces;++stats_.uploads;
    return entries_.back();
}
void ResourceCache::supply(const ResourceUploadRequest& request,PreparedSpriteFrame prepared){
    checkThread();if(resources_.resident(request.id).revision!=request.revision)throw std::runtime_error("Stale resource upload completion");
    if(atlas_)atlas_->supply(request,std::move(prepared));else admit(request,std::move(prepared));
}
bool ResourceCache::advance(const ResourceUploadRequest& request,std::size_t& bytes){
    checkThread();if(resources_.resident(request.id).revision!=request.revision)throw std::runtime_error("Stale pending upload");
    if(atlas_)return atlas_->advance(request,bytes);
    for(auto& e:entries_)if(e.id==request.id&&e.revision==request.revision&&e.frame==request.frame&&e.colours==request.colours&&e.shadow==request.shadow)return e.upload->advanceUpload(bytes);
    return false;
}
UploadedSpriteFrame& ResourceCache::upload(const ResourceUploadRequest& request){
    for(auto it=entries_.begin();it!=entries_.end();++it)if(it->id==request.id&&it->revision==request.revision&&it->frame==request.frame&&it->colours==request.colours&&it->shadow==request.shadow){
        if(!it->upload->ready())throw std::runtime_error("Resource upload is incomplete");
        entries_.splice(entries_.end(),entries_,it);++stats_.hits;return *it->upload;
    }
    if(limits_.preparedOnly)throw std::runtime_error("Missing prepared resource upload");
    auto& entry=admit(request,prepareResourceUpload(resources_.resident(request.id),request));
    auto bytes=std::numeric_limits<std::size_t>::max();while(!entry.upload->advanceUpload(bytes)){}
    return *entry.upload;
}
void ResourceCache::draw(const assets::ResourceId& id,std::size_t frame,SurfaceId destination,int x,int y,
                         std::optional<Rect> viewport,const SpriteColourTable* colours,const SpriteComposite& composite){
    if(composite.mode==CompositeMode::projectedShadow){
        checkThread();if(!viewport)throw std::invalid_argument("Projected shadow requires a viewport");
        const auto& resource=limits_.residentOnly?resources_.resident(id):resources_.load(id);const auto* sprite=std::get_if<assets::Sprite>(&resource.image);
        if(!sprite||frame>=sprite->frames.size())throw std::invalid_argument("Projected shadow requires a sprite frame");
        const auto& f=sprite->frames[frame];if(!f.width||!f.height)return;
        const auto left=std::int64_t(x)-f.originX,top=std::int64_t(y)-f.originY+f.height/2;
        // The original shadow route refuses horizontal clipping as a whole.
        if(left<viewport->left||left+f.width>viewport->right||top>=viewport->bottom||top+f.height<=viewport->top)return;
        const int first=int(std::max<std::int64_t>(0,viewport->top-top));
        const int last=int(std::min<std::int64_t>(f.height,viewport->bottom-top));
        if(first>=last)return;
        SpriteComposite darken;darken.mode=CompositeMode::half;
        drawUpload(request(id,frame,nullptr,ShadowRows{first,last}),destination,int(left),int(top+first),std::nullopt,darken);return;
    }
    checkThread();drawUpload(request(id,frame,colours),destination,x,y,viewport,composite);
}
void ResourceCache::drawUpload(const ResourceUploadRequest& r,SurfaceId destination,int x,int y,std::optional<Rect> viewport,const SpriteComposite& composite){
    if(atlas_){
        if(!atlas_->ready(r)){
            if(limits_.preparedOnly)throw std::runtime_error("Missing prepared atlas upload");
            supply(r,prepareResourceUpload(resources_.resident(r.id),r));
            auto bytes=std::numeric_limits<std::size_t>::max();while(!atlas_->advance(r,bytes)){}
        }
        atlas_->draw(r,destination,x,y,viewport,composite);return;
    }
    auto& uploaded=upload(r);
    if(viewport)uploaded.drawClipped(destination,x,y,*viewport,composite);else uploaded.draw(destination,x,y,composite);
}
void ResourceCache::release(const assets::ResourceId& id){
    checkThread();resources_.recipe(id); // reject invalid/unknown IDs before mutation
    if(atlas_){atlas_->release(id);return;}
    for(auto it=entries_.begin();it!=entries_.end();){if(it->id==id){const auto old=it++;erase(old);}else ++it;}
}
void ResourceCache::retire(const assets::ResourceId& id){release(id);resources_.unload(id);}
void ResourceCache::clear(){checkThread();if(atlas_){atlas_->clear();return;}while(!entries_.empty())erase(entries_.begin());}
ResourceCacheStats ResourceCache::stats() const{checkThread();return atlas_?atlas_->stats():stats_;}
}
