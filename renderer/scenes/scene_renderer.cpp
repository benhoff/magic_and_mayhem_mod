#include "scene_renderer.hpp"
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <limits>

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
    if(draw.colourRectangle){
        const auto& p=draw.colourRectangle->pixels;
        if(draw.additive||draw.colours||!draw.clip||p.width<1||p.height<1||p.width>64||p.height>64||p.pixels.size()!=std::size_t(p.width)*p.height||std::any_of(p.pixels.begin(),p.pixels.end(),[](auto v){return v>65535;})||
            (draw.composite.mode!=CompositeMode::copy&&draw.composite.mode!=CompositeMode::half&&draw.composite.mode!=CompositeMode::quarterSource&&draw.composite.mode!=CompositeMode::quarterDestination))throw std::invalid_argument("Invalid colour rectangle");
        if(draw.viewport){const auto c=*draw.viewport;if(c.left<0||c.top<0||c.right<=c.left||c.bottom<=c.top||c.right>viewport_.right||c.bottom>viewport_.bottom)throw std::invalid_argument("Colour clip outside canvas");}
        return;
    }
    if(draw.additive){
        const auto& a=*draw.additive;
        if(a.width<1||a.height<1||a.width>2048||a.height>2048||draw.colours||draw.composite.mode!=CompositeMode::copy||!draw.clip)
            throw std::invalid_argument("Invalid additive scene geometry/state");
        if(draw.viewport){const auto r=*draw.viewport;if(r.left<0||r.top<0||r.right<=r.left||r.bottom<=r.top||r.right>viewport_.right||r.bottom>viewport_.bottom)throw std::invalid_argument("Additive clip outside canvas");}
        return;
    }
    const auto& resource=limits_.cache.residentOnly?resources_.resident(draw.resource):resources_.load(draw.resource);
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
    if(limits_.cache.preparedOnly)throw std::runtime_error("Prepared scenes require incremental drawing");
    beginFrame(draws);while(!drawNext(limits_.draws)){}
}
void SceneRenderer::adoptNativeCanvas(const Image& image){
    checkThread();
    if(drawing_)throw std::runtime_error("Cannot hand off into an incomplete World frame");
    if(image.width!=viewport_.right||image.height!=viewport_.bottom||
       image.pixels.size()!=std::size_t(image.width)*image.height||
       std::any_of(image.pixels.begin(),image.pixels.end(),[](auto p){return p>65535;}))
        throw std::invalid_argument("Native World handoff requires matching fully defined RGB565 storage");
    valid_=false;
    renderer_.update(canvas_,0,0,image);
    valid_=true;
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
    pending_=std::move(owned);cursor_=0;drawing_=true;need_.reset();supplied_=false;
}
std::optional<ResourceUploadRequest> SceneRenderer::request(const SceneDraw& draw){
    if(!draw.visible||draw.additive||draw.colourRectangle)return {};
    if(draw.composite.mode!=CompositeMode::projectedShadow)return cache_.request(draw.resource,draw.frame,draw.colours?&*draw.colours:nullptr);
    const auto& f=std::get<assets::Sprite>(resources_.resident(draw.resource).image).frames.at(draw.frame);
    const auto r=draw.viewport.value_or(viewport_);
    const auto left=std::int64_t(draw.anchorX)-f.originX,top=std::int64_t(draw.anchorY)-f.originY+f.height/2;
    if(!f.width||!f.height||left<r.left||left+f.width>r.right||top>=r.bottom||top+f.height<=r.top)return {};
    const int first=int(std::max<std::int64_t>(0,r.top-top)),last=int(std::min<std::int64_t>(f.height,r.bottom-top));
    if(first>=last)return {};
    return cache_.request(draw.resource,draw.frame,nullptr,ShadowRows{first,last});
}
std::optional<SceneUploadNeed> SceneRenderer::uploadNeed() const{checkThread();return supplied_?std::nullopt:need_;}
void SceneRenderer::supplyUpload(const SceneUploadNeed& need,PreparedSpriteFrame prepared){
    checkThread();if(!drawing_||!need_||supplied_||need.draw!=cursor_||!(need.request==need_->request))throw std::runtime_error("Stale scene upload completion");
    // Verify geometry without scanning/converting pixels on the GUI thread.
    const auto& resource=resources_.resident(need.request.id);int width=0,height=0,ox=0,oy=0;
    std::visit([&](const auto& image){using T=std::decay_t<decltype(image)>;
        if constexpr(std::is_same_v<T,assets::Sprite>){const auto& f=image.frames.at(need.request.frame);width=f.width;height=f.height;ox=f.originX;oy=f.originY;}
        else{width=image.width;height=image.height;}
    },resource.image);
    if(need.request.shadow){const auto r=*need.request.shadow;width+=r.last/2;height=(r.last-r.first+1)/2;ox=oy=0;}
    if(prepared.pixels.width!=width||prepared.pixels.height!=height||prepared.originX!=ox||prepared.originY!=oy)throw std::runtime_error("Prepared upload geometry differs");
    cache_.supply(need.request,std::move(prepared));supplied_=true;
}
bool SceneRenderer::drawNext(std::size_t budget){
    return drawNext(SceneBatchBudget{budget,std::numeric_limits<std::size_t>::max(),std::chrono::hours(1)});
}
bool SceneRenderer::drawNext(const SceneBatchBudget& budget){
    checkThread();if(!drawing_)throw std::runtime_error("No admitted scene frame");
    if(!budget.draws||budget.draws>limits_.draws||budget.uploadBytes<8192||budget.time.count()<=0)throw std::invalid_argument("Invalid scene batch budget");
    const auto deadline=std::chrono::steady_clock::now()+budget.time;auto bytes=budget.uploadBytes;
    const auto end=cursor_+std::min(budget.draws,pending_.size()-cursor_);
    try{renderer_.batch([&]{
        bool first=true;
        while(cursor_<end && (first||std::chrono::steady_clock::now()<deadline)){first=false;const auto& draw=pending_[cursor_];
            if(limits_.cache.preparedOnly){const auto req=request(draw);
                if(req&&!cache_.ready(*req)){
                    if(need_&&!(need_->request==*req))throw std::runtime_error("Resource changed during pending upload");
                    if(!need_){need_=SceneUploadNeed{cursor_,*req};supplied_=false;}
                    if(!supplied_)break;
                    const auto before=bytes;
                    if(!cache_.advance(*req,bytes)){if(bytes==before)break;else continue;}
                }
            }
            if(draw.visible&&draw.colourRectangle){
                const auto& pixels=draw.colourRectangle->pixels;const auto clip=draw.viewport.value_or(viewport_);
                const auto left=std::max<std::int64_t>(draw.anchorX,clip.left),top=std::max<std::int64_t>(draw.anchorY,clip.top);
                const auto right=std::min<std::int64_t>(std::int64_t(draw.anchorX)+pixels.width,clip.right),bottom=std::min<std::int64_t>(std::int64_t(draw.anchorY)+pixels.height,clip.bottom);
                if(left<right&&top<bottom)renderer_.colourRect(canvas_,pixels,{int(left-draw.anchorX),int(top-draw.anchorY),int(right-draw.anchorX),int(bottom-draw.anchorY)},int(left),int(top),draw.composite);
            }else if(draw.visible&&draw.additive){
                const auto& a=*draw.additive;const auto clip=draw.viewport.value_or(viewport_);
                const auto left=std::max<std::int64_t>(draw.anchorX,clip.left),top=std::max<std::int64_t>(draw.anchorY,clip.top);
                const auto right=std::min<std::int64_t>(std::int64_t(draw.anchorX)+a.width,clip.right),bottom=std::min<std::int64_t>(std::int64_t(draw.anchorY)+a.height,clip.bottom);
                if(left<right&&top<bottom)renderer_.additiveRect(canvas_,{int(left),int(top),int(right),int(bottom)},a.channels);
            }else if(draw.visible)cache_.draw(draw.resource,draw.frame,canvas_,draw.anchorX,draw.anchorY,
                draw.clip?std::make_optional(draw.viewport.value_or(viewport_)):std::nullopt,draw.colours?&*draw.colours:nullptr,draw.composite);
            ++cursor_;need_.reset();supplied_=false;
        }
    });}catch(...){drawing_=false;pending_.clear();need_.reset();throw;}
    const auto used=budget.uploadBytes-bytes;uploadStats_.bytes+=used;++uploadStats_.ticks;uploadStats_.maxTickBytes=std::max(uploadStats_.maxTickBytes,std::uint64_t(used));
    if(cursor_!=pending_.size())return false;
    drawing_=false;pending_.clear();valid_=true;return true;
}
Image SceneRenderer::read(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.read(canvas_);}
QImage SceneRenderer::present(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.present(canvas_);}
GpuFrame SceneRenderer::presentGpu(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.presentGpu(canvas_);}
ResourceCacheStats SceneRenderer::cacheStats() const{checkThread();return cache_.stats();}
void SceneRenderer::release(const assets::ResourceId& id){checkThread();cache_.release(id);}
}
