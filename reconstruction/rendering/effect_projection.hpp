#pragma once
#include "effect_placement.hpp"
#include "effect_trajectory.hpp"

namespace mnm::reconstruction {
struct EffectProjectionState {
    EffectTrajectory trajectory;
    std::array<std::uint32_t,3> previousUnits{};
    std::uint32_t changes=0;
};
// Selected 004883f0 path: five placement types, empty terrain occupancy/no
// creatures, unchanged cell, metadata kind 0/68 with zero distance allowance.
// parameters[9] is the original iteration count (0..8). Authored trajectory
// setup remains separate. Refusals leave both inputs unchanged.
unsigned projectEffectSameCell(EffectPlacementRecord&,EffectProjectionState&,
                              unsigned width,unsigned height,unsigned layers);
}
