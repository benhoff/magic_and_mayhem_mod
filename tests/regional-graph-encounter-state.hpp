#pragma once
#include "../apps/command-session/native_regional.hpp"
#include "../apps/command-session/native_regional_graph.hpp"
#include "paired-encounter-projection.hpp"
#include "regional-encounter-state.hpp"
#include <algorithm>
#include <cstring>
namespace mnm::encounter::regional_graph {
// Generated original-contract graphs from installed terrain and CFG shapes.
inline Profile profile(game::Bytes inputs, unsigned scenario) {
  auto p = decodeInputs(std::move(inputs));
  auto &c = p.initial;
  auto &a = c.world.creatures[0];
  for (auto &v : c.native.cells) {
    v.flags &= std::uint16_t(~3u);
    v.occupant = 0xffff;
  }
  for (auto &actor : c.world.creatures) {
    actor.ai.owner = 0;
    actor.destination = actor.ai.position;
    actor.ai.enemyRadius.fill(0);
    actor.ai.allyRadius.fill(0);
  }
  // A long sealed wall with a passable opening exercises local replanning.
  if (scenario == 1 || scenario == 2)
    for (int z = 0; z < c.world.dimensions.z; ++z)
      for (int y = 1; y < 31; ++y)
        if (scenario == 2 || y < 4 || y > 10)
          c.native.cells[(z * 32 + y) * 32 + 16].flags |= 0x4000;
  enableRegionalNavigation(p);
  auto &regions = c.native.regions;
  cs::NativeCreatureGeometry geometry(c, p.native);
  const auto query = cs::nativeRegionalQuery(geometry, a, {0, 0, 0, 0});
  const auto legal = [&](unsigned i) {
    return cs::nativeRegionalLegal(geometry, a, regions[i].position,
                                   p.native.boundary);
  };
  const auto choose = [&](int x, int y) {
    unsigned best = 0;
    int cost = INT32_MAX;
    for (unsigned i = 0; i < regions.size(); ++i) {
      auto q = regions[i].position;
      int v = std::abs(q.x - x) + std::abs(q.y - y);
      if (v < cost) {
        cost = v;
        best = i;
      }
    }
    return best;
  };
  unsigned source = choose(6, 6), target = choose(26, 26);
  const auto ally = choose(2, 26);
  if (scenario != 2) {
    int best = -1;
    for (unsigned start = 0; start < regions.size(); ++start) {
      if (!legal(start))
        continue;
      std::vector<unsigned> open{start};
      std::vector<int> depth(regions.size(), -1);
      depth[start] = 0;
      for (unsigned at = 0; at < open.size(); ++at) {
        auto node = open[at];
        for (const auto &edge : regions[node].connections)
          if (cs::model::regionConnectionAdmitted(edge, query.movement) &&
              depth.at(unsigned(edge.destination)) == -1) {
            depth[unsigned(edge.destination)] = depth[node] + 1;
            open.push_back(unsigned(edge.destination));
          }
      }
      for (unsigned end = 0; end < regions.size(); ++end)
        if (depth[end] >= 3 && legal(end)) {
          const auto a = regions[start].position, b = regions[end].position;
          int cost = std::abs(a.x - b.x) + std::abs(a.y - b.y);
          if (scenario == 1 && ((a.x < 16) == (b.x < 16)))
            continue;
          if (cost > best && start != ally && end != ally) {
            best = cost;
            source = start;
            target = end;
          }
        }
    }
    if (best < 0) {
      std::string info;
      for (unsigned j = 0; j < regions.size(); ++j) {
        auto v = regions[j].position;
        unsigned edges = 0;
        for (auto &e : regions[j].connections)
          edges += e.cost != 0;
        info += " [" + std::to_string(v.x) + "," + std::to_string(v.y) + ":" +
                std::to_string(edges) + "]";
      }
      throw std::runtime_error("regional connected fixture journey missing:" +
                               info);
    }
  }
  a.ai.position = regions[source].position;
  a.ai.home = a.ai.position;
  c.execution[0].roamHome = a.ai.position;
  c.world.creatures[1].ai.position = regions[ally].position;
  c.world.creatures[1].ai.home = regions[ally].position;
  c.execution[1].roamHome = regions[ally].position;
  c.world.creatures[1].destination = regions[ally].position;
  a.destination = regions[target].position;
  a.ai.behavior = 2;
  a.ai.action = 15;
  a.routePhase = 1;
  c.bindings[0] = cs::model::CommandBehaviorBinding::move;
  if (scenario == 3 ||
      scenario == 4) { // Allied blocker on a retained regional leg.
    auto neighbor = a.ai.position;
    neighbor.x += 2;
    c.world.creatures[1].ai.position = neighbor;
    c.world.creatures[1].destination = neighbor;
    c.execution[1].roamHome = neighbor;
    auto &route = c.actions.segments[0];
    route.count = 1;
    route.route[0].position = {neighbor.x - 1, neighbor.y, neighbor.z};
    route.route[0].remainder = {2, 0, 0, 0};
    a.routePhase = 2;
    a.ai.marker = 0;
    route.regionPending = true;
    route.regionPath.fill(-1);
    route.regionPath[0] = int(source);
    route.regionPath[1] = int(target);
    route.regionCursor = 1;
    c.native.routeMetadata.resize(2);
    c.native.routeMetadata[0].regionDestination = a.destination;
  }
  if (scenario == 4) {
    auto &route = c.actions.segments[0];
    if (regions.size() < 50)
      throw std::runtime_error("alias fixture needs region49");
    route.regionPath.fill(int(source));
    route.regionCursor = 49;
  }
  c.native.route.useRegions = true;
  cs::prepareNativeContinuation(c, p.native);
  return p;
}
inline Profile profile(const std::filesystem::path &install,
                       unsigned scenario) {
  return profile(loadInputs(install, {0, 0, 32, 32}, 0), scenario);
}
using regional::projection;
} // namespace mnm::encounter::regional_graph
