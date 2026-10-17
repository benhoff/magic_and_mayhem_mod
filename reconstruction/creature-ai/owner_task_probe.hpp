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
struct OwnerTaskPathTotals {
  int travel = 0, auxiliary = 0, danger = 0, risk = 0;
};
// 431ce0 per-member successful-probe accumulation. Travel is a maximum;
// auxiliary and destination danger are summed then divided by source count.
void addOwnerTaskPath(OwnerTaskPathTotals &, int cost, int speed, int auxiliary,
                      int destinationDanger, int pathRisk);
int ownerTaskTravelTime(int rawCost, int speed);
} // namespace mnm::reconstruction::combat
