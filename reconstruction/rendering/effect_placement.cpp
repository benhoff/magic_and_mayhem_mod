#include "effect_placement.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::int64_t signedWord(std::uint32_t n){return n<=0x7fffffffu?std::int64_t(n):std::int64_t(n)-0x100000000ll;}
unsigned wrap(std::uint32_t n,unsigned extent){
    auto value=signedWord(n);
    if(value< -std::int64_t(extent) || value>=2*std::int64_t(extent))
        throw std::invalid_argument("Effect coordinate exceeds one-period wrapping scope");
    if(value<0)value+=extent;
    if(value>=extent)value-=extent;
    return unsigned(value);
}
}
EffectPlacementPool::EffectPlacementPool(unsigned width,unsigned height,unsigned layers,unsigned capacity)
    :width_(width),height_(height),layers_(layers){
    if(!width || !height || !layers || width>128 || height>128 || layers>32 || capacity>65535)
        throw std::invalid_argument("Effect placement exceeds owned bounds");
    records_.resize(capacity);cells_.resize(width*height*layers);
}
void EffectPlacementPool::setScan(unsigned scan){
    if(scan>records_.size())throw std::invalid_argument("Effect scan exceeds capacity");
    scan_=scan;
}
void EffectPlacementPool::place(unsigned slot,unsigned type,const std::array<std::uint32_t,63>& input){
    if(slot>=records_.size() || records_[slot].active)throw std::invalid_argument("Effect slot unavailable");
    if(type!=3 && type!=13 && type!=22 && type!=24 && type!=36)
        throw std::invalid_argument("Effect type-specific creation remains outside recovered placement scope");
    if(input[6]!=0xffffffffu)throw std::invalid_argument("Effect creator ownership remains outside placement scope");
    auto candidate=records_[slot];candidate.parameters=input;
    for(unsigned at:{0u,3u}){
        candidate.parameters[at]=wrap(input[at],width_*32);
        candidate.parameters[at+1]=wrap(input[at+1],height_*32);
        if(signedWord(input[at+2])<0)throw std::invalid_argument("Negative effect height");
        candidate.parameters[at+2]=std::min(input[at+2],layers_*16-1);
    }
    candidate.units={candidate.parameters[0],candidate.parameters[1],candidate.parameters[2]};
    candidate.position={candidate.units[0]>>5,candidate.units[1]>>5,candidate.units[2]>>4};
    candidate.initialPosition=candidate.position;candidate.sentinels.fill(0xffffffffu);
    candidate.cell=(candidate.position[2]*height_+candidate.position[1])*width_+candidate.position[0];
    candidate.active=true;candidate.initialized=true;candidate.type=type;
    auto cell=cells_[candidate.cell];unsigned tail=NoEffect;
    if(!(cell.flags&0x40000000u)){
        if(cell.head==NoEffect)cell.head=std::uint16_t(slot);
        else{
            unsigned at=cell.head,steps=0;
            while(true){
                if(at>=records_.size() || at==slot || ++steps>records_.size())
                    throw std::invalid_argument("Invalid effect cell chain");
                if(records_[at].next==NoEffect){tail=at;break;}
                at=records_[at].next;
            }
            candidate.next=NoEffect;
        }
        cell.flags&=~0x80u;
    }
    records_[slot]=std::move(candidate);cells_[records_[slot].cell]=cell;
    if(tail!=NoEffect)records_[tail].next=std::uint16_t(slot);
    scan_=std::max(scan_,slot+1);
}
TerrainLightObject EffectPlacementRecord::lightingSource(const EffectLightingTable& table) const{
    const auto index=table.lightIndex(type);
    if(active && index && (sentinels[6]==0xffffffffu || sentinels[7]==0xffffffffu))
        throw std::invalid_argument("Effect recount position awaits movement refresh");
    return {active,index,position[0],position[1],position[2],sentinels[6],sentinels[7]};
}
}
