#pragma once
#include "regional_navigation.hpp"
namespace mnm::reconstruction::combat {
enum class OwnerTaskProbeResult { pending, failed, reached };
struct OwnerTaskProbe {
  int remaining = 20;
  OwnerTaskProbeResult result = OwnerTaskProbeResult::pending;
  RegionalSearchState search;
  std::vector<int> path; // Original candidate order: best/goal back to source.
};
OwnerTaskProbeResult
advanceOwnerTaskProbe(OwnerTaskProbe &, std::vector<NavigationRegion> &,
                      int from, int to, const RegionalQuery &, Extent,
                      int &budget, const RegionalServices &);
int ownerTaskTravelTime(int rawCost, int speed);
} // namespace mnm::reconstruction::combat
