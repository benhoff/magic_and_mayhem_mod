#include "transfer_reset_owned.hpp"
namespace mnm::reconstruction::combat {
void transferCreatureOwnerResetOwned(
    unsigned actor, AutonomousWorld &world,
    std::vector<CreatureExecution> &execution, CreatureTransferState &state,
    CreatureTransferWorld &selection, const OwnerVisionSharing &sharing,
    CreatureOwnerVisibility &visibility, SourceGraph &graph,
    const std::array<AssociatedOwnerRecord, 8> &owners,
    std::vector<CreatureControlLimit> &limits,
    const OwnerCountServices &countUI, OwnerTargetRepairState &repair,
    const OwnerTargetRepairServices &repairServices,
    const std::function<ExecutionServices(unsigned)> &perActor, int owner,
    const CreatureTransferServices &lower) {
  auto services = lower;
  services.reset = [&](auto &a) {
    auto consequences = perActor(a.ai.slot);
    consequences.releaseEffect = lower.releaseEffect;
    consequences.peerEngagement = [&](auto &peer, unsigned flag) {
      repairServices.peerEngagement(peer, flag != 0);
    };
    resetBehavior(a, execution.at(a.ai.slot), world, consequences);
  };
  transferCreatureOwnerTargetsOwned(
      actor, world, execution.at(actor), state, selection, sharing, visibility,
      graph, owners, limits, countUI, repair, repairServices, owner, services);
}
} // namespace mnm::reconstruction::combat
