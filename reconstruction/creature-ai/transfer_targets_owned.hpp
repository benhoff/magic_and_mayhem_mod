#pragma once
#include "owner_target_repair.hpp"
#include "transfer_owned.hpp"
namespace mnm::reconstruction::combat {
// Extend the owned transfer chain with whole51edf0 target repair on the same
// canonical actors. Prepare/behavior/peer consequences remain explicit; both
// transfer and target repair use lower.reset, matching their shared50ab30 call.
void transferCreatureOwnerTargetsOwned(
    unsigned actor, AutonomousWorld &, CreatureExecution &,
    CreatureTransferState &, CreatureTransferWorld &,
    const OwnerVisionSharing &, CreatureOwnerVisibility &, SourceGraph &,
    const std::array<AssociatedOwnerRecord, 8> &,
    std::vector<CreatureControlLimit> &, const OwnerCountServices &,
    OwnerTargetRepairState &, const OwnerTargetRepairServices &, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
