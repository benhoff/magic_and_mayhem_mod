#pragma once
#include "actor_routing.hpp"
namespace mnm::reconstruction::combat {
struct ActorRegionQuery {
  unsigned actorSlot = 0;
  int owner = 0;
  unsigned command = 0, marker69c = 0;
};
struct ActorRouteRequestServices {
  std::function<int(Cell, const ActorRegionQuery &, int, int)> regionAt;
  std::function<bool(int, int, const ActorRegionQuery &, std::array<int, 50> &,
                     unsigned, bool)>
      regionRoute;
  std::function<int(int, int)> regionDistance;
  std::function<void(AutonomousCreature &, Cell &, unsigned, Cell)>
      regionCandidate;
  std::function<bool(const AutonomousCreature &, Cell)> legal;
  std::function<void(const AutonomousCreature &, Cell &)> adjustLegal;
  std::function<bool(AutonomousCreature &, Cell, ActorRouteSnapshot &)> coarse;
  std::function<void(AutonomousCreature &, unsigned, Cell, ActorRouteContext &,
                     int &)>
      search;
};
unsigned requestCreatureRoute(AutonomousCreature &, CreatureExecution &,
                              MotionActionState &, SegmentCompletionState &,
                              ActorRouteMetadata &, ActorRouteContext &,
                              const SegmentCompletionEnvironment &, Extent,
                              Cell, unsigned mode, bool regionRequest,
                              const ActorRouteRequestServices &);
} // namespace mnm::reconstruction::combat
