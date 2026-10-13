#pragma once
#include "owner_counts.hpp"
#include "owner_transfer.hpp"
#include "ownership_helpers.hpp"
#include "source_graph.hpp"
namespace mnm::reconstruction::combat {
// Compose whole51ec80 with owned recount, visibility, queued markers,
// wizard-kind and source-root activity rules. These operate on the caller's
// existing state. Reset, effects, UI, target repair and source-controller work
// remain explicit lower services. Callbacks must keep actor/order/root storage
// stable.
void transferCreatureOwnerOwned(unsigned actor, AutonomousWorld &,
                                CreatureExecution &, CreatureTransferState &,
                                CreatureTransferWorld &,
                                const OwnerVisionSharing &,
                                CreatureOwnerVisibility &, SourceGraph &,
                                const std::array<AssociatedOwnerRecord, 8> &,
                                std::vector<CreatureControlLimit> &,
                                const OwnerCountServices &, int owner,
                                const CreatureTransferServices &lower);
} // namespace mnm::reconstruction::combat
