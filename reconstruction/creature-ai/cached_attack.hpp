#pragma once
#include "maintenance.hpp"
namespace mnm::reconstruction::combat {
std::optional<unsigned> opponentFacing(const AutonomousCreature &,
                                       const AutonomousCreature &, Extent);
struct ObstacleCell {
  unsigned flags = 0, occupant = 0;
}; // occupant is slot+1; zero is absent
unsigned footprintObstacle(const AutonomousCreature &, Cell, Extent,
                           const std::function<ObstacleCell(Cell)> &);
struct CachedAttackServices {
  std::function<bool(const AutonomousCreature &, const AutonomousCreature &,
                     Cell)>
      reach;
  std::function<unsigned(const AutonomousCreature &, Cell)> obstacle;
  std::function<void(AutonomousCreature &, unsigned)> action;
  std::function<void(AutonomousCreature &, const AutonomousCreature &)> face;
};
bool attackCachedOpponent(AutonomousCreature &, CreatureMaintenance &,
                          AutonomousWorld &, const CachedAttackServices &);
} // namespace mnm::reconstruction::combat
