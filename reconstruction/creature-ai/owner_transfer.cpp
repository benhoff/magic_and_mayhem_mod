#include "owner_transfer.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
void transferCreatureOwner(AutonomousCreature &a, CreatureExecution &e,
                           CreatureTransferState &state,
                           CreatureTransferWorld &world, int owner,
                           const CreatureTransferServices &s) {
  if (owner == a.ai.owner)
    return;
  s.recountCreatures(a);
  a.ai.owner = owner;
  s.refreshVisibility(a);
  a.ai.pending = 0;
  if (a.ai.behavior != 8)
    s.reset(a);
  state.marker6e4 = 0;
  s.recountCreatures(a);
  if (e.marker68) {
    e.marker68 = 0;
    --world.selected;
    world.selectionPresent = world.selected != 0;
    if (e.routeEffect) {
      s.releaseEffect(*e.routeEffect);
      e.routeEffect.reset();
    }
    s.clearMarkers(a);
    if (s.wizard(a) && !a.block7c)
      s.unselectWizard();
  } else if (state.marker70)
    state.marker70 = 0;
  s.color(a, 11);
  s.repairOwnerTargets(a);
  if (a.ai.behavior == 3 || a.ai.behavior == 9 || a.ai.behavior == 10 ||
      a.ai.behavior == 11)
    s.reset(a);
  if (a.ai.pending == 3)
    a.ai.pending = 0;
  for (unsigned i = 0; i < 2; ++i)
    if (state.roots[i])
      s.clearRootActivity(*state.roots[i]);
  s.controlWork(a, false);
  const auto kind = a.ai.kind;
  if ((kind == 0 || kind == 24 || kind == 25 || kind == 26) && !a.block7c)
    s.controlWork(a, true);
  if (a.animationKind == 2) {
    if (!state.display)
      throw std::invalid_argument("transfer display reference");
    // The terminal write uses the requested owner even if a callback changed
    // the actor owner. It reloads the display identity after controller work.
    world.displayOwners.at(*state.display) = owner;
  }
}
} // namespace mnm::reconstruction::combat
