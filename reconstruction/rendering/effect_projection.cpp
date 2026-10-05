#include "effect_projection.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
unsigned wrap(std::uint32_t n,unsigned period){
    auto value=n<0x80000000u?std::int64_t(n):std::int64_t(n)-0x100000000ll;
    if(value< -std::int64_t(period) || value>=2*std::int64_t(period))
        throw std::invalid_argument("Effect projection exceeds one-period wrapping scope");
    if(value<0)value+=period;
    if(value>=period)value-=period;
    return unsigned(value);
}
}
unsigned projectEffectSameCell(EffectPlacementRecord& record,EffectProjectionState& state,
                              unsigned width,unsigned height,unsigned layers){
    if(!width || !height || !layers || width>128 || height>128 || layers>32)
        throw std::invalid_argument("Effect projection dimensions outside owned bounds");
    if(!record.active || !record.initialized ||
       (record.type!=3 && record.type!=13 && record.type!=22 && record.type!=24 && record.type!=36) ||
       record.parameters[6]!=0xffffffffu ||
       (record.parameters[8]!=0 && record.parameters[8]!=68) || record.parameters[9]>8 || state.changes>32)
        throw std::invalid_argument("Effect projection outside empty-cell movement scope");
    if(record.units[0]>=width*32 || record.units[1]>=height*32 || record.units[2]>=layers*16 ||
       record.position!=std::array<unsigned,3>{record.units[0]>>5,record.units[1]>>5,record.units[2]>>4} ||
       !std::equal(record.units.begin(),record.units.end(),record.parameters.begin()) ||
       record.cell!=(record.position[2]*height+record.position[1])*width+record.position[0])
        throw std::invalid_argument("Inconsistent effect projection positions");
    auto candidate=record;auto motion=state;
    for(unsigned step=0;step<record.parameters[9];++step){
        motion.previousUnits=candidate.units;
        std::array<std::uint32_t,3> units{candidate.parameters[0],candidate.parameters[1],candidate.parameters[2]};
        motion.trajectory.step(units,motion.changes);
        if(units[2]>=layers*16)throw std::invalid_argument("Effect height exit is not implemented");
        units[0]=wrap(units[0],width*32);units[1]=wrap(units[1],height*32);
        const std::array<unsigned,3> cells{units[0]>>5,units[1]>>5,units[2]>>4};
        if(cells!=record.position)throw std::invalid_argument("Effect cell transition is not implemented");
        candidate.units=units;
        std::copy(units.begin(),units.end(),candidate.parameters.begin());
        for(unsigned k=0;k<3;++k)candidate.sentinels[6+k]=cells[k];
        candidate.sentinels[0]=(units[0]>>2)&7;
        candidate.sentinels[1]=(units[1]>>2)&7;
        candidate.sentinels[2]=(units[2]>>2)&3;
    }
    record=std::move(candidate);state=std::move(motion);
    return 3;
}
}
