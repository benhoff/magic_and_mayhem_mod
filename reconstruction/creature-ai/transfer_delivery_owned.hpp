#pragma once
#include "transfer_queue_owned.hpp"
namespace mnm::reconstruction::combat {
// Lower command consequences preserve actor/order backing, topology and cursor.
// The recovered dispatcher owns mode selection and the living-target gate.
struct OrderDeliveryServices {
  std::function<void(AutonomousCreature &, Cell)> move, coordinate;
  std::function<void(AutonomousCreature &, AutonomousCreature &)> target;
};
void transferCreatureOwnerDeliveryOwned(
    unsigned actor, AutonomousWorld &, std::vector<CreatureExecution> &,
    OwnershipBehaviorBindings &, CreatureTransferState &,
    CreatureTransferWorld &, const OwnerVisionSharing &,
    CreatureOwnerVisibility &, SourceGraph &,
    const std::array<AssociatedOwnerRecord, 8> &,
    std::vector<CreatureControlLimit> &, const OwnerCountServices &,
    OwnerTargetRepairState &, const OwnerTargetRepairServices &,
    const std::function<ExecutionServices(unsigned)> &,
    const std::function<BehaviorExecutionServices(unsigned)> &,
    const std::function<FollowServices(unsigned)> &,
    const std::function<ExecutionServices(unsigned)> &perQueue,
    const std::function<OrderDeliveryServices(unsigned)> &perDelivery, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
