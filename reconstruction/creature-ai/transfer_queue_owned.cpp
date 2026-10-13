#include "transfer_queue_owned.hpp"
namespace mnm::reconstruction::combat {
void transferCreatureOwnerQueueOwned(
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
    const std::function<FollowServices(unsigned)> &perFollow,
    const std::function<ExecutionServices(unsigned)> &perQueue, int owner,
    const CreatureTransferServices &lower) {
  auto idleConsequences = [&](unsigned slot) {
    auto result = perIdle(slot);
    result.queue = [&](auto &a) {
      auto consequences = perQueue(a.ai.slot);
      consequences.releaseEffect = lower.releaseEffect;
      return advanceOrders(a, execution.at(a.ai.slot), consequences);
    };
    return result;
  };
  transferCreatureOwnerFollowOwned(
      actor, world, execution, bindings, state, selection, sharing, visibility,
      graph, owners, limits, countUI, repair, repairServices, perActor,
      idleConsequences, perFollow, owner, lower);
}
} // namespace mnm::reconstruction::combat
