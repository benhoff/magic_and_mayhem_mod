#pragma once
#include "actor_routing.hpp"
#include "cached_attack.hpp"
namespace mnm::reconstruction::combat {
struct FriendlyDisplacementServices {
  std::function<bool(const AutonomousCreature &, Cell, Cell, bool)> move;
  std::function<unsigned(Cell)> flags;
  std::function<unsigned(const AutonomousCreature &, Cell)> obstacle;
  std::function<bool(const AutonomousCreature &, Cell)> hazard;
  std::function<void(AutonomousCreature &, Cell)> order;
  std::function<void(AutonomousCreature &, unsigned)> action;
};
bool displacementWait(AutonomousCreature &, CreatureExecution &, Cell next,
                      const AutonomousWorld &,
                      const std::vector<CreatureExecution> &,
                      const std::function<ObstacleCell(Cell)> &,
                      const FriendlyDisplacementServices &);
unsigned displacementBlocker(const AutonomousCreature &, Cell next,
                             const FriendlyDisplacementServices &);
bool canDisplaceCreature(const AutonomousCreature &, const AutonomousCreature &,
                         const CreatureExecution &, const AutonomousWorld &);
bool displacementMoving(const AutonomousCreature &, const CreatureExecution &);
bool displacementSweptOverlap(const AutonomousCreature &, Cell next,
                              Cell candidate, unsigned width, Extent);
bool issueFriendlyDisplacement(AutonomousCreature &, AutonomousCreature &,
                               AutonomousWorld &,
                               std::vector<CreatureExecution> &,
                               std::vector<SegmentCompletionState> &,
                               std::vector<MotionActionState> &,
                               std::vector<int> &owners,
                               const FriendlyDisplacementServices &);
} // namespace mnm::reconstruction::combat
