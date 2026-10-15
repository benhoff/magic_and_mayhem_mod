#pragma once
#include "regional_graph.hpp"
namespace mnm::reconstruction::combat {
struct NavigationChangeSignature {
  int ground = 0, free = 0, height = 0;
};
struct NavigationRepairEntry {
  int node = -1, mode = -1, priority = -1;
};
struct NavigationMaintenance {
  std::array<NavigationRepairEntry, 1345> queue{};
  unsigned read = 0, write = 0;
  int active = -1, other = 0, stage = 0, height = 0, remaining = 0;
  bool opening = false, closing = false;
  RegionConnection working{0, 0, {}}, previous{0, 0, {}};
  NavigationProbeState probe;
};
NavigationChangeSignature
navigationChangeSignature(Cell, const RegionalGraphServices &);
// 540430: -1 means the terrain signatures do not require maintenance.
int navigationChangeMode(NavigationChangeSignature, NavigationChangeSignature);
// 540140: sorted wrapped-X neighborhood, merge/priority insertion into ring.
void enqueueNavigationRepair(const std::vector<NavigationRegion> &, Cell, int,
                             Extent, NavigationMaintenance &);
bool navigationMaintenancePending(const NavigationMaintenance &);
// 5404a0/540820/5416d0. One call receives ten work units. A blocked call does
//  nothing; actor routes and regional-search state are independent storage.
void maintainNavigationGraph(std::vector<NavigationRegion> &,
                             const std::array<RegionalCreatureShape, 28> &,
                             NavigationMaintenance &, bool blocked,
                             const RegionalGraphServices &);
} // namespace mnm::reconstruction::combat
