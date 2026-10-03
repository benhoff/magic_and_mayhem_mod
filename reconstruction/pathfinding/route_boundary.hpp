#pragma once
#include "route_movement.hpp"

namespace mnm::reconstruction {
struct BoundaryHelpers {
    std::function<std::int32_t()> boundary;
    std::function<std::uint32_t(Coordinates,const CreatureMovementParameters&)> support; // 4f4330
    std::function<bool(Coordinates,const CreatureMovementParameters&)> valid; // 4f46b0
};
// 0x4f41a0: mode-dependent boundary predicate, not a general acceptance result.
bool test_boundary(Coordinates position,const CreatureMovementParameters& parameters,
    const BoundaryHelpers& helpers);
// Install after record/validity adapters: captures their current callbacks.
MovementHelpers with_boundary_test(MovementHelpers movement);
} // namespace mnm::reconstruction
