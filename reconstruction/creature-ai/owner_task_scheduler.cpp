#include "owner_task_scheduler.hpp"
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction::combat {
void validateOwnerTaskScheduler(const OwnerTaskScheduler &s, unsigned active) {
  if ((s.pendingExecution && s.pendingExecution != active) ||
      (s.executionComplete && !active))
    throw std::invalid_argument("owner scheduler task identity");
}
bool retireOwnerTask(const OwnerPreviousTask &task, bool complete, int tick,
                     const OwnerReplacementPolicy &policy) {
  validateOwnerPreviousTask(task);
  validateOwnerReplacementPolicy(policy);
  if (tick < task.started)
    throw std::invalid_argument("owner scheduler retirement time");
  if (task.value < 1) {
    const double age = double(static_cast<long double>(tick - task.started) /
                              (task.expires - task.started));
    if (std::pow(double(ownerTaskProgress(task, policy)), age) < policy.expiry)
      return true;
  }
  return tick >= task.expires || complete;
}
OwnerTaskSchedulerResult
advanceOwnerTaskScheduler(OwnerTaskScheduler &s, int tick, int total,
                          const OwnerTaskSchedulerServices &p, bool disabled) {
  if (tick < 0 || total < 0 || total > 1000000 || !p.cleanup || !p.plan ||
      !p.active || !p.execute)
    throw std::invalid_argument("owner scheduler input domain");
  validateOwnerTaskScheduler(s, p.active().serial);
  OwnerTaskSchedulerResult out{total / 2, total - total / 2, false};
  if (disabled)
    return out;
  const bool resumed = s.planningPending;
  if (!resumed)
    p.cleanup();
  // 415f/4162: the driver calls the root before testing remaining budget.
  p.plan(out.planningBudget);
  s.planningPending = out.planningBudget < 1;
  if (resumed && !s.planningPending)
    p.cleanup();
  const auto task = p.active();
  if (!s.pendingExecution)
    s.pendingExecution = task.serial;
  if (s.pendingExecution && out.executionBudget > 0) {
    if (s.pendingExecution == task.serial && tick < task.expires &&
        !s.executionComplete && task.sourceAvailable) {
      s.executionComplete = !p.execute(out.executionBudget);
      out.executed = true;
    }
    s.pendingExecution = 0;
  }
  validateOwnerTaskScheduler(s, p.active().serial);
  return out;
}
} // namespace mnm::reconstruction::combat
