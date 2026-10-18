#include "command_actions_owned.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
OwnedCommandActions::OwnedCommandActions(
    AutonomousWorld &world, std::vector<CreatureExecution> &execution,
    OwnedCreatureCommands &commands, CommandActionEnvironment environment,
    CommandActionProviders providers)
    : world_(world), execution_(execution), commands_(commands),
      environment_(environment), providers_(std::move(providers)) {
  if (!commands.owns(world, execution) || world.creatures.empty() ||
      execution.size() != world.creatures.size() ||
      !providers_.selection || !providers_.animation)
    throw std::invalid_argument("owned command action roots/providers differ");
}
void OwnedCommandActions::admit(const AutonomousCreature &a) const {
  if (a.ai.slot >= world_.creatures.size() || &a != &world_.creatures[a.ai.slot])
    throw std::invalid_argument("owned command action actor is foreign");
}
void OwnedCommandActions::transition(AutonomousCreature &a, unsigned kind) {
  admit(a);
  changeAction(a, execution_.at(a.ai.slot), world_, kind,
               environment_.selection, providers_.selection(a.ai.slot));
}
void OwnedCommandActions::tick(AutonomousCreature &a) {
  admit(a);
  auto &e = execution_.at(a.ai.slot);
  const auto handler = e.actionCallback;
  if (!handler) return;
  if (actionHandler(*handler) != handler)
    throw std::invalid_argument("owned command action callback token");
  if (*handler != 8 && *handler != 9 && *handler != 15) {
    if (!providers_.remaining)
      throw std::invalid_argument("owned command action body is missing");
    providers_.remaining(a, *handler, commands_);
    return;
  }
  auto s = providers_.animation(a.ai.slot);
  s.behavior = [this](auto &actor) { commands_.dispatch(actor); };
  if (*handler == 15) tickReadyAction(a, e, s);
  else tickRangedAction(a, e, *handler == 8, environment_.tick,
                        environment_.recovery, environment_.restoreMode, s);
}
} // namespace mnm::reconstruction::combat
