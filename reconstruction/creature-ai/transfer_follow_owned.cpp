#include "transfer_follow_owned.hpp"
namespace mnm::reconstruction::combat {
void transferCreatureOwnerFollowOwned(
    unsigned actor, AutonomousWorld &world,
    std::vector<CreatureExecution> &execution,
    OwnershipBehaviorBindings &bindings, CreatureTransferState &state,
    CreatureTransferWorld &selection, const OwnerVisionSharing &sharing,
    CreatureOwnerVisibility &visibility, SourceGraph &graph,
    const std::array<AssociatedOwnerRecord, 8> &owners,
    std::vector<CreatureControlLimit> &limits,
    const OwnerCountServices &countUI, OwnerTargetRepairState &repair,
    const OwnerTargetRepairServices &repairServices,
    const std::function<ExecutionServices(unsigned)> &perActor,
    const std::function<BehaviorExecutionServices(unsigned)> &perIdle,
    const std::function<FollowServices(unsigned)> &perFollow, int owner,
    const CreatureTransferServices &lower) {
  auto idleConsequences = [&](unsigned slot) {
    auto result = perIdle(slot);
    result.follow = [&](auto &a) {
      return followReference(a, execution.at(a.ai.slot), world,
                             perFollow(a.ai.slot));
    };
    return result;
  };
  transferCreatureOwnerIdleOwned(actor, world, execution, bindings, state,
                                 selection, sharing, visibility, graph, owners,
                                 limits, countUI, repair, repairServices,
                                 perActor, idleConsequences, owner, lower);
}
} // namespace mnm::reconstruction::combat
