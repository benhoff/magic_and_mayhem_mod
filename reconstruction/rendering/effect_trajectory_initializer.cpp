#include "effect_trajectory_initializer.hpp"

namespace mnm::reconstruction {
namespace {
std::int64_t signedWord(std::uint32_t n) noexcept {
    return n<0x80000000u?std::int64_t(n):std::int64_t(n)-0x100000000ll;
}
bool greater(std::uint32_t a,std::uint32_t b) noexcept {
    return signedWord(a)>signedWord(b);
}
std::uint32_t magnitude(std::uint32_t n) noexcept {
    return n<0x80000000u?n:0u-n;
}
std::uint32_t distance(std::uint32_t from,std::uint32_t to,
                       std::uint32_t& direction) noexcept {
    if(greater(from,to)){direction=0xffffffffu;return from-to;}
    direction=1;return to-from;
}
}
void initializeEffectTrajectory(EffectTrajectory& state,
    const std::array<std::uint32_t,3>& from,
    const std::array<std::uint32_t,3>& to,
    const std::array<std::uint32_t,2>& periods,
    std::uint32_t multiplier,std::uint32_t& changes) noexcept {
    auto target=to;
    for(unsigned axis=0;axis<2;++axis){
        const auto period=periods[axis]*multiplier;
        const auto direct=magnitude(from[axis]-to[axis]);
        // Both comparisons use the original unadjusted target independently.
        if(greater(direct,magnitude(from[axis]+period-to[axis])))target[axis]-=period;
        if(greater(direct,magnitude(from[axis]-period-to[axis])))target[axis]+=period;
    }
    auto& s=state.words;
    s[4]=s[5]=s[6]=s[7]=0;
    const auto dx=distance(from[0],target[0],s[4]);
    const auto dy=distance(from[1],target[1],s[5]);
    if(!greater(dy,dx)){s[9]=dy;s[8]=dx;s[6]=s[4];}
    else{s[8]=dy;s[9]=dx;s[7]=s[5];}
    s[10]=s[8];s[8]*=2;s[9]*=2;
    s[0]=s[1]=s[2]=0;
    const auto spanX=magnitude(from[0]-target[0]);
    const auto spanY=magnitude(from[1]-target[1]);
    const auto span=greater(spanX,spanY)?spanX:spanY;
    const auto dz=distance(from[2],to[2],s[1]);
    if(!greater(span,dz)){s[13]=dz;s[12]=span;s[0]=s[1];}
    else{s[13]=span;s[12]=dz;s[2]=1;}
    s[11]=s[13];s[13]*=2;s[12]*=2;
    changes=0;
}
}
