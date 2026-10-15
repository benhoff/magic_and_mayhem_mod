#define MNM_GENERATED_GRAPH
#ifdef MNM_GENERATED_GRAPH
#include "regional-maintenance-encounter-state.hpp"
#else
#include "regional-encounter-state.hpp"
#endif
#include "../game/persistence/snapshot.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "regional-navigation-original.hpp"
#else
#include <iostream>
#endif
#include <fstream>
int main(int argc, char **argv) try {
  using namespace mnm;
  namespace cs = command_session;
#ifdef MNM_GENERATED_GRAPH
  namespace reg = encounter::regional_maintenance;
#else
  namespace reg = encounter::regional;
#endif
#ifdef MNM_PAIRED_REFERENCE
  if (argc != 7)
    throw std::runtime_error("PE INSTALL SCENARIO SEED TICKS NEW_OUTPUT");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
#ifdef MNM_GENERATED_GRAPH
  redirect(0x4dc5e0, reinterpret_cast<std::uintptr_t>(&noArg));
#endif
  sealText(pe);
  initializeSearch();
  using Constructor = void *(__attribute__((thiscall)) *)(void *, unsigned);
  reinterpret_cast<Constructor>(0x53a660)(regionManager, 512);
  const auto scenario = unsigned(std::stoul(argv[3])),
             seed = unsigned(std::stoul(argv[4])),
             ticks = unsigned(std::stoul(argv[5]));
  auto p = reg::profile(argv[2], scenario);
  auto session = encounter::create(p, seed);
  auto initial = cs::decode(session.world().state().systems);
  Original o(p);
  setup(o, initial, seed);
#ifndef MNM_GENERATED_GRAPH
  publishRegions(initial.native.regions);
#endif

#ifdef MNM_GENERATED_GRAPH
  const auto shapes = encounter::navigationShapes(p);
  for (unsigned i = 0; i < 28; ++i) {
    auto *cfg = reinterpret_cast<void *>(0x6a5f80 + i * 0x5c9);
    put(cfg, 8, shapes[i].width);
    put(cfg, 12, shapes[i].height);
    put(cfg, 0x44, shapes[i].movementKind);
  }
  // Build independently from the original terrain, then compare before ticking.
  using Build = void(__attribute__((thiscall)) *)(void *, int);
  reinterpret_cast<Build>(0x53ab20)(regionManager, 1);
  call(0x53d710, regionManager);
  using Connect = void(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
  reinterpret_cast<Connect>(0x53bb40)(regionManager, 1, 0);
  if (get(regionManager, 8) != initial.native.regions.size())
    throw std::runtime_error("generated original node count");
  for (unsigned i = 0; i < initial.native.regions.size(); ++i) {
    auto *raw = reinterpret_cast<unsigned char *>(get(regionManager) + 660 * i);
    const auto &v = initial.native.regions[i];
    if (get(raw, 4) != unsigned(v.position.x) ||
        get(raw, 8) != unsigned(v.position.y) ||
        get(raw, 12) != unsigned(v.position.z) || get(raw, 524) != v.parameter)
      throw std::runtime_error("generated original node");
    for (unsigned j = 0; j < 18; ++j) {
      const auto &edge = v.connections[j];
      auto *record = raw + 16 + 28 * j;
      if (get(record) != unsigned(edge.cost) ||
          get(record, 4) != unsigned(edge.destination) ||
          std::memcmp(record + 8, edge.clearance.data(), 20))
        throw std::runtime_error("generated original connection");
    }
  }
#endif
  global(0x6e1f3c, 1);
  for (unsigned i = 0; i < initial.world.creatures.size(); ++i) {
    auto *raw = o.actors.data() + i * 0xe4b;
    const auto &v = initial.actions.segments[i];
    put(raw, 0xd03, v.regionPending);
    put(raw, 0xdcf, v.regionCursor);
    for (unsigned j = 0; j < 50; ++j)
      put(raw, 0xd07 + 4 * j, v.regionPath[j]);
    if (!initial.native.routeMetadata.empty()) {
      const auto q = initial.native.routeMetadata[i].regionDestination;
      put(raw, 0xdd3, q.x);
      put(raw, 0xdd7, q.y);
      put(raw, 0xddb, q.z);
    }
  }
  std::filesystem::path out = argv[6];
  if (!std::filesystem::create_directory(out))
    throw std::runtime_error("existing regional output");
  game::writeSnapshot(out / "initial.snapshot", session.world().state());
#else
  if (argc != 7)
    throw std::runtime_error(
        "SNAPSHOT SCENARIO TICKS NEW_SNAPSHOT NEW_TRACE CHECKPOINT_INTERVAL");
  auto saved = game::readSnapshot(argv[1]);
  auto scenario = unsigned(std::stoul(argv[2]));
  auto p = reg::profile(saved.campaign, scenario);
  auto ticks = unsigned(std::stoul(argv[3]));
  game::World world(0);
  world.restore(std::move(saved));
  game::CreatureRuleSession session(std::move(world), encounter::runtime(p));
  std::filesystem::path final = argv[4], tracePath = argv[5];
  if (std::filesystem::exists(tracePath))
    throw std::runtime_error("existing regional trace");
  const auto interval = unsigned(std::stoul(argv[6]));
  auto initial = cs::decode(session.world().state().systems);
#endif
  if (scenario >
#ifdef MNM_GENERATED_GRAPH
          3
#else
          3
#endif
      || ticks > 8192)
    throw std::runtime_error("regional fixture bound");
#ifdef MNM_PAIRED_REFERENCE
  const auto final = out / "final.snapshot", tracePath = out / "trace.txt";
#endif
  std::ofstream trace(tracePath);
  std::uint64_t digest = 1469598103934665603ull;
  unsigned regionalTicks = 0, regionChanges = 0, arrivals = 0, yielded = 0,
           restores = 0;
  unsigned lastCursor = initial.actions.segments[0].regionCursor;
  unsigned displacementOrders = 0;
  std::vector<unsigned> priorBehavior;
  for (const auto &a : initial.world.creatures)
    priorBehavior.push_back(a.ai.behavior);
  unsigned terrainEvents = 0, activeRouteEdits = 0, searchEdits = 0,
           repairTicks = 0, graphChanges = 0, reorders = 0;
  const auto graphHash = [](const auto &regions) {
    std::uint64_t h = 1469598103934665603ull;
    for (const auto &r : regions)
      for (const auto &e : r.connections) {
        h = (h ^ unsigned(e.cost)) * 1099511628211ull;
        h = (h ^ unsigned(e.destination)) * 1099511628211ull;
        for (auto b : e.clearance)
          h = (h ^ std::uint8_t(b)) * 1099511628211ull;
      }
    return h;
  };
  auto lastGraph = graphHash(initial.native.regions);
  for (unsigned i = 0; i < ticks; ++i) {
    auto before = cs::decode(session.world().state().systems);
    if (before.native.tick == 1536) {
      const auto q = p.initial.world.creatures[0].destination;
      session.command({{0, before.generations[0]},
                       game::CreatureOrderKind::move,
                       {q.x, q.y, q.z},
                       {},
                       false,
                       false});
#ifdef MNM_PAIRED_REFERENCE
      using Move = void(__attribute__((thiscall)) *)(void *, int, int, int);
      reinterpret_cast<Move>(0x50fae0)(o.actors.data(), q.x, q.y, q.z);
#endif
      ++reorders;
    }
    const auto edits = reg::edits(p, scenario, before.native.tick);
    if (!edits.empty()) {
      ++terrainEvents;
      for (const auto &r : before.actions.segments)
        activeRouteEdits += r.regionPending;
      searchEdits += before.native.regionalSearch.initialized &&
                     !before.native.regionalSearch.dirty;
      encounter::editTerrain(session, p, edits);
#ifdef MNM_PAIRED_REFERENCE
      for (const auto &edit : edits) {
        auto q = edit.position;
        int old[3]{}, now[3]{};
        using Signature =
            void(__attribute__((thiscall)) *)(void *, int, int, int, int *);
        reinterpret_cast<Signature>(0x4f49e0)(
            reinterpret_cast<void *>(0x6c5490), q.x, q.y, q.z, old);
        auto *raw = o.terrain.data() + 12 * ((q.z * 32 + q.y) * 32 + q.x);
        put(raw, 0, edit.kind, 2);
        put(raw, 10, (get(raw, 10, 2) & ~edit.flagsMask) | edit.flagsValue, 2);
        reinterpret_cast<Signature>(0x4f49e0)(
            reinterpret_cast<void *>(0x6c5490), q.x, q.y, q.z, now);
        using Mode =
            int(__attribute__((stdcall)) *)(int, int, int, int, int, int);
        const auto mode = reinterpret_cast<Mode>(0x540430)(
            old[0], old[1], old[2], now[0], now[1], now[2]);
        if (mode != -1) {
          using Enqueue =
              void(__attribute__((thiscall)) *)(void *, int, int, int, int);
          reinterpret_cast<Enqueue>(0x540140)(regionManager, q.x, q.y, q.z,
                                              mode);
        }
      }
#endif
    }
#ifdef MNM_PAIRED_REFERENCE
    step(o);
    call(0x5404a0, regionManager);
#endif
    session.step();
    auto c = cs::decode(session.world().state().systems);
    auto projected = reg::projection(c, p);
#ifdef MNM_PAIRED_REFERENCE
    auto original = projectOriginal(o, false);
    auto add = [&](std::string name, unsigned v) {
      original.push_back({std::move(name), v});
    };
    for (unsigned slot = 0; slot < c.world.creatures.size(); ++slot) {
      auto *raw = o.actors.data() + slot * 0xe4b;
      const auto prefix = "regional.actor[" + std::to_string(slot) + "].";
      add(prefix + "pending", get(raw, 0xd03));
      add(prefix + "cursor", get(raw, 0xdcf));
      for (unsigned j = 0; j < 50; ++j)
        add(prefix + "path[" + std::to_string(j) + "]",
            get(raw, 0xd07 + 4 * j));
      add(prefix + "destination.x", get(raw, 0xdd3));
      add(prefix + "destination.y", get(raw, 0xdd7));
      add(prefix + "destination.z", get(raw, 0xddb));
    }
    for (unsigned j = 0; j < c.native.regions.size(); ++j)
      add("regional.traffic[" + std::to_string(j) + "]",
          get(reinterpret_cast<void *>(get(regionManager) + 660 * j), 528));
    std::uint64_t grid = 1469598103934665603ull;
    for (unsigned j = 0; j < c.native.cells.size(); ++j) {
      const auto *raw = o.terrain.data() + 12 * j;
      const auto &v = c.native.cells[j];
      unsigned occupant = get(raw, 4, 2);
      if (occupant != 0xffff)
        ++occupant;
      const unsigned actual[] = {get(raw, 0, 2), occupant, get(raw, 10, 2),
                                 get(raw, 8, 2)},
                     expected[] = {v.kind, v.occupant, v.flags, v.links};
      for (unsigned k = 0; k < 4; ++k) {
        if (actual[k] != expected[k])
          throw std::runtime_error(
              "regional cell differs tick=" + std::to_string(c.native.tick) +
              " cell=" + std::to_string(j));
        grid = (grid ^ actual[k]) * 1099511628211ull;
      }
    }
    add("regional.grid.low", unsigned(grid));
    add("regional.grid.high", unsigned(grid >> 32));
    for (unsigned j = 0; j < c.native.regions.size(); ++j)
      for (unsigned k = 0; k < 18; ++k) {
        auto *raw = reinterpret_cast<void *>(get(regionManager) + 660 * j + 16 +
                                             28 * k);
        auto prefix = "repair.edge[" + std::to_string(j) + "][" +
                      std::to_string(k) + "].";
        add(prefix + "cost", get(raw));
        add(prefix + "to", get(raw, 4));
        for (unsigned z = 0; z < 20; ++z)
          add(prefix + "clearance[" + std::to_string(z) + "]",
              get(raw, 8 + z, 1));
      }
    for (unsigned j = 0; j < 1345; ++j) {
      auto prefix = "repair.queue[" + std::to_string(j) + "].";
      add(prefix + "node", get(regionManager, 0x1b1 + 12 * j));
      add(prefix + "mode", get(regionManager, 0x1b5 + 12 * j));
      add(prefix + "priority", get(regionManager, 0x1b9 + 12 * j));
    }
    const unsigned offsets[]{0x69cd, 0x69d1, 0x69d5, 0x69d9, 0x69dd,
                             0x69e1, 0x69e5, 0x6a05, 0x6a09};
    for (unsigned j = 0; j < 9; ++j)
      add("repair.field[" + std::to_string(j) + "]",
          get(regionManager, offsets[j]));
    const auto difference = encounter::paired::difference(projected, original);
    if (!difference.empty())
      throw std::runtime_error(
          "regional tick=" + std::to_string(c.native.tick) + "\n" + difference);
#else
    if (interval && (i + 1) % interval == 0) {
      game::World restored(0);
      restored.restore(
          game::decodeSnapshot(game::encodeSnapshot(session.world().state())));
      session =
          game::CreatureRuleSession(std::move(restored), encounter::runtime(p));
      ++restores;
    }
#endif
    repairTicks += cs::model::navigationMaintenancePending(
        c.native.regionalMaintenance->state);
    const auto graph = graphHash(c.native.regions);
    graphChanges += graph != lastGraph;
    lastGraph = graph;
    for (unsigned j = 0; j < c.world.creatures.size(); ++j) {
      displacementOrders +=
          priorBehavior[j] != 13 && c.world.creatures[j].ai.behavior == 13;
      priorBehavior[j] = c.world.creatures[j].ai.behavior;
    }
    const auto &r = c.actions.segments[0];
    regionalTicks += r.regionPending;
    regionChanges += r.regionCursor != lastCursor;
    lastCursor = r.regionCursor;
    yielded += c.scheduler.continuation;
    arrivals += c.world.creatures[0].ai.position ==
                p.initial.world.creatures[0].destination;
    digest = encounter::paired::digest(digest, projected);
    trace << c.native.tick << ' ' << digest << '\n';
  }
  if (!trace)
    throw std::runtime_error("regional trace write");
  game::writeSnapshot(final, session.world().state());
#ifdef MNM_PAIRED_REFERENCE
  checkText();
  call(0x53a840, regionManager);
  destroySearch();
#endif
  std::cout << "{\"success\":true,\"ticks\":" << ticks
            << ",\"digest\":" << digest
            << ",\"regional_ticks\":" << regionalTicks
            << ",\"region_changes\":" << regionChanges
            << ",\"arrivals\":" << arrivals << ",\"budget_yields\":" << yielded
            << ",\"displacement_orders\":" << displacementOrders
            << ",\"reorders\":" << reorders
            << ",\"terrain_events\":" << terrainEvents
            << ",\"active_route_edits\":" << activeRouteEdits
            << ",\"search_edits\":" << searchEdits
            << ",\"repair_ticks\":" << repairTicks
            << ",\"graph_changes\":" << graphChanges
            << ",\"restores\":" << restores << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
