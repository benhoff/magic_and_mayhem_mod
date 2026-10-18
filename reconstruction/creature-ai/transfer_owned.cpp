#include "transfer_owned.hpp"
namespace mnm::reconstruction::combat {
void transferCreatureOwnerOwned(
    unsigned slot, AutonomousWorld &world, CreatureExecution &execution,
    CreatureTransferState &state, CreatureTransferWorld &selection,
    const OwnerVisionSharing &sharing, CreatureOwnerVisibility &visibility,
    SourceGraph &graph, const std::array<AssociatedOwnerRecord, 8> &owners,
    std::vector<CreatureControlLimit> &limits,
    const OwnerCountServices &countUI, int owner,
    const CreatureTransferServices &lower) {
  auto services = lower;
  services.recountCreatures = [&](auto &a) {
    recountOwnerCreatures(a, world, owners, limits, world.localOwner, countUI);
  };
  services.refreshVisibility = [&](auto &a) {
    refreshCreatureOwnerVisibility(a, world, sharing, visibility);
  };
  services.clearMarkers = [&](auto &) {
    clearQueuedMoveMarkers(execution, lower.releaseEffect);
  };
  services.wizard = [](const auto &a) { return isWizardKind(a.ai.kind); };
  services.clearRootActivity = [&](unsigned root) {
    clearRootActivityFlag(graph.at(root).activity);
  };
  transferCreatureOwner(world.creatures.at(slot), execution, state, selection,
                        owner, services);
}
} // namespace mnm::reconstruction::combat
