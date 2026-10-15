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
  const auto scenario = unsigned(std::stoul(argv[3]));
  const unsigned capacity = scenario == 8    ? 1u
                            : scenario == 9  ? 17u
                            : scenario == 10 ? 52u
                                             : 512u;
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  redirect(0x4dc5e0, reinterpret_cast<std::uintptr_t>(&noArg));
  sealText(pe);
  initializeSearch();
  using Constructor = void *(__attribute__((thiscall)) *)(void *, unsigned);
  reinterpret_cast<Constructor>(0x53a660)(regionManager, capacity);
  const auto install = argv[2];
#else
  need(argc == 3, "INSTALL SCENARIO");
  const auto install = argv[1];
  const auto scenario = unsigned(std::stoul(argv[2]));
  const unsigned capacity = scenario == 8    ? 1u
                            : scenario == 9  ? 17u
                            : scenario == 10 ? 52u
                                             : 512u;
#endif
  auto p = encounter::decodeInputs(
      encounter::loadInputs(install, {0, 0, 32, 32}, 0));
  auto c = p.initial;
  for (auto &v : c.native.cells) {
    v.flags &= std::uint16_t(~3u);
    v.occupant = 0xffff;
  }
  need(scenario < 11, "graph fixture scenario");
  if (scenario < 8 && (scenario % 4 == 1 || scenario % 4 == 2))
    for (int z = 0; z < c.world.dimensions.z; ++z)
      for (int y = 1; y < 31; ++y)
        if (scenario % 4 == 2 || y < 4 || y > 10)
          c.native.cells[(z * 32 + y) * 32 + 16].flags |= 0x4000;
  if (scenario < 8 && scenario % 4 == 3)
    for (int z = 7; z <= 9; ++z)
      for (int y = 8; y < 24; ++y)
        for (int x = 7; x < 25; ++x)
          c.native.cells[(z * 32 + y) * 32 + x].flags |= 0x4000;
  if (scenario >= 4 && scenario < 8)
    p.native.boundary = 6;
  const auto shapes = encounter::navigationShapes(p);
  cs::NativeCreatureGeometry geometry(c, p.native);
  auto services = cs::nativeRegionalGraphServices(c, p.native, geometry);
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
#endif
  unsigned checks = 0;
  std::uint64_t digest = 1469598103934665603ull;
  const auto record = [&](unsigned actual, unsigned expected,
                          std::string name) {
    if (actual != expected)
      throw std::runtime_error(name + " native=" + std::to_string(actual) +
                               " original=" + std::to_string(expected));
    digest = (digest ^ actual) * 1099511628211ull;
    ++checks;
  };
  for (int width : {0, 1, 2})
    for (int kind : {0, 1, 2, 3})
      for (int mode : {0, 1, 2}) {
        auto list =
            m::navigationHeights(shapes, mode == 0, mode == 1, kind, width);
#ifdef MNM_PAIRED_REFERENCE
        using Heights =
            void(__attribute__((thiscall)) *)(void *, int, int, int, int);
        reinterpret_cast<Heights>(0x53cd30)(regionManager, mode == 0, mode == 1,
                                            kind, width);
        for (unsigned j = 0; j < 6; ++j)
          record(list[j], get(regionManager, 0x14 + 4 * j), "heights");
#else
        for (auto v : list)
          record(v, v, "heights");
#endif
      }
  for (unsigned i = 0; i < 512; ++i) {
    m::Cell point{int(i * 7 % 32), int(i / 7 % 32), int(i % 15)};
    auto adjusted = m::adjustNavigationNode(point, services);
#ifdef MNM_PAIRED_REFERENCE
    auto expected = point;
    using Adjust = void(__attribute__((stdcall)) *)(int *, int *, int *);
    reinterpret_cast<Adjust>(0x53b600)(&expected.x, &expected.y, &expected.z);
    record(adjusted.x, expected.x, "adjust.x");
    record(adjusted.y, expected.y, "adjust.y");
    record(adjusted.z, expected.z, "adjust.z");
#else
    record(adjusted.x, adjusted.x, "adjust.x");
    record(adjusted.y, adjusted.y, "adjust.y");
    record(adjusted.z, adjusted.z, "adjust.z");
#endif
  }
  for (unsigned i = 0; i < 512; ++i) {
    m::Cell from{int(i * 7 % 32), int(i * 11 % 32), int(1 + i % 9)},
        to{int(i * 13 % 32), int(i / 13 % 32), int(1 + i / 3 % 9)};
    reconstruction::CreatureMovementParameters q{int(1 + i % 5),
                                                 int(1 + i / 5 % 2),
                                                 int(i / 10 % 2),
                                                 int(i / 20 % 4),
                                                 1,
                                                 0,
                                                 0};
    int budget = int(i % 130), cost = 0;
    const auto reached =
        m::navigationDirectRoute(from, to, q, budget, cost, services);
#ifdef MNM_PAIRED_REFERENCE
    auto *raw = static_cast<unsigned char *>(regionManager);
    put(raw, 0x54, from.x);
    put(raw, 0x58, from.y);
    put(raw, 0x5c, from.z);
    put(raw, 0x76, to.x);
    put(raw, 0x7a, to.y);
    put(raw, 0x7e, to.z);
    std::memcpy(raw + 0x60, &q, 22);
    int originalBudget = int(i % 130), originalCost = 0;
    using Direct = unsigned(__attribute__((thiscall)) *)(void *, int *, int *);
    const auto expected = reinterpret_cast<Direct>(0x542220)(
        regionManager, &originalCost, &originalBudget);
    record(reached, expected, "direct.reached " + std::to_string(i));
    record(budget, originalBudget, "direct.budget " + std::to_string(i));
    record(cost, originalCost, "direct.cost " + std::to_string(i));
#else
    record(reached, reached, "direct.reached");
    record(budget, budget, "direct.budget");
    record(cost, cost, "direct.cost");
#endif
  }
  if (capacity >= 32) {
    std::vector<m::NavigationRegion> tied(32);
    for (unsigned i = 0; i < tied.size(); ++i) {
      tied[i].position = {int(i % 4) * 5, int(i % 3) * 4, 3};
      tied[i].parameter = i;
    }
#ifdef MNM_PAIRED_REFERENCE
    call(0x53aa30, regionManager);
    publishRegions(tied);
    for (unsigned i = 0; i < tied.size(); ++i)
      put(reinterpret_cast<void *>(get(regionManager) + 660 * i), 0, 1);
    call(0x53d710, regionManager);
#endif
    m::sortNavigationNodes(tied, capacity, c.world.dimensions);
    for (unsigned i = 0; i < tied.size(); ++i) {
#ifdef MNM_PAIRED_REFERENCE
      record(tied[i].parameter,
             get(reinterpret_cast<void *>(get(regionManager) + 660 * i), 524),
             "sort duplicate " + std::to_string(i));
#else
      record(tied[i].parameter, tied[i].parameter, "sort duplicate");
#endif
    }
  }
  std::cerr << "helpers complete\n";
  auto nodes = m::buildNavigationNodes(capacity, services);
  m::sortNavigationNodes(nodes, capacity, c.world.dimensions);
  std::cerr << "native nodes=" << nodes.size() << '\n';
#ifdef MNM_PAIRED_REFERENCE
  using Build = void(__attribute__((thiscall)) *)(void *, int);
  reinterpret_cast<Build>(0x53ab20)(regionManager, 1);
  call(0x53d710, regionManager);
  record(nodes.size(), get(regionManager, 8), "node count");
#else
  record(nodes.size(), nodes.size(), "node count");
#endif
  for (unsigned i = 0; i < nodes.size(); ++i) {
    const auto &v = nodes[i];
#ifdef MNM_PAIRED_REFERENCE
    auto *raw = reinterpret_cast<void *>(get(regionManager) + 660 * i);
    record(v.position.x, get(raw, 4), "node.x " + std::to_string(i));
    record(v.position.y, get(raw, 8), "node.y " + std::to_string(i));
    record(v.position.z, get(raw, 12), "node.z " + std::to_string(i));
    record(v.parameter, get(raw, 524), "node.parameter " + std::to_string(i));
#else
    record(v.position.x, v.position.x, "node.x");
    record(v.position.y, v.position.y, "node.y");
    record(v.position.z, v.position.z, "node.z");
    record(v.parameter, v.parameter, "node.parameter");
#endif
  }
  std::cerr << "node comparison complete\n";
  m::buildNavigationConnections(nodes, shapes, true, services);
  std::cerr << "native connections complete\n";
#ifdef MNM_PAIRED_REFERENCE
  using Connect = void(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
  reinterpret_cast<Connect>(0x53bb40)(regionManager, 1, 0);
#endif
  unsigned edges = 0;
  for (unsigned i = 0; i < nodes.size(); ++i)
    for (unsigned j = 0; j < 18; ++j) {
      const auto &v = nodes[i].connections[j];
      edges += v.cost != 0;
#ifdef MNM_PAIRED_REFERENCE
      auto *raw = reinterpret_cast<unsigned char *>(get(regionManager) +
                                                    660 * i + 16 + 28 * j);
      const auto name =
          "edge[" + std::to_string(i) + "][" + std::to_string(j) + "]";
      record(v.cost, get(raw), name + "cost");
      record(v.destination, get(raw, 4), name + "destination");
      for (unsigned k = 0; k < 20; ++k)
        record(unsigned(std::uint8_t(v.clearance[k])), get(raw, 8 + k, 1),
               name + "clearance[" + std::to_string(k) + "]");
#else
      record(v.cost, v.cost, "edge.cost");
      record(v.destination, v.destination, "edge.destination");
      for (auto value : v.clearance)
        record(unsigned(std::uint8_t(value)), unsigned(std::uint8_t(value)),
               "edge.clearance");
#endif
    }
  auto published = c;
  cs::prepareNativeRegionalGraph(published, p.native, shapes, capacity);
  need(published.native.regions.size() == nodes.size(),
       "native graph publication count");
  auto encoded = cs::encode(published);
  need(cs::encode(cs::decode(encoded)) == encoded,
       "generated graph portable roundtrip");
  auto before = cs::encode(published);
  bool refused = false;
  try {
    cs::prepareNativeRegionalGraph(published, p.native, shapes, capacity);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  need(refused && before == cs::encode(published),
       "nonempty graph rebuild was not atomic");
#ifdef MNM_PAIRED_REFERENCE
  checkText();
  call(0x53a840, regionManager);
  destroySearch();
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"digest\":" << digest << ",\"nodes\":" << nodes.size()
            << ",\"edges\":" << edges << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
