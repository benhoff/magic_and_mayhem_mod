#pragma once
#include "transfer_targets_owned.hpp"
namespace mnm::reconstruction::combat {
// Whole ownership transfer/target repair with owned reset50ab30 and behavior
// transition50aa30/50a930. Actor and execution backing remain stable during
// callbacks. Behavior bindings/bodies and color restoration are supplied by
// per-actor services; effect release and peer engagement share the transfer
// and repair endpoints rather than introducing separate consequences.
void transferCreatureOwnerResetOwned(
    unsigned actor, AutonomousWorld &, std::vector<CreatureExecution> &,
    CreatureTransferState &, CreatureTransferWorld &,
    const OwnerVisionSharing &, CreatureOwnerVisibility &, SourceGraph &,
    const std::array<AssociatedOwnerRecord, 8> &,
    std::vector<CreatureControlLimit> &, const OwnerCountServices &,
    OwnerTargetRepairState &, const OwnerTargetRepairServices &,
    const std::function<ExecutionServices(unsigned actor)> &, int owner,
    const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
