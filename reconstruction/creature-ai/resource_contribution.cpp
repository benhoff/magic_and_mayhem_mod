#include "resource_contribution.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int word(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
} // namespace
void contributeCreatureResource(AutonomousCreature &source,
                                AutonomousWorld &world,
                                ResourceContributionState &state,
                                unsigned grant,
                                const ResourceContributionServices &s) {
  if (source.ai.owner < 0 || source.ai.owner >= 8)
    throw std::invalid_argument("resource contribution owner domain");
  if (state.controllers[unsigned(source.ai.owner)] == -1)
    return;
  const auto subject = s.subject(source.ai.owner);
  if (!subject)
    throw std::invalid_argument("resource contribution missing subject");
  auto &actor = world.creatures.at(*subject);
  const float before = float(state.resource.at(*subject) >> 8);
  s.change(actor, grant << 8);
  unsigned aggregate = state.resource.at(*subject);
  const float difference = float(aggregate >> 8) - before;
  for (const auto &item : actor.inventory)
    if (item.available && item.kind == 19)
      aggregate += item.magnitude << 8;
  const float amount = float(word(aggregate) / 256);
  const unsigned credited = s.account(source.ai.type, amount, difference);
  for (auto root : state.roots.at(source.ai.slot)) {
    if (!root)
      continue;
    unsigned current = *root, steps = 0;
    while (true) {
      if (++steps > state.ancestors.size())
        throw std::invalid_argument("cyclic contribution ancestry");
      const auto &node = state.ancestors.at(current);
      if (node.parent)
        current = *node.parent;
      else {
        if (node.recipient)
          state.totals.at(*node.recipient) += credited;
        break;
      }
    }
  }
}
} // namespace mnm::reconstruction::combat
