#include "entity_map.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool ok, int line) {
  if (!ok)
    throw std::runtime_error("Scene map navigation line " +
                             std::to_string(line));
}
#define require(...) check((__VA_ARGS__), __LINE__)
template <class F> static void refused(F f) {
  bool caught = false;
  try {
    f();
  } catch (const std::exception &) {
    caught = true;
  }
  require(caught);
}
int main() try {
  assets::MapAsset map;
  map.width = map.height = 16;
  map.layers = 3;
  map.cells.resize(768);
  for (auto &c : map.cells)
    c.references = {0xffff, 0xffff, 0xffff};
  for (unsigned i = 0; i < 256; ++i) {
    map.cells[i].definition = 1;
    map.cells[i].flags10 = 1;
  }
  assets::TerrainCatalog catalog;
  catalog.records.resize(2);
  catalog.records[1][0x94] = 16;
  catalog.records[1][0xb0] = 8;
  scene::MapNavigationConfig config;
  config.capacity = 128;
  for (auto &type : config.shapes)
    type = {1, 1, 0};
  scene::MapNavigationRepair repair(map, catalog, config);
  require(!repair.regions().empty());
  const reconstruction::combat::Cell at{8, 8, 1};
  auto before = repair.signature(at);
  require(before.ground);
  const auto graphHash = [](const auto &graph) {
    std::uint64_t hash = 1469598103934665603ull;
    for (const auto &node : graph)
      for (const auto &edge : node.connections) {
        hash = (hash ^ unsigned(edge.cost)) * 1099511628211ull;
        hash = (hash ^ unsigned(edge.destination)) * 1099511628211ull;
        for (auto n : edge.clearance)
          hash = (hash ^ std::uint8_t(n)) * 1099511628211ull;
      }
    return hash;
  };
  const auto initialGraph = graphHash(repair.regions());
  auto changed = map;
  for (unsigned y = 0; y < 16; ++y) {
    changed.cells[(1 * 16 + y) * 16 + 8].definition = 1;
    changed.cells[(1 * 16 + y) * 16 + 8].flags10 = 1;
  }
  repair.update(changed);
  auto after = repair.signature(at);
  require(!after.ground && repair.pending() && repair.revision() == 2);
  const auto cursor = repair.maintenance().read;
  repair.step(true);
  require(repair.maintenance().read == cursor && repair.pending());
  unsigned steps = 0;
  while (repair.pending() && steps++ < 8192)
    repair.step();
  require(!repair.pending() && steps > 0);
  require(graphHash(repair.regions()) != initialGraph);
  const auto closedGraph = graphHash(repair.regions());
  repair.update(map);
  require(repair.signature(at).ground && repair.pending());
  steps = 0;
  while (repair.pending() && steps++ < 8192)
    repair.step();
  require(!repair.pending());
  require(graphHash(repair.regions()) != closedGraph);
  const auto revision = repair.revision();
  auto bad = map;
  bad.cells[0].definition = 99;
  refused([&] { repair.update(bad); });
  require(repair.revision() == revision && repair.signature(at).ground);
  bad = map;
  bad.width = 8;
  refused([&] { repair.update(bad); });
  auto copy = repair;
  changed.cells[(1 * 16 + 8) * 16 + 8].definition = 1;
  copy.update(changed);
  require(!copy.signature(at).ground && repair.signature(at).ground);
  // A real linked map reset opens a supported cell. The owning map publishes
  // geometry and the recovered repair queue in the same transaction.
  auto source = map;
  source.cells[(1 * 16 + 8) * 16 + 8].definition = 0;
  source.cells[(1 * 16 + 8) * 16 + 8].flags10 = 8;
  assets::Animation animation;
  animation.starts = {0, 2};
  animation.records = {{0, 1, {}}, {6, 0, {}}};
  scene::EntityMap entityMap(source, catalog, animation);
  entityMap.enableNavigation(config);
  require(entityMap.navigation() &&
          !entityMap.navigation()->signature(at).ground);
  entityMap.resetCell((1 * 16 + 8) * 16 + 8);
  require(entityMap.navigation()->signature(at).ground &&
          entityMap.navigation()->pending());
  steps = 0;
  while (entityMap.navigation()->pending() && steps++ < 8192)
    entityMap.maintainNavigation();
  require(!entityMap.navigation()->pending());
  refused([&] {
    scene::EntityMap unconfigured(map, catalog, {});
    unconfigured.maintainNavigation();
  });
  std::cout
      << "{\"success\":true,\"actual_terrain_queries\":true,\"real_repair_"
         "queue_and_graph\":true,\"blocked_and_resumed_slices\":true,\"owned_"
         "copy_and_atomic_refusal\":true,\"map_reset_integration\":true}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
