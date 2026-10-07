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
    if(width && !draw.clip && (x<0 || y<0 || x+width>viewport_.right || y+height>viewport_.bottom))
        throw std::invalid_argument("Unclipped scene draw outside canvas");
}
void SceneRenderer::draw(const std::vector<SceneDraw>& draws){
    checkThread();if(draws.size()>limits_.draws)throw std::invalid_argument("Scene draw budget exceeded");
    for(const auto& draw:draws)validate(draw);
    // Admission errors preserve the previous completed frame. An execution error
    // refuses presentation of partial pixels until a later successful full draw.
    valid_=false;
    renderer_.copy(background_,canvas_,viewport_,0,0);
    for(const auto& draw:draws)if(draw.visible)
        cache_.draw(draw.resource,draw.frame,canvas_,draw.anchorX,draw.anchorY,
                    draw.clip?std::make_optional(viewport_):std::nullopt,draw.colours?&*draw.colours:nullptr);
    valid_=true;
}
Image SceneRenderer::read(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.read(canvas_);}
QImage SceneRenderer::present(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.present(canvas_);}
GpuFrame SceneRenderer::presentGpu(){checkThread();if(!valid_)throw std::runtime_error("No complete scene frame");return renderer_.presentGpu(canvas_);}
ResourceCacheStats SceneRenderer::cacheStats() const{checkThread();return cache_.stats();}
void SceneRenderer::release(const assets::ResourceId& id){checkThread();cache_.release(id);}
}
