#include "output_timeline.hpp"
#include <algorithm>

namespace mnm::audio {
OutputTimeline::Point OutputTimeline::point(BufferId id) const{
    const auto& b=device_.buffers_.at(id);
    auto voice=b.voice;voice.frequency=voice.frequency?voice.frequency:b.format->rate;
    const auto it=presented_.find(id);
    return {id,it==presented_.end()?0:it->second.epoch,voice,b.phase};
}
OutputTimeline::Point OutputTimeline::advance(Point p,std::uint64_t frames,std::uint32_t rate){
    if(p.voice.playback!=Playback::playing)return p;
    // Each block is bounded by Device::maxMixFrames; multiplication cannot overflow.
    const auto clock=std::uint64_t(p.phase)+frames*p.voice.frequency;
    const auto step=clock/rate;p.phase=std::uint32_t(clock%rate);
    auto& v=p.voice;
    if(v.looping)v.frame=(v.frame+step)%v.frames;
    else if(step>=v.frames-v.frame){v.frame=v.frames;v.playback=Playback::completed;p.phase=0;}
    else v.frame+=step;
    return p;
}
Error OutputTimeline::render(std::size_t frames,std::vector<std::int16_t>& output){
    if(frames>Device::maxMixFrames)return Error::limit;
    if(!frames)return device_.mixStereo(0,output);
    if(blocks_.size()>=maxBlocks || generated_-consumed_+frames>std::uint64_t(device_.outputRate())*2)return Error::busy;
    Block block{generated_,frames,{}};block.points.reserve(device_.count());
    for(const auto& entry:device_.buffers_)if(!entry.second.primary){
        auto p=point(entry.first);
        if(presented_.find(p.id)==presented_.end())presented_.emplace(p.id,p);
        block.points.push_back(p);
    }
    blocks_.push_back(std::move(block)); // Allocate journal before advancing playback.
    Error error;
    try{error=device_.mixStereo(frames,output);}catch(...){blocks_.pop_back();throw;}
    if(error!=Error::ok){blocks_.pop_back();return error;}
    generated_+=frames;return Error::ok;
}
void OutputTimeline::consume(std::uint64_t frames){
    frames=std::min(frames,generated_);
    if(frames<=consumed_)return;
    while(!blocks_.empty()){
        const auto& block=blocks_.front();if(frames<block.begin)break;
        const auto within=std::min(frames-block.begin,block.frames);
        for(const auto& p:block.points){
            const auto it=presented_.find(p.id);
            if(it!=presented_.end() && it->second.epoch==p.epoch)it->second=advance(p,within,device_.outputRate());
        }
        if(within<block.frames)break;
        blocks_.pop_front();
    }
    consumed_=frames;
}
std::optional<VoiceInfo> OutputTimeline::voice(BufferId id) const{
    const auto generated=device_.voice(id);if(!generated)return {};
    const auto it=presented_.find(id);if(it==presented_.end())return generated;
    auto result=it->second.voice;
    result.volume=generated->volume;result.pan=generated->pan;result.frequency=generated->frequency;
    return result;
}
std::optional<std::size_t> OutputTimeline::cursor(BufferId id) const{
    const auto v=voice(id);if(!v)return {};
    return std::size_t(v->frame%v->frames)*device_.buffers_.at(id).format->alignment;
}
void OutputTimeline::changed(BufferId id){
    const auto it=device_.buffers_.find(id);
    if(it==device_.buffers_.end() || it->second.primary){presented_.erase(id);return;}
    auto p=point(id);p.epoch=nextEpoch_++;presented_.insert_or_assign(id,p);
}
Error OutputTimeline::play(BufferId id,std::uint32_t flags){
    const auto v=voice(id);
    // Repeated Play must not rewind a one-shot whose generated tail is still queued.
    if(v && v->playback==Playback::playing && flags==(v->looping?1u:0u))return Error::ok;
    const auto old=presented_.find(id);
    const auto prior=old==presented_.end()?std::optional<Point>{}:old->second;
    const auto error=device_.play(id,flags);
    if(error==Error::ok){
        changed(id);
        if(v && v->playback==Playback::playing && prior){
            auto& now=presented_.at(id);now.voice.frame=prior->voice.frame;now.phase=prior->phase;
        }
    }
    return error;
}
void OutputTimeline::restore(const Point& p){
    const auto it=device_.buffers_.find(p.id);if(it==device_.buffers_.end() || it->second.primary)return;
    auto& b=it->second;b.voice.frame=p.voice.frame;b.voice.playback=p.voice.playback;
    b.voice.looping=p.voice.looping;b.phase=p.phase;
}
Error OutputTimeline::stop(BufferId id){
    const auto it=presented_.find(id);if(it!=presented_.end())restore(it->second);
    const auto error=device_.stop(id);if(error==Error::ok)changed(id);return error;
}
Error OutputTimeline::seek(BufferId id,std::size_t offset){
    const auto prior=voice(id);
    const auto error=device_.setPosition(id,offset);if(error!=Error::ok)return error;
    // The renderer may already be completed while its audible tail is playing.
    // Seeking that logical playing voice must keep it active.
    if(prior && prior->playback==Playback::playing)device_.play(id,prior->looping?1u:0u);
    changed(id);return Error::ok;
}
void OutputTimeline::discard(){
    for(const auto& entry:presented_)restore(entry.second);
    blocks_.clear();presented_.clear();generated_=consumed_=0;
}
}
