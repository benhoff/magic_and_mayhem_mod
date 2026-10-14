#pragma once
#include "movement_behavior.hpp"
#include "transfer_commands_owned.hpp"
namespace mnm::reconstruction::combat {
struct CommandBehaviorEnvironment {
  unsigned tick = 0, restoreMode = 0;
};
struct CommandBehaviorProviders {
  std::function<MovementBehaviorServices(unsigned)> movement;
  std::function<BehaviorExecutionServices(unsigned)> behavior;
  std::function<RangedExecutionServices(unsigned)> ranged;
};
// Borrow the same staged roots and command context. Geometry, action, animation,
// renew/landing/occupancy/health and World removal are still explicit providers.
class OwnedCommandBehaviors {
  AutonomousWorld &world_;
  std::vector<CreatureExecution> &execution_;
  std::vector<CreatureMaintenance> &maintenance_;
  const std::vector<MovementBehaviorState> &movement_;
  OwnedCreatureCommands &commands_;
  CommandBehaviorEnvironment environment_;
  CommandBehaviorProviders providers_;
  BehaviorExecutionServices behaviorServices(unsigned);
  void maintain(AutonomousCreature &);
  void ranged(AutonomousCreature &);
public:
  OwnedCommandBehaviors(AutonomousWorld &, std::vector<CreatureExecution> &,
                       std::vector<CreatureMaintenance> &,
                       const std::vector<MovementBehaviorState> &,
                       OwnedCreatureCommands &, CommandBehaviorEnvironment,
                       CommandBehaviorProviders);
  void dispatch(AutonomousCreature &, unsigned);
};
} // namespace mnm::reconstruction::combat
