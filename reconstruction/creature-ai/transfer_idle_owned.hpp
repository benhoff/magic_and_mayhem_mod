#pragma once
#include "transfer_reset_owned.hpp"
namespace mnm::reconstruction::combat {
// Logical callback continuation; a callback may edit the behavior word without
// replacing its binding. The supplied state preserves the caller's other
// bodies.
enum class OwnershipBehaviorBinding { supplied, idle };
using OwnershipBehaviorBindings = std::vector<OwnershipBehaviorBinding>;
void transferCreatureOwnerIdleOwned(
    unsigned actor, AutonomousWorld &, std::vector<CreatureExecution> &,
    OwnershipBehaviorBindings &, CreatureTransferState &,
    CreatureTransferWorld &, const OwnerVisionSharing &,
    CreatureOwnerVisibility &, SourceGraph &,
    const std::array<AssociatedOwnerRecord, 8> &,
    std::vector<CreatureControlLimit> &, const OwnerCountServices &,
    OwnerTargetRepairState &, const OwnerTargetRepairServices &,
    const std::function<ExecutionServices(unsigned)> &,
    const std::function<BehaviorExecutionServices(unsigned)> &, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
