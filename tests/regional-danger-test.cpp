#include "../apps/command-session/native_regional.hpp"
#include "../reconstruction/creature-ai/regional_danger.hpp"
#ifdef MNM_PAIRED_REFERENCE
#include "regional-danger-original.hpp"
#else
#include "paired-encounter-projection.hpp"
#include <iostream>
#endif
#include <algorithm>
namespace {
using namespace mnm;
namespace m = command_session::model;
void need(bool ok, const std::string &what) {
  if (!ok)
    throw std::runtime_error(what);
}
#ifdef MNM_PAIRED_REFERENCE

#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_PAIRED_REFERENCE
  need(argc == 3, "PE INSTALL");
  auto pe = read(argv[1]);
  map_image(pe);
  endpoints();
  for (unsigned at : {0x411e50u, 0x466870u}) {
    auto prior = redirects.at(at);
    std::memcpy(reinterpret_cast<void *>(at), prior.data(), prior.size());
    redirects.erase(at);
  }
  sealText(pe);
  initializeSearch();
  using Constructor = void *(__attribute__((thiscall)) *)(void *, unsigned);
  reinterpret_cast<Constructor>(0x53a660)(regionManager, 512);
  const auto install = argv[2];
#else
  need(argc == 2, "INSTALL");
  const auto install = argv[1];
#endif
  auto p = encounter::decodeInputs(
      encounter::loadInputs(install, {0, 0, 32, 32}, 0));
#ifdef MNM_PAIRED_REFERENCE
  Original base(p);
  setup(base, p.initial, 17);
#endif
  unsigned checks = 0, slices = 0;
  unsigned nonzeroDanger = 0, nonzeroDeduction = 0, nonzeroBase = 0,
           pending = 0;
#ifdef MNM_PAIRED_REFERENCE
#define DANGER_WORD(value, base, offset, size, label)                          \
  record(value, get(base, offset, size), label)
#else
#define DANGER_WORD(value, base, offset, size, label)                          \
  record(value, value, label)
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto record = [&](unsigned a, unsigned b, const std::string &what) {
    if (a != b)
      throw std::runtime_error(what + " slice=" + std::to_string(slices) +
                               " native=" + std::to_string(a) +
                               " original=" + std::to_string(b));
    ++checks;
    digest = (digest ^ a) * 1099511628211ull;
  };
  for (unsigned scenario = 0; scenario < 8; ++scenario) {
    const unsigned count = 8 + scenario * 9;
    std::vector<m::NavigationRegion> g(count);
    for (unsigned i = 0; i < count; ++i) {
      auto &v = g[i];
      v.position = {int(i % 16) * 2, int(i / 16) * 3, int(2 + i % 5)};
      v.parameter = i % 2;
      for (unsigned j = 0; j < 4; ++j) {
        auto &e = v.connections[j];
        e.cost = 1 + int(j);
        e.destination = int((i + (j < 2 ? j + 1 : count - (j - 1))) % count);
        e.clearance.fill(3);
      }
    }
    m::RegionalDangerInputs in;
    in.dimensions = {32, 32, 23};
    in.tick = 100;
    in.firePower = 17;
    in.fireRange = 6;
    in.animationRange = 7;
    for (unsigned i = 0; i < 8; ++i) {
      in.enabled[i] = i < (scenario % 3 + 1);
      in.commander[i] = int(i);
      in.allied[i][i] = in.shared[i][i] = 1;
      if (scenario & 1)
        for (unsigned j = 0; j < 8; ++j)
          in.allied[i][j] = in.shared[i][j] = (i % 2 == j % 2);
    }
    for (unsigned i = 0; i < 12; ++i) {
      m::RegionalDangerActor a;
      a.slot = int(i);
      a.owner = int(i % 3);
      a.region = int(i % count);
      a.position = g[unsigned(a.region)].position;
      a.active = true;
      a.health = 360;
      a.resource = 1000 + i * 150;
      a.maximumResource = 50 + i * 20;
      a.threat = 100 + i * 700;
      a.aging = 7;
      a.type = i + 1;
      a.typeClass = i % 4 ? 1 : 24;
      a.ranged = i % 5 == 0;
      a.attackRange = a.ranged ? 1 + int(i % 9) : 0;
      a.status114 = i % 7 == 0;
      a.flag104 = i % 2;
      a.block7c = i % 6 == 0;
      a.height = 1 + i % 3;
      a.width = 1 + i % 2;
      a.movementKind = i % 4;
      in.actors.push_back(a);
    }
    if (scenario >= 4)
      for (unsigned owner = 0; owner < 8; ++owner)
        for (unsigned i = 0; i < 9; ++i) {
          auto a = in.actors[(i + owner) % in.actors.size()];
          a.estimatedResource.fill(5000);
          in.memories[owner].push_back({int(i * 3), 90, a});
        }
    if (scenario & 1) {
      for (unsigned kind : {0x41u, 0x45u, 0x46u, 0x5bu, 0x5cu})
        in.effects.push_back(
            {kind, int(kind % 3), 6, g[kind % count].position});
      in.hazards = {
          {{1, 2, 3}, 3, 10}, {{11, 2, 3}, 4, 7}, {{31, 3, 3}, 5, 13}};
      in.observationPoints = {{1, 1, 3}, {17, 1, 3}};
    }
    m::RegionalDangerState s;
#ifdef MNM_PAIRED_REFERENCE
    publishRegions(g);
    DangerOriginal original;
    original.inputs(in);
    original.memoryInputs(in);
    original.effectInputs(in);
#endif
    for (unsigned tick = 0; tick < 128; ++tick) {
      in.tick = 100 + int(tick);
      ++slices;
      m::updateRegionalDanger(g, s, in);
      m::updateRegionalDeduction(g, s, in);
#ifdef MNM_PAIRED_REFERENCE
      original.inputs(in);
      call(0x41a060, original.context.data());
      call(0x41d270, original.context.data());
#endif
      const auto cmp = [&](unsigned value, unsigned at, const char *name,
                           unsigned size = 4) {
        DANGER_WORD(value, original.context.data(), at, size, name);
        (void)at;
        (void)size;
      };
      cmp(s.resetDanger, 0x12073, "danger.reset", 1);
      cmp(s.dangerOwner, 0x12074, "danger.owner");
      cmp(s.memoryKey, 0x12078, "danger.memory");
      cmp(s.dangerRegion, 0x1207c, "danger.region");
      cmp(s.dangerTotal, 0x12084, "danger.total");
      cmp(s.resetDeduction, 0x12098, "deduction.reset", 1);
      cmp(s.deductionOwner, 0x12099, "deduction.owner");
      cmp(s.actorCursor, 0x1209d, "deduction.actor");
      cmp(s.deductionRegion, 0x120a1, "deduction.region");
      cmp(s.deductionTotal, 0x120a9, "deduction.total");
      pending += s.dangerRegion != 0 || s.deductionRegion != 0;
      for (unsigned i = 0; i < count; ++i) {
#ifdef MNM_PAIRED_REFERENCE
        const auto *raw =
            reinterpret_cast<void *>(get(regionManager) + 660 * i);
#endif
        for (unsigned owner = 0; owner < 8; ++owner)
          for (unsigned j = 0; j < 7; ++j) {
            DANGER_WORD(g[i].danger[owner][j], raw, 532 + 7 * owner + j, 1,
                        "danger.row[" + std::to_string(i) + "][" +
                            std::to_string(owner) + "][" + std::to_string(j) +
                            "]");
            DANGER_WORD(g[i].deductedDanger[owner][j], raw, 588 + 7 * owner + j,
                        1, "deduction.row");
            nonzeroDanger += g[i].danger[owner][j] != 0;
            nonzeroDeduction += g[i].deductedDanger[owner][j] != 0;
          }
        for (unsigned j = 0; j < 7; ++j) {
          DANGER_WORD(s.pendingDanger[i][j], raw, 645 + j, 1, "danger.scratch");
          DANGER_WORD(s.pendingDeduction[i][j], raw, 652 + j, 1,
                      "deduction.scratch");
        }
        DANGER_WORD(g[i].dangerBase, raw, 644, 1, "danger.base");
        DANGER_WORD(s.pendingBase[i], raw, 659, 1, "danger.base.scratch");
        nonzeroBase += g[i].dangerBase != 0;
      }
      for (unsigned i = 0; i < in.actors.size(); ++i)
        DANGER_WORD(s.actorSupport[i],
                    original.pool.data() + unsigned(in.actors[i].slot) * 0xe4b,
                    0x92a, 4, "actor.support");
      for (unsigned i = 0; i < 8; ++i)
        if (in.enabled[i]) {
          DANGER_WORD(s.dangerTotals[i],
                      original.pool.data() + unsigned(in.commander[i]) * 0xe4b,
                      0x922, 4, "owner.dangerTotal");
          DANGER_WORD(s.deductionTotals[i],
                      original.pool.data() + unsigned(in.commander[i]) * 0xe4b,
                      0x926, 4, "owner.deductionTotal");
        }
    }
#ifdef MNM_PAIRED_REFERENCE
    // The fixture owns the set allocation lifetime; no game pointer survives.
    using Erase = void(__attribute__((thiscall)) *)(void *, unsigned *,
                                                    unsigned, unsigned);
    unsigned result = 0;
    reinterpret_cast<Erase>(0x42acc0)(original.context.data() + 0x12088,
                                      &result, get(original.header.data()),
                                      address(original.header.data()));
#endif
  }
  // A hazard produced through the incremental base pass changes the preferred
  // branch. Both searches consume the published rows, not a hand-written cost.
  auto c = p.initial;
  c.native.regions.assign(4, {});
  auto &g = c.native.regions;
  g[0].position = {1, 1, 3};
  g[1].position = {2, 1, 3};
  g[2].position = {2, 3, 3};
  g[3].position = {4, 1, 3};
  auto edge = [&](unsigned from, unsigned index, int to, int cost) {
    auto &v = g[from].connections[index];
    v.destination = to;
    v.cost = cost;
    v.clearance.fill(8);
  };
  edge(0, 0, 1, 100);
  edge(0, 1, 2, 120);
  edge(1, 0, 3, 100);
  edge(2, 0, 3, 120);
  m::RegionalDangerInputs hazard;
  hazard.dimensions = c.world.dimensions;
  hazard.hazards = {{{2, 1, 3}, 0, 10}};
  m::RegionalDangerState hs;
#ifdef MNM_PAIRED_REFERENCE
  publishRegions(g);
  DangerOriginal ho;
  ho.inputs(hazard);
  ho.memoryInputs(hazard);
  ho.effectInputs(hazard);
#endif
  for (unsigned i = 0; i < 8; ++i) {
    m::updateRegionalDanger(g, hs, hazard);
#ifdef MNM_PAIRED_REFERENCE
    call(0x41a060, ho.context.data());
#endif
  }
  need(g[1].dangerBase == 10 && g[2].dangerBase == 0,
       "produced hazard witness");
#ifdef MNM_PAIRED_REFERENCE
  for (unsigned i = 0; i < 4; ++i)
    record(g[i].dangerBase,
           get(reinterpret_cast<void *>(get(regionManager) + 660 * i), 644, 1),
           "hazard.base");
  setup(base, c, 17);
#else
  for (unsigned i = 0; i < 4; ++i)
    record(g[i].dangerBase, g[i].dangerBase, "hazard.base");
#endif
  command_session::NativeCreatureGeometry geometry(c, p.native);
  auto &a = c.world.creatures[0];
  a.ai.owner = 0;
  auto query = command_session::nativeRegionalQuery(geometry, a, {0, 0, 0, 0});
  auto services =
      command_session::nativeRegionalServices(c, p.native, geometry, a);
  std::array<std::array<int, 50>, 2> routes;
  for (unsigned enabled = 0; enabled < 2; ++enabled) {
    query.dangerPenalty = enabled;
    routes[enabled].fill(-9);
    m::RegionalSearchState state;
    int budget = 100;
    const bool reached = m::searchNavigationRegions(
        g, 0, 3, query, c.world.dimensions, routes[enabled], budget, true,
        state, services);
    need(reached, "hazard route missing");
#ifdef MNM_PAIRED_REFERENCE
    publishRegions(g);
    auto expected = routes[enabled];
    expected.fill(-9);
    using Search = unsigned(__attribute__((thiscall)) *)(
        void *, int, int, WireQuery, int *, unsigned, unsigned);
    record(reached,
           reinterpret_cast<Search>(0x53e5e0)(
               regionManager, 0, 3, wire(base, query), expected.data(), 100, 1),
           "hazard.search");
    for (unsigned j = 0; j < 50; ++j)
      record(routes[enabled][j], expected[j], "hazard.path");
#else
    record(reached, reached, "hazard.search");
    for (auto v : routes[enabled])
      record(v, v, "hazard.path");
#endif
  }
  need(routes[0] != routes[1], "published danger did not change route");
#ifdef MNM_PAIRED_REFERENCE
  checkText();
  call(0x53a840, regionManager);
  destroySearch();
#endif
  need(nonzeroDanger && nonzeroDeduction && nonzeroBase && pending,
       "danger branch witnesses absent");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"nonzero_danger\":" << nonzeroDanger
            << ",\"nonzero_deduction\":" << nonzeroDeduction
            << ",\"nonzero_base\":" << nonzeroBase << ",\"pending\":" << pending
            << ",\"route_choice_changed\":true,\"slices\":" << slices
            << ",\"digest\":" << digest << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
