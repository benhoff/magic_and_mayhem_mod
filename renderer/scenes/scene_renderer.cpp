#include "scene_renderer.hpp"
#include <stdexcept>
#include <type_traits>

namespace mnm::render {
SceneRenderer::SceneRenderer(GlBlitter& renderer,assets::ResourceManager& resources,const Image& background,SceneLimits limits)
    :renderer_(renderer),resources_(resources),cache_(renderer,resources,limits.cache),limits_(limits),
     viewport_{0,0,background.width,background.height},thread_(std::this_thread::get_id()){
    if(!limits.draws || limits.draws>65536)throw std::invalid_argument("Scene draw budget must be 1..65536");
    background_=renderer_.create(background,spriteFormat);
    try{canvas_=renderer_.create(background,spriteFormat);}catch(...){renderer_.destroy(background_);throw;}
}
SceneRenderer::~SceneRenderer(){cache_.clear();renderer_.destroy(canvas_);renderer_.destroy(background_);}
void SceneRenderer::checkThread() const{
    if(std::this_thread::get_id()!=thread_)throw std::runtime_error("SceneRenderer used from another thread");
}
void SceneRenderer::validate(const SceneDraw& draw){
    if(!draw.visible)return;
    const auto& resource=resources_.load(draw.resource);
    if(draw.frame>=resource.frameCount())throw std::out_of_range("Scene frame outside resource");
    std::int64_t width=0,height=0,x=draw.anchorX,y=draw.anchorY;
    bool indexed=false;
    std::visit([&](const auto& image){using T=std::decay_t<decltype(image)>;
        if constexpr(std::is_same_v<T,assets::Sprite>){
            const auto& frame=image.frames[draw.frame];width=frame.width;height=frame.height;x-=frame.originX;y-=frame.originY;
            indexed=image.storage==assets::SpriteStorage::indexed8;
        }else{width=image.width;height=image.height;indexed=std::is_same_v<T,assets::PcxImage>;}
    },resource.image);
    if(width>2048 || height>2048 || std::uint64_t(width*height)*2>limits_.cache.pixels)
        throw std::runtime_error("Scene frame exceeds renderer/cache extent");
    if(draw.colours && !indexed)throw std::invalid_argument("Scene colour table requires indexed input");
    if(draw.viewport){const auto r=*draw.viewport;if(!draw.clip||r.left<0||r.top<0||r.right<=r.left||r.bottom<=r.top||r.right>viewport_.right||r.bottom>viewport_.bottom)throw std::invalid_argument("Scene draw viewport outside canvas");}
    if(draw.composite.mode!=CompositeMode::copy&&draw.composite.mode!=CompositeMode::half&&draw.composite.mode!=CompositeMode::quarterSource&&draw.composite.mode!=CompositeMode::quarterDestination&&draw.composite.mode!=CompositeMode::displace&&draw.composite.mode!=CompositeMode::projectedShadow)throw std::invalid_argument("Unsupported scene composite");
    if(draw.composite.mode==CompositeMode::projectedShadow){
        const auto* sprite=std::get_if<assets::Sprite>(&resource.image);if(!sprite||!draw.clip)throw std::invalid_argument("Projected shadow requires clipped SPR input");
        const auto r=draw.viewport.value_or(viewport_);const auto top=y+height/2;
        if(x>=r.left&&x+width<=r.right&&top<r.bottom&&top+height>r.top){const auto last=std::min(height,std::int64_t(r.bottom)-top);if(width+last/2>2048||x+width+last/2>viewport_.right)throw std::invalid_argument("Projected shadow exceeds storage");}
    }
    if(draw.composite.mode==CompositeMode::displace){const auto& c=draw.composite;if(!c.rowPeriod||c.rowPeriod>16)throw std::invalid_argument("Invalid displacement period");int reach=0;for(unsigned i=0;i<c.rowPeriod;++i){if(c.rowOffsets[i]<0||c.rowOffsets[i]>16)throw std::invalid_argument("Invalid displacement offset");reach=std::max(reach,c.rowOffsets[i]);}const auto r=draw.viewport.value_or(viewport_);if(std::min(x+width,std::int64_t(r.right))+reach>viewport_.right)throw std::invalid_argument("Scene displacement samples outside canvas");}
    if(width && !draw.clip && (x<0 || y<0 || x+width>viewport_.right || y+height>viewport_.bottom))
        throw std::invalid_argument("Unclipped scene draw outside canvas");
}
void SceneRenderer::draw(const std::vector<SceneDraw>& draws){
    beginFrame(draws);while(!drawNext(limits_.draws)){}
}
void SceneRenderer::beginFrame(const std::vector<SceneDraw>& draws,SceneStart start){
    checkThread();if(draws.size()>limits_.draws)throw std::invalid_argument("Scene draw budget exceeded");
    if(drawing_)throw std::runtime_error("Previous scene frame is incomplete");
    if(start!=SceneStart::background&&start!=SceneStart::retainedCanvas)throw std::invalid_argument("Unsupported scene start");
    if(start==SceneStart::retainedCanvas&&!valid_)throw std::runtime_error("No complete native canvas to retain");
    for(const auto& draw:draws)validate(draw);
    // Admission errors preserve the previous completed frame. An execution error
    // refuses presentation of partial pixels until a later successful full draw.
    auto owned=draws;valid_=false;
    if(start==SceneStart::background)renderer_.copy(background_,canvas_,viewport_,0,0);
    pending_=std::move(owned);cursor_=0;drawing_=true;
}
bool SceneRenderer::drawNext(std::size_t budget){
    checkThread();if(!drawing_)throw std::runtime_error("No admitted scene frame");
    if(!budget||budget>limits_.draws)throw std::invalid_argument("Invalid scene draw batch budget");
    const auto end=cursor_+std::min(budget,pending_.size()-cursor_);
    try{renderer_.batch([&]{
        for(;cursor_<end;++cursor_){const auto& draw=pending_[cursor_];if(draw.visible)
            cache_.draw(draw.resource,draw.frame,canvas_,draw.anchorX,draw.anchorY,
                        draw.clip?std::make_optional(draw.viewport.value_or(viewport_)):std::nullopt,draw.colours?&*draw.colours:nullptr,draw.composite);
        }
    });}catch(...){drawing_=false;pending_.clear();throw;}
    if(cursor_!=pending_.size())return false;
    drawing_=false;pending_.clear();valid_=true;return true;
}
Image SceneRenderer::read(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.read(canvas_);}
QImage SceneRenderer::present(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.present(canvas_);}
GpuFrame SceneRenderer::presentGpu(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.presentGpu(canvas_);}
ResourceCacheStats SceneRenderer::cacheStats() const{checkThread();return cache_.stats();}
void SceneRenderer::release(const assets::ResourceId& id){checkThread();cache_.release(id);}
}
