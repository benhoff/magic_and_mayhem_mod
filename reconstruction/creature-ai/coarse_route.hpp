#pragma once
#include "actor_routing.hpp"
namespace mnm::reconstruction::combat {
// Original513370: terrain-only direct trajectory; occupied destination cells
// are admitted here and checked later when each segment starts.
bool coarseCreatureRoute(
    const AutonomousCreature &, unsigned direction, Extent, Cell target,
    ActorRouteSnapshot &,
    const std::function<bool(Cell, Cell, unsigned &)> &move,
    const std::function<bool(Cell)> &hazard);
} // namespace mnm::reconstruction::combat
