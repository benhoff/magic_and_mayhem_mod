#pragma once
#include "transfer_commands_owned.hpp"
namespace mnm::reconstruction::combat {
struct CommandActionEnvironment {
  ActionEnvironment selection;
  unsigned tick = 0, recovery = 0, restoreMode = 0;
};
struct CommandActionProviders {
  std::function<ActionExecutionServices(unsigned)> selection;
  std::function<AnimationActionServices(unsigned)> animation;
  // Other valid installed handlers remain explicit providers. A missing one
  // refuses; it must never silently substitute the current action word.
  std::function<void(AutonomousCreature &, unsigned, OwnedCreatureCommands &)> remaining;
};
// Original action selection and8/9/15 callbacks share staged command roots.
// ANI event generation, casting effects, form and other action bodies remain
// explicit. Installed callback identity is serialized in CreatureExecution.
class OwnedCommandActions {
  AutonomousWorld &world_;
  std::vector<CreatureExecution> &execution_;
  OwnedCreatureCommands &commands_;
  CommandActionEnvironment environment_;
  CommandActionProviders providers_;
  void admit(const AutonomousCreature &) const;
public:
  OwnedCommandActions(AutonomousWorld &, std::vector<CreatureExecution> &,
                      OwnedCreatureCommands &, CommandActionEnvironment,
                      CommandActionProviders);
  void transition(AutonomousCreature &, unsigned);
  void tick(AutonomousCreature &);
};
} // namespace mnm::reconstruction::combat
