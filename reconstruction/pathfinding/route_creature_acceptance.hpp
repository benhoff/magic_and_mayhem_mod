#pragma once
#include "route_neighbors.hpp"

namespace mnm::reconstruction {
// 22-byte member built by 0x40e250, passed to 0x4f3990 and 0x4f47d0.
// Names describe source offsets; gameplay meanings remain unresolved.
#pragma pack(push, 1)
struct CreatureMovementParameters {
    std::int32_t type_0c = 0, type_08 = 0, object_104 = 0, type_44 = 0;
    std::uint8_t unknown_10 = 0;
    std::int32_t type_index = 0;
    std::uint8_t special_state = 0;
};
#pragma pack(pop)
static_assert(sizeof(CreatureMovementParameters) == 22);
static_assert(offsetof(CreatureMovementParameters, type_index) == 0x11);
static_assert(offsetof(CreatureMovementParameters, special_state) == 0x15);

// Values read from the creature and the record pointed to by creature +0xac.
// No host pointer is interpreted as an engine pointer.
struct CreatureAcceptanceState {
    std::int32_t object_00 = 0, type_index = 0, object_104 = 0;
    std::int32_t object_5ec = 0;
    std::uint8_t object_723 = 0;
    std::int32_t type_08 = 0, type_0c = 0, type_44 = 0;
};
struct CreatureAcceptanceHelpers {
    MapDimensions dimensions;
    std::function<CreatureAcceptanceState(std::uint32_t)> object_state;
    // 0x4f3990: from/to, parameter member, category out, final override -1.
    std::function<bool(Coordinates, Coordinates, const CreatureMovementParameters&,
        std::int32_t& category, std::int32_t record_override)> movement_test;
    // 0x4f47d0: XYZ, creature DWORD +0, parameter member.
    std::function<bool(Coordinates, std::int32_t object_00,
        const CreatureMovementParameters&)> cell_test;
};

CreatureMovementParameters creature_movement_parameters(const CreatureAcceptanceState& state);
// 0x4f5a40 wrapper: construct the parameter member and call 0x4f47d0.
bool test_creature_cell(std::uint32_t object, Coordinates position,
    const CreatureAcceptanceHelpers& helpers);
// 0x514360: construct parameters, base move test, occupancy and intermediate
// groups. Every recovered rejection resets category to 5; success preserves it.
bool accept_creature_move(std::uint32_t object, Coordinates from, Coordinates to,
    bool special, std::int32_t& category, const CreatureAcceptanceHelpers& helpers);
// Replace the mode-zero generator acceptance callback. Keep the existing
// mode-nonzero callback for the other standard path.
NeighborHelpers with_creature_acceptance(NeighborHelpers neighbors,
    CreatureAcceptanceHelpers acceptance);
} // namespace mnm::reconstruction
