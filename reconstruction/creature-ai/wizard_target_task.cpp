#include "wizard_target_task.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
bool executeWizardTargetTask(WizardTargetTask &t, const AutonomousWorld &world,
                             const std::vector<unsigned> &mana,
                             const std::vector<unsigned> &cost,
                             const WizardTargetTaskServices &s) {
  if (t.spell != 41 && t.spell != 74 && t.spell != 78 && t.spell != 79 &&
      t.spell != 83 && t.spell != 84 && t.spell != 85 && t.spell != 42)
    throw std::invalid_argument("wizard target task spell family");
  const auto &target = world.creatures.at(t.target);
  if (!target.ai.health ||
      (t.spell == 41 ? target.ai.health == target.ai.maximumHealth
                     : s.blocked(t.target)))
    return false;
  if (!t.issued && s.wait(t.source, t.spell, 1) == 0) {
    if ((mana.at(t.source) >> 8) < cost.at(t.spell)) {
      s.useMana(t.source);
      if (t.spell != 74)
        return true;
    }
    // Lower services may mutate the caller's staged World. Reload health after
    // the wait helper exactly where the original performs its second read.
    if ((t.spell == 79 || world.creatures.at(t.target).ai.health) &&
        s.prepare(t.source, t.spell, t.target) == 0) {
      s.cast(t.source, t.spell, t.target);
      t.issued = true;
    }
  }
  return true;
}
} // namespace mnm::reconstruction::combat
