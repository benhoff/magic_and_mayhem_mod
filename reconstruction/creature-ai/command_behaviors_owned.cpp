#include "command_behaviors_owned.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
void safeFineAim(const AutonomousCreature &target) {
  for (int coordinate : {target.finePosition.x, target.finePosition.y}) {
    const auto sum = std::int64_t(coordinate) + target.aimXY;
    if (sum < std::numeric_limits<std::int32_t>::min() ||
        sum > std::numeric_limits<std::int32_t>::max())
      throw std::invalid_argument("unsafe command behavior arithmetic");
  }
}
} // namespace
OwnedCommandBehaviors::OwnedCommandBehaviors(
    AutonomousWorld &w, std::vector<CreatureExecution> &e,
    std::vector<CreatureMaintenance> &m,
    const std::vector<MovementBehaviorState> &v, OwnedCreatureCommands &commands,
    CommandBehaviorEnvironment environment, CommandBehaviorProviders providers)
    : world_(w), execution_(e), maintenance_(m), movement_(v),
      commands_(commands), environment_(environment), providers_(std::move(providers)) {
  const auto n = w.creatures.size();
  if (!commands.owns(w, e) || !n || e.size() != n || m.size() != n || v.size() != n ||
      w.dimensions.x <= 0 || w.dimensions.y <= 0 ||
      !providers_.movement || !providers_.behavior || !providers_.ranged)
    throw std::invalid_argument("owned command behavior roots/providers differ");
}
BehaviorExecutionServices OwnedCommandBehaviors::behaviorServices(unsigned slot) {
  auto s = providers_.behavior(slot);
  s.behavior = [this](auto &a, unsigned kind) { commands_.transition(a, kind); };
  s.reset = [this](auto &a) { commands_.reset(a); };
  s.maintainTarget = [this](auto &a) { maintain(a); };
  s.ranged = [this](auto &a) { ranged(a); };
  return s;
}
void OwnedCommandBehaviors::maintain(AutonomousCreature &a) {
  maintainTargetBehavior(a, execution_.at(a.ai.slot), world_,
                         environment_.restoreMode, behaviorServices(a.ai.slot));
}
void OwnedCommandBehaviors::ranged(AutonomousCreature &a) {
  safeFineAim(world_.creatures.at(unsigned(a.ai.target)));
  auto s = providers_.ranged(a.ai.slot);
  s.maintainTarget = [this](auto &actor) { maintain(actor); };
  rangedTactic(a, execution_.at(a.ai.slot), world_, rangedStyle(a.ai.kind),
               environment_.tick, s);
}
void OwnedCommandBehaviors::dispatch(AutonomousCreature &a, unsigned kind) {
  if (a.ai.slot >= world_.creatures.size() ||
      &a != &world_.creatures[a.ai.slot])
    throw std::invalid_argument("owned command behavior actor is foreign");
  auto &e = execution_.at(a.ai.slot);
  if (kind == 2) {
    // The recovered body uses signed subtraction/abs. Unsafe native saved
    // inputs are outside its comparison domain and must refuse before use.
    for (auto pair : {std::pair<int, int>{a.ai.position.x, a.destination.x},
                      {a.ai.position.y, a.destination.y},
                      {a.ai.position.z, a.destination.z}}) {
      const auto difference = std::int64_t(pair.first) - pair.second;
      const auto limit = std::numeric_limits<std::int32_t>::max();
      if (difference < -std::int64_t(limit) || difference > limit)
        throw std::invalid_argument("unsafe command behavior arithmetic");
    }
    auto s = providers_.movement(a.ai.slot);
    s.renewMove = [this](auto &actor, Cell p) { commands_.move(actor, p); };
    s.reset = [this](auto &actor) { commands_.reset(actor); };
    movementBehavior(a, e, maintenance_.at(a.ai.slot), movement_.at(a.ai.slot),
                     world_.dimensions, s);
  } else if (kind == 3) {
    if (a.ai.target >= 0 && unsigned(a.ai.target) < world_.creatures.size())
      safeFineAim(world_.creatures[unsigned(a.ai.target)]);
    targetAttackBehavior(a, e, world_, behaviorServices(a.ai.slot));
  } else if (kind == 4)
    coordinateAttackBehavior(a, e, environment_.tick, behaviorServices(a.ai.slot));
  else
    throw std::invalid_argument("owned command behavior selector");
}
} // namespace mnm::reconstruction::combat
