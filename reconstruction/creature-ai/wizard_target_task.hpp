#pragma once
#include "../combat/autonomy.hpp"

namespace mnm::reconstruction::combat {
struct WizardTargetTask {
  unsigned spell = 41, source = 0, target = 0;
  bool issued = false;
};
struct WizardTargetTaskServices {
  std::function<int(unsigned source, unsigned spell, unsigned count)> wait;
  std::function<void(unsigned source)> useMana;
  // 40ad00 owns cast positioning/line admission. Zero means ready to cast.
  std::function<int(unsigned source, unsigned spell, unsigned target)> prepare;
  std::function<void(unsigned source, unsigned spell, unsigned target)> cast;
  std::function<bool(unsigned target)> blocked;
};
// Whole4466f0 Cure and the common actor-target status-task execution family.
// Spell-specific target status checks remain the explicit blocked service.
// Returns original continuing flag; the caller owns expiry/retirement.
bool executeWizardTargetTask(WizardTargetTask &, const AutonomousWorld &,
                             const std::vector<unsigned> &mana,
                             const std::vector<unsigned> &cost,
                             const WizardTargetTaskServices &);
} // namespace mnm::reconstruction::combat
