#include "owner_target_repair.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
void repairCreatureOwnerTargets(AutonomousCreature &source,
                                AutonomousWorld &world,
                                OwnerTargetRepairState &state,
                                const OwnerTargetRepairServices &s) {
  s.prepare(source);
  for (unsigned i = 0; i < state.count; ++i) {
    if (state.count > world.creatures.size())
      throw std::invalid_argument("owner target iteration capacity");
    auto &actor = world.creatures.at(i);
    if (!actor.ai.active || !actor.ai.health)
      continue;
    if (actor.ai.slot != source.ai.slot) {
      if (source.ai.owner < 0 || source.ai.owner >= 8 || actor.ai.owner < 0 ||
          actor.ai.owner >= 8)
        throw std::invalid_argument("owner target relation domain");
      if (!world.relation[unsigned(source.ai.owner)][unsigned(actor.ai.owner)])
        continue;
    }
    const auto behavior = actor.ai.behavior;
    if (actor.ai.action == 20 && actor.alternateTarget == int(source.ai.slot)) {
      if (actor.ai.engaged) {
        actor.ai.engaged = 0;
        s.peerEngagement(world.creatures.at(unsigned(actor.alternateTarget)),
                         false);
      }
      state.markers6a4.at(actor.ai.slot) = 0;
      actor.alternateTarget = -1;
      s.behavior(actor);
    }
    if (actor.ai.target == int(source.ai.slot) &&
        (behavior == 3 || behavior == 9 || behavior == 10 || behavior == 11))
      s.reset(actor);
    if (actor.ai.target == int(source.ai.slot) && actor.ai.pending == 3)
      actor.ai.pending = 0;
  }
}
} // namespace mnm::reconstruction::combat
