#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include "../reconstruction/creature-ai/special_flight_actions.hpp"
#include <map>
namespace {
struct SegmentFixture {
  Fixture base;
  MotionActionState motion;
  MovementBehaviorState landing;
  SegmentCompletionState state;
  std::array<unsigned, 28> widths{};
};
SegmentFixture *segment;
SegmentFixture generateSegment(unsigned n) {
  SegmentFixture f;
  f.base = generate(n);
  f.state.count = 1 + n / 7 % 16;
  f.state.cursor = n % f.state.count;
  for (unsigned i = 0; i < 16; ++i)
    f.state.route[i] = {
        {int((n + i) % 16), int((n / 16 + i) % 16), int((n + i) % 4)},
        {int(i % 8), int(i % 3) - 1, int(i % 2), int(i % 5)}};
  for (unsigned i = 0; i < 28; ++i)
    f.widths[i] = 1 + (n / 28 + i) % 2;
  f.landing.landingKind = n % 4;
  return f;
}
unsigned tileKind(Cell p) {
  return unsigned(p.x + 3 * p.y + 5 * p.z + int(fixture->number)) % 4;
}
int tileHeight(unsigned k) { return int(k) * 3 - 7; }
int largeHeight(Cell p) {
  return (p.x + p.y + p.z + int(fixture->number)) % 33 - 16;
}
PositionSnapServices snapServices() {
  return {[](Cell p) { return tileKind(p); },
          [](unsigned k) { return tileHeight(k); },
          [](Cell p) {
            record(310, 0, 0, p);
            return largeHeight(p);
          }};
}
struct FlightFixture {
  SegmentFixture segment;
  SpecialFlightEnvironment environment;
  unsigned randomCursor = 0;
};
FlightFixture *flight;
unsigned flightRandom() {
  return fixture->number * 2654435761u + (++flight->randomCursor) * 2246822519u;
}
bool flightOccupied(const AutonomousCreature &a, Cell p) {
  const auto d = flight->environment.dimensions;
  return p.x < 0 || p.x >= d.x || p.y < 0 || p.y >= d.y || p.z < 0 ||
         p.z >= d.z ||
         unsigned(p.x + 3 * p.y + 5 * p.z + int(fixture->number) +
                  int(a.width)) %
                 7 ==
             0;
}
bool flightPassable(Cell p) {
  return unsigned(p.x + 3 * p.y + 5 * p.z + int(fixture->number)) % 5 != 0;
}
bool flightGround(Cell p) {
  return p.z == 0 || unsigned(p.x + p.y + p.z + int(fixture->number)) % 3 == 0;
}
int flightRegion(Cell p) {
  return unsigned(p.x + 3 * p.y + p.z + int(fixture->number)) % 5 == 0
             ? -1
             : (p.x + p.y + p.z) % 3;
}
SpecialFlightServices flightServices() {
  SpecialFlightServices s;
  s.poll = [](auto &a, bool overlay) {
    record(700, a.ai.slot, overlay);
    return overlay ? -1 : int(fixture->number % 3);
  };
  s.rewind = [](auto &a) { record(701, a.ai.slot); };
  s.facing = [](auto &a) {
    record(702, a.ai.slot);
    fixture->execution[a.ai.slot].animationFacing =
        (flight->segment.motion.direction - 2 * (fixture->number / 13 % 4)) &
        7u;
  };
  s.animation = [](auto &a, unsigned frame, unsigned flags) {
    record(703, a.ai.slot, frame, {int(flags), 0, 0});
  };
  s.completeSegment = [](auto &a) {
    record(704, a.ai.slot);
    a.ai.pending = fixture->number % 17;
    if (fixture->number % 7 == 0)
      a.ai.active = false;
  };
  s.stun = [](auto &a, unsigned duration, unsigned mode, unsigned flags) {
    record(705, a.ai.slot, duration, {int(mode), int(flags), 0});
  };
  s.occupied = [](const auto &a, Cell p) {
    record(706, a.ai.slot, 0, p);
    return flightOccupied(a, p);
  };
  s.passable = [](const auto &a, Cell p) {
    record(707, a.ai.slot, 0, p);
    return flightPassable(p);
  };
  s.ground = [](const auto &a, Cell p) {
    record(708, a.ai.slot, 0, p);
    return flightGround(p);
  };
  s.regionAt = [](const auto &a, Cell p) {
    record(709, a.ai.slot, 0, p);
    return flightRegion(p);
  };
  s.regionRoute = [](const auto &a, int from, int to) {
    record(710, a.ai.slot, 0, {from, to, 0});
    return fixture->number % 3 != 0;
  };
  s.random = []() {
    record(711, 0);
    return flightRandom();
  };
  s.diagnostic = [](unsigned line) { record(712, 0, line); };
  s.release = [](auto &a, Cell p) { record(713, a.ai.slot, 0, p); };
  s.reserve = [](auto &a, Cell p) { record(714, a.ai.slot, 0, p); };
  s.snap = [](auto &a, Cell p, unsigned type) {
    record(715, a.ai.slot, type, p);
    snapCreaturePosition(a, p, flight->segment.widths.at(type), snapServices());
  };
  s.relocateDead = [](auto &a, Cell p) {
    record(716, a.ai.slot, 0, p);
    a.ai.position = p;
  };
  s.environment = [](auto &a) {
    record(717, a.ai.slot);
    if (fixture->number % 19 == 0)
      a.ai.active = false;
  };
  s.animationUpdate = [](auto &a) {
    record(718, a.ai.slot);
    ++a.cooldown;
  };
  s.terrain = [](auto &a) { record(719, a.ai.slot); };
  s.reset = [](auto &a) { record(720, a.ai.slot); };
  s.adjustedLanding = [](const auto &a, Cell &p, unsigned movementKind,
                         bool flag) {
    if (movementKind != 3 || !flag)
      throw std::runtime_error("flight landing query overrides");
    record(721, a.ai.slot, 0, p);
    p.x = (p.x + 1) % flight->environment.dimensions.x;
    p.y = (p.y + 2) % flight->environment.dimensions.y;
    return fixture->number % 4 != 0;
  };
  s.response = [](auto &a) {
    record(722, a.ai.slot);
    return fixture->number % 7 == 0;
  };
  s.behavior = [](auto &a, unsigned kind) {
    record(723, a.ai.slot, kind);
    a.ai.behavior = kind;
  };
  s.fallDamage = [](auto &a, unsigned kind, int z) {
    record(724, a.ai.slot, kind, {z, 0, 0});
    return fixture->number * 7 + 11;
  };
  s.defended = [](auto &a, unsigned amount, int source, bool flag) {
    record(725, a.ai.slot, amount, {source, flag, 0});
    if (fixture->number % 17 == 0) {
      a.ai.health = 0;
      a.ai.active = false;
    }
  };
  return s;
}
FlightFixture generateFlight(unsigned n) {
  FlightFixture f;
  f.segment = generateSegment(n);
  auto &b = f.segment.base;
  auto &a = b.world.creatures[0];
  auto &e = b.execution[0];
  f.environment = {{n / 17 % 2 ? 12 : 16, n / 19 % 2 ? 14 : 16, 8},
                   n / 23 % 9,
                   n / 5 % 2 != 0};
  b.world.dimensions = f.environment.dimensions;
  a.ai.position = {int(n % unsigned(b.world.dimensions.x)),
                   int(n / 16 % unsigned(b.world.dimensions.y)), int(n % 5)};
  a.ai.active = true;
  a.ai.health = n / 3 % 3 ? 400 : 0;
  a.ai.corpseA = n / 7 % 2;
  a.ai.corpseB = n / 11 % 2;
  constexpr unsigned classes[] = {0, 24, 25, 26, 1, 2, 3};
  a.ai.kind = classes[n / 3 % 7];
  constexpr unsigned actions[] = {10, 20, 23, 24};
  a.ai.action = actions[n / 7 % 4];
  a.ai.engaged = n % 2;
  a.ai.target = int(n % b.world.creatures.size());
  a.alternateTarget = int((n + 1) % b.world.creatures.size());
  for (auto &v : b.world.creatures)
    v.ai.incoming = n / 29 % 3;
  a.ai.motion = n / 2 % 2;
  a.block71c = n / 17 % 2;
  a.mode = n / 31 % 3;
  a.flag104 = n % 3;
  e.block722 = 1;
  e.block962 = 2;
  e.block963 = 9;
  e.absorptionTimer = 17;
  a.height = 1 + n / 13 % 4;
  a.width = 1 + n / 23 % 2;
  a.movementKind = n / 3 % 4;
  a.formCapability = 1;
  f.segment.motion.direction = n % 37 == 0   ? 0x7fffffffu
                               : n % 41 == 0 ? 0xffffffffu
                                             : n % 11;
  a.finePosition = {37, -39, 48};
  return f;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes flightBodies[3];
unsigned addr(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
void flightExtra(void *raw, const AutonomousCreature &a,
                 const CreatureExecution &e, const MotionActionState &m,
                 const MovementBehaviorState &landing,
                 const SegmentCompletionState &r) {
  write32(raw, 0x14, unsigned(a.finePosition.x));
  write32(raw, 0x18, unsigned(a.finePosition.y));
  write32(raw, 0x1c, unsigned(a.finePosition.z));
  write32(raw, 0x104, a.flag104);
  write32(raw, 0x7cd, a.animationKind);
  write32(raw, 0xa4, landing.landingKind);
  write32(raw, 0x977, r.cursor);
  write32(raw, 0x97b, r.count);
  for (unsigned i = 0; i < 16; ++i) {
    const auto &p = r.route[i];
    unsigned o = 0x97f + i * 28;
    write32(raw, o, unsigned(p.position.x));
    write32(raw, o + 4, unsigned(p.position.y));
    write32(raw, o + 8, unsigned(p.position.z));
    for (unsigned j = 0; j < 4; ++j)
      write32(raw, o + 12 + j * 4, unsigned(p.remainder[j]));
  }
  write32(raw, 0x608, m.direction);
  write32(raw, 0x60c, e.animationFacing);
  write32(raw, 0xe0, e.animationOffset);
  write32(raw, 0xf0, a.ai.corpseA);
  write32(raw, 0xf4, a.ai.corpseB);
}
unsigned flightSlot(void *p, unsigned offset) {
  return unsigned(static_cast<unsigned char *>(p) - actors.data() - offset) /
         0xe4b;
}
extern "C" int __attribute__((thiscall)) pollFlightStub(void *p) {
  const unsigned offset =
      unsigned(static_cast<unsigned char *>(p) - actors.data()) % 0xe4b;
  bool overlay = offset == 0x7d5;
  if (!overlay && offset != 0xb0)
    throw std::runtime_error("flight poll ABI");
  record(700, flightSlot(p, offset), overlay);
  return overlay ? -1 : int(fixture->number % 3);
}
extern "C" void __attribute__((thiscall)) rewindFlightStub(void *p) {
  record(701, flightSlot(p, 0xb0));
}
extern "C" void __attribute__((thiscall)) facingFlightStub(void *p) {
  record(702, slot(p));
  write32(p, 0x60c, (read32(p, 0x608) - 2 * (fixture->number / 13 % 4)) & 7u);
}
extern "C" void __attribute__((thiscall))
animationFlightStub(void *p, unsigned frame, unsigned flags) {
  record(703, flightSlot(p, 0xb0), frame, {int(flags), 0, 0});
}
extern "C" void __attribute__((thiscall)) completeFlightStub(void *p) {
  record(704, slot(p));
  write32(p, 0x5f0, fixture->number % 17);
  if (fixture->number % 7 == 0)
    write32(p, 4, 0);
}
extern "C" void __attribute__((thiscall))
stunFlightStub(void *p, unsigned duration, unsigned mode, unsigned flags) {
  record(705, slot(p), duration, {int(mode), int(flags), 0});
}
extern "C" unsigned __attribute__((thiscall))
occupiedFlightStub(void *p, int x, int y, int z, unsigned height,
                   unsigned width, unsigned short id) {
  const auto &a = fixture->world.creatures[0];
  if (p != reinterpret_cast<void *>(0x6c5490) || height != a.height ||
      width != a.width || id != a.ai.slot)
    throw std::runtime_error("flight footprint ABI");
  record(706, 0, 0, {x, y, z});
  return flightOccupied(a, {x, y, z});
}
extern "C" unsigned __attribute__((thiscall)) passableFlightStub(void *p, int x,
                                                                 int y, int z) {
  record(707, slot(p), 0, {x, y, z});
  return flightPassable({x, y, z});
}
extern "C" unsigned __attribute__((thiscall)) groundFlightStub(void *p, int x,
                                                               int y, int z) {
  record(708, slot(p), 0, {x, y, z});
  return flightGround({x, y, z});
}
struct FlightRegionWire {
  unsigned char bytes[84];
};
void flightRegionQuery(const FlightRegionWire &q) {
  if (read32(q.bytes, 0) || read32(q.bytes, 4) != addr(actors.data()))
    throw std::runtime_error("flight region query ABI");
}
extern "C" int __attribute__((thiscall)) regionFlightStub(void *p, int x, int y,
                                                          int z,
                                                          FlightRegionWire q,
                                                          int dx, int dy) {
  if (p != reinterpret_cast<void *>(0x6eaf08) || dx || dy)
    throw std::runtime_error("flight region lookup ABI");
  flightRegionQuery(q);
  record(709, 0, 0, {x, y, z});
  return flightRegion({x, y, z});
}
extern "C" unsigned __attribute__((thiscall))
regionRouteFlightStub(void *p, int from, int to, FlightRegionWire q, int *out,
                      unsigned budget, unsigned flag) {
  if (p != reinterpret_cast<void *>(0x6eaf08) || budget != 100 || flag != 1)
    throw std::runtime_error("flight regional route ABI");
  flightRegionQuery(q);
  for (unsigned i = 0; i < 8; ++i)
    out[i] = int(i);
  record(710, 0, 0, {from, to, 0});
  return fixture->number % 3 != 0;
}
extern "C" unsigned __attribute__((thiscall)) randomFlightStub(void *p) {
  if (p != reinterpret_cast<void *>(0x6e1ac8))
    throw std::runtime_error("flight random ABI");
  record(711, 0);
  return flightRandom();
}
extern "C" void __attribute__((thiscall))
diagnosticFlightStub(void *p, void *file, unsigned line) {
  if (p != reinterpret_cast<void *>(0x657d78) ||
      file != reinterpret_cast<void *>(0x5e9c10))
    throw std::runtime_error("flight diagnostic ABI");
  record(712, 0, line);
}
extern "C" void __attribute__((thiscall)) releaseFlightStub(void *p, int x,
                                                            int y, int z) {
  record(713, slot(p), 0, {x, y, z});
}
extern "C" void __attribute__((thiscall)) reserveFlightStub(void *p, int x,
                                                            int y, int z) {
  record(714, slot(p), 0, {x, y, z});
}
extern "C" void __attribute__((thiscall)) snapFlightStub(void *p, int x, int y,
                                                         int z, unsigned type) {
  record(715, slot(p), type, {x, y, z});
  write32(p, 8, unsigned(x));
  write32(p, 12, unsigned(y));
  write32(p, 16, unsigned(z));
  write32(p, 0x14, unsigned(x) * 32u);
  write32(p, 0x18, unsigned(y) * 32u);
  int fine = z * 16;
  unsigned k = tileKind({x, y, z});
  if (flight->segment.widths.at(type) == 2)
    fine += largeHeight({x, y, z});
  else if (k)
    fine += tileHeight(k);
  if (flight->segment.widths.at(type) == 2)
    record(310, 0, 0, {x, y, z});
  write32(p, 0x1c, unsigned(fine));
}
extern "C" void __attribute__((thiscall)) deadFlightStub(void *p, int x, int y,
                                                         int z) {
  record(716, slot(p), 0, {x, y, z});
  write32(p, 8, unsigned(x));
  write32(p, 12, unsigned(y));
  write32(p, 16, unsigned(z));
}
extern "C" void __attribute__((thiscall)) environmentFlightStub(void *p) {
  record(717, slot(p));
  if (fixture->number % 19 == 0)
    write32(p, 4, 0);
}
extern "C" void __attribute__((thiscall)) updateFlightStub(void *p) {
  record(718, slot(p));
  write32(p, 0x66c, read32(p, 0x66c) + 1);
}
extern "C" void __attribute__((thiscall)) terrainFlightStub(void *p) {
  record(719, slot(p));
}
extern "C" void __attribute__((thiscall)) resetFlightStub(void *p) {
  record(720, slot(p));
}
extern "C" unsigned __attribute__((thiscall))
adjustedFlightStub(void *, unsigned id, void *d, int *x, int *y, int *z) {
  const auto &a = fixture->world.creatures[0];
  if (id != a.ai.slot || read32(d, 0) != a.height || read32(d, 4) != a.width ||
      read32(d, 8) || read32(d, 12) != 3 || read32(d, 17) != a.ai.type ||
      static_cast<unsigned char *>(d)[16] != 1 ||
      static_cast<unsigned char *>(d)[21] !=
          unsigned(a.block723 || a.ai.behavior == 14))
    throw std::runtime_error("flight adjusted landing ABI");
  record(721, 0, 0, {*x, *y, *z});
  *x = (*x + 1) % flight->environment.dimensions.x;
  *y = (*y + 2) % flight->environment.dimensions.y;
  return fixture->number % 4 != 0;
}
extern "C" unsigned __attribute__((thiscall)) responseFlightStub(void *p) {
  record(722, slot(p));
  return fixture->number % 7 == 0;
}
extern "C" void __attribute__((thiscall)) behaviorFlightStub(void *p,
                                                             unsigned kind) {
  record(723, slot(p), kind);
  write32(p, 0x5ec, kind);
}
extern "C" unsigned __attribute__((thiscall))
fallFlightStub(void *p, unsigned kind, int z) {
  record(724, slot(p), kind, {z, 0, 0});
  return fixture->number * 7 + 11;
}
extern "C" void __attribute__((thiscall))
defendedFlightStub(void *p, unsigned amount, int source, unsigned flag) {
  record(725, slot(p), amount, {source, int(flag), 0});
  if (source != -1 || flag)
    throw std::runtime_error("flight defended ABI");
  if (fixture->number % 17 == 0) {
    write32(p, 0xe4, 0);
    write32(p, 4, 0);
  }
}
void compareFlight(const FlightFixture &initial, const FlightFixture &expected,
                   unsigned op, const std::vector<Event> &trace) {
  auto input = initial;
  flight = &input;
  segment = &input.segment;
  fixture = &input.segment.base;
  events.clear();
  prepare();
  for (auto v : {std::pair<unsigned, void *>{
                     0x464ec0, reinterpret_cast<void *>(pollFlightStub)},
                 {0x464d70, reinterpret_cast<void *>(rewindFlightStub)},
                 {0x50b1f0, reinterpret_cast<void *>(facingFlightStub)},
                 {0x464cb0, reinterpret_cast<void *>(animationFlightStub)},
                 {0x512460, reinterpret_cast<void *>(completeFlightStub)},
                 {0x50d060, reinterpret_cast<void *>(stunFlightStub)},
                 {0x4f13f0, reinterpret_cast<void *>(occupiedFlightStub)},
                 {0x414940, reinterpret_cast<void *>(passableFlightStub)},
                 {0x414890, reinterpret_cast<void *>(groundFlightStub)},
                 {0x53d7d0, reinterpret_cast<void *>(regionFlightStub)},
                 {0x53e5e0, reinterpret_cast<void *>(regionRouteFlightStub)},
                 {0x54e200, reinterpret_cast<void *>(randomFlightStub)},
                 {0x485b10, reinterpret_cast<void *>(diagnosticFlightStub)},
                 {0x5093e0, reinterpret_cast<void *>(releaseFlightStub)},
                 {0x509230, reinterpret_cast<void *>(reserveFlightStub)},
                 {0x5070e0, reinterpret_cast<void *>(snapFlightStub)},
                 {0x509630, reinterpret_cast<void *>(deadFlightStub)},
                 {0x51a8f0, reinterpret_cast<void *>(environmentFlightStub)},
                 {0x50f850, reinterpret_cast<void *>(updateFlightStub)},
                 {0x51aa30, reinterpret_cast<void *>(terrainFlightStub)},
                 {0x50ab30, reinterpret_cast<void *>(resetFlightStub)},
                 {0x4f5230, reinterpret_cast<void *>(adjustedFlightStub)},
                 {0x513af0, reinterpret_cast<void *>(responseFlightStub)},
                 {0x50aa30, reinterpret_cast<void *>(behaviorFlightStub)},
                 {0x513c20, reinterpret_cast<void *>(fallFlightStub)},
                 {0x514860, reinterpret_cast<void *>(defendedFlightStub)}})
    redirect(v.first, v.second);
  for (unsigned i = 0; i < input.segment.base.world.creatures.size(); ++i)
    flightExtra(actors.data() + i * 0xe4b,
                input.segment.base.world.creatures[i],
                input.segment.base.execution[i], input.segment.motion,
                input.segment.landing, input.segment.state);
  global(0x6e1f30, input.environment.diagnostic);
  global(0x6c5c80, input.environment.waterline);
  auto before = actors;
  using F = void(__attribute__((thiscall)) *)(void *);
  constexpr unsigned entries[] = {0x521790, 0x5217e0, 0x521dd0};
  reinterpret_cast<F>(entries[op])(actors.data());
  if (events != trace) {
    std::cerr << "flight n=" << input.segment.base.number << " op=" << op
              << '\n';
    for (auto &v : events)
      std::cerr << "A " << v.kind << "," << v.argument
                << " xyz=" << v.position.x << "," << v.position.y << ","
                << v.position.z << '\n';
    for (auto &v : trace)
      std::cerr << "E " << v.kind << "," << v.argument
                << " xyz=" << v.position.x << "," << v.position.y << ","
                << v.position.z << '\n';
    throw std::runtime_error("flight ordered effects");
  }
  for (unsigned i = 0; i < expected.segment.base.world.creatures.size(); ++i) {
    auto *p = before.data() + i * 0xe4b;
    packActor(p, expected.segment.base.world.creatures[i],
              expected.segment.base.execution[i]);
    flightExtra(p, expected.segment.base.world.creatures[i],
                expected.segment.base.execution[i],
                i ? initial.segment.motion : expected.segment.motion,
                i ? initial.segment.landing : expected.segment.landing,
                initial.segment.state);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "flight actor n=" + std::to_string(input.segment.base.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  if (input.randomCursor != expected.randomCursor)
    throw std::runtime_error("flight random cursor");
  for (unsigned i = 0; i < 3; ++i)
    if (std::memcmp(reinterpret_cast<void *>(entries[i]),
                    flightBodies[i].data(), flightBodies[i].size()))
      throw std::runtime_error("original flight body modified");
}
#endif
unsigned flightRefusals() {
  unsigned refused = 0;
  for (unsigned kind = 0; kind < 5; ++kind) {
    auto f = generateFlight(11);
    flight = &f;
    segment = &f.segment;
    fixture = &f.segment.base;
    events.clear();
    auto &a = fixture->world.creatures[0];
    auto &e = fixture->execution[0];
    auto services = flightServices();
    switch (kind) {
    case 0:
      f.environment.dimensions.x = 0;
      break;
    case 1:
      f.environment.dimensions.y = 0;
      break;
    case 2:
      a.ai.position.z = 0;
      a.height = 1;
      a.ai.motion = true;
      f.segment.state.cursor = 16;
      services.occupied = [](const auto &, Cell) { return false; };
      services.passable = [](const auto &, Cell) { return true; };
      break;
    case 3:
      a.ai.type = 0;
      a.ai.engaged = 1;
      a.ai.action = 10;
      a.ai.target = int(fixture->world.creatures.size());
      services.occupied = [](const auto &, Cell) { return true; };
      break;
    case 4:
      f.environment.dimensions.z = 4100;
      a.ai.position.z = 4097;
      a.height = 4;
      a.ai.kind = 0;
      services.occupied = [](const auto &, Cell) { return false; };
      services.passable = [](const auto &, Cell) { return true; };
      services.ground = [](const auto &, Cell) { return false; };
      break;
    }
    bool rejected = false;
    try {
      if (kind == 1)
        tickDescendingAction(a, e, f.segment.state, f.segment.landing,
                             f.environment, services);
      else
        tickAscendingAction(a, e, f.segment.motion, f.segment.landing,
                            f.segment.state, fixture->world, f.environment,
                            services);
    } catch (const std::invalid_argument &) {
      rejected = kind == 0 || kind == 1 || kind == 4;
    } catch (const std::out_of_range &) {
      rejected = kind == 2 || kind == 3;
    }
    if (!rejected)
      throw std::runtime_error("flight native refusal absent " +
                               std::to_string(kind));
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
  constexpr unsigned start[] = {0x521790, 0x5217e0, 0x521dd0},
                     end[] = {0x5217e0, 0x521dc9, 0x5221fa};
  for (unsigned i = 0; i < 3; ++i)
    flightBodies[i] = Bytes(reinterpret_cast<unsigned char *>(start[i]),
                            reinterpret_cast<unsigned char *>(end[i]));
#else
  (void)argc;
  (void)argv;
#endif
  std::map<std::string, unsigned> coverage;
  unsigned cases = 0;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 3; ++op) {
      auto input = generateFlight(n), expected = input;
      flight = &expected;
      segment = &expected.segment;
      fixture = &expected.segment.base;
      events.clear();
      auto &a = fixture->world.creatures[0];
      auto &e = fixture->execution[0];
      auto s = flightServices();
      if (!op)
        tickFinishingAction(a, e, s);
      else if (op == 1)
        tickAscendingAction(a, e, expected.segment.motion,
                            expected.segment.landing, expected.segment.state,
                            fixture->world, expected.environment, s);
      else
        tickDescendingAction(a, e, expected.segment.state,
                             expected.segment.landing, expected.environment, s);
      auto trace = events;
      for (auto &v : trace)
        ++coverage["service_" + std::to_string(v.kind)];
      if (!op && !a.ai.active)
        ++coverage["finish_after_removed"];
      if (op == 1 &&
          a.ai.position.z ==
              input.segment.base.world.creatures[0].ai.position.z + 1)
        ++coverage["direct_ascent"];
      if (op == 1 && expected.randomCursor == 10)
        ++coverage["five_attempts"];
      if (op == 2 && !a.ai.active && !a.flag104 &&
          input.segment.base.world.creatures[0].flag104)
        ++coverage["inactive_before_flag_restore"];
      if (op == 2 && input.segment.base.world.creatures[0].ai.health == 0)
        ++coverage["dead_descent"];
      if (op == 1 && input.segment.base.world.creatures[0].ai.engaged &&
          !a.ai.engaged)
        ++coverage["engagement_release"];
      mix(expected.randomCursor);
      mix(expected.segment.motion.direction);
      mix(expected.segment.landing.landingKind);
      for (auto &v : fixture->world.creatures) {
        mix(v.ai.behavior);
        mix(v.ai.pending);
        mix(v.ai.active);
        mix(v.ai.health);
        mix(v.ai.engaged);
        mix(v.ai.incoming);
        mix(v.ai.flying);
        mix(v.flag104);
        mix(v.mode);
        mix(v.blockDE3);
        mix(unsigned(v.ai.position.x));
        mix(unsigned(v.ai.position.y));
        mix(unsigned(v.ai.position.z));
        mix(unsigned(v.finePosition.x));
        mix(unsigned(v.finePosition.y));
        mix(unsigned(v.finePosition.z));
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
      compareFlight(input, expected, op, trace);
#endif
      ++cases;
    }
  for (unsigned id = 700; id <= 725; ++id)
    if (!coverage["service_" + std::to_string(id)])
      throw std::runtime_error("flight service absent " + std::to_string(id));
  for (const char *branch :
       {"finish_after_removed", "direct_ascent", "five_attempts",
        "inactive_before_flag_restore", "dead_descent", "engagement_release"})
    if (!coverage[branch])
      throw std::runtime_error(std::string("flight branch absent ") + branch);
  const unsigned refused = flightRefusals();
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"native_refusal_cases\":" << refused
            << ",\"functions\":3,\"outcome_digest\":" << digest
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
