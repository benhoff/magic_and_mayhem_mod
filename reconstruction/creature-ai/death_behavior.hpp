#pragma once
#include "movement_behavior.hpp"
namespace mnm::reconstruction::combat {
struct DeathEnvironment {
  unsigned corpseLifetime = 0, explosionMode = 0;
  int explosionBase = 0, explosionScale = 0;
  bool corpseConfigured = false;
};
struct DeathServices {
  std::function<void(AutonomousCreature &, unsigned)> action;
  std::function<void(AutonomousCreature &, int, unsigned)> explode;
  std::function<void(AutonomousCreature &)> specialRemoval, remove,
      releaseUpkeep, registerCorpse, releaseReference;
  std::function<void(AutonomousCreature &, Cell)> releaseOccupancy;
};
void deathBehavior(AutonomousCreature &, const CreatureMaintenance &,
                   const MovementBehaviorState &, const DeathEnvironment &,
                   const DeathServices &);
} // namespace mnm::reconstruction::combat
