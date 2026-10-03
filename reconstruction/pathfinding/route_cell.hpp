#pragma once
#include "route_creature_acceptance.hpp"

namespace mnm::reconstruction {
struct CellHelpers {
    MapDimensions dimensions;
    // 0x4f3550: use with_occupancy_test to supply the recovered column rules.
    std::function<bool(Coordinates, std::int32_t, const CreatureMovementParameters&)> check;
};
// 0x4f47d0: one cell, or an ordered wrapped 2x2 footprint for parameter +4 == 2.
bool test_cell(Coordinates position, std::int32_t object_00,
    const CreatureMovementParameters& parameters, const CellHelpers& helpers);
CreatureAcceptanceHelpers with_cell_test(CreatureAcceptanceHelpers acceptance, CellHelpers cells);
} // namespace mnm::reconstruction
