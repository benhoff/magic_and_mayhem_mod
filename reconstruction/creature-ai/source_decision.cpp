#include "source_decision.hpp"
#include <cmath>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
void validateOwnerSourceDecision(const OwnerSourceDecision &s) {
  if (s.stage > 5 ||
      (s.winner != -1 && s.winner != 0 && s.winner != 2 && s.winner != 4) ||
      !std::isfinite(s.bias) || s.bias < -2 || s.bias > 0 ||
      !std::isfinite(s.winningBias) || s.winningBias < -2 || s.winningBias > 0)
    throw std::invalid_argument("source decision state");
}
std::vector<unsigned>
ownerSourceSplitMembers(const OwnerSourceDecisionInputs &i) {
  if (i.members.size() != 2 || i.speeds.size() != 2)
    throw std::invalid_argument("source decision two-member partition");
  // The first generated mask selects the faster member; equal speed
  // preserves the first member in this bounded two-entry projection.
  return {i.members[i.speeds[1] > i.speeds[0] ? 1 : 0]};
}
void validateOwnerSourceDecisionInputs(const OwnerSourceDecisionInputs &i) {
  std::set<unsigned> unique(i.members.begin(), i.members.end());
  unique.insert(i.otherMembers.begin(), i.otherMembers.end());
  if (i.members.empty() || i.members.size() + i.otherMembers.size() > 2 ||
      i.members.size() != i.speeds.size() ||
      unique.size() != i.members.size() + i.otherMembers.size() ||
      (i.otherTask && i.otherMembers.empty()))
    throw std::invalid_argument("source decision input domain");
  if (i.otherTask)
    validateOwnerPreviousTask(*i.otherTask);
}
bool advanceOwnerSourceDecision(OwnerSourceDecision &s, OwnerTaskSelection &q,
                                const OwnerSourceDecisionInputs &i, int &budget,
                                RandomState &random,
                                const OwnerSourceDecisionServices &services) {
  validateOwnerSourceDecision(s);
  validateOwnerSourceDecisionInputs(i);
  if (!services.select || budget < 0)
    throw std::invalid_argument("source decision services/budget");
  if (s.reset) {
    s = {};
    s.reset = false;
    s.stage = i.splitFirst && i.members.size() == 2 ? 3 : 0;
    q = {};
    q.weighted = i.weighted;
  }
  while (s.stage < 5) {
    if (s.stage == 1) {
      if (i.otherMembers.empty()) {
        s.stage = 5;
        continue;
      }
      // Partial shuffle consumes one draw even for the sole merge candidate.
      (void)nextRandom(random);
      s.bias = -(i.otherTask ? i.otherTask->value + 1.f : 0.f);
      s.stage = 2;
    }
    if (s.stage == 3) {
      if (i.members.size() != 2) {
        s.stage = 5;
        continue;
      }
      s.bias = 0;
      s.stage = 4;
    }
    auto members = i.members;
    const OwnerPreviousTask *old = nullptr;
    if (s.stage == 2) {
      members.insert(members.end(), i.otherMembers.begin(),
                     i.otherMembers.end());
      old = i.otherTask ? &*i.otherTask : nullptr;
    } else if (s.stage == 4)
      members = ownerSourceSplitMembers(i);
    const bool changed =
        services.select(members, true, s.bias, old, s.stage == 2, budget);
    if (changed) {
      s.winner = int(s.stage);
      s.winningBias = s.bias;
    }
    // 423590 tests budget before moving its stage cursor, even if the
    // evaluator sweep just completed on the last unit.
    if (budget < 1)
      return false;
    if (!q.reset)
      throw std::invalid_argument("source evaluator yielded with spare budget");
    if (s.stage == 0)
      s.stage = 1;
    else if (s.stage == 2)
      s.stage = 5;
    else
      s.stage = 5;
  }
  return true;
}
} // namespace mnm::reconstruction::combat
