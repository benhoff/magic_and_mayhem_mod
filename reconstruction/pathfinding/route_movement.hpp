#pragma once
#include "route_creature_acceptance.hpp"

namespace mnm::reconstruction {
enum class MovementCheck : std::uint32_t {
    check_46b0 = 0x4f46b0, check_41a0 = 0x4f41a0,
    check_44e0 = 0x4f44e0, check_45b0 = 0x4f45b0
};
struct MovementHelpers {
    MapDimensions dimensions;
    std::function<std::int32_t()> boundary;    // map receiver +7f0
    std::function<std::int32_t()> layer_count; // global 5e1780
    std::function<std::uint32_t(Coordinates, const CreatureMovementParameters&)> record; // 4f4330 support result, not a host pointer
    std::function<bool(MovementCheck, Coordinates, const CreatureMovementParameters&)> check;
};
// 0x4f3990. source_record == -1 resolves source; other DWORDs are tested for nonzero as supplied support overrides.
bool test_movement(Coordinates from, Coordinates to, const CreatureMovementParameters& parameters,
    std::int32_t& category, std::int32_t source_record, const MovementHelpers& helpers);
CreatureAcceptanceHelpers with_movement_test(CreatureAcceptanceHelpers acceptance,
    MovementHelpers movement);
// Mode-nonzero uses descriptor +18's exact 22-byte member and its setup record.
// Install before with_creature_acceptance to supply both standard branches.
NeighborHelpers with_standard_movement_test(NeighborHelpers neighbors, MovementHelpers movement);
} // namespace mnm::reconstruction
