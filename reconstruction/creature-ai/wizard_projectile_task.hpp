#pragma once
#include "wizard_target_task.hpp"
namespace mnm::reconstruction::combat {
struct WizardPostMove {
  unsigned command = 0, argument = 0;
  Cell destination;
};
struct WizardProjectileTask {
  WizardTargetTask cast;
  Cell stand;
  WizardPostMove after;
};
// Whole44ae70 Magic Sphere73 task body. Prediction/candidate generation and
// the post-move command's lower ownership remain independent contracts.
bool executeWizardProjectileTask(
    WizardProjectileTask &, const AutonomousWorld &,
    const std::vector<unsigned> &mana, const std::vector<unsigned> &cost,
    const WizardTargetTaskServices &,
    const std::function<void(unsigned, const WizardPostMove &)> &move);
} // namespace mnm::reconstruction::combat
