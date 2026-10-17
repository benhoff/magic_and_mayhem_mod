#include "owner_task_inputs.hpp"
#include <climits>
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
bool wizard(unsigned kind) {
  return kind == 0 || kind == 24 || kind == 25 || kind == 26;
}
int added(int before, unsigned amount) {
  if (before < 0 || before >= INT_MAX || amount >= unsigned(INT_MAX - before))
    throw std::invalid_argument("owner task credit overflow");
  return before + int(amount);
}
} // namespace
bool ownerTaskDistressed(unsigned kind, bool transformed, unsigned health,
                         unsigned maximum, int danger, int support) {
  if (danger < 0 || support < 0 || support == INT_MAX)
    throw std::invalid_argument("owner task distress domain");
  return wizard(kind) && !transformed && health <= maximum / 2 &&
         static_cast<long double>(danger) / (support + 1) > 0.5L;
}
bool ownerTaskOnSpecialCell(unsigned slot, const std::vector<unsigned> &occupants) {
  return std::find(occupants.begin(), occupants.end(), slot) != occupants.end();
}
void addOwnerTaskCost(OwnerPreviousTask &task, unsigned amount) {
  task.attempts = added(task.attempts, amount);
}
void addOwnerTaskReward(OwnerPreviousTask &task, unsigned amount) {
  task.successes = added(task.successes, amount);
}
void addOwnerTaskDamage(OwnerPreviousTask *source, OwnerPreviousTask *victim,
                        unsigned kind, unsigned amount) {
  const auto reward = static_cast<unsigned long long>(amount) *
                      (wizard(kind) ? 2u : 1u);
  if (reward >= INT_MAX)
    throw std::invalid_argument("owner task damage reward overflow");
  // Compute both before mutation, including the same-task/self-damage case.
  const int earned = source ? added(source->successes, unsigned(reward)) : 0;
  const int cost = victim ? added(victim->attempts, amount) : 0;
  if (source)
    source->successes = earned;
  if (victim)
    victim->attempts = cost;
}
} // namespace mnm::reconstruction::combat
