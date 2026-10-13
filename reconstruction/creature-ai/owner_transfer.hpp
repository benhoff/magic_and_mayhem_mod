#pragma once
#include "execution.hpp"
namespace mnm::reconstruction::combat {
struct CreatureTransferState {
  unsigned marker70 = 0, marker6e4 = 0;
  std::array<std::optional<unsigned>, 2> roots;
  std::optional<unsigned> display;
};
struct CreatureTransferWorld {
  unsigned selected = 0, selectionPresent = 0;
  std::vector<int> displayOwners;
};
struct CreatureTransferServices {
  std::function<void(AutonomousCreature &)> recountCreatures, refreshVisibility,
      reset, clearMarkers, repairOwnerTargets;
  std::function<void(unsigned)> releaseEffect, clearRootActivity;
  std::function<bool(const AutonomousCreature &)> wizard;
  std::function<void()> unselectWizard;
  std::function<void(AutonomousCreature &, unsigned)> color;
  std::function<void(AutonomousCreature &, bool wizardController)> controlWork;
};
// Whole51ec80 control flow. Lower
// counts/visibility/reset/UI/color/control/root-state consequences remain
// explicit services; callback mutation order is preserved.
void transferCreatureOwner(AutonomousCreature &, CreatureExecution &,
                           CreatureTransferState &, CreatureTransferWorld &,
                           int owner, const CreatureTransferServices &);
} // namespace mnm::reconstruction::combat
