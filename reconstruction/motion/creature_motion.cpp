#include "creature_motion.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace mnm::reconstruction {
bool advance_creature_motion(MotionState& committed,const MotionInputs& p) {
    auto s=committed;
    if(p.rate<1 || p.rate>1000000 || p.duration<1 || p.duration>1000000 ||
       p.direction<0 || p.direction>7 || std::abs(std::int64_t(p.gridX))>1000000 ||
       std::abs(std::int64_t(p.gridY))>1000000 || std::abs(std::int64_t(p.heightOrigin))>1000000 ||
       std::abs(std::int64_t(p.heightDelta))>32 || s.accumulator<0 || s.accumulator>2000000 ||
       s.progress<0 || s.progress>=192 || s.frame>=12 ||
       std::abs(std::int64_t(s.travelX))>384 || std::abs(std::int64_t(s.travelY))>384 ||
       std::abs(std::int64_t(s.fineX))>32000400 || std::abs(std::int64_t(s.fineY))>32000400 ||
       std::abs(std::int64_t(s.fineZ))>1000032 || std::abs(std::int64_t(s.residualX))>1000000 || std::abs(std::int64_t(s.residualY))>1000000)
        throw std::invalid_argument("unsupported bounded motion input");
    for(auto sample:p.samples) if(sample<1 || sample>192) throw std::invalid_argument("unsupported motion sample");
    s.accumulator+=p.rate;
    auto count=s.accumulator/p.duration;
    if(count>4096) throw std::invalid_argument("motion substep bound exceeded");
    // The original type-12/category-2-or-3 path forces one substep, only after
    // the accumulator first admits a positive iteration count.
    if(count>0 && p.force32) count=1;
    constexpr int dx[8]={0,1,1,1,0,-1,-1,-1};
    constexpr int dy[8]={-1,-1,0,1,1,1,0,-1};
    for(int i=0;i<count;++i) {
        const auto step=p.force32?32:p.samples[s.frame]*(p.vertical?2:1);
        s.progress+=step;
        s.travelX+=p.vertical?0:dx[p.direction]*step;
        s.travelY+=p.vertical?0:dy[p.direction]*step;
        auto oldX=s.fineX,oldY=s.fineY;
        s.fineX=p.gridX*32+s.travelX/6;
        s.fineY=p.gridY*32+s.travelY/6;
        s.fineZ=p.heightOrigin+std::clamp(p.heightDelta*s.progress/192,-16,16);
        s.residualX+=oldX-s.fineX;s.residualY+=oldY-s.fineY;
        if(++s.frame==12) {s.frame=0;s.residualX=0;s.residualY=0;}
        s.accumulator-=p.duration;
        if(s.progress>=192) {committed=s;return true;}
    }
    committed=s;return false;
}
}
