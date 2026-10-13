#include "transfer_delivery_owned.hpp"
namespace mnm::reconstruction::combat {
void transferCreatureOwnerDeliveryOwned(
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
    const std::function<ExecutionServices(unsigned)> &perQueue,
    const std::function<OrderDeliveryServices(unsigned)> &perDelivery, int owner,
    const CreatureTransferServices &lower) {
  auto queueConsequences = [&](unsigned slot) {
    auto result = perQueue(slot);
    result.performOrder = [&, slot](const CreatureOrder &order) {
      auto consequences = perDelivery(slot);
      auto &a = world.creatures.at(slot);
      deliverOrder(a, order, world,
                   [&](Cell p) { consequences.move(a, p); },
                   [&](auto &b) { consequences.target(a, b); },
                   [&](Cell p) { consequences.coordinate(a, p); });
    };
    return result;
  };
  transferCreatureOwnerQueueOwned(
      actor, world, execution, bindings, state, selection, sharing, visibility,
      graph, owners, limits, countUI, repair, repairServices, perActor, perIdle,
      perFollow, queueConsequences, owner, lower);
}
} // namespace mnm::reconstruction::combat
