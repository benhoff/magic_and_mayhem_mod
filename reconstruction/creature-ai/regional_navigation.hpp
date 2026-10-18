#pragma once
#include "actor_route_request.hpp"
#include "../pathfinding/route_creature_acceptance.hpp"
#include <map>
namespace mnm::reconstruction::combat {
struct RegionConnection {
  int cost = 0, destination = -1;
  std::array<std::int8_t, 20> clearance{};
};
struct NavigationRegion {
  Cell position;
  std::array<RegionConnection, 18> connections{};
  unsigned parameter = 0, traffic = 0;
  std::array<std::array<std::uint8_t, 7>, 8> danger{}, deductedDanger{};
  std::uint8_t dangerBase = 0;
};
struct RegionalQuery {
  CreatureMovementParameters movement;
  bool creature = true, dangerPenalty = false, trafficPenalty = false;
};
struct RegionalSearchRecord {
  int node = -1, priority = INT32_MAX, predecessor = -1;
  MovementPayload movement;
};
struct RegionalOpenNode { int priority = 0, node = -1; };
struct RegionalSearchState {
  bool dirty = true, initialized = false;
  int start = -1, destination = -1, best = -1, bestHeuristic = 0;
  std::vector<RegionalOpenNode> open;
  std::vector<RegionalSearchRecord> records;
};
int regionEdgeDistance(const std::vector<NavigationRegion> &, int, int);
int regionHeuristic(Cell, Cell, Extent);
// Recovered53e520 index; this is not std::lower_bound at the high endpoint.
int regionLowerBound(const std::vector<NavigationRegion> &, int x);
bool regionConnectionAdmitted(const RegionConnection &, const CreatureMovementParameters &);
int regionDanger(const AutonomousCreature &, const NavigationRegion &,
                 std::array<unsigned, 2> powers, bool subtract = false);
struct RegionalServices {
  std::function<bool(Cell)> legal;
  std::function<bool(Cell, Cell)> line;
  // 53d7d0's fresh12-expansion tile search, including the discarded last
  // expansion. Return whether its reconstructed path contains the endpoint.
  std::function<bool(Cell, Cell)> boundedRoute;
  std::function<int(int)> danger;
};
int lookupNavigationRegion(const std::vector<NavigationRegion> &, Cell,
                           const RegionalQuery &, Extent, int boundary,
                           int hintX, int hintY, const RegionalServices &);
bool searchNavigationRegions(std::vector<NavigationRegion> &, int start, int end,
                             const RegionalQuery &, Extent,
                             std::array<int, 50> &, int &budget, bool reset,
                             RegionalSearchState &, const RegionalServices &);
//513cf0: flying-height candidate and movement-kind2 boundary correction.
void regionalCandidate(const AutonomousCreature &, Cell &, unsigned parameter,
                       Cell desired, bool flight, unsigned flightClearance,
                       int movementKind, int boundary, int layers,
                       const std::function<bool(Cell)> &cell);
} // namespace mnm::reconstruction::combat
