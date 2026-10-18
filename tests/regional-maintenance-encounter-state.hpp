#pragma once
#include "regional-graph-encounter-state.hpp"
namespace mnm::encounter::regional_maintenance {
inline Profile profile(game::Bytes inputs, unsigned scenario) {
  auto p = decodeInputs(std::move(inputs));
  auto &c = p.initial;
  for (auto &v : c.native.cells) {
    v.flags &= std::uint16_t(~3u);
    v.occupant = 0xffff;
  }
  // Split the torus at x0 and x16. The only crossing is the mutable x16 gate.
  for (int z = 0; z < c.world.dimensions.z; ++z)
    for (int y = 0; y < 32; ++y) {
      c.native.cells[(z * 32 + y) * 32].flags |= 0x4000;
      if ((scenario & 1) || y < 4 || y > 10)
        c.native.cells[(z * 32 + y) * 32 + 16].flags |= 0x4000;
    }
  for (auto &a : c.world.creatures) {
    a.ai.owner = 0;
    a.ai.enemyRadius.fill(0);
    a.ai.allyRadius.fill(0);
  }
  // Build before reserving actors; fixture positions are selected below.
  cs::prepareNativeRegionalGraph(c, p.native, navigationShapes(p));
  cs::NativeCreatureGeometry geometry(c, p.native);
  auto &a = c.world.creatures[0];
  const auto choose = [&](int x, int y) {
    int best = -1, distance = INT32_MAX;
    for (unsigned i = 0; i < c.native.regions.size(); ++i) {
      const auto q = c.native.regions[i].position;
      if ((x < 16) != (q.x < 16) || !cs::nativeRegionalLegal(geometry, a, q, 0))
        continue;
      const int cost =
          std::abs(x - q.x) + std::abs(y - q.y) + std::abs(3 - q.z);
      if (cost < distance) {
        best = int(i);
        distance = cost;
      }
    }
    if (best < 0)
      throw std::runtime_error("dynamic gate legal endpoint absent");
    return c.native.regions[unsigned(best)].position;
  };
  if (scenario & 1) {
    const auto open = profile(p.bytes, 0);
    a.ai.position = open.initial.world.creatures[0].ai.position;
    a.destination = open.initial.world.creatures[0].destination;
  } else {
    const auto query = cs::nativeRegionalQuery(geometry, a, {0, 0, 0, 0});
    int best = INT32_MAX;
    bool found = false;
    for (unsigned start = 0; start < c.native.regions.size(); ++start) {
      const auto from = c.native.regions[start].position;
      if (from.x >= 16 || !cs::nativeRegionalLegal(geometry, a, from, 0))
        continue;
      std::vector<int> depth(c.native.regions.size(), -1);
      std::vector<unsigned> open{start};
      depth[start] = 0;
      for (unsigned at = 0; at < open.size(); ++at)
        for (const auto &edge : c.native.regions[open[at]].connections)
          if (cs::model::regionConnectionAdmitted(edge, query.movement) &&
              depth.at(unsigned(edge.destination)) == -1) {
            depth[unsigned(edge.destination)] = depth[open[at]] + 1;
            open.push_back(unsigned(edge.destination));
          }
      for (unsigned end = 0; end < c.native.regions.size(); ++end) {
        const auto to = c.native.regions[end].position;
        if (to.x < 16 || depth[end] < 3 ||
            !cs::nativeRegionalLegal(geometry, a, to, 0))
          continue;
        const int distance = std::abs(from.x - 7) + std::abs(from.y - 7) +
                             std::abs(to.x - 25) + std::abs(to.y - 7);
        if (distance < best) {
          best = distance;
          a.ai.position = from;
          a.destination = to;
          found = true;
        }
      }
    }
    if (!found)
      throw std::runtime_error(
          "dynamic gate lacks admitted cross-wall journey");
  }
  a.ai.home = a.ai.position;
  c.execution[0].roamHome = a.ai.position;
  a.ai.behavior = 2;
  a.ai.action = 15;
  a.routePhase = 1;
  c.bindings[0] = cs::model::CommandBehaviorBinding::move;
  auto &b = c.world.creatures[1];
  b.ai.position = choose(4, 25);
  b.ai.home = b.destination = b.ai.position;
  c.execution[1].roamHome = b.ai.position;
  cs::prepareNativeContinuation(c, p.native);
  return p;
}
inline Profile profile(const std::filesystem::path &install,
                       unsigned scenario) {
  return profile(loadInputs(install, {0, 0, 32, 32}, 0), scenario);
}
inline std::vector<cs::NativeTerrainEdit>
edits(const Profile &p, unsigned scenario, unsigned tick) {
  bool update = false, closed = false;
  if (scenario == 0) {
    update = tick == 16 || tick == 192 || tick == 384 || tick == 640;
    closed = tick == 16 || tick == 384;
  } else if (scenario == 1) {
    update = tick == 64 || tick == 256 || tick == 512;
    closed = tick == 256;
  } else if (scenario == 2) {
    update = tick == 1 || tick == 2 || tick == 3 || tick == 4 || tick == 256;
    closed = tick == 1 || tick == 3;
  } else if (scenario == 3) {
    update = tick == 1 || tick == 128 || tick == 129 || tick == 512;
    closed = tick == 128;
  }
  if (!update)
    return {};
  std::vector<cs::NativeTerrainEdit> out;
  for (int z = 0; z < p.initial.world.dimensions.z; ++z)
    for (int y = 4; y <= 10; ++y)
      out.push_back({{16, y, z},
                     p.initial.native.cells[(z * 32 + y) * 32 + 16].kind,
                     0x4000,
                     std::uint16_t(closed ? 0x4000 : 0)});
  return out;
}
inline paired::Projection projection(const cs::Continuation &c,
                                     const Profile &p) {
  auto out = regional::projection(c, p);
  auto add = [&](std::string n, unsigned v) {
    out.push_back({std::move(n), v});
  };
  for (unsigned i = 0; i < c.native.regions.size(); ++i)
    for (unsigned j = 0; j < 18; ++j) {
      const auto &e = c.native.regions[i].connections[j];
      auto prefix =
          "repair.edge[" + std::to_string(i) + "][" + std::to_string(j) + "].";
      add(prefix + "cost", e.cost);
      add(prefix + "to", e.destination);
      for (unsigned k = 0; k < 20; ++k)
        add(prefix + "clearance[" + std::to_string(k) + "]",
            std::uint8_t(e.clearance[k]));
    }
  const auto &m = c.native.regionalMaintenance->state;
  for (unsigned i = 0; i < m.queue.size(); ++i) {
    const auto &e = m.queue[i];
    auto prefix = "repair.queue[" + std::to_string(i) + "].";
    add(prefix + "node", e.node);
    add(prefix + "mode", e.mode);
    add(prefix + "priority", e.priority);
  }
  const unsigned values[]{m.read,
                          m.write,
                          unsigned(m.active),
                          unsigned(m.other),
                          unsigned(m.stage),
                          unsigned(m.height),
                          unsigned(m.remaining),
                          unsigned(m.opening),
                          unsigned(m.closing)};
  for (unsigned i = 0; i < 9; ++i)
    add("repair.field[" + std::to_string(i) + "]", values[i]);
  return out;
}
} // namespace mnm::encounter::regional_maintenance
