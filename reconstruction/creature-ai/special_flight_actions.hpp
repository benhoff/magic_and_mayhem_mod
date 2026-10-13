#pragma once
#include "actor_route_request.hpp"
namespace mnm::reconstruction::combat {
struct SpecialFlightEnvironment {
  Extent dimensions;
  unsigned waterline = 0;
  bool diagnostic = false;
};
struct SpecialFlightServices {
  std::function<int(AutonomousCreature &, bool)> poll;
  std::function<void(AutonomousCreature &)> rewind, facing, completeSegment,
      environment, animationUpdate, terrain, reset;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> animation;
  std::function<void(AutonomousCreature &, unsigned, unsigned, unsigned)> stun;
  std::function<bool(const AutonomousCreature &, Cell)> occupied, passable,
      ground;
  std::function<int(const AutonomousCreature &, Cell)> regionAt;
  std::function<bool(const AutonomousCreature &, int, int)> regionRoute;
  std::function<unsigned()> random;
  std::function<void(unsigned)> diagnostic;
  std::function<void(AutonomousCreature &, Cell)> release, reserve,
      relocateDead;
  std::function<void(AutonomousCreature &, Cell, unsigned)> snap;
  std::function<bool(const AutonomousCreature &, Cell &, unsigned, bool)>
      adjustedLanding;
  std::function<bool(AutonomousCreature &)> response;
  std::function<void(AutonomousCreature &, unsigned)> behavior;
  std::function<unsigned(AutonomousCreature &, unsigned, int)> fallDamage;
  std::function<void(AutonomousCreature &, unsigned, int, bool)> defended;
};
void tickFinishingAction(AutonomousCreature &, CreatureExecution &,
                         const SpecialFlightServices &);
void tickAscendingAction(AutonomousCreature &, CreatureExecution &,
                         MotionActionState &, MovementBehaviorState &,
                         const SegmentCompletionState &, AutonomousWorld &,
                         const SpecialFlightEnvironment &,
                         const SpecialFlightServices &);
void tickDescendingAction(AutonomousCreature &, CreatureExecution &,
                          const SegmentCompletionState &,
                          const MovementBehaviorState &,
                          const SpecialFlightEnvironment &,
                          const SpecialFlightServices &);
} // namespace mnm::reconstruction::combat
