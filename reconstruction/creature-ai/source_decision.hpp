#pragma once
#include "../combat/control.hpp"
#include "owner_task_lifecycle.hpp"
namespace mnm::reconstruction::combat {
// Bounded423590 stages0..4: at most three available ordinary friendly members,
// at most one other root, pinned merge/split proposal caps1. World admission,
// evaluator storage and selected topology commits are explicit services.
struct OwnerSourceDecision {
  bool reset = true;
  unsigned stage = 0;
  int winner = -1; // -1 none,0 unchanged,2 merge,4 split
  float bias = 0, winningBias = 0;
};
struct OwnerSourceDecisionInputs {
  std::vector<unsigned> members;
  std::vector<int> speeds;
  std::vector<unsigned> otherMembers;
  std::optional<OwnerPreviousTask> otherTask;
  bool splitFirst = false, weighted = false;
};
struct OwnerSourceDecisionServices {
  // Whole4284e0 projection: the service owns evaluator continuation and
  // candidate save payload; selection score/ties remain shared across phases.
  std::function<bool(const std::vector<unsigned> &, bool root, float bias,
                     const OwnerPreviousTask *, bool rejectOne, int &budget)>
      select;
  // Optional stage1 producer owns the nearest-two shortlist and its RNG draw.
  // The absent producer preserves the old sole-candidate projection.
  std::function<void(RandomState &)> chooseOther;
};
Cell ownerSourceCenter(const std::vector<Cell> &, Extent);
int ownerSourceDistance(const std::vector<Cell> &, const std::vector<Cell> &,
                        Extent);
struct OwnerMergeCandidate {
  unsigned source = 0;
  int distance = 0;
};
std::optional<unsigned>
chooseOwnerMergeCandidate(std::vector<OwnerMergeCandidate>, RandomState &);
// Weighted selection retains a shared maximum; removing a merge bias can
// produce a commit score above1. This is separate from raw evaluator admission.
bool replaceOwnerSourceTask(float, const OwnerPreviousTask *, int,
                            const OwnerReplacementPolicy &);
std::vector<unsigned>
ownerSourceSplitMembers(const OwnerSourceDecisionInputs &);
void validateOwnerSourceDecision(const OwnerSourceDecision &);
void validateOwnerSourceDecisionInputs(const OwnerSourceDecisionInputs &);
bool advanceOwnerSourceDecision(OwnerSourceDecision &, OwnerTaskSelection &,
                                const OwnerSourceDecisionInputs &, int &budget,
                                RandomState &,
                                const OwnerSourceDecisionServices &);
} // namespace mnm::reconstruction::combat
