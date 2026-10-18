#pragma once
#include "execution.hpp"
namespace mnm::reconstruction::combat {
struct OwnerTargetRepairState {
  unsigned count = 0;
  std::vector<unsigned> markers6a4;
};
struct OwnerTargetRepairServices {
  std::function<void(AutonomousCreature &)> prepare, behavior, reset;
  std::function<void(AutonomousCreature &, bool)> peerEngagement;
};
// Whole51edf0 with explicit lower5178c0/50eb60/behavior/reset services.
// Actor storage/identities stay fixed; count and later actor/relation fields
// can change during callbacks. Behavior is cached before the action callback.
void repairCreatureOwnerTargets(AutonomousCreature &, AutonomousWorld &,
                                OwnerTargetRepairState &,
                                const OwnerTargetRepairServices &);
} // namespace mnm::reconstruction::combat
