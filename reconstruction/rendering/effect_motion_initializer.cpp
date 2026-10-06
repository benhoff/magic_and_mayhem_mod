#include "effect_motion_initializer.hpp"
#include "effect_trajectory_initializer.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction {
void initializeEffectMotionRecord(EffectPlacementRecord& record,
    EffectTransitionState& movement,std::array<std::uint32_t,3>& startupUnits,
    const std::array<std::uint32_t,3>& from,const std::array<std::uint32_t,3>& to,
    unsigned width,unsigned height,unsigned layers,const std::vector<EffectCell>& cells){
    if(!width || !height || !layers || width>128 || height>128 || layers>32 ||
       cells.size()!=std::size_t(width)*height*layers ||
       from[0]>=width*32 || from[1]>=height*32 || from[2]>=layers*16)
        throw std::invalid_argument("Effect motion initialization outside owned start/cell bounds");
    const std::array<unsigned,3> position{from[0]>>5,from[1]>>5,from[2]>>4};
    const unsigned cell=(position[2]*height+position[1])*width+position[0];
    EffectTrajectory trajectory{};
    std::uint32_t changes=0;
    initializeEffectTrajectory(trajectory,from,to,{width,height},32,changes);
    std::fill_n(record.sentinels.begin(),9,0xffffffffu);
    record.units=from;record.position=position;record.cell=cell;
    movement.motion.trajectory=trajectory;movement.motion.changes=changes;
    movement.terrain=cells[cell].terrain;startupUnits=from;
}
}
