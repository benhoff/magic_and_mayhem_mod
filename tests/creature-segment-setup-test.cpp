#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include "../reconstruction/creature-ai/full_segment_setup.hpp"
#include <map>
namespace {
struct SetupFixture {
  Fixture base;
  MotionActionState motion;
  SegmentCompletionState route;
  SegmentInitializationState state;
  SegmentInitializationEnvironment environment;
};
SetupFixture *setup;
unsigned setupTile(Cell p) { return unsigned(p.x + p.y * 3 + p.z * 5) % 4; }
int setupHeight(unsigned kind) { return int(kind) * 3 - 7; }
int setupSample(unsigned bank, unsigned i) {
  unsigned u = (bank * 37u + i * 53u + fixture->number) % 151u;
  if (fixture->number % 23 == 0)
    u += 0x7fffff00u;
  return u <= 0x7fffffffu ? int(u) : -1 - int(~u);
}
unsigned rawBase() {
  return fixture->number % 37 == 0 ? 0xffffffd3u : 123 + fixture->number % 15;
}
unsigned scalarRate(unsigned prior) { return prior + 43; }
unsigned scalarDirection(Cell p, Cell target) {
  return unsigned(p.x + 3 * p.y + 5 * p.z) + setupTile(target) * 11u;
}
unsigned setupDistance(Cell d) {
  return unsigned(std::abs(d.x) + std::abs(d.y) + std::abs(d.z)) + 1;
}
int setupTurn(unsigned old, unsigned next) {
  return int((next - old + 8) & 7u) - 3;
}
void visualTrace(const AutonomousCreature &a, const CreatureExecution &e) {
  if (a.animationKind == 3) {
    record(400, a.ai.slot, 4);
    record(401, a.ai.slot, e.animationFacing + 2);
  } else if (a.animationKind == 2 && setup->motion.visualSprite)
    record(402, a.ai.slot, e.animationFacing);
  if (e.block855)
    record(403, a.ai.slot, e.animationFacing + 8);
}
SegmentInitializationServices setupServices() {
  SegmentInitializationServices s;
  s.precheck = [](auto &a) {
    record(404, a.ai.slot);
    return fixture->number % 17 == 0;
  };
  s.immediate = [](auto &a) {
    record(405, a.ai.slot);
    return fixture->number % 3 == 0;
  };
  s.fallback = [](auto &a) {
    record(406, a.ai.slot);
    return fixture->number % 2 == 0;
  };
  s.visualFacing = [](auto &a) {
    visualTrace(a, fixture->execution[a.ai.slot]);
  };
  s.base = [](const auto &a, unsigned mode, Cell d) {
    record(407, a.ai.slot, mode, d);
    return rawBase();
  };
  s.scalarArgument = [](Cell p, Cell to) {
    record(408, 0, setupTile(to), p);
    return scalarDirection(p, to);
  };
  s.scalar = [](const auto &a, unsigned argument, unsigned mode, Cell d,
                int base, unsigned &rate, int &length) {
    record(409, a.ai.slot, argument, d);
    record(410, a.ai.slot, mode, {base, int(rate), 0});
    rate = scalarRate(rate);
    length = 7;
  };
  s.accept = [](auto &a, Cell from, Cell to, unsigned &mode) {
    record(411, a.ai.slot, 0, from);
    record(412, a.ai.slot, 0, to);
    mode = fixture->execution[a.ai.slot].mode10c + (fixture->number % 13 == 0);
    return fixture->number % 19 != 0;
  };
  s.hazard = [](const auto &a, Cell p) {
    record(413, a.ai.slot, 0, p);
    return !(p == a.ai.position) && fixture->number % 3 != 0;
  };
  s.release = [](auto &a, Cell p) { record(414, a.ai.slot, 0, p); };
  s.reserve = [](auto &a, Cell p) { record(415, a.ai.slot, 0, p); };
  s.turn = [](unsigned old, unsigned next) {
    record(416, 0, old, {int(next), 0, 0});
    return setupTurn(old, next);
  };
  s.distance = [](Cell d) {
    record(417, 0, 0, d);
    return setupDistance(d);
  };
  s.tileKind = [](Cell p) { return setupTile(p); };
  s.tileHeight = [](unsigned k) { return setupHeight(k); };
  s.animation = [](auto &a, unsigned frame, unsigned reverse) {
    record(418, a.ai.slot, frame, {int(reverse), 0, 0});
  };
  s.sample = [](const auto &, unsigned bank, unsigned i) {
    if (i >= 48)
      throw std::out_of_range("setup samples");
    return setupSample(bank, i);
  };
  s.attack = [](auto &a, const auto &b) { record(419, a.ai.slot, b.ai.slot); };
  s.action = [](auto &a, unsigned kind) {
    record(420, a.ai.slot, kind);
    a.ai.action = kind;
  };
  return s;
}
SetupFixture generateSetup(unsigned n, unsigned op) {
  SetupFixture f;
  f.base = generate(n);
  auto &a = f.base.world.creatures[0];
  auto &e = f.base.execution[0];
  f.base.world.dimensions = {16, 16, 4};
  a.ai.position = {int(n % 16), int(n / 16 % 16), 1 + int(n % 2)};
  a.finePosition = {47, -39, 41};
  a.animationKind = n / 11 % 4;
  a.block723 = n / 7 % 2;
  a.protection8c9 = n / 53 % 2;
  a.hazardSensitive = n / 29 % 2;
  a.formCapability = n / 5 % 2;
  a.form = n / 3 % 2;
  a.rangedDistance = n / 23 % 2;
  a.ai.target = n % 3 == 0 ? -1 : int(n % f.base.world.creatures.size());
  for (unsigned i = 0; i < f.base.world.creatures.size(); ++i) {
    auto &v = f.base.world.creatures[i];
    v.ai.owner = int((n + i) % 8);
    for (unsigned j = 0; j < 8; ++j)
      f.base.world.relation[unsigned(v.ai.owner)][j] =
          unsigned((n / 31 + j) % 2);
  }
  a.ai.type = op == 2 ? 12 : n % 28;
  a.ai.action = n / 3 % 2 ? 2 : 15;
  e.mode10c = n / 17 % 6;
  e.routeRenewal = n / 19 % 2;
  e.markerB73 = n / 41 % 2;
  e.block722 = n / 2 % 2;
  e.block855 = n / 13 % 2;
  e.moveVariant = 37;
  e.animationFacing = 2;
  e.animationOffset = 71;
  e.previousMove = 9;
  e.actionRouteCounters = {81, 82, 83};
  f.motion.direction = n % 10;
  f.motion.visualSprite = n / 7 % 2;
  f.motion.progress = 239;
  f.motion.travelX = -311;
  f.motion.travelY = 277;
  f.motion.deltaX = int(n % 3) - 1;
  f.motion.deltaY = int(n / 3 % 3) - 1;
  f.motion.accumulator = 73;
  f.motion.duration = 53;
  f.motion.sampleCursor = 1;
  f.motion.initialCursor = 3;
  f.motion.initialResidualX = -6;
  f.motion.initialResidualY = 7;
  f.motion.heightOrigin = 91;
  f.motion.heightDelta = -17;
  f.state.vertical = int(n / 11 % 3) - 1;
  f.motion.vertical = f.state.vertical != 0;
  f.state.slow778 = n / 19 % 3 == 0 ? 1 : -1;
  f.state.fast77c = n / 29 % 3 == 0 ? 1 : -1;
  f.state.specialRate73c = 53;
  f.state.inlineSample = 19;
  f.environment = {n / 7 % 4, 720};
  f.route.count = 1 + n / 37 % 16;
  f.route.cursor = n % f.route.count;
  for (unsigned i = 0; i < 16; ++i)
    f.route.route[i] = {{int((n + 3 + i) % 16), int((n / 16 + 1 + i) % 16),
                         1 + int((n + i) % 2)},
                        {int((n + i) / 3 % 10), int((n + i) / 7 % 3) - 1,
                         int(i % 2), int((n + i) / 13 % 5)}};
  if (op == 1) {
    auto &q = f.route.route[f.route.cursor];
    e.mode10c %= 5;
    q.remainder = {int(f.motion.direction), f.state.vertical, 2,
                   int(e.mode10c)};
    a.ai.action = 2;
  }
  if (n % 43 == 0)
    f.route.cursor = f.route.count;
  return f;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes setupBody, setupTerrain, setupMetadata, setupVisual(2 * 276),
    setupVisualDefinition(0x200);
unsigned setupAddr(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
unsigned setupBankPointer(void *raw, unsigned bank, unsigned cursor) {
  const unsigned base =
      bank == 0x71e ? setupAddr(actors.data()) + read32(raw, 0) * 0xe4b + 0x71e
                    : read32(raw, 0xac) + bank;
  return base + cursor * 4u;
}
void setupExtra(void *raw, const AutonomousCreature &a,
                const CreatureExecution &e, const MotionActionState &m,
                const SegmentInitializationState &s,
                const SegmentCompletionState &r) {
  for (auto v :
       {std::pair<unsigned, unsigned>{0x14, unsigned(a.finePosition.x)},
        {0x18, unsigned(a.finePosition.y)},
        {0x1c, unsigned(a.finePosition.z)},
        {0x608, m.direction},
        {0x610, unsigned(s.vertical)},
        {0x614, e.previousMove},
        {0x60c, e.animationFacing},
        {0x7cd, a.animationKind},
        {0x7d1, m.visualSprite ? 0xc001u : 0u},
        {0x855, e.block855},
        {0x778, unsigned(s.slow778)},
        {0x77c, unsigned(s.fast77c)},
        {0x73c, s.specialRate73c},
        {0x71e, unsigned(s.inlineSample)},
        {0x8c9, a.protection8c9},
        {0xe0, e.animationOffset},
        {0x977, r.cursor},
        {0x97b, r.count},
        {0xb3f, unsigned(m.progress)},
        {0xb47, unsigned(m.travelX)},
        {0xb4b, unsigned(m.travelY)},
        {0xb4f, unsigned(m.deltaX)},
        {0xb53, unsigned(m.deltaY)},
        {0xb57, unsigned(m.heightDelta)},
        {0xb5f, unsigned(m.heightOrigin)},
        {0xb63, e.moveVariant},
        {0xb67, unsigned(m.duration)},
        {0xb6b, unsigned(m.accumulator)},
        {0xb77, e.actionRouteCounters[0]},
        {0xb7b, e.actionRouteCounters[1]},
        {0xb7f, e.actionRouteCounters[2]},
        {0xb83, unsigned(m.initialResidualX)},
        {0xb87, unsigned(m.initialResidualY)},
        {0xb8f, setupBankPointer(raw, s.sampleBank, m.sampleCursor)},
        {0xb93, setupBankPointer(raw, s.initialSampleBank, m.initialCursor)}})
    write32(raw, v.first, v.second);
  for (unsigned i = 0; i < 16; ++i) {
    const auto &q = r.route[i];
    unsigned o = 0x97f + i * 28;
    write32(raw, o, unsigned(q.position.x));
    write32(raw, o + 4, unsigned(q.position.y));
    write32(raw, o + 8, unsigned(q.position.z));
    for (unsigned j = 0; j < 4; ++j)
      write32(raw, o + 12 + j * 4, unsigned(q.remainder[j]));
  }
}
extern "C" unsigned __attribute__((thiscall)) setupPre(void *p) {
  record(404, slot(p));
  return fixture->number % 17 == 0;
}
extern "C" unsigned __attribute__((thiscall)) setupImmediate(void *p) {
  record(405, slot(p));
  return fixture->number % 3 == 0;
}
extern "C" unsigned __attribute__((thiscall)) setupFallback(void *p) {
  record(406, slot(p));
  return fixture->number % 2 == 0;
}
unsigned configSlot(void *p) {
  return unsigned(static_cast<unsigned char *>(p) - configs.data()) / 0x5c9;
}
extern "C" unsigned __attribute__((thiscall))
setupBase(void *p, unsigned mode, int dx, int dy, int dz) {
  record(407, configSlot(p), mode, {dx, dy, dz});
  return rawBase();
}
extern "C" unsigned __attribute__((thiscall)) setupDirection(void *cell, int x,
                                                             int y, int z) {
  const auto &to = setup->route.route[setup->route.cursor].position;
  if (cell != setupTerrain.data() + ((to.z * 16 + to.y) * 16 + to.x) * 12)
    throw std::runtime_error("setup scalar argument target-cell ABI");
  record(408, 0, setupTile(to), {x, y, z});
  return scalarDirection({x, y, z}, to);
}
extern "C" void __attribute__((thiscall))
setupScalar(void *p, unsigned arg, unsigned mode, int dx, int dy, int dz,
            int base, unsigned *rate, int *length) {
  record(409, configSlot(p), arg, {dx, dy, dz});
  record(410, configSlot(p), mode, {base, int(*rate), 0});
  *rate = scalarRate(*rate);
  *length = 7;
}
extern "C" unsigned __attribute__((thiscall))
setupAccept(void *p, int x, int y, int z, int tx, int ty, int tz,
            unsigned *mode, unsigned flag) {
  if (flag)
    throw std::runtime_error("setup acceptance flag");
  record(411, slot(p), 0, {x, y, z});
  record(412, slot(p), 0, {tx, ty, tz});
  *mode = read32(p, 0x10c) + (fixture->number % 13 == 0);
  return fixture->number % 19 != 0;
}
extern "C" unsigned __attribute__((thiscall)) setupHazard(void *p, int x, int y,
                                                          int z) {
  record(413, slot(p), 0, {x, y, z});
  return (x != int(read32(p, 8)) || y != int(read32(p, 12)) ||
          z != int(read32(p, 16))) &&
         fixture->number % 3 != 0;
}
extern "C" void __attribute__((thiscall)) setupRelease(void *p, int x, int y,
                                                       int z) {
  record(414, slot(p), 0, {x, y, z});
}
extern "C" void __attribute__((thiscall)) setupReserve(void *p, int x, int y,
                                                       int z) {
  record(415, slot(p), 0, {x, y, z});
}
extern "C" int __attribute__((fastcall)) setupTurnStub(unsigned old,
                                                       unsigned next) {
  record(416, 0, old, {int(next), 0, 0});
  return setupTurn(old, next);
}
extern "C" unsigned __attribute__((fastcall)) setupDistanceStub(int dx, int dy,
                                                                int dz) {
  record(417, 0, 0, {dx, dy, dz});
  return setupDistance({dx, dy, dz});
}
extern "C" void __attribute__((thiscall))
setupAnimation(void *p, unsigned frame, unsigned reverse) {
  const auto offset = unsigned(static_cast<unsigned char *>(p) - actors.data());
  const unsigned i = offset / 0xe4b;
  if (offset % 0xe4b == 0xb0)
    record(418, i, frame, {int(reverse), 0, 0});
  else if (offset % 0xe4b == 0x7d5 && reverse == 0)
    record(401, i, frame);
  else
    throw std::runtime_error("setup animation ABI");
}
extern "C" void __attribute__((thiscall)) setupBind(void *p, void *definition) {
  unsigned o = unsigned(static_cast<unsigned char *>(p) - actors.data());
  if (o % 0xe4b != 0x7d5 || definition != setupVisual.data() + 276)
    throw std::runtime_error("setup overlay bind ABI");
  record(400, o / 0xe4b, 4);
}
extern "C" void __attribute__((thiscall)) setupSprite(void *p, unsigned face) {
  if (setupAddr(p) != 0xc001)
    throw std::runtime_error("setup sprite ABI");
  record(402, 0, face);
}
extern "C" void __attribute__((thiscall)) setupAux(void *p, unsigned face) {
  unsigned o = unsigned(static_cast<unsigned char *>(p) - actors.data());
  if (o % 0xe4b != 0x825)
    throw std::runtime_error("setup auxiliary ABI");
  record(403, o / 0xe4b, face);
}
extern "C" void __attribute__((thiscall)) setupAttack(void *p, void *target) {
  record(419, slot(p), slot(target));
}
extern "C" void __attribute__((thiscall)) setupAction(void *p, unsigned kind) {
  record(420, slot(p), kind);
  write32(p, 0x5fc, kind);
}
void compareSetup(const SetupFixture &initial, const SetupFixture &expected,
                  const std::vector<Event> &trace) {
  auto input = initial;
  setup = &input;
  fixture = &input.base;
  events.clear();
  prepare();
  for (auto v : {std::pair<unsigned, void *>{
                     0x516ee0, reinterpret_cast<void *>(setupPre)},
                 {0x516d60, reinterpret_cast<void *>(setupImmediate)},
                 {0x511c40, reinterpret_cast<void *>(setupFallback)},
                 {0x505840, reinterpret_cast<void *>(setupBase)},
                 {0x4eb030, reinterpret_cast<void *>(setupDirection)},
                 {0x505920, reinterpret_cast<void *>(setupScalar)},
                 {0x514360, reinterpret_cast<void *>(setupAccept)},
                 {0x522f50, reinterpret_cast<void *>(setupHazard)},
                 {0x5093e0, reinterpret_cast<void *>(setupRelease)},
                 {0x509230, reinterpret_cast<void *>(setupReserve)},
                 {0x4ead10, reinterpret_cast<void *>(setupTurnStub)},
                 {0x4eaca0, reinterpret_cast<void *>(setupDistanceStub)},
                 {0x464cb0, reinterpret_cast<void *>(setupAnimation)},
                 {0x464ca0, reinterpret_cast<void *>(setupBind)},
                 {0x494a50, reinterpret_cast<void *>(setupSprite)},
                 {0x464e20, reinterpret_cast<void *>(setupAux)},
                 {0x50ad10, reinterpret_cast<void *>(setupAttack)},
                 {0x50db60, reinterpret_cast<void *>(setupAction)}})
    redirect(v.first, v.second);
  for (unsigned i = 0; i < input.base.world.creatures.size(); ++i) {
    setupExtra(actors.data() + i * 0xe4b, input.base.world.creatures[i],
               input.base.execution[i], input.motion, input.state, input.route);
    for (unsigned j = 0; j < 48; ++j)
      write32(configs.data() + i * 0x5c9, 0xd8 + j * 4,
              unsigned(setupSample(0xd8 + (j / 12) * 48, j % 12)));
  }
  setupTerrain.assign(16 * 16 * 4 * 12, 0);
  setupMetadata.assign(4 * 356, 0);
  for (unsigned z = 0; z < 4; ++z)
    for (unsigned y = 0; y < 16; ++y)
      for (unsigned x = 0; x < 16; ++x) {
        unsigned short k = setupTile({int(x), int(y), int(z)});
        std::memcpy(setupTerrain.data() + ((z * 16 + y) * 16 + x) * 12, &k, 2);
      }
  for (unsigned i = 0; i < 4; ++i) {
    global(0x6cb8c2 + i * 4, i * 256);
    write32(setupMetadata.data() + i * 356, 0x94, unsigned(setupHeight(i)));
  }
  for (unsigned i = 0; i < 16; ++i)
    global(0x6cb942 + i * 4, i * 16);
  global(0x6c54dc, setupAddr(setupTerrain.data()));
  global(0x65660c, setupAddr(setupMetadata.data()));
  global(0x6e9850, input.environment.cameraTurn);
  global(0x6c007d, input.environment.defaultScalar);
  global(0x6898d0, setupAddr(setupVisualDefinition.data()));
  write32(setupVisualDefinition.data(), 0x160, 2);
  write32(setupVisualDefinition.data(), 0x164, 1);
  global(0x6894a0, setupAddr(setupVisual.data()));
  for (unsigned type = 0; type < 28; ++type)
    global(0x6a5f80 + type * 0x5c9 + 0x5ad,
           input.base.world.creatures[0].hazardSensitive);
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      *reinterpret_cast<unsigned char *>(0x643a80 + i * 0x154 + 0x18 + j) =
          input.base.world.relation[i][j];
  auto before = actors, configBefore = configs, terrainBefore = setupTerrain,
       metadataBefore = setupMetadata, visualBefore = setupVisual,
       visualDefinitionBefore = setupVisualDefinition;
  using F = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<F>(0x510e80)(actors.data());
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "setup trace n=" << input.base.number << " i=" << i
              << " actual=" << events.size() << " expected=" << trace.size()
              << '\n';
    for (auto &v : events)
      std::cerr << "A " << v.kind << "," << v.argument
                << " xyz=" << v.position.x << "," << v.position.y << ","
                << v.position.z << '\n';
    for (auto &v : trace)
      std::cerr << "E " << v.kind << "," << v.argument
                << " xyz=" << v.position.x << "," << v.position.y << ","
                << v.position.z << '\n';
    throw std::runtime_error("setup service order");
  }
  for (unsigned i = 0; i < expected.base.world.creatures.size(); ++i) {
    auto *raw = before.data() + i * 0xe4b;
    packActor(raw, expected.base.world.creatures[i],
              expected.base.execution[i]);
    setupExtra(raw, expected.base.world.creatures[i],
               expected.base.execution[i], i ? initial.motion : expected.motion,
               i ? initial.state : expected.state, initial.route);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "setup actor n=" + std::to_string(input.base.number) +
        " offset=" + std::to_string(i % 0xe4b) + " actual=" +
        std::to_string(read32(actors.data() + i - i % 0xe4b, i % 0xe4b)) +
        " expected=" +
        std::to_string(read32(before.data() + i - i % 0xe4b, i % 0xe4b)));
  }
  if (configs != configBefore || setupTerrain != terrainBefore ||
      setupMetadata != metadataBefore || setupVisual != visualBefore ||
      setupVisualDefinition != visualDefinitionBefore)
    throw std::runtime_error("setup storage guard");
  if (std::memcmp(reinterpret_cast<void *>(0x510e80), setupBody.data(),
                  setupBody.size()))
    throw std::runtime_error("setup original body modified");
}
#endif
unsigned setupRefusals() {
  unsigned refusals = 0;
  for (unsigned kind = 0; kind < 10; ++kind) {
    auto f = generateSetup(11, 0);
    setup = &f;
    fixture = &f.base;
    events.clear();
    auto &a = f.base.world.creatures[0];
    auto &e = f.base.execution[0];
    f.route.cursor = 0;
    f.route.count = 1;
    f.route.route[0] = {{4, 4, 2}, {2, 0, 0, 2}};
    a.block723 = a.form = 1;
    a.formCapability = 0;
    a.rangedDistance = 1;
    a.ai.target = 1;
    a.ai.owner = 0;
    a.ai.type = 12;
    a.ai.action = 15;
    f.base.world.creatures[1].ai.active = true;
    f.base.world.creatures[1].ai.health = 400;
    f.base.world.creatures[1].ai.owner = 1;
    e.block722 = 1;
    auto services = setupServices();
    services.precheck = [](auto &) { return false; };
    services.accept = [](auto &, Cell, Cell, unsigned &mode) {
      mode = 2;
      return true;
    };
    switch (kind) {
    case 0:
      f.route.count = 17;
      break;
    case 1:
      f.route.route[0].remainder[3] = 5;
      break;
    case 2:
      f.base.world.dimensions.x = 0;
      break;
    case 3:
      a.ai.position.x = -1;
      break;
    case 4:
      f.route.route[0].position.z = 4;
      break;
    case 5:
      services.distance = [](Cell) { return 0; };
      break;
    case 6:
      a.ai.target = int(f.base.world.creatures.size());
      break;
    case 7:
      a.ai.owner = -1;
      break;
    case 8:
      f.base.world.creatures[1].ai.owner = -1;
      break;
    case 9:
      a.formCapability = 1;
      services.sample = [](const auto &, unsigned, unsigned) -> int {
        throw std::out_of_range("missing owned sample");
      };
      break;
    }
    bool refused = false;
    try {
      initializeMotionSegment(a, e, f.motion, f.state, f.route, f.base.world,
                              f.environment, services);
    } catch (const std::invalid_argument &) {
      refused = kind != 6 && kind != 9;
    } catch (const std::out_of_range &) {
      refused = kind == 6 || kind == 9;
    }
    if (!refused)
      throw std::runtime_error("native setup refusal absent " +
                               std::to_string(kind));
    ++refusals;
  }
  return refusals;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE required");
  map_image(read(argv[1]));
  setupBody = Bytes(reinterpret_cast<unsigned char *>(0x510e80),
                    reinterpret_cast<unsigned char *>(0x511c40));
#else
  (void)argc;
  (void)argv;
#endif
  std::map<std::string, unsigned> coverage;
  unsigned cases = 0;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 3; ++op) {
      auto input = generateSetup(n, op), expected = input;
      setup = &expected;
      fixture = &expected.base;
      events.clear();
      initializeMotionSegment(
          expected.base.world.creatures[0], expected.base.execution[0],
          expected.motion, expected.state, expected.route, expected.base.world,
          expected.environment, setupServices());
      auto trace = events;
      for (auto &v : trace)
        ++coverage["service_" + std::to_string(v.kind)];
      const auto &a = expected.base.world.creatures[0];
      const auto &e = expected.base.execution[0];
      if (a.ai.action == 2 &&
          e.animationOffset != input.base.execution[0].animationOffset)
        ++coverage["profile_" + std::to_string(e.mode10c) + "_vertical_" +
                   std::to_string(expected.state.vertical != 0) + "_type12_" +
                   std::to_string(a.ai.type == 12)];
      if (expected.motion.progress == 47)
        ++coverage["continuity"];
      if (input.base.execution[0].block722 &&
          expected.motion.initialCursor != input.motion.initialCursor)
        ++coverage["reverse"];
      mix(a.ai.action);
      mix(a.ai.marker);
      mix(a.ai.flying);
      mix(a.ai.motion);
      mix(unsigned(a.cooldown));
      mix(e.animationOffset);
      mix(e.animationFacing);
      mix(e.mode10c);
      mix(e.moveVariant);
      mix(e.previousMove);
      mix(unsigned(expected.motion.accumulator));
      mix(unsigned(expected.motion.duration));
      mix(unsigned(expected.motion.progress));
      mix(unsigned(expected.motion.travelX));
      mix(unsigned(expected.motion.travelY));
      mix(unsigned(expected.motion.heightDelta));
      mix(unsigned(expected.motion.heightOrigin));
      mix(unsigned(expected.motion.initialResidualX));
      mix(unsigned(expected.motion.initialResidualY));
      mix(expected.motion.sampleCursor);
      mix(expected.motion.initialCursor);
      mix(expected.state.sampleBank);
      mix(expected.state.initialSampleBank);
      mix(unsigned(expected.state.inlineSample));
      for (auto v : e.actionRouteCounters)
        mix(v);
      for (auto &v : trace) {
        mix(v.kind);
        mix(v.slot);
        mix(v.argument);
        mix(unsigned(v.position.x));
        mix(unsigned(v.position.y));
        mix(unsigned(v.position.z));
      }
#ifdef MNM_EXECUTION_REFERENCE
      compareSetup(input, expected, trace);
#endif
      ++cases;
    }
  for (unsigned service = 400; service <= 420; ++service)
    if (!coverage["service_" + std::to_string(service)])
      throw std::runtime_error("missing setup service " +
                               std::to_string(service));
  if (!coverage["continuity"] || !coverage["reverse"])
    throw std::runtime_error("setup continuity/reverse absent");
  const unsigned refusals = setupRefusals();
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"native_refusal_cases\":" << refusals
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
