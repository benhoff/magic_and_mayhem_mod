#include "buffers.hpp"
#include <algorithm>

namespace mnm::audio {
Error Device::setPrimaryVolume(std::int32_t value){
    if(value < -10000 || value > 0)return Error::invalid;
    primary_.volume=value;return Error::ok;
}
Error Device::playPrimary(std::uint32_t flags){
    if(flags!=1)return Error::unsupported;
    primary_.explicitlyPlaying=true;return Error::ok;
}
Error Device::setPrimaryOutputFormat(const PcmFormat& format){
    if(!validPcm(format))return Error::badFormat;
    primary_.format=format;return Error::ok;
}

Error Device::play(BufferId id,std::uint32_t flags){
    const auto it=buffers_.find(id);
    if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    if(flags&~1u)return Error::unsupported;
    auto& v=it->second.voice;
    // Explicit native completion policy: restarting a consumed one-shot rewinds.
    if(v.frame==v.frames){v.frame=0;it->second.phase=0;}
    v.looping=(flags&1)!=0;v.playback=Playback::playing;return Error::ok;
}
Error Device::stop(BufferId id){
    const auto it=buffers_.find(id);
    if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    it->second.voice.playback=Playback::stopped;return Error::ok;
}
Error Device::resetPosition(BufferId id){
    const auto it=buffers_.find(id);
    if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    auto& v=it->second.voice;v.frame=0;
    it->second.phase=0;
    if(v.playback==Playback::completed)v.playback=Playback::stopped;
    return Error::ok;
}
Error Device::setVolume(BufferId id,std::int32_t value){
    const auto it=buffers_.find(id);
    if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    if(value < -10000 || value > 0)return Error::invalid;
    it->second.voice.volume=value;return Error::ok;
}
Error Device::setPan(BufferId id,std::int32_t value){
    const auto it=buffers_.find(id);
    if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    if(value < -10000 || value > 10000)return Error::invalid;
    it->second.voice.pan=value;return Error::ok;
}
Error Device::advanceFrames(BufferId id,std::uint64_t frames,AdvanceResult& output){
    const auto it=buffers_.find(id);
    if(it==buffers_.end())return Error::invalid;
    if(it->second.primary)return Error::unsupported;
    advanceVoice(it->second,frames,output);return Error::ok;
}
void Device::advanceVoice(Buffer& buffer,std::uint64_t frames,AdvanceResult& output){
    auto& v=buffer.voice;
    output={};
    if(v.playback!=Playback::playing || !frames)return;
    const auto remaining=v.frames-v.frame;
    if(v.looping){
        const auto step=frames%v.frames;
        v.frame=step<remaining?v.frame+step:step-remaining;
        output.consumed=frames;
    }else{
        output.consumed=std::min(frames,remaining);v.frame+=output.consumed;
        if(v.frame==v.frames){v.playback=Playback::completed;buffer.phase=0;output.completed=true;}
    }
}
std::optional<VoiceInfo> Device::voice(BufferId id) const{
    const auto it=buffers_.find(id);
    if(it==buffers_.end() || it->second.primary)return {};
    return it->second.voice;
}
}
