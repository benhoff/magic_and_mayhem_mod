#include "owner_counts.hpp"
#include "ownership_helpers.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
void recountOwnerCreatures(const AutonomousCreature &source,
                           AutonomousWorld &world,
                           const std::array<AssociatedOwnerRecord, 8> &owners,
                           std::vector<CreatureControlLimit> &limits,
                           int localOwner, const OwnerCountServices &s) {
  if (world.creatures.size() > 0x7fffffffu)
    throw std::invalid_argument("owner count capacity domain");
  const auto subject = associatedCreature(source.ai.owner, owners,
                                          unsigned(world.creatures.size()));
  if (!subject)
    return;
  auto &actor = world.creatures.at(*subject);
  unsigned count = 0;
  for (const auto &candidate : world.creatures)
    if (candidate.ai.active && candidate.ai.health &&
        candidate.ai.owner == actor.ai.owner &&
        candidate.ai.slot != actor.ai.slot && candidate.ai.type != 27)
      ++count;
  auto &limit = limits.at(*subject);
  const int previous = limit.current;
  limit.current = int(count);
  if (isWizardKind(actor.ai.kind) && !actor.block7c &&
      actor.ai.owner == localOwner) {
    s.display(actor, count);
    const int maximum = limit.maximum;
    if (previous == maximum && limit.current < maximum)
      s.belowLimit(actor);
    else if (previous < maximum && limit.current == maximum)
      s.atLimit(actor);
  }
}
} // namespace mnm::reconstruction::combat
