#pragma once
#include "owner_task_lifecycle.hpp"
#include <functional>
namespace mnm::reconstruction::combat {
// Ordinary single direct-root projection of415c40, count-budget mode only.
// Specialized source queues and source/perception maintenance are services.
struct OwnerTaskScheduler {
  bool planningPending = false;
  unsigned pendingExecution = 0;
  // Task+14 execution result, distinct from replacement-invalidated task+15
  // (OwnerPreviousTask::completed in the historical replacement API).
  bool executionComplete = false;
};
struct OwnerScheduledTask {
  unsigned serial = 0;
  int expires = 0;
  bool sourceAvailable = true;
};
struct OwnerTaskSchedulerServices {
  std::function<void()> cleanup;
  std::function<void(int &)> plan;
  std::function<OwnerScheduledTask()> active;
  std::function<bool(int &)> execute;
};
struct OwnerTaskSchedulerResult {
  int planningBudget = 0, executionBudget = 0;
  bool executed = false;
};
void validateOwnerTaskScheduler(const OwnerTaskScheduler &, unsigned active);
// Nonnegative total is split floor(total/2), total-floor(total/2). Planning
// enters the pending root even with zero budget. Execution checks budget first.
OwnerTaskSchedulerResult
advanceOwnerTaskScheduler(OwnerTaskScheduler &, int tick, int total,
                          const OwnerTaskSchedulerServices &,
                          bool disabled = false);
// Whole428b40's ordinary enabled-source retirement predicate. The task6 expiry
// virtual method is a no-op. Feedback/training and source destruction excluded.
bool retireOwnerTask(const OwnerPreviousTask &, bool executionComplete,
                     int tick, const OwnerReplacementPolicy &);
} // namespace mnm::reconstruction::combat
