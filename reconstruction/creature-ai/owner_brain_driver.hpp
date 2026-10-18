#pragma once
#include "source_lifecycle.hpp"
#include <functional>
namespace mnm::reconstruction::combat {
// 415c40 count mode. Indices and serials replace original pointers/map keys.
// Snapshot, source maintenance, task bodies and421f70 are explicit services.
struct OwnerBrainDriver {
  int clock = 0;
  unsigned phase = 0;
  std::optional<unsigned> current;
  std::vector<unsigned> execution;
};
struct OwnerBrainDriverServices {
  std::function<void()> cleanup;
  std::function<void(bool)> snapshot; // true=fresh, false=restore
  std::function<std::vector<unsigned>()> sources;
  std::function<bool(unsigned)> hasTask;
  std::function<void(unsigned, bool, int &)> plan; // specialized general queue
  std::function<std::vector<unsigned>()> tasks; // ascending installed map keys
  std::function<void(unsigned, int &)> execute;
};
struct OwnerBrainBudget {
  int planning = 0, execution = 0;
};
void validateOwnerBrainDriver(const OwnerBrainDriver &);
// World46b5be..46b5e8 supplies floor(allocation/2) independently to each
// brain. Odd remainders are discarded at this World boundary.
int ownerWorldBrainAllocation(int total);
OwnerBrainBudget advanceOwnerBrainDriver(OwnerBrainDriver &,
                                         OwnerSourceStorage &, int total,
                                         const OwnerBrainDriverServices &,
                                         bool disabled = false);
} // namespace mnm::reconstruction::combat
