#include "ownership_helpers.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
bool isWizardKind(unsigned kind) {
  return kind == 0 || kind == 24 || kind == 25 || kind == 26;
}
void clearRootActivityFlag(unsigned &activity) {
  if (activity == 1)
    activity = 0;
  else if (activity == 3)
    activity = 2;
}
void refreshCreatureOwnerVisibility(AutonomousCreature &a,
                                    const AutonomousWorld &world,
                                    const OwnerVisionSharing &sharing,
                                    CreatureOwnerVisibility &state) {
  if (a.ai.owner < 0 || a.ai.owner >= 8)
    throw std::invalid_argument("owner visibility relation domain");
  for (unsigned owner = 0; owner < 8; ++owner) {
    const auto source = unsigned(a.ai.owner);
    a.ai.visible[owner] = (owner == source || (world.relation[source][owner] &&
                                               sharing[source][owner]))
                              ? 30
                              : 0;
    state.secondary[owner] = 0;
  }
}
void clearQueuedMoveMarkers(CreatureExecution &e,
                            const std::function<void(unsigned)> &release) {
  for (auto &order : e.orders)
    if (order.kind == 2 && order.effect) {
      release(*order.effect);
      order.effect.reset();
    }
}
} // namespace mnm::reconstruction::combat
