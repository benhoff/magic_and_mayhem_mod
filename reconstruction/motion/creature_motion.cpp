#include "creature_motion.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace mnm::reconstruction {
bool advance_creature_motion(MotionState& committed,const MotionInputs& p,const MotionAnimation* animation) {
    auto s=committed;
    if(p.rate<0 || p.rate>1000000 || p.duration<1 || p.duration>1000000 ||
       p.direction<0 || p.direction>7 || std::abs(std::int64_t(p.gridX))>1000000 ||
       std::abs(std::int64_t(p.gridY))>1000000 || std::abs(std::int64_t(p.heightOrigin))>1000000 ||
       std::abs(std::int64_t(p.heightDelta))>32 || s.accumulator<0 || s.accumulator>2000000 ||
       s.progress<0 || s.progress>=192 || s.frame>=(p.separateCursor?48U:12U) ||
       (p.separateCursor && (s.animationFrame>=(animation?50U:12U) || s.initialFrame>=48 || std::abs(std::int64_t(s.initialResidualX))>1000000 || std::abs(std::int64_t(s.initialResidualY))>1000000)) ||
       std::abs(std::int64_t(s.travelX))>384 || std::abs(std::int64_t(s.travelY))>384 ||
       std::abs(std::int64_t(s.fineX))>32000400 || std::abs(std::int64_t(s.fineY))>32000400 ||
       std::abs(std::int64_t(s.fineZ))>1000032 || std::abs(std::int64_t(s.residualX))>1000000 || std::abs(std::int64_t(s.residualY))>1000000)
        throw std::invalid_argument("unsupported bounded motion input");
    if(animation && (!p.separateCursor || !animation->tick || !animation->restart)) throw std::invalid_argument("incomplete motion animation driver");
    for(unsigned n=0;n<(p.separateCursor?48U:12U);++n) if(p.samples[n]<1 || p.samples[n]>192) throw std::invalid_argument("unsupported motion sample");
    s.accumulator+=p.rate;
    auto count=s.accumulator/p.duration;
    if(count>4096) throw std::invalid_argument("motion substep bound exceeded");
    // The original type-12/category-2-or-3 path forces one substep, only after
    // the accumulator first admits a positive iteration count.
    if(count>0 && p.force32) count=1;
    constexpr int dx[8]={0,1,1,1,0,-1,-1,-1};
    constexpr int dy[8]={-1,-1,0,1,1,1,0,-1};
    for(int i=0;i<count;++i) {
        if(s.frame>=p.samples.size()) throw std::invalid_argument("sample cursor outside owned profile");
        const auto step=p.force32?32:p.samples[s.frame]*(p.vertical?2:1);
        s.progress+=step;
        s.travelX+=p.vertical?0:dx[p.direction]*step;
        s.travelY+=p.vertical?0:dy[p.direction]*step;
        auto oldX=s.fineX,oldY=s.fineY;
        s.fineX=p.gridX*32+s.travelX/6;
        s.fineY=p.gridY*32+s.travelY/6;
        s.fineZ=p.heightOrigin+std::clamp(p.heightDelta*s.progress/192,-16,16);
        s.residualX+=oldX-s.fineX;s.residualY+=oldY-s.fineY;
        ++s.frame;
        if(animation) {
            ++s.animationFrame;
            const auto event=animation->tick();
            if(event==2) {animation->restart();s.animationFrame=0;s.frame=s.initialFrame;s.residualX=s.initialResidualX;s.residualY=s.initialResidualY;}
            else if(event!=0) throw std::invalid_argument("unsupported movement animation event");
        } else if(p.separateCursor) {
            if(++s.animationFrame==12) {s.animationFrame=0;s.frame=s.initialFrame;s.residualX=s.initialResidualX;s.residualY=s.initialResidualY;}
        } else if(s.frame==12) {s.frame=0;s.residualX=0;s.residualY=0;}
        s.accumulator-=p.duration;
        if(s.progress>=192) {committed=s;return true;}
    }
    committed=s;return false;
}
}
