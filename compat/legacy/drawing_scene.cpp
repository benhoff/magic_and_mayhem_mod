#include "drawing_scene.hpp"
#include "world_raster_batch.hpp"
#include "../../protocols/include/mnm/canvas_producers_v3.h"
#include <limits>
#include <stdexcept>

namespace mnm::legacy {
DrawingScene::DrawingScene(render::GlBlitter &renderer,const assets::AssetStore &store,
    std::function<std::vector<std::uint8_t>(const std::string &)> source)
    :renderer_(renderer),store_(store),replay_(std::move(source)),world_(std::make_unique<CanvasWorld>(renderer,store)){}
DrawingScene::~DrawingScene(){if(output_)renderer_.destroy(output_);}
std::optional<DrawingCheckpoint> DrawingScene::apply(const CanvasProducer &c){
    if(poisoned_||sealed_)throw std::runtime_error("Drawing scene is poisoned or sealed");
    try{
        rasterResult_.reset();
        if(output_){renderer_.destroy(output_);output_=0;}
        const auto &r=c.fields;
        if(sequence_==std::numeric_limits<unsigned>::max()||r[1]!=sequence_+1||
           r[2]<1||r[2]>MNM_PRODUCER_COLOUR_RECT||r[23]||
           c.bytes().size()>128u*1024u*1024u-96||r[0]!=96+c.bytes().size()||r[18]!=c.bytes().size())
            throw std::invalid_argument("Drawing scene source sequence or record is invalid");
        const bool independentCopy=r[2]==MNM_PRODUCER_COPY&&
            r[3]!=world_->canvas()&&r[4]!=world_->canvas();
        if(world_->active()&&!independentCopy&&r[2]!=MNM_PRODUCER_RASTER&&r[2]!=MNM_PRODUCER_CLIP&&
           r[2]!=MNM_PRODUCER_CHECKPOINT&&r[2]!=MNM_PRODUCER_QUEUE_RETURN&&r[2]!=MNM_PRODUCER_PAYLOAD_SOURCE)
            throw std::invalid_argument("Unadmitted producer inside native World scene");
        if(r[2]==MNM_PRODUCER_QUEUE_RETURN&&(!world_->active()||r[3]!=world_->canvas()||r[14]!=activeQueue_))
            throw std::invalid_argument("Drawing scene World return identity changed");
        if(r[2]==MNM_PRODUCER_QUEUE_RETURN){
            const auto prior=replay_.read(r[3]);
            if(prior.width!=int(r[5])||prior.height!=int(r[6]))
                throw std::invalid_argument("Drawing scene World return extent changed");
        }
        if(r[2]==MNM_PRODUCER_QUEUE_ENTRY&&
           (completedQueues_==std::numeric_limits<unsigned>::max()||r[14]!=completedQueues_+1))
            throw std::invalid_argument("Drawing scene World sequence gap or exhaustion");
        if(r[2]==MNM_PRODUCER_RASTER&&world_->active()&&worldDraws_==std::numeric_limits<std::uint64_t>::max())
            throw std::invalid_argument("Drawing scene raster count exhausted");
        replay_.apply(c);
        if(r[2]==MNM_PRODUCER_QUEUE_ENTRY){
            if(world_->canvas()&&(world_->canvas()!=r[3]||worldWidth_!=r[5]||worldHeight_!=r[6])){
                world_=std::make_unique<CanvasWorld>(renderer_,store_);worldSequence_=0;
            }
            if(worldSequence_==std::numeric_limits<std::uint64_t>::max())
                throw std::invalid_argument("Drawing scene World generation exhausted");
            // CanvasWorld's rolling interface uses a local queue ordinal. The
            // outer adapter retains and validates the original global identity.
            auto normalized=c;normalized.fields[14]=1;
            world_->beginRolling(normalized,replay_.read(r[3]),++worldSequence_);
            activeQueue_=r[14];worldWidth_=r[5];worldHeight_=r[6];
        }else if(world_->active()){
            if(r[2]==MNM_PRODUCER_RASTER){
                world_->append(c);++worldDraws_;
                // Recorded original EAX (field21) never supplies this result.
                rasterResult_=DrawingRasterResult{r[1],activeQueue_,r[3],r[15],
                    r[15]==8?std::nullopt:std::optional<unsigned>(worldRasterAX(c))};
            }
            else if((r[2]==MNM_PRODUCER_CHECKPOINT&&r[3]==world_->canvas())||r[2]==MNM_PRODUCER_QUEUE_RETURN){
                replay_.commitNativeWorld(world_->canvas(),world_->complete(replay_.read(world_->canvas())));
                if(r[2]==MNM_PRODUCER_QUEUE_RETURN){
                    auto normalized=c;normalized.fields[14]=1;world_->end(normalized);
                    ++completedQueues_;activeQueue_=0;
                }
            }
        }
        if(r[2]==MNM_PRODUCER_RELEASE&&r[3]==world_->canvas()){
            world_=std::make_unique<CanvasWorld>(renderer_,store_);worldSequence_=0;
            worldWidth_=worldHeight_=0;
        }
        std::optional<DrawingCheckpoint> completed;
        if(r[2]==MNM_PRODUCER_CHECKPOINT){
            if(!r[14]||r[14]!=checkpoints_+1)throw std::invalid_argument("Drawing checkpoint identity gap");
            auto image=replay_.read(r[3]);
            if(image.width!=int(r[5])||image.height!=int(r[6]))
                throw std::invalid_argument("Drawing checkpoint extent changed");
            completed=DrawingCheckpoint{r[1],r[14],r[3],std::move(image)};
            ++checkpoints_;
        }
        sequence_=r[1];return completed;
    }catch(...){poisoned_=true;throw;}
}
std::optional<DrawingRasterResult> DrawingScene::rasterResult() const {
    if(poisoned_)throw std::runtime_error("Drawing scene is poisoned");
    return rasterResult_;
}
void DrawingScene::publish(DrawingBoundary boundary){
    if(poisoned_||sealed_)throw std::runtime_error("Drawing scene is poisoned or sealed");
    try{
        if(world_->active()||boundary.queues!=completedQueues_||
           !boundary.sequence||boundary.sequence!=sequence_||boundary.sequence<=publishedSequence_||
           frames_==std::numeric_limits<unsigned>::max()||!boundary.canvas||!boundary.width||!boundary.height)
            throw std::invalid_argument("Incomplete declared drawing scene boundary");
        const auto image=replay_.read(boundary.canvas);
        if(image.width!=int(boundary.width)||image.height!=int(boundary.height))
            throw std::invalid_argument("Drawing scene boundary extent changed");
        output_=renderer_.create(image,{16,{0xf800,0x07e0,0x001f}});
        if(renderer_.read(output_).pixels!=image.pixels)
            throw std::runtime_error("Completed scene GPU mirror differs");
        published_=boundary;publishedSequence_=sequence_;++frames_;
    }catch(...){poisoned_=true;throw;}
}
void DrawingScene::seal(DrawingBoundary boundary){
    if(poisoned_||sealed_)throw std::runtime_error("Drawing scene is poisoned or sealed");
    if(output_&&boundary.sequence==publishedSequence_){
        if(boundary.queues!=published_.queues||boundary.canvas!=published_.canvas||
           boundary.width!=published_.width||boundary.height!=published_.height){
            poisoned_=true;throw std::invalid_argument("Sealed boundary differs from published frame");
        }
    }else publish(boundary);
    sealed_=true;
}
void DrawingScene::requirePublished() const{
    if(poisoned_||!output_||publishedSequence_!=sequence_)
        throw std::runtime_error("Drawing scene has no complete current published output");
}
render::Image DrawingScene::sourceSnapshot(unsigned canvas) const {
    if(poisoned_||!canvas||(world_->active()&&world_->canvas()==canvas))
        throw std::runtime_error("Native source snapshot is unavailable");
    return replay_.read(canvas);
}
render::Image DrawingScene::read(){requirePublished();return renderer_.read(output_);}
render::GpuFrame DrawingScene::presentGpu(){requirePublished();return renderer_.presentGpu(output_);}
}
