#include "positional_audio.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::audio {
namespace {
std::int32_t checked(std::int64_t value){
    if(value<std::numeric_limits<std::int32_t>::min() || value>std::numeric_limits<std::int32_t>::max())
        throw std::out_of_range("Outside inspected nonoverflow positional domain");
    return std::int32_t(value);
}
}
std::int32_t wrappedDifference(std::int32_t source,std::int32_t listener,std::int32_t extent){
    if(extent<=0)throw std::invalid_argument("Map extent must be positive");
    auto delta=std::int64_t(checked(std::int64_t(source)-listener))%extent;
    if(delta<0)delta+=extent;
    if(delta>extent/2)delta-=extent; // Exact half-map tie remains positive.
    return std::int32_t(delta);
}
PositionalControls positionalControls(const PositionalInput& in){
    if(in.orientation>3 || in.range<=0 || in.panWidth<=0)
        throw std::invalid_argument("Expected orientation 0..3 and positive range/pan width");
    static constexpr int biasX[]={-6,6,6,-6},biasY[]={-6,-6,6,6};
    const auto x=wrappedDifference(in.sourceX,checked(std::int64_t(in.listenerX)+biasX[in.orientation]),in.mapWidth);
    const auto y=wrappedDifference(in.sourceY,checked(std::int64_t(in.listenerY)+biasY[in.orientation]),in.mapHeight);
    const auto ax=x<0?-std::int64_t(x):std::int64_t(x),ay=y<0?-std::int64_t(y):std::int64_t(y);
    const auto distance=checked(2*std::max(ax,ay)+std::min(ax,ay))/2;
    PositionalControls out;if(distance>in.range)return out;
    out.volume=checked(std::int64_t(in.range-distance)*5000)/in.range-5000;
    if(in.mapByte && *in.mapByte<in.byteThreshold){
        const auto numerator=checked(std::int64_t(*in.mapByte)-in.byteThreshold);
        const auto denominator=checked(-127ll-in.byteThreshold);
        if(!denominator)throw std::invalid_argument("Original map-byte divisor would be zero");
        const long double adjusted=(1.0L-static_cast<long double>(numerator)/denominator)*(out.volume+5000)-5000;
        if(adjusted<std::numeric_limits<std::int32_t>::min() || adjusted>std::numeric_limits<std::int32_t>::max())
            throw std::out_of_range("Map-byte adjustment outside integer domain");
        out.volume=std::int32_t(adjusted); // 0x59bee0 forces truncation toward zero.
    }
    if(out.volume<=-5000)return out;
    const auto difference=checked(std::int64_t(x)-y),sum=checked(std::int64_t(x)+y);
    std::int32_t lateral=0;
    switch(in.orientation){case 0:lateral=difference/2;break;case 1:lateral=sum/2;break;
        case 2:lateral=-(difference/2);break;case 3:lateral=-(sum/2);break;}
    out.pan=positionalPan(lateral,in.panWidth);out.panWritten=true;return out;
}
Status updatePositionalVoice(PositionalBackend& b,VoiceWrapper*& slot,const PositionalControls& c,
                             bool worldPresent,bool enabled){
    if(!worldPresent)return 0;
    auto& v=*slot; // Original requires a valid active slot on this path.
    if(c.volume<=-5000){
        Status result=0;
        if(enabled && v.buffer){
            std::uint32_t flags=0;
            if(b.getStatus(v.buffer,flags)!=0 || (flags&1)){
                result=b.stop(v.buffer);b.clearVoiceSchedule(v.identity);
                if(!result)result=b.position(v.buffer,0);
            }
        }
        slot=nullptr;return result;
    }
    if(v.cachedVolume!=c.volume){
        v.requestedVolume=c.volume;v.cachedVolume=c.volume;
        if(enabled && !b.volume(v.buffer,c.volume))b.positionalVolumeRecord(v.identity,c.volume);
    }
    if(enabled)b.pan(v.buffer,c.pan);
    return 0; // Audible update ignores volume/pan errors for its return.
}
}
