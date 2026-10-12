#pragma once
#include "maintenance.hpp"
namespace mnm::reconstruction::combat {
struct MovementBehaviorState {
  std::array<unsigned, 8> renewedOrder{};
  unsigned landingKind = 0;
  Cell nextRouteCell;
};
struct MovementBehaviorServices {
  std::function<void(AutonomousCreature &, const std::array<unsigned, 8> &)>
      renew;
  std::function<void(AutonomousCreature &, Cell)> move, releaseOccupancy,
      relocate;
  std::function<void(AutonomousCreature &, unsigned)> action;
  std::function<void(AutonomousCreature &)> beginMotion, reset, clearOverlay,
      invalidate, settle, refresh, remove;
  std::function<bool(AutonomousCreature &)> fallback;
  std::function<unsigned(AutonomousCreature &, unsigned, int)> landingHealth;
  std::function<void(AutonomousCreature &, unsigned)> setHealth;
  std::function<bool(const AutonomousCreature &, Cell &)> adjust;
  std::function<void(unsigned)> releaseEffect;
};
void movementBehavior(AutonomousCreature &, CreatureExecution &,
                      CreatureMaintenance &, const MovementBehaviorState &,
                      Extent, const MovementBehaviorServices &);
} // namespace mnm::reconstruction::combat
