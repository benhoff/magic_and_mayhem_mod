#include "../apps/command-session/native_regional.hpp"
#include "../apps/command-session/native_regional_graph.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "regional-navigation-original.hpp"
#else
#include "paired-encounter-projection.hpp"
#include <iostream>
#endif
#include <algorithm>
namespace {
using namespace mnm;
namespace cs = command_session;
namespace m = cs::model;
void need(bool ok, const std::string &s) {
  if (!ok)
    throw std::runtime_error(s);
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_PAIRED_REFERENCE
  need(argc == 4, "PE INSTALL SCENARIO");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  redirect(0x4dc5e0, reinterpret_cast<std::uintptr_t>(&noArg));
  sealText(pe);
  initializeSearch();
  using Constructor = void *(__attribute__((thiscall)) *)(void *, unsigned);
  reinterpret_cast<Constructor>(0x53a660)(regionManager, 512);
  const auto install = argv[2];
  const unsigned scenario = std::stoul(argv[3]);
#else
  need(argc == 3, "INSTALL SCENARIO");
  const auto install = argv[1];
  const unsigned scenario = std::stoul(argv[2]);
#endif
  need(scenario < 4, "maintenance scenario");
  auto p = encounter::decodeInputs(
      encounter::loadInputs(install, {0, 0, 32, 32}, 0));
  auto c = p.initial;
  for (auto &v : c.native.cells) {
    v.flags &= std::uint16_t(~3u);
    v.occupant = 0xffff;
  }
  if (scenario & 1)
    p.native.boundary = 6;
  if (scenario & 2)
    for (int z = 0; z < c.world.dimensions.z; ++z)
      for (int y = 1; y < 31; ++y)
        if (y < 4 || y > 10)
          c.native.cells[(z * 32 + y) * 32 + 16].flags |= 0x4000;
  const auto shapes = encounter::navigationShapes(p);
  cs::prepareNativeRegionalGraph(c, p.native, shapes);
  cs::prepareNativeContinuation(c, p.native);
#ifdef MNM_PAIRED_REFERENCE
  Original o(p);
  setup(o, c, 17);
  global(0x6c5c80, p.native.boundary);
  for (unsigned i = 0; i < 28; ++i) {
    auto *cfg = reinterpret_cast<void *>(0x6a5f80 + i * 0x5c9);
    put(cfg, 8, shapes[i].width);
    put(cfg, 12, shapes[i].height);
    put(cfg, 0x44, shapes[i].movementKind);
  }
  using Build = void(__attribute__((thiscall)) *)(void *, int);
  reinterpret_cast<Build>(0x53ab20)(regionManager, 1);
  call(0x53d710, regionManager);
  using Connect = void(__attribute__((thiscall)) *)(void *, int, int);
  reinterpret_cast<Connect>(0x53bb40)(regionManager, 1, 0);
#endif
  unsigned checks = 0, steps = 0, yields = 0, edits = 0;
  std::uint64_t digest = 1469598103934665603ull;
  auto record = [&](unsigned actual, unsigned expected,
                    const std::string &name) {
    if (actual != expected) {
      const auto &m = c.native.regionalMaintenance->state;
      std::cerr << "context active=" << m.active << " other=" << m.other
                << " stage=" << m.stage << " height=" << m.height
                << " workcost=" << m.working.cost
                << " oldcost=" << m.previous.cost << '\n';
      throw std::runtime_error(name + " step=" + std::to_string(steps) +
                               " native=" + std::to_string(actual) +
                               " original=" + std::to_string(expected));
    }
    digest = (digest ^ actual) * 1099511628211ull;
    ++checks;
  };
  auto compare = [&] {
    for (unsigned i = 0; i < c.native.regions.size(); ++i) {
      const auto &v = c.native.regions[i];
#ifdef MNM_PAIRED_REFERENCE
      auto *raw =
          reinterpret_cast<unsigned char *>(get(regionManager) + 660 * i);
      record(v.position.x, get(raw, 4), "node.x");
      record(v.position.y, get(raw, 8), "node.y");
      record(v.position.z, get(raw, 12), "node.z");
#else
      record(v.position.x, v.position.x, "node.x");
      record(v.position.y, v.position.y, "node.y");
      record(v.position.z, v.position.z, "node.z");
#endif
      for (unsigned j = 0; j < 18; ++j) {
        const auto &e = v.connections[j];
#ifdef MNM_PAIRED_REFERENCE
        auto *edge = raw + 16 + 28 * j;
        const auto name =
            "edge[" + std::to_string(i) + "][" + std::to_string(j) + "]";
        record(e.cost, get(edge), name + "cost");
        record(e.destination, get(edge, 4), name + "to");
        for (unsigned k = 0; k < 20; ++k)
          record(std::uint8_t(e.clearance[k]), get(edge, 8 + k, 1),
                 name + "clearance" + std::to_string(k));
#else
        record(e.cost, e.cost, "edge.cost");
        record(e.destination, e.destination, "edge.to");
        for (auto b : e.clearance)
          record(std::uint8_t(b), std::uint8_t(b), "edge.clearance");
#endif
      }
    }
    const auto &m = c.native.regionalMaintenance->state;
    for (unsigned i = 0; i < m.queue.size(); ++i) {
      const auto &e = m.queue[i];
#ifdef MNM_PAIRED_REFERENCE
      record(e.node, get(regionManager, 0x1b1 + 12 * i), "queue.node");
      record(e.mode, get(regionManager, 0x1b5 + 12 * i), "queue.mode");
      record(e.priority, get(regionManager, 0x1b9 + 12 * i), "queue.priority");
#else
      record(e.node, e.node, "queue.node");
      record(e.mode, e.mode, "queue.mode");
      record(e.priority, e.priority, "queue.priority");
#endif
    }
    if (m.active != -1) {
      const auto prefix = "work a=" + std::to_string(m.active) +
                          " b=" + std::to_string(m.other) +
                          " s=" + std::to_string(m.stage) +
                          " h=" + std::to_string(m.height) + " ";
#ifdef MNM_PAIRED_REFERENCE
      record(m.working.cost, get(regionManager, 0x2c), prefix + "cost");
      record(m.working.destination, get(regionManager, 0x30),
             prefix + "destination");
      for (unsigned k = 0; k < 20; ++k)
        record(std::uint8_t(m.working.clearance[k]),
               get(regionManager, 0x34 + k, 1),
               prefix + "clearance" + std::to_string(k));
      if (m.probe.initialized) {
        record(m.probe.open.size(), get(regionManager, 0xbb),
               prefix + "open size");
        record(m.probe.best,
               (get(regionManager, 0xa3) - address(o.terrain.data())) / 12 + 1,
               prefix + "best");
        record(m.probe.heuristic, get(regionManager, 0x9f),
               prefix + "heuristic");
      }
#else
      record(m.working.cost, m.working.cost, prefix + "cost");
      record(m.working.destination, m.working.destination,
             prefix + "destination");
      for (auto value : m.working.clearance)
        record(std::uint8_t(value), std::uint8_t(value), prefix + "clearance");
      if (m.probe.initialized) {
        record(m.probe.open.size(), m.probe.open.size(), prefix + "open size");
        record(m.probe.best, m.probe.best, prefix + "best");
        record(m.probe.heuristic, m.probe.heuristic, prefix + "heuristic");
      }
#endif
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
    for (unsigned i = 0; i < 9; ++i) {
#ifdef MNM_PAIRED_REFERENCE
      const unsigned offsets[]{0x69cd, 0x69d1, 0x69d5, 0x69d9, 0x69dd,
                               0x69e1, 0x69e5, 0x6a05, 0x6a09};
      record(values[i], get(regionManager, offsets[i]),
             "maintenance.field" + std::to_string(i));
#else
      record(values[i], values[i], "maintenance.field");
#endif
    }
  };
  compare();
  {
    cs::NativeCreatureGeometry geometry(c, p.native);
    auto services = cs::nativeRegionalGraphServices(c, p.native, geometry);
    for (unsigned id : {0u, unsigned(c.native.regions.size() / 2),
                        unsigned(c.native.regions.size() - 1)}) {
      m::reconnectNavigationNode(c.native.regions, id, shapes, services);
#ifdef MNM_PAIRED_REFERENCE
      using Reconnect = void(__attribute__((thiscall)) *)(void *, int);
      reinterpret_cast<Reconnect>(0x53c070)(regionManager, id);
#endif
      compare();
    }
    for (unsigned i = 0; i < 512; ++i) {
      const m::Cell q{int(i * 7 % 32), int(i * 11 % 32), int(i % 23)};
      auto a = m::navigationChangeSignature(q, services);
#ifdef MNM_PAIRED_REFERENCE
      int b[3]{};
      using Signature =
          void(__attribute__((thiscall)) *)(void *, int, int, int, int *);
      reinterpret_cast<Signature>(0x4f49e0)(reinterpret_cast<void *>(0x6c5490),
                                            q.x, q.y, q.z, b);
      record(a.ground, b[0], "signature.ground");
      record(a.free, b[1], "signature.free");
      record(a.height, b[2], "signature.height");
#else
      record(a.ground, a.ground, "signature.ground");
      record(a.free, a.free, "signature.free");
      record(a.height, a.height, "signature.height");
#endif
    }
  }
  // Exhaust all signatures, including height changes and the no-update branch.
  for (int i = 0; i < 64; ++i) {
    m::NavigationChangeSignature a{i & 1, (i >> 1) & 1, (i >> 2) & 1},
        b{(i >> 3) & 1, (i >> 4) & 1, (i >> 5) & 1};
    const auto mode = m::navigationChangeMode(a, b);
#ifdef MNM_PAIRED_REFERENCE
    using Mode = int(__attribute__((stdcall)) *)(int, int, int, int, int, int);
    record(mode,
           reinterpret_cast<Mode>(0x540430)(a.ground, a.free, a.height,
                                            b.ground, b.free, b.height),
           "change mode " + std::to_string(i));
#else
    record(mode, mode, "change mode " + std::to_string(i));
#endif
  }
  // Pause an independent regional search and retain an actor path across all
  // geometry changes. Maintenance must not reset either continuation.
  m::RegionalQuery query;
  query.creature = false;
  query.movement = {2, 2, 0, 0, 1, 17, 0};
  auto search = [&](bool reset, int budget) {
    cs::NativeCreatureGeometry geometry(c, p.native);
    auto services =
        cs::nativeRegionalServices(c, p.native, geometry, c.world.creatures[0]);
    std::array<int, 50> path;
    path.fill(-1);
    [[maybe_unused]] const auto originalBudget = budget;
    const auto result = m::searchNavigationRegions(
        c.native.regions, 0, int(c.native.regions.size() - 1), query,
        c.world.dimensions, path, budget, reset, c.native.regionalSearch,
        services);
#ifdef MNM_PAIRED_REFERENCE
    using Search = unsigned(__attribute__((thiscall)) *)(
        void *, int, int, WireQuery, int *, unsigned, unsigned);
    std::array<int, 50> expected;
    expected.fill(-1);
    record(result,
           reinterpret_cast<Search>(0x53e5e0)(regionManager, 0,
                                              int(c.native.regions.size() - 1),
                                              wire(o, query), expected.data(),
                                              unsigned(originalBudget), reset),
           "interleaved search result");
    for (unsigned i = 0; i < 50; ++i)
      record(path[i], expected[i], "interleaved search path");
    record(c.native.regionalSearch.dirty, get(regionManager, 0x154, 1),
           "interleaved search dirty");
#else
    record(result, result, "interleaved search result");
    for (int id : path)
      record(id, id, "interleaved search path");
    record(c.native.regionalSearch.dirty, c.native.regionalSearch.dirty,
           "interleaved search dirty");
#endif
  };
  search(true, 1);
  need(!c.native.regionalSearch.dirty, "regional search did not pause");
  auto retained = c.actions.segments[0];
  retained.regionPending = true;
  retained.regionCursor = 1;
  retained.regionPath.fill(0);
  retained.regionPath[1] = int(c.native.regions.size() - 1);
  c.actions.segments[0] = retained;
#ifdef MNM_PAIRED_REFERENCE
  put(o.actors.data(), 0xd03, 1);
  put(o.actors.data(), 0xdcf, 1);
  for (unsigned i = 0; i < 50; ++i)
    put(o.actors.data(), 0xd07 + 4 * i, unsigned(retained.regionPath[i]));
  const auto retainedActors = o.actors;
#endif
  // Repeated requests exercise merge, mixed modes, priority decreases, wrap and
  // duplicate requests for a currently active node.
  for (unsigned n = 0; n < 32; ++n) {
    const m::Cell q{int(n * 7 % 32), int(n * 13 % 32), int(n % 10)};
    const int mode = n % 3;
    m::enqueueNavigationRepair(c.native.regions, q, mode, c.world.dimensions,
                               c.native.regionalMaintenance->state);
#ifdef MNM_PAIRED_REFERENCE
    using Enqueue =
        void(__attribute__((thiscall)) *)(void *, int, int, int, int);
    reinterpret_cast<Enqueue>(0x540140)(regionManager, q.x, q.y, q.z, mode);
#endif
  }
  compare();
  // Stop/resume at every maintenance slice, changing terrain while work
  // remains.
  for (unsigned round = 0; round < 8; ++round) {
    std::vector<cs::NativeTerrainEdit> batch;
    const int x = round % 2 ? 16 : 12;
    for (int z = 1; z < 11; ++z)
      for (int y = 4; y <= 10; ++y) {
        const auto at = unsigned((z * 32 + y) * 32 + x);
        batch.push_back({{x, y, z},
                         c.native.cells[at].kind,
                         0x4000,
                         std::uint16_t(round % 2 ? 0 : 0x4000)});
      }
    cs::editNativeTerrain(c, p.native, batch);
    edits += batch.size();
#ifdef MNM_PAIRED_REFERENCE
    for (const auto &edit : batch) {
      auto q = edit.position;
      int before[3]{}, after[3]{};
      using Signature =
          void(__attribute__((thiscall)) *)(void *, int, int, int, int *);
      reinterpret_cast<Signature>(0x4f49e0)(reinterpret_cast<void *>(0x6c5490),
                                            q.x, q.y, q.z, before);
      const auto at = unsigned((q.z * 32 + q.y) * 32 + q.x);
      auto *raw = o.terrain.data() + 12 * at;
      put(raw, 0, edit.kind, 2);
      put(raw, 10,
          std::uint16_t((get(raw, 10, 2) & ~edit.flagsMask) | edit.flagsValue),
          2);
      reinterpret_cast<Signature>(0x4f49e0)(reinterpret_cast<void *>(0x6c5490),
                                            q.x, q.y, q.z, after);
      using Mode =
          int(__attribute__((stdcall)) *)(int, int, int, int, int, int);
      const auto mode = reinterpret_cast<Mode>(0x540430)(
          before[0], before[1], before[2], after[0], after[1], after[2]);
      if (mode != -1) {
        using Enqueue =
            void(__attribute__((thiscall)) *)(void *, int, int, int, int);
        reinterpret_cast<Enqueue>(0x540140)(regionManager, q.x, q.y, q.z, mode);
      }
    }
#endif
    compare();
    for (unsigned j = 0; j < 16; ++j) {
      const bool blocked = j == 3;
      cs::maintainNativeRegionalGraph(c, p.native, blocked);
#ifdef MNM_PAIRED_REFERENCE
      global(0x6e1f48, blocked);
      call(0x5404a0, regionManager);
      global(0x6e1f48, 0);
#endif
      ++steps;
      compare();
      yields += c.native.regionalMaintenance->state.active != -1;
      auto bytes = cs::encode(c);
      c = cs::decode(bytes);
      need(cs::encode(c) == bytes, "maintenance checkpoint differs");
    }
  }
  while (m::navigationMaintenancePending(c.native.regionalMaintenance->state) &&
         steps < 4096) {
    cs::maintainNativeRegionalGraph(c, p.native);
#ifdef MNM_PAIRED_REFERENCE
    call(0x5404a0, regionManager);
#endif
    ++steps;
    compare();
  }
  need(!m::navigationMaintenancePending(c.native.regionalMaintenance->state),
       "maintenance did not drain");
  need(yields > 0, "maintenance fixture did not yield");
  need(c.actions.segments[0].regionPending &&
           c.actions.segments[0].regionCursor == retained.regionCursor &&
           c.actions.segments[0].regionPath == retained.regionPath,
       "maintenance discarded actor path");
#ifdef MNM_PAIRED_REFERENCE
  need(o.actors == retainedActors, "original maintenance changed actor bytes");
#endif
  search(false, 53);
  const auto before = cs::encode(c);
  bool refused = false;
  try {
    cs::editNativeTerrain(
        c, p.native, {{{2, 2, 2}, 0, 0x4000, 0x4000}, {{-1, 0, 0}, 0, 0, 0}});
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  need(refused && cs::encode(c) == before, "invalid batch did not roll back");
  auto old = c;
  old.native.regionalMaintenance.reset();
  need(!cs::decode(cs::encode(old, 7)).native.regionalMaintenance,
       "v7 maintenance migration");
  refused = false;
  try {
    cs::encode(c, 7);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  need(refused, "v7 silently lost graph maintenance");
  for (unsigned i = 0; i < 9; ++i) {
    auto bad = c;
    auto &m = bad.native.regionalMaintenance->state;
    if (i == 0)
      m.read = 1345;
    else if (i == 1)
      m.active = -2;
    else if (i == 2)
      m.stage = 9;
    else if (i == 3)
      m.height = 7;
    else if (i == 4)
      m.remaining = 126;
    else if (i == 5)
      m.queue[0] = {0, -1, 0};
    else if (i == 6) {
      m.probe = {};
      m.probe.initialized = true;
    } else if (i == 7)
      bad.native.regionalMaintenance->shapes[0].height = 6;
    else
      bad.native.route.useRegions = false;
    refused = false;
    try {
      cs::encode(bad);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    need(refused, "malformed maintenance continuation accepted");
  }
  refused = false;
  try {
    cs::editNativeTerrain(c, p.native, {{{2, 2, 2}, 0, 3, 1}});
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  need(refused && cs::encode(c) == before,
       "terrain edit changed actor reservation bits");
  // A separate declared1500-node graph reaches ring overflow and wraps both
  // ends. It tests the recovered queue only, outside native CMD1's byte bound.
  std::vector<m::NavigationRegion> dense(1500);
  for (auto &v : dense)
    v.position = {2, 2, 3};
  m::NavigationMaintenance ring;
  ring.read = ring.write = 1340;
#ifdef MNM_PAIRED_REFERENCE
  call(0x53a840, regionManager);
  reinterpret_cast<Constructor>(0x53a660)(regionManager, 2048);
  put(regionManager, 8, unsigned(dense.size()));
  put(regionManager, 0x69cd, 1340);
  put(regionManager, 0x69d1, 1340);
  for (unsigned i = 0; i < dense.size(); ++i) {
    auto *raw = reinterpret_cast<void *>(get(regionManager) + 660 * i);
    put(raw, 0, 1);
    put(raw, 4, 2);
    put(raw, 8, 2);
    put(raw, 12, 3);
  }
#endif
  for (unsigned n = 0; n < 4; ++n) {
    const m::Cell q{n == 2 ? 5 : 6, 2, 3};
    const int mode = n == 0 ? 1 : n == 1 ? 2 : 0;
    if (n == 3) {
      ring.queue[ring.read].node = -1;
#ifdef MNM_PAIRED_REFERENCE
      put(regionManager, 0x1b1 + 12 * ring.read, unsigned(-1));
#endif
      ring.read = (ring.read + 1) % 1345;
#ifdef MNM_PAIRED_REFERENCE
      put(regionManager, 0x69cd, ring.read);
#endif
    }
    m::enqueueNavigationRepair(dense, q, mode, c.world.dimensions, ring);
#ifdef MNM_PAIRED_REFERENCE
    using Enqueue =
        void(__attribute__((thiscall)) *)(void *, int, int, int, int);
    reinterpret_cast<Enqueue>(0x540140)(regionManager, q.x, q.y, q.z, mode);
#endif
    unsigned count = 0;
    for (unsigned i = 0; i < ring.queue.size(); ++i) {
      const auto &e = ring.queue[i];
      count += e.node != -1;
#ifdef MNM_PAIRED_REFERENCE
      record(e.node, get(regionManager, 0x1b1 + 12 * i), "ring.node");
      record(e.mode, get(regionManager, 0x1b5 + 12 * i), "ring.mode");
      record(e.priority, get(regionManager, 0x1b9 + 12 * i), "ring.priority");
#else
      record(e.node, e.node, "ring.node");
      record(e.mode, e.mode, "ring.mode");
      record(e.priority, e.priority, "ring.priority");
#endif
    }
    need(count == 1345, "ring capacity was not exercised");
#ifdef MNM_PAIRED_REFERENCE
    record(ring.read, get(regionManager, 0x69cd), "ring.read");
    record(ring.write, get(regionManager, 0x69d1), "ring.write");
#else
    record(ring.read, ring.read, "ring.read");
    record(ring.write, ring.write, "ring.write");
#endif
  }
#ifdef MNM_PAIRED_REFERENCE
  checkText();
  call(0x53a840, regionManager);
  destroySearch();
#endif
  need(!before.empty(), "empty maintenance checkpoint");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"digest\":" << digest << ",\"steps\":" << steps
            << ",\"yields\":" << yields << ",\"edits\":" << edits << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
