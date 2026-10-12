#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include "../reconstruction/creature-ai/actor_routing.hpp"
#include <map>
namespace {
struct RoutingFixture {
  Fixture base;
  ActorRouteContext context;
  std::vector<int> displacementOwners;
  unsigned blocker = 0;
};
RoutingFixture *routing;
ActorDisplacementServices displacementServices() {
  ActorDisplacementServices s;
  s.rangedReached = [](const auto &a, const auto *b) {
    record(500, a.ai.slot, b ? b->ai.slot : 0xffffffffu);
    return fixture->number % 3 == 0;
  };
  s.meleeReached = [](const auto &a, const auto *b, Cell p) {
    record(501, a.ai.slot, b ? b->ai.slot : 0xffffffffu, p);
    return fixture->number % 5 == 0;
  };
  s.blocker = [](auto &a) {
    record(502, a.ai.slot);
    return routing->blocker;
  };
  s.canDisplace = [](auto &a, unsigned i) {
    record(503, a.ai.slot, i);
    return fixture->number % 7 != 0;
  };
  s.random = [] {
    record(504, 0);
    return fixture->number * 2654435761u;
  };
  s.diagnostic = [](unsigned line) { record(505, 0, line); };
  s.action = [](auto &a, unsigned kind) {
    record(506, a.ai.slot, kind);
    a.ai.action = kind;
  };
  s.reset = [](auto &a) {
    record(507, a.ai.slot);
    a.ai.position.x = (a.ai.position.x + 1) % 16;
    ++a.cooldown;
  };
  s.obstacle = [](auto &a, Cell p) {
    record(508, a.ai.slot, 0, p);
    return fixture->number % 3 ? routing->blocker : routing->blocker + 1u;
  };
  s.update = [](auto &a, auto &c) {
    record(509, a.ai.slot);
    c.active = fixture->number / 23 % 3;
    a.cooldown = 21;
    return fixture->number / 17 % 2;
  };
  s.issue = [](auto &a, unsigned i) {
    record(510, a.ai.slot, i);
    return fixture->number % 11 != 0;
  };
  return s;
}
RoutingFixture generateRouting(unsigned n) {
  RoutingFixture f;
  f.base = generate(n);
  const unsigned count = f.base.world.creatures.size();
  f.displacementOwners.resize(count);
  for (unsigned i = 0; i < count; ++i) {
    f.displacementOwners[i] = int((n + i) % count);
    auto &a = f.base.world.creatures[i];
    f.base.execution[i].displacementDestination =
        n / 3 % 2 ? a.ai.position : Cell{3, 4, 2};
  }
  auto &a = f.base.world.creatures[0];
  constexpr unsigned behaviors[] = {3, 11, 13, 2, 14};
  constexpr unsigned substates[] = {1, 2, 3, 5, 6, 0xffffffffu};
  a.ai.behavior = behaviors[n / 3 % 5];
  a.substate = substates[n / 7 % 6];
  a.ai.target = n % 5 == 0 ? -1 : n % 13 == 0 ? -2 : int(n / 17 % count);
  f.blocker = n % 4 == 0 ? 0 : 1 + n / 11 % count;
  if (f.blocker) {
    const unsigned i = f.blocker - 1;
    if (n / 5 % 2 == 0)
      f.base.world.creatures[i].ai.behavior = 13;
    f.displacementOwners[i] = n % 3 ? 0 : int(count);
    if (n % 2)
      f.base.execution[i].displacementDestination =
          f.base.world.creatures[i].ai.position;
    if (n % 7 == 1)
      f.displacementOwners[0] = int(i);
  }
  f.context.active = n / 9 % 3;
  f.context.displacementContinuation = n / 11 % 2;
  f.context.diagnostic = n / 13 % 2;
  return f;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes displacementBody, lookupBody;
void routingExtra(void *raw, const CreatureExecution &e, int owner) {
  write32(raw, 0x6ec, unsigned(owner));
  write32(raw, 0x6f8, unsigned(e.displacementDestination.x));
  write32(raw, 0x6fc, unsigned(e.displacementDestination.y));
  write32(raw, 0x700, unsigned(e.displacementDestination.z));
}
extern "C" unsigned __attribute__((thiscall))
rangedDisplacementStub(void *p, void *target) {
  record(500, slot(p), target ? slot(target) : 0xffffffffu);
  return fixture->number % 3 == 0;
}
extern "C" unsigned __attribute__((thiscall))
meleeDisplacementStub(void *p, void *target, int x, int y, int z) {
  record(501, slot(p), target ? slot(target) : 0xffffffffu, {x, y, z});
  return fixture->number % 5 == 0;
}
extern "C" unsigned __attribute__((thiscall)) blockerDisplacementStub(void *p) {
  record(502, slot(p));
  return routing->blocker;
}
extern "C" unsigned __attribute__((thiscall)) canDisplacementStub(void *p,
                                                                  unsigned i) {
  record(503, slot(p), i);
  return fixture->number % 7 != 0;
}
extern "C" unsigned __attribute__((thiscall)) randomDisplacementStub(void *p) {
  if (p != reinterpret_cast<void *>(0x6e1ac8))
    throw std::runtime_error("displacement random ABI");
  record(504, 0);
  return fixture->number * 2654435761u;
}
extern "C" void __attribute__((thiscall))
diagnosticDisplacementStub(void *p, void *file, unsigned line) {
  if (p != reinterpret_cast<void *>(0x657d78) ||
      file != reinterpret_cast<void *>(0x5e9c10))
    throw std::runtime_error("displacement diagnostic ABI");
  record(505, 0, line);
}
extern "C" void __attribute__((thiscall))
actionDisplacementStub(void *p, unsigned kind) {
  record(506, slot(p), kind);
  write32(p, 0x5fc, kind);
}
extern "C" void __attribute__((thiscall)) resetDisplacementStub(void *p) {
  record(507, slot(p));
  write32(p, 8, (read32(p, 8) + 1) % 16);
  write32(p, 0x66c, read32(p, 0x66c) + 1);
}
extern "C" unsigned __attribute__((thiscall))
obstacleDisplacementStub(void *p, int x, int y, int z) {
  record(508, slot(p), 0, {x, y, z});
  return fixture->number % 3 ? routing->blocker : routing->blocker + 1u;
}
extern "C" unsigned __attribute__((thiscall)) updateDisplacementStub(void *p) {
  record(509, slot(p));
  global(0x6df184, fixture->number / 23 % 3);
  write32(p, 0x66c, 21);
  return fixture->number / 17 % 2;
}
extern "C" unsigned __attribute__((thiscall))
issueDisplacementStub(void *p, unsigned i) {
  record(510, slot(p), i);
  return fixture->number % 11 != 0;
}
void compareDisplacement(const RoutingFixture &initial,
                         const RoutingFixture &expected, bool result,
                         const std::vector<Event> &trace) {
  auto input = initial;
  routing = &input;
  fixture = &input.base;
  events.clear();
  prepare();
  for (auto v :
       {std::pair<unsigned, void *>{
            0x50bd40, reinterpret_cast<void *>(rangedDisplacementStub)},
        {0x50bd60, reinterpret_cast<void *>(meleeDisplacementStub)},
        {0x51ae60, reinterpret_cast<void *>(blockerDisplacementStub)},
        {0x51b710, reinterpret_cast<void *>(canDisplacementStub)},
        {0x54e200, reinterpret_cast<void *>(randomDisplacementStub)},
        {0x485b10, reinterpret_cast<void *>(diagnosticDisplacementStub)},
        {0x50db60, reinterpret_cast<void *>(actionDisplacementStub)},
        {0x50ab30, reinterpret_cast<void *>(resetDisplacementStub)},
        {0x51b130, reinterpret_cast<void *>(obstacleDisplacementStub)},
        {0x51b300, reinterpret_cast<void *>(updateDisplacementStub)},
        {0x51b7f0, reinterpret_cast<void *>(issueDisplacementStub)}})
    redirect(v.first, v.second);
  for (unsigned i = 0; i < input.base.world.creatures.size(); ++i)
    routingExtra(actors.data() + i * 0xe4b, input.base.execution[i],
                 input.displacementOwners[i]);
  global(0x6df184, input.context.active);
  global(0x6df1a0, input.context.displacementContinuation);
  global(0x6e1f30, input.context.diagnostic);
  auto before = actors;
  using F = unsigned(__attribute__((thiscall)) *)(void *);
  unsigned actual = reinterpret_cast<F>(0x51a600)(actors.data());
  if (actual != unsigned(result) || events != trace) {
    std::cerr << "displacement trace n=" << input.base.number
              << " result=" << actual << " expected=" << result << '\n';
    for (auto &v : events)
      std::cerr << "A " << v.kind << "," << v.argument << '\n';
    for (auto &v : trace)
      std::cerr << "E " << v.kind << "," << v.argument << '\n';
    throw std::runtime_error("displacement effects");
  }
  for (unsigned i = 0; i < expected.base.world.creatures.size(); ++i) {
    auto *raw = before.data() + i * 0xe4b;
    packActor(raw, expected.base.world.creatures[i],
              expected.base.execution[i]);
    routingExtra(raw, expected.base.execution[i],
                 expected.displacementOwners[i]);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "displacement actor n=" + std::to_string(input.base.number) +
        " offset=" + std::to_string(i % 0xe4b));
  }
  if (*reinterpret_cast<unsigned *>(0x6df184) != expected.context.active ||
      *reinterpret_cast<unsigned *>(0x6df1a0) !=
          expected.context.displacementContinuation ||
      *reinterpret_cast<unsigned *>(0x6e1f30) !=
          unsigned(expected.context.diagnostic))
    throw std::runtime_error("displacement shared continuation guard");
  if (std::memcmp(reinterpret_cast<void *>(0x51a600), displacementBody.data(),
                  displacementBody.size()) ||
      std::memcmp(reinterpret_cast<void *>(0x4970b0), lookupBody.data(),
                  lookupBody.size()))
    throw std::runtime_error("original displacement/lookup modified");
}
#endif
unsigned displacementRefusals() {
  unsigned refused = 0;
  for (unsigned kind = 0; kind < 4; ++kind) {
    auto f = generateRouting(11);
    routing = &f;
    fixture = &f.base;
    events.clear();
    auto &a = f.base.world.creatures[0];
    a.ai.behavior = kind == 3 ? 13 : 2;
    f.context.active = kind == 3 ? 2 : 0;
    f.blocker = 2;
    f.base.world.creatures[1].ai.behavior = 13;
    f.displacementOwners[1] = 0;
    auto services = displacementServices();
    services.canDisplace = [](auto &, unsigned) { return true; };
    services.obstacle = [](auto &, Cell) { return 2; };
    switch (kind) {
    case 0:
      f.blocker = unsigned(f.base.world.creatures.size()) + 1;
      break;
    case 1:
      f.displacementOwners.clear();
      break;
    case 2:
      f.base.execution.clear();
      break;
    case 3:
      f.displacementOwners.clear();
      break;
    }
    bool rejected = false;
    try {
      displaceCreatureRoute(a, f.base.world, f.displacementOwners,
                            f.base.execution, f.context, services);
    } catch (const std::out_of_range &) {
      rejected = true;
    }
    if (!rejected)
      throw std::runtime_error("displacement admission failure absent");
    ++refused;
  }
  return refused;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE required");
  map_image(read(argv[1]));
  displacementBody = Bytes(reinterpret_cast<unsigned char *>(0x51a600),
                           reinterpret_cast<unsigned char *>(0x51a878));
  lookupBody = Bytes(reinterpret_cast<unsigned char *>(0x4970b0),
                     reinterpret_cast<unsigned char *>(0x4970d0));
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  std::map<std::string, unsigned> coverage;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generateRouting(n), expected = input;
    routing = &expected;
    fixture = &expected.base;
    events.clear();
    bool result = displaceCreatureRoute(
        expected.base.world.creatures[0], expected.base.world,
        expected.displacementOwners, expected.base.execution, expected.context,
        displacementServices());
    auto trace = events;
    for (auto &v : trace)
      ++coverage["service_" + std::to_string(v.kind)];
    mix(result);
    mix(expected.context.active);
    mix(expected.context.displacementContinuation);
    for (auto &a : expected.base.world.creatures) {
      mix(a.ai.action);
      mix(a.ai.marker);
      mix(a.routePhase);
      mix(unsigned(a.cooldown));
      mix(unsigned(a.destination.x));
      mix(unsigned(a.destination.y));
      mix(unsigned(a.destination.z));
      mix(unsigned(a.ai.position.x));
    }
    for (auto &v : trace) {
      mix(v.kind);
      mix(v.slot);
      mix(v.argument);
      mix(unsigned(v.position.x));
      mix(unsigned(v.position.y));
      mix(unsigned(v.position.z));
    }
#ifdef MNM_EXECUTION_REFERENCE
    compareDisplacement(input, expected, result, trace);
#endif
    ++cases;
  }
  for (unsigned service = 500; service <= 510; ++service)
    if (!coverage["service_" + std::to_string(service)])
      throw std::runtime_error("missing displacement service " +
                               std::to_string(service));
  const unsigned refused = displacementRefusals();
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"native_refusal_cases\":" << refused
            << ",\"functions\":1,\"outcome_digest\":" << digest
            << ",\"coverage\":{";
  bool first = true;
  for (auto &v : coverage) {
    if (!first)
      std::cout << ',';
    first = false;
    std::cout << '\"' << v.first << "\":" << v.second;
  }
  std::cout << "}}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
