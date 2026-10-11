#include "drawing_scene.hpp"
#include "../../protocols/include/mnm/canvas_producers_v3.h"
#include <limits>
#include <stdexcept>

namespace mnm::legacy {
DrawingScene::DrawingScene(render::GlBlitter &renderer,const assets::AssetStore &store,
    std::function<std::vector<std::uint8_t>(const std::string &)> source)
    :renderer_(renderer),replay_(std::move(source)),world_(renderer,store){}
DrawingScene::~DrawingScene(){if(output_)renderer_.destroy(output_);}
std::optional<DrawingCheckpoint> DrawingScene::apply(const CanvasProducer &c){
    if(poisoned_||sealed_)throw std::runtime_error("Drawing scene is poisoned or sealed");
    try{
        if(output_){renderer_.destroy(output_);output_=0;}
        const auto &r=c.fields;
        if(sequence_==std::numeric_limits<unsigned>::max()||r[1]!=sequence_+1||
           r[2]<1||r[2]>MNM_PRODUCER_PAYLOAD_SOURCE||r[23]||
           c.bytes().size()>128u*1024u*1024u-96||r[0]!=96+c.bytes().size()||r[18]!=c.bytes().size())
            throw std::invalid_argument("Drawing scene source sequence or record is invalid");
        if(world_.active()&&r[2]!=MNM_PRODUCER_RASTER&&r[2]!=MNM_PRODUCER_CLIP&&
           r[2]!=MNM_PRODUCER_CHECKPOINT&&r[2]!=MNM_PRODUCER_QUEUE_RETURN&&r[2]!=MNM_PRODUCER_PAYLOAD_SOURCE)
            throw std::invalid_argument("Unadmitted producer inside native World scene");
        if(r[2]==MNM_PRODUCER_QUEUE_RETURN&&(!world_.active()||r[3]!=world_.canvas()||r[14]!=world_.queue()))
            throw std::invalid_argument("Drawing scene World return identity changed");
        if(r[2]==MNM_PRODUCER_QUEUE_RETURN){
            const auto prior=replay_.read(r[3]);
            if(prior.width!=int(r[5])||prior.height!=int(r[6]))
                throw std::invalid_argument("Drawing scene World return extent changed");
        }
        replay_.apply(c);
        if(r[2]==MNM_PRODUCER_QUEUE_ENTRY)world_.begin(c,replay_.read(r[3]));
        else if(world_.active()){
            if(r[2]==MNM_PRODUCER_RASTER)world_.append(c);
            else if((r[2]==MNM_PRODUCER_CHECKPOINT&&r[3]==world_.canvas())||r[2]==MNM_PRODUCER_QUEUE_RETURN){
                replay_.commitNativeWorld(world_.canvas(),world_.complete(replay_.read(world_.canvas())));
                if(r[2]==MNM_PRODUCER_QUEUE_RETURN)world_.end(c);
            }
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
void DrawingScene::publish(DrawingBoundary boundary){
    if(poisoned_||sealed_)throw std::runtime_error("Drawing scene is poisoned or sealed");
    try{
        if(world_.active()||boundary.queues!=world_.completedQueues()||
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
render::Image DrawingScene::read(){requirePublished();return renderer_.read(output_);}
render::GpuFrame DrawingScene::presentGpu(){requirePublished();return renderer_.presentGpu(output_);}
}
