#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
namespace {
constexpr unsigned entries[] = {
    0x50acb0, 0x50afa0, 0x50b0c0, 0x50ccd0, 0x50d870, 0x50d8f0, 0x51c230,
    0x50d840, 0x50d860, 0x50d850, 0x521750, 0x521770, 0x521780};
constexpr unsigned specialKinds[] = {6, 7, 8, 14, 15, 16};
bool selection(AutonomousCreature &a, unsigned kind, unsigned divisor) {
  record(kind, a.ai.slot);
  bool result = fixture->number % divisor != 0;
  if (result && (kind == 70 || kind == 75))
    a.ai.target = int((a.ai.slot + 1) % fixture->world.creatures.size());
  return result;
}
BehaviorExecutionServices behaviorServices() {
  BehaviorExecutionServices s;
  s.immediate = [](auto &a) { return selection(a, 70, 3); };
  s.interaction = [](auto &a) { return selection(a, 71, 5); };
  s.follow = [](auto &a) { return selection(a, 72, 7); };
  s.queue = [](auto &a) { return selection(a, 73, 11); };
  s.blocked = [](auto &a) { return selection(a, 74, 13); };
  s.rangedOpponent = [](auto &a) { return selection(a, 75, 17); };
  s.action = executionServices().action;
  s.behavior = [](auto &a, unsigned v) {
    record(76, a.ai.slot, v);
    a.ai.behavior = v;
  };
  s.restoreForm = [](auto &a, unsigned mode) {
    record(77, a.ai.slot, mode);
    if (fixture->number % 2)
      a.ai.behavior = 2;
  };
  s.reset = [](auto &a) { record(23, a.ai.slot); };
  s.beginMotion = routeServices().beginMotion;
  s.ranged = [](auto &a) { record(78, a.ai.slot); };
  s.maintainTarget = [](auto &a) { record(79, a.ai.slot); };
  s.reach = routeServices().reach;
  s.face = [](auto &a, const auto &b) { record(80, a.ai.slot, b.ai.slot); };
  s.coordinateRange = [](auto &a, Cell p, unsigned r) {
    record(81, a.ai.slot, r, p);
    return fixture->number % 3 != 0;
  };
  s.coordinateFace = executionServices().orient;
  return s;
}
void runBehavior(Fixture &f, unsigned op) {
  auto &a = f.world.creatures[0];
  auto &e = f.execution[0];
  auto s = behaviorServices();
  switch (op) {
  case 0:
    idleBehavior(a, s);
    break;
  case 1:
    targetAttackBehavior(a, e, f.world, s);
    break;
  case 2:
    maintainTargetBehavior(a, e, f.world, 42, s);
    break;
  case 3:
    coordinateAttackBehavior(a, e, 1000, s);
    break;
  case 4:
    immediateAttackBehavior(a, f.world, s);
    break;
  case 5:
    standingRangedBehavior(a, e, f.world, 1000, s);
    break;
  case 6:
    displacementBehavior(a, e, s);
    break;
  default:
    specialBehavior(a, e, specialKinds[op - 7], s);
    break;
  }
}
#ifdef MNM_EXECUTION_REFERENCE
void behaviorExtra(void *p, const AutonomousCreature &a,
                   const CreatureExecution &e) {
  for (auto v : {std::pair<unsigned, unsigned>{0x6a4, a.targetMovingAttacker},
                 {0x7cd, a.animationKind},
                 {0x160, e.rangedDeadline},
                 {0x6f4, e.displacementActive},
                 {0x6f8, unsigned(e.displacementDestination.x)},
                 {0x6fc, unsigned(e.displacementDestination.y)},
                 {0x700, unsigned(e.displacementDestination.z)},
                 {0x62c, unsigned(e.castAim.x)},
                 {0x630, unsigned(e.castAim.y)},
                 {0x634, unsigned(e.castAim.z)},
                 {0x14, unsigned(a.finePosition.x)},
                 {0x18, unsigned(a.finePosition.y)},
                 {0x1c, unsigned(a.finePosition.z)}})
    write32(p, v.first, v.second);
  for (unsigned i = 0; i < 8; ++i)
    write32(p, 0x24 + i * 4, a.ai.visible[i]);
}
extern "C" unsigned __attribute__((thiscall)) immediateBehaviorStub(void *p) {
  record(70, slot(p));
  bool yes = fixture->number % 3 != 0;
  if (yes)
    write32(p, 0x64c, (slot(p) + 1) % fixture->world.creatures.size());
  return yes;
}
extern "C" unsigned __attribute__((thiscall)) interactionBehaviorStub(void *p) {
  record(71, slot(p));
  return fixture->number % 5 != 0;
}
extern "C" unsigned __attribute__((thiscall)) followBehaviorStub(void *p) {
  record(72, slot(p));
  return fixture->number % 7 != 0;
}
extern "C" unsigned __attribute__((thiscall)) queueBehaviorStub(void *p) {
  record(73, slot(p));
  return fixture->number % 11 != 0;
}
extern "C" unsigned __attribute__((thiscall)) blockedBehaviorStub(void *p) {
  record(74, slot(p));
  return fixture->number % 13 != 0;
}
extern "C" unsigned __attribute__((thiscall)) rangedOpponentStub(void *p) {
  record(75, slot(p));
  bool yes = fixture->number % 17 != 0;
  if (yes)
    write32(p, 0x64c, (slot(p) + 1) % fixture->world.creatures.size());
  return yes;
}
extern "C" void __attribute__((thiscall)) setBehaviorStub(void *p, unsigned v) {
  record(76, slot(p), v);
  write32(p, 0x5ec, v);
}
extern "C" void __attribute__((thiscall)) restoreFormBehaviorStub(void *p,
                                                                  unsigned v) {
  record(77, slot(p), v);
  if (fixture->number % 2)
    write32(p, 0x5ec, 2);
}
extern "C" void __attribute__((thiscall)) rangedBehaviorStub(void *p) {
  record(78, slot(p));
}
extern "C" void __attribute__((thiscall)) maintainBehaviorStub(void *p) {
  record(79, slot(p));
}
extern "C" void __attribute__((thiscall)) faceBehaviorStub(void *p, void *b) {
  record(80, slot(p), slot(b));
}
extern "C" unsigned __attribute__((thiscall))
coordinateRangeStub(void *p, int x, int y, int z, unsigned r) {
  record(81, slot(p), r, {x, y, z});
  return fixture->number % 3 != 0;
}
void installBehaviors() {
  for (auto v : {std::pair<unsigned, void *>{
                     0x516220, reinterpret_cast<void *>(immediateBehaviorStub)},
                 {0x516800, reinterpret_cast<void *>(interactionBehaviorStub)},
                 {0x513850, reinterpret_cast<void *>(followBehaviorStub)},
                 {0x524130, reinterpret_cast<void *>(queueBehaviorStub)},
                 {0x517750, reinterpret_cast<void *>(blockedBehaviorStub)},
                 {0x5164f0, reinterpret_cast<void *>(rangedOpponentStub)},
                 {0x50aa30, reinterpret_cast<void *>(setBehaviorStub)},
                 {0x5142b0, reinterpret_cast<void *>(restoreFormBehaviorStub)},
                 {0x50b2c0, reinterpret_cast<void *>(rangedBehaviorStub)},
                 {0x50b0c0, reinterpret_cast<void *>(maintainBehaviorStub)},
                 {0x508810, reinterpret_cast<void *>(faceBehaviorStub)},
                 {0x508250, reinterpret_cast<void *>(orientStub)},
                 {0x515350, reinterpret_cast<void *>(coordinateRangeStub)},
                 {0x50ab30, reinterpret_cast<void *>(resetStub)}})
    redirect(v.first, v.second);
}
void compareBehavior(const Fixture &input, const Fixture &expected, unsigned op,
                     const std::vector<Event> &trace) {
  Fixture f = input;
  fixture = &f;
  events.clear();
  prepare();
  for (unsigned i = 0; i < f.world.creatures.size(); ++i) {
    behaviorExtra(actors.data() + i * 0xe4b, f.world.creatures[i],
                  f.execution[i]);
    write32(configs.data() + i * 0x5c9, 0x34,
            unsigned(f.world.creatures[i].rangedDistance));
    write32(configs.data() + i * 0x5c9, 0x3c,
            f.world.creatures[i].formCapability);
    write32(configs.data() + i * 0x5c9, 0x198,
            unsigned(f.world.creatures[i].aimXY));
  }
  global(0x6bf3c4, 42);
  global(0x6c4830, 1000);
  auto before = actors;
  installBehaviors();
  unsigned entry = entries[op];
  const Bytes *body = nullptr;
  for (const auto &g : originalBodies)
    if (g.first == entry)
      body = &g.second;
  if (!body)
    throw std::runtime_error("behavior guard absent");
  std::memcpy(reinterpret_cast<void *>(entry), body->data(), body->size());
  using Step = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Step>(entry)(actors.data());
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "trace i=" << i << " actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual " << events[i].kind << "," << events[i].argument
                << " xyz=" << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected " << trace[i].kind << "," << trace[i].argument
                << " xyz=" << trace[i].position.x << "," << trace[i].position.y
                << "," << trace[i].position.z << '\n';
    throw std::runtime_error(
        "behavior trace n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.world.creatures[i],
              expected.execution[i]);
    behaviorExtra(before.data() + i * 0xe4b, expected.world.creatures[i],
                  expected.execution[i]);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "behavior write n=" + std::to_string(input.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  if (std::memcmp(reinterpret_cast<void *>(entry), body->data(), body->size()))
    throw std::runtime_error("behavior text modified");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("expected pinned PE");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  for (unsigned n = 0; n < 4096; ++n)
    for (unsigned op = 0; op < 13; ++op) {
      auto input = generate(n);
      for (unsigned i = 0; i < input.world.creatures.size(); ++i) {
        auto &a = input.world.creatures[i];
        auto &e = input.execution[i];
        a.ai.marker = (n + i) % 3 == 0;
        a.ai.readiness = (n + i) % 3 == 0;
        a.rangedDistance = (n + i) % 4 ? 3 : 0;
        a.ai.flying = (n + i) % 2;
        a.form = (n + i) % 5 == 0;
        a.formCapability = (n + i) % 7 == 0;
        a.animationKind = (n + i) % 4;
        a.finePosition = {a.ai.position.x * 32, a.ai.position.y * 32,
                          a.ai.position.z * 32};
        a.aimXY = int((n + i) % 32);
        a.chosenDestination = n % 2 ? a.ai.position : Cell{7, 8, 1};
        a.destination = n % 4 == 0 ? a.ai.position
                        : n % 4 == 1
                            ? Cell{a.ai.position.x + 1, a.ai.position.y, 2}
                            : Cell{5, 6, 2};
        e.displacementDestination = n % 3 ? Cell{1, 2, 1} : a.ai.position;
        e.displacementActive = 3;
        e.castAim = {int(n % 512), int(n % 256), 32};
        e.rangedDeadline = n % 5 ? 1000 : 1001;
        for (unsigned j = 0; j < 8; ++j)
          a.ai.visible[j] = (n + i + j) % 2;
      }
      if (op == 1 && n % 19 == 0)
        input.world.creatures[0].ai.target = -1;
      auto expected = input;
      fixture = &expected;
      events.clear();
      runBehavior(expected, op);
      auto trace = events;
      for (const auto &a : expected.world.creatures) {
        mix(a.ai.behavior);
        mix(a.ai.action);
        mix(unsigned(a.ai.target));
        mix(a.targetMovingAttacker);
        mix(unsigned(a.cooldown));
        mix(a.routePhase);
      }
      for (const auto &e : expected.execution) {
        mix(e.marker6a0);
        mix(e.routeRenewal);
        mix(e.displacementActive);
      }
      for (const auto &v : trace) {
        mix(v.kind);
        mix(v.slot);
        mix(v.argument);
        mix(unsigned(v.position.x));
        mix(unsigned(v.position.y));
        mix(unsigned(v.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareBehavior(input, expected, op, trace);
#endif
      ++cases;
    }
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":13,\"outcome_digest\":" << digest << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
