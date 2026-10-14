#pragma once
#include "route_movement.hpp"
namespace mnm::reconstruction {
struct CreatureLineServices {
  MapDimensions dimensions;
  std::function<bool(Coordinates, Coordinates, const CreatureMovementParameters &,
                     std::int32_t &, std::int32_t)> movement;
};
// 4f4ba0: greedy movement line used by diagonal reach. Category is one retained
// original out-word across lower calls. The lower movement contract overwrites
// it on entry; the original caller does not initialize that stack word.
bool test_creature_line(Coordinates from, Coordinates target,
    const CreatureMovementParameters &, const CreatureLineServices &);
} // namespace mnm::reconstruction
