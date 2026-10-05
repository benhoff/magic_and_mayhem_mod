#include "segment_setup.hpp"
#include <cstdlib>
#include <stdexcept>
namespace mnm::reconstruction {
int ordinary_creature_height(int layer,int height) {
    if(layer<0 || layer>32 || height<-16 || height>16) throw std::invalid_argument("unsupported ordinary terrain height");
    return layer*16+height;
}
SegmentSetup initialize_creature_segment(const SegmentPrevious& old,const SegmentRequest& p,const CreatureScalarState& scalar) {
    const bool vertical=p.vertical!=0;
    if((p.category!=0 && p.category!=4) || p.vertical<-1 || p.vertical>1 || p.delta.z<-1 || p.delta.z>1 ||
       (vertical && (p.delta.x || p.delta.y || p.vertical!=p.delta.z)) || p.direction<0 || p.direction>7 ||
       old.direction<0 || old.direction>7 || old.category<0 || old.category>4 ||
       old.vertical<-1 || old.vertical>1 || p.gridZ<0 || p.gridZ>32 ||
       p.destinationTerrainHeight<-16 || p.destinationTerrainHeight>16 ||
       p.heightOrigin<-16 || p.heightOrigin>528 ||
       p.delta.x<-1 || p.delta.x>1 || p.delta.y<-1 || p.delta.y>1 || (!vertical && !p.delta.x && !p.delta.y))
        throw std::invalid_argument("unsupported segment setup profile");
    auto turn=(p.direction-old.direction+8)&7;if(turn>4) turn-=8;
    SegmentSetup out;out.state=old.state;
    out.heightOrigin=p.heightOrigin;
    out.heightDelta=ordinary_creature_height(p.gridZ+p.delta.z,p.destinationTerrainHeight)-p.heightOrigin;
    if(out.heightDelta<-32 || out.heightDelta>32) throw std::invalid_argument("unsupported segment terrain height difference");
    const auto first=creature_movement_scalar(4,p.category,p.delta,old.rate,scalar);
    out.rate=first.scalar;out.duration=first.base;
    if(old.direction==p.direction && old.vertical==p.vertical && old.action==2 && old.category==p.category) {
        if(old.state.progress<192 || old.state.progress>=384) throw std::invalid_argument("unsupported carried progress");
        out.carried=true;out.state.progress-=192;
        constexpr int dx[8]={0,1,1,1,0,-1,-1,-1},dy[8]={-1,-1,0,1,1,1,0,-1};
        out.state.travelX-=vertical?0:dx[p.direction]*192;out.state.travelY-=vertical?0:dy[p.direction]*192;
        out.sampleBank=p.direction&1;out.animation=p.direction;
        return out;
    }
    out.state.progress=out.state.travelX=out.state.travelY=0;
    out.state.residualX=out.state.residualY=0;out.state.frame=0;
    if(old.action!=2 && (!old.preserveCandidate || old.vertical!=p.vertical || std::abs(turn)>1)) {
        out.resetRate=true;out.state.accumulator=0;out.rate=0;
    }
    out.rate=creature_movement_scalar(4,p.category,p.delta,out.rate,scalar).scalar;
    out.sampleBank=p.direction&1;out.animation=p.direction;
    out.state.frame=out.state.initialFrame=out.sampleBank*12;out.state.animationFrame=0;
    out.state.initialResidualX=out.state.initialResidualY=0;
    return out;
}
}
