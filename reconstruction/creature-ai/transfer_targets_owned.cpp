#include "transfer_targets_owned.hpp"
namespace mnm::reconstruction::combat {
void transferCreatureOwnerTargetsOwned(
    unsigned actor, AutonomousWorld &world, CreatureExecution &execution,
    CreatureTransferState &state, CreatureTransferWorld &selection,
    const OwnerVisionSharing &sharing, CreatureOwnerVisibility &visibility,
    SourceGraph &graph, const std::array<AssociatedOwnerRecord, 8> &owners,
    std::vector<CreatureControlLimit> &limits,
    const OwnerCountServices &countUI, OwnerTargetRepairState &repair,
    const OwnerTargetRepairServices &repairServices, int owner,
    const CreatureTransferServices &lower) {
  auto services = lower;
  auto repairConsequences = repairServices;
  repairConsequences.reset = lower.reset;
  services.repairOwnerTargets = [&](auto &source) {
    repairCreatureOwnerTargets(source, world, repair, repairConsequences);
  };
  transferCreatureOwnerOwned(actor, world, execution, state, selection, sharing,
                             visibility, graph, owners, limits, countUI, owner,
                             services);
}
} // namespace mnm::reconstruction::combat
