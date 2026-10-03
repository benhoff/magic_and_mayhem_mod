#pragma once
#include "route_cell_validity.hpp"

namespace mnm::reconstruction {
// These predicates read the fixed global map, not the incoming map receiver.
// They return true for a detected blocker, not for accepted movement.
bool test_any_terrain(Coordinates position,const CreatureMovementParameters& parameters,
    const CellValidityMapView& global_map); // 4f44e0
bool test_tall_terrain(Coordinates position,const CreatureMovementParameters& parameters,
    const CellValidityMapView& global_map); // 4f45b0
MovementHelpers with_clearance_tests(MovementHelpers movement,CellValidityMapView global_map);
} // namespace mnm::reconstruction
