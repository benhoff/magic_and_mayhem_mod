#include "wizard_projectile_task.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
bool executeWizardProjectileTask(
    WizardProjectileTask &t, const AutonomousWorld &world,
    const std::vector<unsigned> &mana, const std::vector<unsigned> &cost,
    const WizardTargetTaskServices &s,
    const std::function<void(unsigned, const WizardPostMove &)> &move) {
  if (t.cast.spell != 73)
    throw std::invalid_argument("wizard projectile task family");
  auto post = [&] {
    if (world.dimensions.x <= 0 || world.dimensions.y <= 0)
      throw std::invalid_argument("wizard post-move extent");
    auto after = t.after;
    auto norm = [](int n, int extent) {
      n %= extent;
      return n < 0 ? n + extent : n;
    };
    after.destination.x = norm(after.destination.x, world.dimensions.x);
    after.destination.y = norm(after.destination.y, world.dimensions.y);
    move(t.cast.source, after);
  };
  if (!world.creatures.at(t.cast.target).ai.health) {
    post();
    return false;
  }
  if (!t.cast.issued && s.wait(t.cast.source, 73, 1) == 0) {
    if ((mana.at(t.cast.source) >> 8) < cost.at(73)) {
      s.useMana(t.cast.source);
      return true;
    }
    const auto &a = world.creatures.at(t.cast.source).ai;
    if (a.position == t.stand && a.action != 2 &&
        s.prepare(t.cast.source, 73, t.cast.target) == 0) {
      s.cast(t.cast.source, 73, t.cast.target);
      post();
      t.cast.issued = true;
    }
  }
  return true;
}
} // namespace mnm::reconstruction::combat
