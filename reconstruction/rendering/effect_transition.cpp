#include "effect_transition.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
unsigned wrap(std::uint32_t n,unsigned period){
    auto value=n<0x80000000u?std::int64_t(n):std::int64_t(n)-0x100000000ll;
    if(value< -std::int64_t(period) || value>=2*std::int64_t(period))
        throw std::invalid_argument("Effect transition exceeds one-period wrapping scope");
    if(value<0)value+=period;
    if(value>=period)value-=period;
    return unsigned(value);
}
bool near(std::array<unsigned,3> a,std::array<unsigned,3> b,unsigned w,unsigned h){
    auto delta=[](unsigned x,unsigned y){return x>y?x-y:y-x;};
    const auto dx=delta(a[0],b[0]),dy=delta(a[1],b[1]);
    return std::min(dx,w-dx)<=1 && std::min(dy,h-dy)<=1 && delta(a[2],b[2])<=1;
}
void validateCell(const EffectCell& cell){
    if(cell.terrain>3)
        throw std::invalid_argument("Effect transition requires empty terrain entries in catalog 0..3");
}
}
unsigned transitionEffectEmptyWorld(EffectPlacementPool& pool,unsigned slot,EffectTransitionState& state,
                                    unsigned width,unsigned height,unsigned layers,
                                    const std::vector<EffectCleanupColumn>& columns){
    if(!width || !height || !layers || width>128 || height>128 || layers>32 ||
       pool.cells().size()!=std::size_t(width)*height*layers || slot>=pool.records().size())
        throw std::invalid_argument("Effect transition pool dimensions/slot outside owned bounds");
    effectCleanupEligible(0,width,height,layers,columns); // Validate owned table before mutation.
    // Reuse the existing initial-record/projection admission without moving it.
    auto check=pool.records()[slot];const auto iterations=check.parameters[9];
    if(iterations>8)throw std::invalid_argument("Effect transition iteration bound exceeded");
    check.parameters[9]=0;auto checkMotion=state.motion;
    projectEffectSameCell(check,checkMotion,width,height,layers);
    for(auto pos:{check.initialPosition,state.previousPosition})
        if(pos[0]>=width || pos[1]>=height || pos[2]>=layers)
            throw std::invalid_argument("Effect transition cache outside dimensions");
    validateCell(pool.cells()[check.cell]);
    if(state.terrain!=pool.cells()[check.cell].terrain)
        throw std::invalid_argument("Effect transition terrain reference mismatch");
    std::vector<bool> seen(pool.records().size(),false);
    for(unsigned cell=0;cell<pool.cells().size();++cell){
        for(unsigned at=pool.cells()[cell].head;at!=NoEffect;at=pool.records()[at].next){
            if(at>=seen.size() || seen[at] || !pool.records()[at].active || pool.records()[at].cell!=cell)
                throw std::invalid_argument("Invalid or duplicate effect membership chain");
            seen[at]=true;
        }
    }
    if(!seen[slot])throw std::invalid_argument("Moving effect is absent from its cell chain");
    auto candidate=pool;auto movement=state;auto& r=candidate.records()[slot];
    for(unsigned step=0;step<iterations;++step){
        movement.motion.previousUnits=r.units;
        std::array<std::uint32_t,3> units{r.parameters[0],r.parameters[1],r.parameters[2]};
        movement.motion.trajectory.step(units,movement.motion.changes);
        if(units[2]>=layers*16){
            // Original trajectory writes parameters before checking height. Fine
            // units, wrapping, recount and membership still describe the last valid step.
            std::copy(units.begin(),units.end(),r.parameters.begin());
            pool=std::move(candidate);state=std::move(movement);return 2;
        }
        units[0]=wrap(units[0],width*32);units[1]=wrap(units[1],height*32);
        const std::array<unsigned,3> position{units[0]>>5,units[1]>>5,units[2]>>4};
        r.units=units;std::copy(units.begin(),units.end(),r.parameters.begin());
        for(unsigned k=0;k<3;++k)r.sentinels[6+k]=position[k];
        if(position!=r.position){
            const auto destination=(position[2]*height+position[1])*width+position[0];
            validateCell(candidate.cells()[destination]);
            auto& oldCell=candidate.cells()[r.cell];
            if(oldCell.head==slot)oldCell.head=r.next;
            else{
                unsigned before=oldCell.head;
                while(candidate.records()[before].next!=slot)before=candidate.records()[before].next;
                candidate.records()[before].next=r.next;
            }
            r.next=NoEffect;
            cleanupEmptyEffectCell(oldCell,r.cell,width,height,layers,columns);
            movement.previousPosition=r.position;r.position=position;r.cell=destination;
            auto& newCell=candidate.cells()[destination];
            if(!(newCell.flags&0x40000000u)){
            if(newCell.head==NoEffect)newCell.head=std::uint16_t(slot);
            else{
                unsigned tail=newCell.head;
                while(candidate.records()[tail].next!=NoEffect)tail=candidate.records()[tail].next;
                candidate.records()[tail].next=std::uint16_t(slot);
            }
            newCell.flags&=~0x80u;
            }
            movement.terrain=newCell.terrain;
            if(!near(r.position,r.initialPosition,width,height))r.initialPosition=movement.previousPosition;
            if(newCell.flags&0x40000000u){
                r.parameters[7]=0xffffffffu;
                pool=std::move(candidate);state=std::move(movement);return 1;
            }
        }
        r.sentinels[0]=(units[0]>>2)&7;r.sentinels[1]=(units[1]>>2)&7;r.sentinels[2]=(units[2]>>2)&3;
    }
    pool=std::move(candidate);state=std::move(movement);return 3;
}
}
