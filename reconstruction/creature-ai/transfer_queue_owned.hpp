#pragma once
#include "transfer_follow_owned.hpp"
namespace mnm::reconstruction::combat {
// Whole524130 advancement on the same owned order vectors. Delivery remains
// explicit; release shares the transfer/reset endpoint. Callbacks preserve
// order backing, cursor and topology during their own invocation.
void transferCreatureOwnerQueueOwned(
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
    const std::function<ExecutionServices(unsigned)> &perQueue, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
