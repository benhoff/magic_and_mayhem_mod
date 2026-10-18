#pragma once
#include "command_actions_owned.hpp"
#include "special_flight_actions.hpp"
#include "object_action.hpp"
namespace mnm::reconstruction::combat {
struct CommandActionBodyState {
  std::vector<ActionCallbackState> callbacks;
  std::vector<MotionActionState> motion;
  std::vector<SegmentCompletionState> segments;
  ObjectActionWorld objects;
};
struct CommandActionBodyEnvironment {
  ActionCallbackEnvironment callbacks;
  SpecialFlightEnvironment flight;
};
struct CommandActionBodyProviders {
  std::function<ActionCallbackServices(unsigned)> callbacks;
  std::function<MotionActionServices(unsigned)> motion;
  std::function<SpecialFlightServices(unsigned)> flight;
  std::function<ObjectActionServices(unsigned)> objects;
};
// All remaining logical handler tokens from50da50. Lower ANI/health/effects,
// segment completion and geometry are typed providers; missing handlers never
// fall back to editing an action word or pretending a frame completed.
class OwnedCommandActionBodies {
  AutonomousWorld &world_;
  std::vector<CreatureExecution> &execution_;
  std::vector<CreatureMaintenance> &maintenance_;
  std::vector<MovementBehaviorState> &movement_;
  CommandActionBodyState &state_;
  OwnedCreatureCommands &commands_;
  CommandActionBodyEnvironment environment_;
  CommandActionBodyProviders providers_;
public:
  OwnedCommandActionBodies(AutonomousWorld &, std::vector<CreatureExecution> &,
      std::vector<CreatureMaintenance> &, std::vector<MovementBehaviorState> &,
      CommandActionBodyState &, OwnedCreatureCommands &,
      CommandActionBodyEnvironment, CommandActionBodyProviders);
  void tick(AutonomousCreature &, unsigned handler);
};
} // namespace mnm::reconstruction::combat
