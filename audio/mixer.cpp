#include "buffers.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

namespace mnm::audio {
namespace {
double gain(std::int32_t attenuation){
    return attenuation<=-10000?0.0:std::pow(10.0,double(attenuation)/2000.0);
}
std::int16_t quantize(double sample){
    return std::int16_t(std::round(std::clamp(sample,-32768.0,32767.0)));
}
}
Error Device::mixStereo(std::size_t frames,std::vector<std::int16_t>& output){
    if(frames>maxMixFrames)return Error::limit;
    // All allocation occurs before playback mutation. No sample snapshots/copies.
    std::vector<std::int16_t> result(frames*2,0);
    struct Input {Buffer* buffer;double left,right;};
    std::vector<std::pair<BufferId,Buffer*>> ordered;
    for(auto& entry:buffers_)if(!entry.second.primary && entry.second.voice.playback==Playback::playing)
        ordered.emplace_back(entry.first,&entry.second);
    std::sort(ordered.begin(),ordered.end(),[](const auto& a,const auto& b){return a.first<b.first;});
    std::vector<Input> inputs;inputs.reserve(ordered.size());
    for(const auto& entry:ordered){
        const auto& v=entry.second->voice;const auto volume=gain(v.volume);
        inputs.push_back({entry.second,volume*(v.pan>0?gain(-v.pan):1.0),volume*(v.pan<0?gain(v.pan):1.0)});
    }
    const auto master=gain(primary_.volume);
    for(std::size_t frame=0;frame<frames;++frame){
        double left=0,right=0;
        for(const auto& input:inputs){
            auto& b=*input.buffer;const auto& v=b.voice;
            if(v.playback!=Playback::playing)continue;
            const auto next=v.frame+1<v.frames?v.frame+1:(v.looping?0:v.frame);
            const auto fraction=double(b.phase)/outputRate_;
            const auto l=sample(b,v.frame,0),r=sample(b,v.frame,1);
            left+=(l+(sample(b,next,0)-l)*fraction)*input.left;
            right+=(r+(sample(b,next,1)-r)*fraction)*input.right;
            const auto phase=std::uint64_t(b.phase)+b.format->rate;
            AdvanceResult advanced;advanceVoice(b,phase/outputRate_,advanced);
            b.phase=b.voice.playback==Playback::playing?std::uint32_t(phase%outputRate_):0;
        }
        result[frame*2]=quantize(left*master);result[frame*2+1]=quantize(right*master);
    }
    output.swap(result);return Error::ok;
}
}
