#pragma once
#include "route_neighbors.hpp"

namespace mnm::reconstruction {
struct CreatureScalarState {
    std::int32_t type_3c = 0, type_10 = 0;
    std::array<std::uint32_t, 48> type_d8{}; // four 12-DWORD banks
    std::int32_t object_778 = 0, object_77c = 0;
    std::uint32_t default_scalar = 0; // global unaligned DWORD 6c007d
    float slope_global = 0; // mutable global 5e15f0, never assumed constant
};
struct ScalarResult { std::uint32_t scalar, base; };
// 505840 and 505920, composed by 5205b0. Delta XY must be in [-1,1].
std::uint32_t base_movement_scalar(std::int32_t category, Coordinates delta,
    const CreatureScalarState& state);
ScalarResult creature_movement_scalar(std::int32_t argument, std::int32_t category,
    Coordinates delta, std::uint32_t prior, const CreatureScalarState& state);
NeighborHelpers with_creature_scalar(NeighborHelpers neighbors,
    std::function<CreatureScalarState(std::uint32_t)> object_state);
} // namespace mnm::reconstruction
