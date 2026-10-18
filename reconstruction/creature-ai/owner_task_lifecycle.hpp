#pragma once
#include "owner_planner.hpp"
namespace mnm::reconstruction::combat {
// Logical predicate inputs to whole431be0, not offsets in a host actor.
struct OwnerTask6Admission {
  bool enabled = true;
  unsigned brain = 0;
  bool parent = false, allAssigned = true, allOffSpecialCells = true,
       singletonWizardReady = false, ownerAvailable = true,
       ownerDistressed = false;
};
bool admitOwnerTask6(const OwnerTask6Admission &);
// Ordinary direct-root task state. Progress counters retain their original
// separate numerators/denominators; they are not inferred from movement.
struct OwnerPreviousTask {
  int started = 0, expires = 1;
  bool completed = false, weighted = false;
  float value = -1, baseline = 0;
  int successes = 0, attempts = 0;
};
struct OwnerReplacementPolicy {
  float improvement = 0x1.ccccccp-1f, expiry = 0x1.0624dep-10f;
  bool ownerAvailable = false;
  int ownerNumerator = 0, ownerDenominator = 0;
};
void validateOwnerPreviousTask(const OwnerPreviousTask &);
void validateOwnerReplacementPolicy(const OwnerReplacementPolicy &);
long double ownerTaskProgress(const OwnerPreviousTask &,
                              const OwnerReplacementPolicy &);
bool replaceOwnerTask(float candidate, const OwnerPreviousTask *, int tick,
                      const OwnerReplacementPolicy &);
// Standalone40b3f0(mean100) draw with truncation; not the full task6 lifetime.
int ownerTaskDuration(RandomState &);
// Whole431ce0 lifetime: truncate(exponential(mean100) + produced travel + 600).
// One RNG draw per successfully completed candidate, including losing ones.
int ownerTask6Duration(RandomState &, int travelTime);
// 4284e0 old-task filter before bias/ties/weighting. The ordinary direct-root
// 423590 path selects without this filter, then applies replaceOwnerTask.
bool advanceOwnerTaskReplanning(OwnerTaskSelection &, int &, RandomState &,
                                const OwnerTaskSelectionServices &,
                                const OwnerPreviousTask *, int tick,
                                const OwnerReplacementPolicy &, float bias = 0,
                                bool save = true, bool rejectOne = false);
} // namespace mnm::reconstruction::combat
