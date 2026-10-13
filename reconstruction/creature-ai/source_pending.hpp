#pragma once
#include "source_controller.hpp"
#include <deque>
namespace mnm::reconstruction::combat {
struct SourcePendingQueues {
  std::deque<unsigned> general, wizard, other;
};
// Whole416fe0 logical queue admission: optional general registration and a
// deduplicated wizard/other category, both at the front. All source traversal
// slots are drained. Queue allocation/block mechanics remain native storage.
void registerSourceRoot(AutonomousWorld &, SourceControllerState &,
                        SourcePendingQueues &, unsigned root);
} // namespace mnm::reconstruction::combat
