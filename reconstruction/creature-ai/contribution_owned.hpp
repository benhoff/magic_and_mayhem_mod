#pragma once
#include "contribution_account.hpp"
#include "execution.hpp"
#include "resource.hpp"
#include "resource_contribution.hpp"
namespace mnm::reconstruction::combat {
struct ContributionOwnership {
  std::array<unsigned, 8> ownerEnabled{};
  std::vector<unsigned> maximum;
  ContributionProfiles profiles;
};
struct ContributionDisplayServices {
  std::function<void(AutonomousCreature &, unsigned)> primary, secondary;
};
// Compose466870,507f00 and416d80 under the inline maintenance contribution.
// ResourceContributionState.resource is the sole persistent current value;
// controllers supply the exact owner-table actor index. Display reentry reads
// and writes that same owned value. Native primitive boundaries remain
// explicit.
void contributeCreatureResourceOwned(AutonomousCreature &, AutonomousWorld &,
                                     std::vector<CreatureExecution> &,
                                     ResourceContributionState &,
                                     const ContributionOwnership &,
                                     unsigned grant, int localOwner,
                                     const ContributionDisplayServices &);
} // namespace mnm::reconstruction::combat
