#pragma once
#include "route_cell.hpp"
#include "route_movement.hpp"

namespace mnm::reconstruction {
struct ValidityHelpers {
    MapDimensions dimensions;
    // 0x4f3440 on the caller's map context, rather than the occupancy map.
    std::function<bool(Coordinates, const CreatureMovementParameters&)> check;
};
// 0x4f46b0: ordered footprint check; parameters +4 == 2 enables the 2x2 path.
bool test_validity(Coordinates position, const CreatureMovementParameters& parameters,
    const ValidityHelpers& helpers);
// Install the recovered 46b0 check, retaining the other movement providers.
MovementHelpers with_validity_test(MovementHelpers movement, ValidityHelpers validity);
} // namespace mnm::reconstruction
