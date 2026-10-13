#pragma once
#include "execution.hpp"
namespace mnm::reconstruction::combat {
using QueuedMarkerParameters = std::array<std::uint32_t, 63>;
struct QueuedMarkerServices {
  std::function<std::optional<unsigned>()> allocate;
  std::function<void(unsigned, const QueuedMarkerParameters &)> initialize;
};
// 512220 updates local-owner kind2 visual markers. Allocation/initialization
// preserve owned queue topology/backing. Initialization failure retains the
// token.
void updateQueuedMarkers(const AutonomousCreature &, CreatureExecution &,
                         int localOwner, const QueuedMarkerServices &);
} // namespace mnm::reconstruction::combat
