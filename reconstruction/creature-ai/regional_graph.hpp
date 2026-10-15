#pragma once
#include "regional_navigation.hpp"
namespace mnm::reconstruction::combat {
struct RegionalCreatureShape {
  int height = 0, width = 0, movementKind = 0;
};
struct NavigationProbeOpen {
  int priority = 0;
  unsigned node = 0;
};
struct NavigationProbeNode {
  unsigned node = 0;
  NodeRecord record;
};
struct NavigationProbeState {
  std::vector<NavigationProbeOpen> open;
  std::vector<NavigationProbeNode> records;
  unsigned best = 0;
  int heuristic = 0;
  bool initialized = false;
};
struct RegionalGraphServices {
  Extent dimensions;
  int boundary = 0;
  std::function<unsigned(Cell)> kind;
  std::function<bool(Cell, const CreatureMovementParameters &)> valid, legal;
  std::function<bool(Cell, Cell, const CreatureMovementParameters &)> line,
      move;
  // Generic mode-one descriptor search, not a creature-owned route.
  std::function<std::vector<Cell>(Cell, Cell,
                                  const CreatureMovementParameters &, int &)>
      route;
  std::function<std::vector<Cell>(Cell, Cell,
                                  const CreatureMovementParameters &, int &,
                                  bool, NavigationProbeState &)>
      resumeRoute;
};
// Original53b600: prefer a two-cell footprint without changing selected
// height/kind.
bool navigationDirectRoute(Cell, Cell, const CreatureMovementParameters &,
                           int &budget, int &cost,
                           const RegionalGraphServices &);
Cell adjustNavigationNode(Cell, const RegionalGraphServices &);
// Original53cd30: distinct configured heights, smallest first/largest second.
std::array<int, 6>
navigationHeights(const std::array<RegionalCreatureShape, 28> &, bool ground,
                  bool free, int kind, int width);
std::vector<NavigationRegion>
buildNavigationNodes(unsigned capacity, const RegionalGraphServices &);
void sortNavigationNodes(std::vector<NavigationRegion> &, unsigned capacity,
                         Extent);
void buildNavigationConnections(std::vector<NavigationRegion> &,
                                const std::array<RegionalCreatureShape, 28> &,
                                bool wrap, const RegionalGraphServices &,
                                int budgetAdjustment = 0);
// 53c070: reconnect an existing node, with ground rather than kind3 admission.
void reconnectNavigationNode(std::vector<NavigationRegion> &, unsigned,
                             const std::array<RegionalCreatureShape, 28> &,
                             const RegionalGraphServices &);
} // namespace mnm::reconstruction::combat
