#include "effect_trajectory.hpp"

namespace mnm::reconstruction {
namespace {
bool positive(std::uint32_t n) noexcept {return n!=0 && n<0x80000000u;}
}
void EffectTrajectory::step(std::array<std::uint32_t,3>& units,std::uint32_t& changes) noexcept {
    auto& s=words;
    s[11]-=s[12];
    const bool primary=positive(s[11]);
    if(!primary)s[11]+=s[13];
    const auto delta=s[primary?0:1];
    units[2]+=delta;
    s[3]=primary?1u:0u;
    if(delta!=0)++changes;
    if(primary && s[2]==0)return;
    s[10]-=s[9];
    if(positive(s[10])){
        units[0]+=s[6];units[1]+=s[7];changes+=2;
    }else{
        s[10]+=s[8];units[0]+=s[4];units[1]+=s[5];changes+=3;
    }
}
}
