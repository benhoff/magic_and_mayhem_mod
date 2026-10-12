#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
namespace {
unsigned strategyRandom = 1;
unsigned strategyDraw() {
  strategyRandom ^= strategyRandom << 13;
  strategyRandom ^= strategyRandom >> 17;
  strategyRandom ^= strategyRandom << 5;
  record(60, 0, strategyRandom);
  return strategyRandom;
}
StrategyEnvironment strategyEnvironment() {
  StrategyEnvironment e;
  e.regionCapacity = 8;
  e.waterline = 2;
  for (unsigned i = 0; i < 8; ++i)
    e.regions.push_back(
        {int(i * 3), int((i + fixture->number) % 32), int(i % 4)});
  for (unsigned i = 0; i < 2; ++i)
    e.blockedColumns.push_back({int((fixture->number + i * 3) % 32),
                                int((fixture->number + i * 7) % 32), int(i)});
  for (unsigned i = 0; i < 2; ++i)
    e.scenarios.push_back({1, 1, (fixture->number + i) % 3 == 0 ? 0u : 1u, i});
  e.scenarioActor = [](const auto &s) -> std::optional<unsigned> {
    record(61, 0, s.key);
    if ((fixture->number + s.key) % 5 == 0)
      return 0;
    return std::nullopt;
  };
  return e;
}
StrategyServices strategyServices() {
  StrategyServices s;
  s.random = strategyDraw;
  s.admit = [](const auto &a, const auto &b) {
    record(62, a.ai.slot, b.ai.slot);
    return (fixture->number + b.ai.slot) % 3 != 0;
  };
  s.attack = [](auto &a, const auto &b) { record(63, a.ai.slot, b.ai.slot); };
  s.substate = [](auto &a, unsigned n) {
    record(64, a.ai.slot, n);
    a.substate = n;
    a.ai.behavior = 11;
  };
  s.action = executionServices().action;
  s.releaseAlternate = [](auto &, const auto &b) {
    record(9, b.ai.slot, 0);
    fixture->world.creatures.at(b.ai.slot).ai.incoming = 0;
  };
  s.insideInnerRadius = [](Cell a, Cell b, int radius) {
    record(65, 0, unsigned(radius), b);
    return creatureDistance(a, b, fixture->world.dimensions) <= radius;
  };
  s.legal = [](const auto &, Cell p) {
    record(66, 0, 0, p);
    return (p.x + p.y + p.z + int(fixture->number)) % 4 != 0;
  };
  s.water = [](const auto &, Cell p) {
    record(67, 0, 0, p);
    return (p.x + p.y + p.z) % 2;
  };
  s.regionReach = [](const auto &, int from, int to, unsigned limit) {
    record(68, 0, limit, {from, to, 0});
    return fixture->number % 3 != 0;
  };
  s.move = [](auto &a, Cell p) { record(69, a.ai.slot, 0, p); };
  return s;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes strategyRegions, strategyScenarios, strategyReferences;
void restoreStrategyBody(unsigned entry) {
  for (const auto &g : originalBodies)
    if (g.first == entry) {
      std::memcpy(reinterpret_cast<void *>(entry), g.second.data(),
                  g.second.size());
      return;
    }
  throw std::runtime_error("strategy body absent");
}
void strategyExtra(void *p, const AutonomousCreature &a,
                   const CreatureExecution &e) {
  for (auto v : {std::pair<unsigned, unsigned>{0x8e5, e.strategyGuard},
                 {0xbcf, unsigned(e.roamHome.x)},
                 {0xbd3, unsigned(e.roamHome.y)},
                 {0xbd7, unsigned(e.roamHome.z)},
                 {0xbdb, unsigned(e.roamRange)},
                 {0xbdf, unsigned(e.roamKind)},
                 {0xbe3, unsigned(e.roamAnnulus)},
                 {0xbe7, unsigned(e.roamInterval)},
                 {0xbeb, unsigned(e.roamCooldown)},
                 {0xbef, unsigned(e.fearBias)},
                 {0x68c, unsigned(a.range)},
                 {0x104, a.flag104}})
    write32(p, v.first, v.second);
}
void prepareStrategy() {
  prepare();
  auto env = strategyEnvironment();
  for (unsigned i = 0; i < fixture->world.creatures.size(); ++i) {
    auto &a = fixture->world.creatures[i];
    strategyExtra(actors.data() + i * 0xe4b, a, fixture->execution[i]);
    auto *c = configs.data() + i * 0x5c9;
    for (unsigned j = 0; j < 28; ++j)
      write32(c, 0x3d0 + j * 4, unsigned(a.ai.enemyRadius[j]));
    for (unsigned j = 0; j < 8; ++j)
      write32(actors.data() + i * 0xe4b, 0x24 + j * 4, a.ai.visible[j]);
  }
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      *reinterpret_cast<unsigned char *>(0x643a80 + i * 0x154 + 0x18 + j) =
          fixture->world.relation[i][j];
  global(0x6df180, fixture->world.creatures.size());
  strategyRegions.assign(8 * 0x294, 0);
  for (unsigned i = 0; i < 8; ++i) {
    auto p = env.regions[i];
    write32(strategyRegions.data() + i * 0x294, 4, p.x);
    write32(strategyRegions.data() + i * 0x294, 8, p.y);
    write32(strategyRegions.data() + i * 0x294, 12, p.z);
  }
  global(0x6eaf08, reinterpret_cast<std::uintptr_t>(strategyRegions.data()));
  global(0x6eaf0c, 8);
  global(0x6eaf10, 8);
  global(0x6c5c80, 2);
  global(0x6c5c84, 2);
  global(0x6e1f30, 0);
  strategyScenarios.assign(2 * 0x47c, 0);
  strategyReferences.assign(2 * 0x2c, 0);
  for (unsigned i = 0; i < 2; ++i) {
    auto &v = env.scenarios[i];
    auto *p = strategyScenarios.data() + i * 0x47c;
    write32(p, 0, v.active);
    write32(p, 4, v.kind);
    write32(p, 8, v.state);
    write32(p, 0x2cc, v.key);
    write32(strategyReferences.data() + i * 0x2c, 0x28, 0);
  }
  global(0x6f3a70, reinterpret_cast<std::uintptr_t>(strategyScenarios.data()));
  global(0x6f3a74, 2);
  global(0x6f3a78, 2);
}
extern "C" unsigned __attribute__((thiscall)) strategicAdmitStub(void *p,
                                                                 void *b) {
  record(62, slot(p), slot(b));
  return (fixture->number + slot(b)) % 3 != 0;
}
extern "C" unsigned __attribute__((thiscall)) strategicAttackStub(void *p,
                                                                  void *b) {
  record(63, slot(p), slot(b));
  return 1;
}
extern "C" void __attribute__((thiscall)) strategicSubstateStub(void *p,
                                                                unsigned n) {
  record(64, slot(p), n);
  write32(p, 0x684, n);
  write32(p, 0x5ec, 11);
}
extern "C" unsigned __attribute__((fastcall))
innerStub(int radius, unsigned zero, int x, int y, int z, int u, int v, int w) {
  if (zero)
    throw std::runtime_error("inner radius flags");
  record(65, 0, unsigned(radius), {u, v, w});
  return creatureDistance({x, y, z}, {u, v, w}, fixture->world.dimensions) <=
         radius;
}
extern "C" unsigned __attribute__((thiscall))
strategicLegalStub(void *, int x, int y, int z, void *) {
  record(66, 0, 0, {x, y, z});
  return (x + y + z + int(fixture->number)) % 4 != 0;
}
extern "C" unsigned __attribute__((thiscall)) waterStub(void *, int x, int y,
                                                        int z) {
  record(67, 0, 0, {x, y, z});
  return (x + y + z) % 2;
}
extern "C" void __attribute__((thiscall)) columnStub(void *, unsigned i, int *x,
                                                     int *y, int *z) {
  auto p = strategyEnvironment().blockedColumns.at(i);
  *x = p.x;
  *y = p.y;
  *z = p.z;
}
extern "C" void *__attribute__((thiscall)) scenarioStub(void *,
                                                        void *recordData) {
  unsigned key = read32(recordData, 0);
  record(61, 0, key);
  return (fixture->number + key) % 5 == 0
             ? strategyReferences.data() + key * 0x2c
             : nullptr;
}
extern "C" unsigned __attribute__((thiscall))
regionReachStub(void *, int from, int to, RegionQuery, void *, unsigned limit,
                unsigned flag) {
  if (flag != 1)
    throw std::runtime_error("strategic region reach flags");
  record(68, 0, limit, {from, to, 0});
  return fixture->number % 3 != 0;
}
extern "C" void __attribute__((thiscall)) strategicMoveStub(void *p, int x,
                                                            int y, int z) {
  record(69, slot(p), 0, {x, y, z});
}
extern "C" unsigned __attribute__((thiscall)) strategicRandomStub(void *) {
  return strategyDraw();
}
void installStrategy() {
  redirect(0x54e200, reinterpret_cast<void *>(strategicRandomStub));
  redirect(0x524910, reinterpret_cast<void *>(strategicAdmitStub));
  redirect(0x50ad10, reinterpret_cast<void *>(strategicAttackStub));
  redirect(0x51c3a0, reinterpret_cast<void *>(strategicSubstateStub));
  redirect(0x4e1380, reinterpret_cast<void *>(innerStub));
  redirect(0x4f4900, reinterpret_cast<void *>(strategicLegalStub));
  redirect(0x414890, reinterpret_cast<void *>(waterStub));
  redirect(0x4f3840, reinterpret_cast<void *>(columnStub));
  redirect(0x56cfe0, reinterpret_cast<void *>(scenarioStub));
  redirect(0x53e5e0, reinterpret_cast<void *>(regionReachStub));
  redirect(0x50fae0, reinterpret_cast<void *>(strategicMoveStub));
}
void compareStrategy(const Fixture &input, const Fixture &expected, unsigned op,
                     RoamChoice choice, const std::vector<Event> &trace) {
  Fixture source = input;
  fixture = &source;
  events.clear();
  strategyRandom = 1 + source.number;
  prepareStrategy();
  auto before = actors;
  unsigned entry = op ? 0x523470 : 0x523860;
  restoreStrategyBody(entry);
  unsigned result = 0;
  int index = -1;
  if (op) {
    using Work = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Work>(entry)(actors.data());
  } else {
    using Choose = unsigned(__attribute__((thiscall)) *)(void *, int *);
    result = reinterpret_cast<Choose>(entry)(actors.data(), &index);
    if (result != unsigned(choice.admitted) || index != choice.region)
      throw std::runtime_error("roam result mismatch n=" +
                               std::to_string(input.number));
  }
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "strategy trace index=" << i << " actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual=" << events[i].kind << "," << events[i].argument
                << " pos=" << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected=" << trace[i].kind << "," << trace[i].argument
                << " pos=" << trace[i].position.x << "," << trace[i].position.y
                << "," << trace[i].position.z << '\n';
    throw std::runtime_error(
        "strategy trace mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.world.creatures[i],
              expected.execution[i]);
    strategyExtra(before.data() + i * 0xe4b, expected.world.creatures[i],
                  expected.execution[i]);
  }
  if (actors != before) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "strategy write mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  for (const auto &g : originalBodies)
    if (g.first == entry && std::memcmp(reinterpret_cast<void *>(entry),
                                        g.second.data(), g.second.size()))
      throw std::runtime_error("strategic original text changed");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("expected pinned PE");
  install(argv[1]);
  installStrategy();
  restoreStrategyBody(0x523860);
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  for (unsigned n = 0; n < 4096; ++n)
    for (unsigned op = 0; op < 2; ++op) {
      auto input = generate(n);
      auto &a = input.world.creatures[0];
      auto &e = input.execution[0];
      a.ai.behavior = n % 3 == 0 ? 1 : 3;
      a.flag104 = n % 2;
      a.region = n % 5 == 0 ? -1 : 0;
      a.width = 1 + n % 2;
      a.height = 1 + n % 3;
      e.roamHome = {int(n % 32), int(n % 32), 0};
      e.roamRange = 1 + n % 15;
      e.roamKind = n % 5;
      e.roamAnnulus = n % 12;
      e.roamInterval = 90 + n % 180;
      e.roamCooldown = int(n % 200) - 40;
      e.fearBias = int(n % 7) - 3;
      e.strategyGuard = n % 2;
      for (unsigned i = 0; i < 28; ++i)
        a.ai.enemyRadius[i] = int((n + i) % 17) - 8;
      for (unsigned i = 0; i < 8; ++i)
        for (unsigned j = 0; j < 8; ++j)
          input.world.relation[i][j] = i == j || (n + i + j) % 7 == 0;
      auto expected = input;
      fixture = &expected;
      events.clear();
      strategyRandom = n + 1;
      RoamChoice choice;
      if (op)
        strategicCreatureWork(expected.world.creatures[0],
                              expected.execution[0], expected.world,
                              strategyEnvironment(), strategyServices());
      else
        choice = chooseRoamRegion(expected.world.creatures[0],
                                  expected.execution[0], expected.world,
                                  strategyEnvironment(), strategyServices());
      auto trace = events;
      mix(choice.admitted);
      mix(unsigned(choice.region));
      auto &v = expected.world.creatures[0];
      mix(unsigned(v.ai.target));
      mix(v.ai.behavior);
      mix(v.substate);
      mix(unsigned(expected.execution[0].roamCooldown));
      for (auto z : trace) {
        mix(z.kind);
        mix(z.slot);
        mix(z.argument);
        mix(unsigned(z.position.x));
        mix(unsigned(z.position.y));
        mix(unsigned(z.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareStrategy(input, expected, op, choice, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":2,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
