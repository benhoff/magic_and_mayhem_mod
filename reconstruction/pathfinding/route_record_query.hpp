#pragma once
#include "route_movement.hpp"

namespace mnm::reconstruction {
struct RecordQueryHelpers {
    MapDimensions dimensions;
    std::function<std::int32_t()> boundary; // caller's map +7f0, read on zero lower result
    std::function<std::uint32_t(Coordinates,const CreatureMovementParameters&)> query; // 4f3320
};
// 0x4f4330: single lower query, or balanced four-column support aggregation.
std::uint32_t query_record(Coordinates position,const CreatureMovementParameters& parameters,
    const RecordQueryHelpers& helpers);
MovementHelpers with_record_query(MovementHelpers movement,RecordQueryHelpers query);
} // namespace mnm::reconstruction
