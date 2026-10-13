#pragma once
#include "source_pending.hpp"
namespace mnm::reconstruction::combat {
// Compose grouping and registration on one owned source tree and queue set.
void updateCreatureSourceOwned(unsigned actor, AutonomousWorld &,
                               SourceControllerState &, SourceManager &,
                               SourcePendingQueues &,
                               const SourceControllerConfiguration &,
                               const std::function<bool(unsigned)> &allocate);
} // namespace mnm::reconstruction::combat
