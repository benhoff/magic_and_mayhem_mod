#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
namespace {
unsigned randomState = 1;
unsigned draw() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;
  record(30, 0, randomState);
  return randomState;
}
ValuationEnvironment valueEnvironment() {
  ValuationEnvironment v;
  for (unsigned i = 0; i < 28; ++i) {
    v.configured[i] = (fixture->number + i) % 3 != 0;
    v.response[i] = int((fixture->number * 7 + i * 11) % 300) - 15;
  }
  v.statusBonus = {25, 50};
  v.hthBonus = 23;
  v.responseBonus = 19;
  v.factor5a9 = fixture->number % 9;
  v.factor5ad = fixture->number % 11;
  v.factor5c5 = int(fixture->number % 70) - 20;
  v.factor589 = int(fixture->number % 300) - 50;
  return v;
}
ActionExecutionServices actionServices() {
  ActionExecutionServices s;
  s.random = draw;
  s.animation = [](auto &a, unsigned seq, unsigned loop) {
    record(31, a.ai.slot, loop, {int(seq), 0, 0});
  };
  s.overlay = [](auto &a, unsigned config) { record(32, a.ai.slot, config); };
  s.overlayAnimation = [](auto &a, unsigned seq, unsigned loop) {
    record(33, a.ai.slot, loop, {int(seq), 0, 0});
  };
  s.attackAnimation = [](auto &a, const auto *b) {
    record(34, a.ai.slot, b ? b->ai.slot + 1 : 0);
  };
  s.flightAnimation = [](auto &a) {
    record(35, a.ai.slot);
    return fixture->number % 2;
  };
  s.repairOverlay = [](auto &a) { record(36, a.ai.slot); };
  return s;
}
AnimationActionServices eventServices() {
  AnimationActionServices s;
  s.poll = [](auto &a, bool overlay) {
    record(38, a.ai.slot, overlay);
    return int((fixture->number + (overlay ? 1 : 0)) % 5);
  };
  s.cast = [](auto &a) { record(39, a.ai.slot, a.castSpell); };
  s.behavior = [](auto &a) { executionServices().behavior(a, a.ai.behavior); };
  s.restoreForm = [](auto &a, unsigned mode) {
    record(37, a.ai.slot, mode);
    if (fixture->number % 2)
      a.ai.behavior = 2;
  };
  s.animation = actionServices().animation;
  return s;
}
PeriodicEnvironment periodicEnvironment() {
  PeriodicEnvironment e;
  e.terrainHasRegions = fixture->number % 2;
  e.relocationStrength = 7 + fixture->number % 30;
  e.regionCapacity = 8;
  for (int i = 0; i < 8; ++i)
    e.regions.push_back({i * 2, 7 - i, i % 4});
  return e;
}
PeriodicServices periodicServices() {
  PeriodicServices s;
  s.random = draw;
  s.terrain = [](Cell p) {
    record(40, 0, 0, p);
    return (p.x + p.y + p.z + int(fixture->number)) % 4 != 0;
  };
  s.reset = [](auto &a) { record(23, a.ai.slot); };
  s.move = [](auto &a, Cell p) { record(41, a.ai.slot, 0, p); };
  return s;
}
RangedExecutionServices rangedServices() {
  RangedExecutionServices s;
  s.blocked = [](auto &a) {
    record(42, a.ai.slot);
    return fixture->number % 7 == 0;
  };
  s.inRange = [](auto &a, const auto &b, unsigned radius) {
    record(43, a.ai.slot, b.ai.slot, {int(radius), 0, 0});
    return fixture->number % 3 != 0;
  };
  s.closeResponse = [](auto &a, Cell from, Cell to) {
    record(44, a.ai.slot, 0, from);
    record(45, a.ai.slot, 0, to);
    return fixture->number % 4 != 0;
  };
  s.prepareResponse = [](auto &a) {
    record(46, a.ai.slot);
    return fixture->number % 5 != 0;
  };
  s.maintainTarget = [](auto &a) { record(47, a.ai.slot); };
  s.orient = executionServices().orient;
  s.action = executionServices().action;
  return s;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes extraConfigs, overlayConfig, overlayDefinitions, regionRecords;
void restoreBody(unsigned entry) {
  for (auto &g : originalBodies)
    if (g.first == entry) {
      std::memcpy(reinterpret_cast<void *>(entry), g.second.data(),
                  g.second.size());
      return;
    }
  throw std::runtime_error("missing body guard");
}
void checkBody(unsigned entry) {
  for (auto &g : originalBodies)
    if (g.first == entry) {
      if (std::memcmp(reinterpret_cast<void *>(entry), g.second.data(),
                      g.second.size()))
        throw std::runtime_error("original executed body changed");
      return;
    }
  throw std::runtime_error("missing body guard");
}
void packExtra(void *p, const AutonomousCreature &a,
               const CreatureExecution &e) {
  for (auto v : {std::pair<unsigned, unsigned>{0xe0, e.animationOffset},
                 {0x60c, e.animationFacing},
                 {0x16c, e.previousMove},
                 {0x170, e.previousMoveVariant},
                 {0xb63, e.moveVariant},
                 {0xb77, e.actionRouteCounters[0]},
                 {0xb7b, e.actionRouteCounters[1]},
                 {0xb7f, e.actionRouteCounters[2]},
                 {0x7cd, a.animationKind},
                 {0xf4, a.ai.corpseB},
                 {0x104, a.flag104},
                 {0xf8, a.ai.physical},
                 {0x88, unsigned(a.ai.bravery)},
                 {0x94, unsigned(a.ai.bloodLust)},
                 {0x84, e.power84},
                 {0x90, e.mitigation90},
                 {0x8c, a.specialVision},
                 {0x8ed, e.valueScale8ed},
                 {0x124, e.bonusMode124},
                 {0x92e, e.dirtyValue},
                 {0x8d1, e.terrainRelocation},
                 {0x855, e.block855},
                 {0x160, e.rangedDeadline},
                 {0x670, e.rangedLatch},
                 {0x65c, e.targetDistance},
                 {0x14, unsigned(a.finePosition.x)},
                 {0x18, unsigned(a.finePosition.y)},
                 {0x1c, unsigned(a.finePosition.z)},
                 {0x61c, a.castSpell}})
    write32(p, v.first, v.second);
  for (unsigned i = 0; i < 6; ++i)
    write32(p, 0x90a + 4 * i, unsigned(e.combatValue[i]));
}
unsigned cursorSlot(void *p, unsigned offset) {
  return unsigned((static_cast<unsigned char *>(p) - actors.data() - offset) /
                  0xe4b);
}
extern "C" unsigned __attribute__((thiscall)) randomStub(void *) {
  return draw();
}
extern "C" void __attribute__((thiscall)) playStub(void *p, unsigned seq,
                                                   unsigned loop) {
  auto delta = static_cast<unsigned char *>(p) - actors.data();
  bool overlay = delta % 0xe4b == 0x7d5;
  record(overlay ? 33 : 31, cursorSlot(p, overlay ? 0x7d5 : 0xb0), loop,
         {int(seq), 0, 0});
}
extern "C" void __attribute__((thiscall)) overlayStub(void *p, void *config) {
  record(32, cursorSlot(p, 0x7d5), read32(config, 0));
}
extern "C" void __attribute__((thiscall)) attackAnimationStub(void *p,
                                                              void *target) {
  record(34, slot(p), target ? slot(target) + 1 : 0);
}
extern "C" unsigned __attribute__((thiscall)) flightStub(void *p) {
  record(35, slot(p));
  return fixture->number % 2;
}
extern "C" void __attribute__((thiscall)) repairStub(void *p) {
  record(36, slot(p));
}
extern "C" void __attribute__((thiscall)) formStub(void *p, unsigned mode) {
  record(37, slot(p), mode);
  if (fixture->number % 2)
    write32(p, 0x5ec, 2);
}
extern "C" int __attribute__((thiscall)) pollStub(void *p) {
  auto delta = static_cast<unsigned char *>(p) - actors.data();
  bool overlay = delta % 0xe4b == 0x7d5;
  record(38, cursorSlot(p, overlay ? 0x7d5 : 0xb0), overlay);
  return int((fixture->number + (overlay ? 1 : 0)) % 5);
}
extern "C" void __attribute__((thiscall)) castStub(void *p) {
  record(39, cursorSlot(p, 0x61c), read32(p, 0));
}
extern "C" unsigned __attribute__((thiscall)) terrainStub(void *, int x, int y,
                                                          int z) {
  record(40, 0, 0, {x, y, z});
  return (x + y + z + int(fixture->number)) % 4 != 0;
}
extern "C" void __attribute__((thiscall)) periodicMoveStub(void *p, int x,
                                                           int y, int z) {
  record(41, slot(p), 0, {x, y, z});
}
extern "C" unsigned __attribute__((thiscall)) blockedStub(void *p) {
  record(42, slot(p));
  return fixture->number % 7 == 0;
}
extern "C" unsigned __attribute__((thiscall)) rangeStub(void *p, void *target,
                                                        unsigned radius) {
  record(43, slot(p), slot(target), {int(radius), 0, 0});
  return fixture->number % 3 != 0;
}
extern "C" unsigned __attribute__((thiscall))
closeStub(void *p, int x, int y, int z, int u, int v, int w) {
  record(44, slot(p), 0, {x, y, z});
  record(45, slot(p), 0, {u, v, w});
  return fixture->number % 4 != 0;
}
extern "C" unsigned __attribute__((thiscall)) responseStub(void *p) {
  record(46, slot(p));
  return fixture->number % 5 != 0;
}
extern "C" void __attribute__((thiscall)) maintainStub(void *p) {
  record(47, slot(p));
}
void prepareExtra() {
  prepare();
  auto env = valueEnvironment();
  auto regions = periodicEnvironment();
  for (unsigned i = 0; i < fixture->world.creatures.size(); ++i) {
    auto &a = fixture->world.creatures[i];
    auto &e = fixture->execution[i];
    auto *p = actors.data() + i * 0xe4b;
    packExtra(p, a, e);
    auto *c = configs.data() + i * 0x5c9;
    write32(p, 0x600,
            e.actionCallback ? actionAddresses[*e.actionCallback - 1] : 0);
    write32(p, 0x604, 0);
    write32(p, 0x5f4, behaviorAddresses[a.ai.behavior - 1]);
    write32(p, 0x5f8, 0);
    for (unsigned j = 0; j < 28; ++j) {
      write32(c, 0x1a0 + 4 * j, unsigned(a.ai.hth[j]));
      write32(c, 0x280 + 4 * j, unsigned(env.response[j]));
    }
    write32(c, 0x5a9, env.factor5a9);
    write32(c, 0x5ad, env.factor5ad);
    write32(c, 0x5c5, env.factor5c5);
    write32(c, 0x589, env.factor589);
    write32(c, 0x14, 20);
    write32(c, 0x30, 9);
    write32(c, 0x198, a.aimXY);
    global(0x6a5fd8 + i % 28 * 0x5c9, env.configured[i % 28]);
  }
  for (unsigned i = 0; i < 28; ++i)
    global(0x6a5fd8 + i * 0x5c9, env.configured[i]);
  global(0x5e1910, 25);
  global(0x5e1914, 50);
  global(0x6bfd88, 23);
  global(0x6c27c7, 19);
  overlayConfig.assign(0x248, 0);
  overlayDefinitions.assign(8 * 276, 0);
  for (unsigned i = 0; i < 8; ++i)
    write32(overlayDefinitions.data() + i * 276, 0, i);
  write32(overlayConfig.data(), 0x240, 64);
  write32(overlayConfig.data(), 0x244, fixture->number % 4);
  write32(overlayConfig.data(), 0x230, 32);
  write32(overlayConfig.data(), 0x234, 4 + fixture->number % 4);
  global(0x6898d0, reinterpret_cast<std::uintptr_t>(overlayConfig.data()));
  global(0x6894a0, reinterpret_cast<std::uintptr_t>(overlayDefinitions.data()));
  regionRecords.assign(8 * 0x294, 0);
  for (unsigned i = 0; i < 8; ++i) {
    write32(regionRecords.data() + i * 0x294, 4, regions.regions[i].x);
    write32(regionRecords.data() + i * 0x294, 8, regions.regions[i].y);
    write32(regionRecords.data() + i * 0x294, 12, regions.regions[i].z);
  }
  global(0x6a49cc, regions.terrainHasRegions);
  global(0x6beb51, regions.relocationStrength);
  global(0x6eaf08, reinterpret_cast<std::uintptr_t>(regionRecords.data()));
  global(0x6eaf0c, 8);
  global(0x6eaf10, 8);
  global(0x6e1f30, 0);
  global(0x6c4830, 100);
  global(0x6bf3c4, 7);
}
void installExtra() {
  redirect(0x54e200, reinterpret_cast<void *>(randomStub));
  redirect(0x464cb0, reinterpret_cast<void *>(playStub));
  redirect(0x464ca0, reinterpret_cast<void *>(overlayStub));
  redirect(0x50e400, reinterpret_cast<void *>(attackAnimationStub));
  redirect(0x510480, reinterpret_cast<void *>(flightStub));
  redirect(0x513f50, reinterpret_cast<void *>(repairStub));
  redirect(0x5142b0, reinterpret_cast<void *>(formStub));
  redirect(0x464ec0, reinterpret_cast<void *>(pollStub));
  redirect(0x57b710, reinterpret_cast<void *>(castStub));
  redirect(0x534490, reinterpret_cast<void *>(terrainStub));
  redirect(0x517750, reinterpret_cast<void *>(blockedStub));
  redirect(0x515620, reinterpret_cast<void *>(rangeStub));
  redirect(0x50efc0, reinterpret_cast<void *>(closeStub));
  redirect(0x50b650, reinterpret_cast<void *>(responseStub));
  redirect(0x50b0c0, reinterpret_cast<void *>(maintainStub));
}
void checkActions(const Fixture &input, const Fixture &expected, unsigned op,
                  unsigned argument, const std::vector<Event> &trace) {
  Fixture source = input;
  fixture = &source;
  events.clear();
  randomState = 1 + source.number;
  prepareExtra();
  Bytes before = actors;
  constexpr unsigned entries[] = {0x50db60, 0x5244f0, 0x522ad0, 0x50b2c0,
                                  0x50e670, 0x50e6f0, 0x50f7c0};
  unsigned entry = entries[op];
  restoreBody(entry);
  redirect(0x50ab30, reinterpret_cast<void *>(resetStub));
  if (op == 2)
    redirect(0x50fae0, reinterpret_cast<void *>(periodicMoveStub));
  using Step = void(__attribute__((thiscall)) *)(void *);
  using Action = void(__attribute__((thiscall)) *)(void *, unsigned);
  if (op == 0)
    reinterpret_cast<Action>(entry)(actors.data(), argument);
  else
    reinterpret_cast<Step>(entry)(actors.data());
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "event index=" << i << " actual count=" << events.size()
              << " expected count=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual=" << events[i].kind << "," << events[i].argument
                << "," << events[i].position.x << '\n';
    if (i < trace.size())
      std::cerr << "expected=" << trace[i].kind << "," << trace[i].argument
                << "," << trace[i].position.x << '\n';
    throw std::runtime_error(
        "action trace mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.world.creatures[i],
              expected.execution[i]);
    packExtra(before.data() + i * 0xe4b, expected.world.creatures[i],
              expected.execution[i]);
  }
  if (op == 0) {
    auto handler = expected.execution[0].actionCallback;
    write32(before.data(), 0x600, handler ? actionAddresses[*handler - 1] : 0);
    write32(before.data(), 0x604, 0);
  }
  if (actors != before) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    std::cerr << "actual byte=" << unsigned(actors[i])
              << " expected=" << unsigned(before[i])
                << " actualDWORD="<<read32(actors.data(),0x91e) << '\n';
    throw std::runtime_error(
        "action writes mismatch n=" + std::to_string(input.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  checkBody(entry);
  if (op == 0)
    redirect(0x50db60, reinterpret_cast<void *>(actionStub));
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("expected pinned PE");
  install(argv[1]);
  installExtra();
  for (unsigned kind = 0; kind < 40; ++kind) {
    unsigned c = kind;
    using Style = unsigned(__attribute__((thiscall)) *)(void *);
    if (reinterpret_cast<Style>(0x505cf0)(&c) != rangedStyle(kind))
      throw std::runtime_error("ranged style mismatch");
  }
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  for (unsigned n = 0; n < 4096; ++n)
    for (unsigned op = 0; op < 7; ++op) {
      Fixture input = generate(n);
      auto &a = input.world.creatures[0];
      auto &e = input.execution[0];
      a.ai.behavior = 1 + (n % 11);
      a.ai.kind=n%28;
      a.ai.action = 1 + n % 25;
      a.ai.motion = n % 7 == 0;
      a.animationKind = n % 4;
      a.ai.flying = n % 3 == 0;
      a.formCapability = n % 2;
      a.ai.corpseB = n % 6;
      a.flag104 = n % 2;
      a.form = n % 3 == 0;
      a.specialVision = n % 7 == 0;
      a.ai.bravery = int(n % 400);
      a.ai.physical = n % 2;
      a.ai.bloodLust = n % 3 == 0;
      a.rangedDistance = int(n % 6);
      a.selectedSpell = n % 104;
      a.castSpell = n % 104;
      a.aimXY = int(n % 7);
      a.finePosition = {int(n % 16) * 32, 32, 16};
      a.ai.readiness = n % 3;
      a.cooldown = int(n % 5) - 1;
      e.animationFacing = n % 8;
      e.animationOffset = n % 300;
      e.moveVariant = n % 7;
      e.previousMove = n % 2;
      e.previousMoveVariant = n % 6;
      e.actionRouteCounters = {3, 4, 5};
      e.valueScale8ed = n % 4;
      e.bonusMode124 = n % 50;
      e.power84 = n % 60;
      e.mitigation90 = n % 9 == 0;
      e.dirtyValue = n % 3 != 0;
      e.terrainRelocation = n % 3;
      e.block855 = n % 4 == 0;
      e.rangedDeadline = 95 + n % 10;
      e.rangedLatch = n % 2;
      e.targetDistance = n % 30;
      for (unsigned j = 0; j < 28; ++j)
        a.ai.hth[j] = int((n * 13 + j * 17) % 500) - 20;
      for (auto &b : input.world.creatures) {
        b.finePosition = {b.ai.position.x * 32, b.ai.position.y * 32,
                          b.ai.position.z * 16};
        b.aimXY = int(n % 7);
      }
      auto expected = input;
      fixture = &expected;
      events.clear();
      randomState = 1 + n;
      auto &v = expected.world.creatures[0];
      auto &x = expected.execution[0];
      unsigned argument = 1 + n % 25;
      if (op == 0)
        changeAction(v, x, expected.world, argument, {n % 4, 64, 4 + n % 4, 32},
                     actionServices());
      else if (op == 1)
        updateCreatureValue(v, x, valueEnvironment());
      else if (op == 2)
        periodicTerrain(v, x, 20, periodicEnvironment(), periodicServices());
      else if (op == 3)
        rangedTactic(v, x, expected.world, rangedStyle(v.ai.kind), 100,
                     rangedServices());
      else if (op == 4 || op == 5)
        tickRangedAction(v, x, op == 5, 100, 9, 7, eventServices());
      else
        tickReadyAction(v, x, eventServices());
      auto trace = events;
      mix(v.ai.action);
      mix(v.ai.readiness);
      mix(unsigned(v.cooldown));
      mix(x.animationOffset);
      mix(x.rangedDeadline);
      mix(x.terrainRelocation);
      mix(x.dirtyValue);
      mix(x.rangedLatch);
      mix(x.targetDistance);
      for (auto z : x.combatValue)
        mix(unsigned(z));
      for (auto z : trace) {
        mix(z.kind);
        mix(z.slot);
        mix(z.argument);
        mix(unsigned(z.position.x));
        mix(unsigned(z.position.y));
        mix(unsigned(z.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      checkActions(input, expected, op, argument, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":8,\"outcome_digest\":" << digest << "}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
