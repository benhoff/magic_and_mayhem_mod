#pragma once
#include "transfer_idle_owned.hpp"
namespace mnm::reconstruction::combat {
// Own whole513850 follow pursuit in the idle/ownership chain. Geometry
// adjustment and move consequences remain explicit; actor/execution backing
// and positive world dimensions are fixed for the call.
void transferCreatureOwnerFollowOwned(
    unsigned actor, AutonomousWorld &, std::vector<CreatureExecution> &,
    OwnershipBehaviorBindings &, CreatureTransferState &,
    CreatureTransferWorld &, const OwnerVisionSharing &,
    CreatureOwnerVisibility &, SourceGraph &,
    const std::array<AssociatedOwnerRecord, 8> &,
    std::vector<CreatureControlLimit> &, const OwnerCountServices &,
    OwnerTargetRepairState &, const OwnerTargetRepairServices &,
    const std::function<ExecutionServices(unsigned)> &,
    const std::function<BehaviorExecutionServices(unsigned)> &,
    const std::function<FollowServices(unsigned)> &, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
