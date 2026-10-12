#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#include "../reconstruction/creature-ai/segment_completion.hpp"
#include <map>
namespace {
struct SegmentFixture {
  Fixture base;
  MotionActionState motion;
  MovementBehaviorState landing;
  SegmentCompletionState state;
  SegmentCompletionEnvironment environment;
  std::array<unsigned, 28> widths{};
};
SegmentFixture *segment;
unsigned tileKind(Cell p) {
  return unsigned(p.x + 3 * p.y + 5 * p.z + int(fixture->number)) % 4;
}
int tileHeight(unsigned kind) { return int(kind) * 3 - 7; }
int largeHeight(Cell p) {
  return (p.x + p.y + p.z + int(fixture->number)) % 33 - 16;
}
unsigned scoreAmount() { return fixture->number % 5 ? 37u : 0xfffffff3u; }
unsigned floatWord(float v) {
  unsigned w;
  std::memcpy(&w, &v, 4);
  return w;
}
PositionSnapServices snapServices() {
  return {[](Cell p) { return tileKind(p); },
          [](unsigned k) { return tileHeight(k); },
          [](Cell p) {
            record(310, 0, 0, p);
            return largeHeight(p);
          }};
}
SegmentCompletionServices segmentServices() {
  SegmentCompletionServices s;
  s.stopOverlay = [](auto &a) { record(300, a.ai.slot); };
  s.releaseOverlay = [](unsigned t) { record(301, 0, t); };
  s.fallDamage = [](auto &a, unsigned kind, int z) {
    record(302, a.ai.slot, kind, {z, 0, 0});
    return fixture->number * 7 + 11;
  };
  s.defended = [](auto &a, unsigned amount, int source, bool flag) {
    record(303, a.ai.slot, amount, {source, flag, 0});
    if (fixture->number % 17 == 0) {
      a.ai.health = 0;
      a.ai.active = false;
    }
  };
  s.snap = [](auto &a, Cell p, unsigned type) {
    snapCreaturePosition(a, p, segment->widths.at(type), snapServices());
  };
  s.score = [](unsigned type, float ratio) {
    record(305, 0, type, {int(floatWord(ratio)), 0, 0});
    return scoreAmount();
  };
  s.environment = [](auto &a) {
    record(306, a.ai.slot);
    if (fixture->number % 19 == 0)
      a.ai.active = false;
  };
  s.animation = [](auto &a) {
    record(307, a.ai.slot);
    ++a.cooldown;
  };
  s.terrain = [](auto &a) { record(308, a.ai.slot); };
  s.reset = [](auto &a) { record(309, a.ai.slot); };
  s.regionCandidate = [](auto &a, Cell &p, unsigned parameter, Cell target) {
    record(311, a.ai.slot, parameter, target);
    if (fixture->number % 4 == 0)
      p = a.ai.position;
    else if (fixture->number % 4 == 1)
      p = {3, 4, 2};
  };
  s.legal = [](const auto &a, Cell p) {
    record(312, a.ai.slot, a.block723 || a.ai.behavior == 14, p);
    return (fixture->number / 128) % 3 != 0;
  };
  return s;
}
SegmentFixture generateSegment(unsigned n) {
  SegmentFixture f;
  f.base = generate(n);
  auto &a = f.base.world.creatures[0];
  auto &e = f.base.execution[0];
  auto &p = f.state;
  a.width = 1 + (n / 7) % 2;
  a.height = 1 + (n / 13) % 4;
  a.movementKind = (n / 23) % 8;
  a.flag104 = n % 7;
  a.ai.position.z = 1 + int(n % 3);
  a.ai.position.y = int((n / 16) % 16);
  a.finePosition = {42, -17, 103};
  a.animationKind = (n / 11) % 4;
  a.block71c = n % 2;
  a.block723 = n % 3;
  a.blockDE3 = 4;
  e.mode10c = (n / 2) % 4;
  e.block963 = n % 4 == 3 ? 0xffffffffu : n % 3;
  e.block962 = 2;
  f.motion.heightDelta = -15;
  f.motion.heightStep = 7;
  f.landing.landingKind = n % 4;
  p.count = 1 + (n / 7) % 16;
  p.cursor = n % p.count;
  p.landing110 = (n / 3) % 2;
  p.landingOverlay71d = (n / 5) % 2;
  p.overlayFrame809 = 13;
  p.overlayLink805 = n % 2 ? std::optional<unsigned>(45) : std::nullopt;
  for (unsigned i = 0; i < 16; ++i)
    p.route[i] = {
        {int((n + 3 * i) % 16), int((n / 16 + i) % 16), 1 + int((n + i) % 3)},
        {int(i % 8), int(i % 3) - 1, 71 + int(i), 81 + int(i)}};
  p.scorePendingFc = (n / 3) % 2;
  p.contributionNodes = {
      {{2}, {}},
      {{2}, {}},
      {{}, {n % 2}},
      {{}, (n / 2) % 2 ? std::optional<unsigned>(2) : std::nullopt}};
  p.contributionTotals = {0xfffffffeu, 17, 22};
  p.contributionRoots = {n % 2 ? std::optional<unsigned>(0) : std::nullopt,
                         n % 3 ? std::optional<unsigned>(1)
                               : std::optional<unsigned>(3)};
  p.regionPending = (n / 7) % 2;
  p.regionCursor = n % 50;
  for (unsigned i = 0; i < 50; ++i)
    p.regionPath[i] = int((n + i) % 5) - 1;
  f.environment.regionContinuation = (n / 11) % 2;
  f.environment.scoreNumerator = 37;
  f.environment.scoreDenominator = 3000;
  f.environment.regions = {{{7, 8, 2}, 5}, {{1, 2, 1}, 9}, {{11, 12, 3}, 13}};
  unsigned lookahead = p.count + 2;
  int region = p.regionPath[p.regionCursor];
  if (lookahead < 16 && region >= 0 && region < 3 && (n / 8) % 3 == 0)
    p.route[lookahead].position =
        n % 4 == 1 ? Cell{3, 4, 2}
                   : f.environment.regions[unsigned(region)].position;
  for (unsigned i = 0; i < 28; ++i)
    f.widths[i] = 1 + (n / 28 + i) % 2;
  return f;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes segmentBody, snapBody, terrain, metadata, nodes, owners, regions;
unsigned addr(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
void segmentExtra(void *raw, const AutonomousCreature &a,
                  const MotionActionState &m,
                  const MovementBehaviorState &landing,
                  const SegmentCompletionState &s, bool primary) {
  for (auto v :
       {std::pair<unsigned, unsigned>{0x14, unsigned(a.finePosition.x)},
        {0x18, unsigned(a.finePosition.y)},
        {0x1c, unsigned(a.finePosition.z)},
        {0x104, a.flag104},
        {0x110, s.landing110},
        {0x7cd, a.animationKind},
        {0x809, s.overlayFrame809},
        {0x805, s.overlayLink805.value_or(0)},
        {0xfc, unsigned(s.scorePendingFc)},
        {0xb57, unsigned(m.heightDelta)},
        {0xb5b, unsigned(m.heightStep)},
        {0xa4, landing.landingKind},
        {0x977, s.cursor},
        {0x97b, s.count},
        {0xd03, unsigned(s.regionPending)},
        {0xdcf, s.regionCursor},
        {0x678, primary && s.contributionRoots[0]
                    ? addr(nodes.data() + *s.contributionRoots[0] * 64)
                    : 0},
        {0x67c, primary && s.contributionRoots[1]
                    ? addr(nodes.data() + *s.contributionRoots[1] * 64)
                    : 0}})
    write32(raw, v.first, v.second);
  static_cast<unsigned char *>(raw)[0x71d] = s.landingOverlay71d;
  for (unsigned i = 0; i < 16; ++i) {
    const auto &q = s.route[i];
    unsigned offset = 0x97f + i * 28;
    write32(raw, offset, unsigned(q.position.x));
    write32(raw, offset + 4, unsigned(q.position.y));
    write32(raw, offset + 8, unsigned(q.position.z));
    for (unsigned j = 0; j < 4; ++j)
      write32(raw, offset + 12 + j * 4, unsigned(q.remainder[j]));
  }
  for (unsigned i = 0; i < 50; ++i)
    write32(raw, 0xd07 + i * 4, unsigned(s.regionPath[i]));
}
unsigned spriteActor(void *p) {
  return unsigned(static_cast<unsigned char *>(p) - actors.data()) / 0xe4b;
}
extern "C" void __attribute__((thiscall)) stopSegmentStub(void *p) {
  record(300, spriteActor(p));
}
extern "C" void __attribute__((thiscall))
releaseSegmentStub(void *p, unsigned zero, unsigned one) {
  if (zero || one != 1)
    throw std::runtime_error("segment overlay release ABI");
  record(301, 0, addr(p));
}
extern "C" unsigned __attribute__((thiscall))
fallSegmentStub(void *p, unsigned kind, int z) {
  record(302, slot(p), kind, {z, 0, 0});
  return fixture->number * 7 + 11;
}
extern "C" void __attribute__((thiscall))
defenseSegmentStub(void *p, unsigned amount, int source, unsigned flag) {
  record(303, slot(p), amount, {source, int(flag), 0});
  if (source != -1 || flag)
    throw std::runtime_error("segment defense ABI");
  if (fixture->number % 17 == 0) {
    write32(p, 0xe4, 0);
    write32(p, 4, 0);
  }
}
extern "C" unsigned __attribute__((fastcall))
scoreSegmentStub(unsigned type, unsigned zeroReg, unsigned zeroA,
                 unsigned zeroB, unsigned zeroC, float ratio) {
  if (zeroReg || zeroA || zeroB || zeroC)
    throw std::runtime_error("segment score ABI");
  record(305, 0, type, {int(floatWord(ratio)), 0, 0});
  return scoreAmount();
}
extern "C" void __attribute__((thiscall)) environmentSegmentStub(void *p) {
  record(306, slot(p));
  if (fixture->number % 19 == 0)
    write32(p, 4, 0);
}
extern "C" void __attribute__((thiscall)) animationSegmentStub(void *p) {
  record(307, slot(p));
  write32(p, 0x66c, read32(p, 0x66c) + 1);
}
extern "C" void __attribute__((thiscall)) terrainSegmentStub(void *p) {
  record(308, slot(p));
}
extern "C" void __attribute__((thiscall)) resetSegmentStub(void *p) {
  record(309, slot(p));
}
extern "C" unsigned __attribute__((thiscall))
heightSegmentStub(void *, const void *cell) {
  unsigned index =
      unsigned(static_cast<const unsigned char *>(cell) - terrain.data()) / 12;
  Cell p{int(index % 16), int(index / 16 % 16), int(index / 256)};
  record(310, 0, 0, p);
  return unsigned(largeHeight(p));
}
extern "C" void __attribute__((thiscall))
candidateSegmentStub(void *p, int *x, int *y, int *z, unsigned parameter,
                     int tx, int ty, int tz) {
  record(311, slot(p), parameter, {tx, ty, tz});
  if (fixture->number % 4 == 0) {
    *x = int(read32(p, 8));
    *y = int(read32(p, 12));
    *z = int(read32(p, 16));
  } else if (fixture->number % 4 == 1) {
    *x = 3;
    *y = 4;
    *z = 2;
  }
}
extern "C" unsigned __attribute__((thiscall))
legalSegmentStub(void *, int x, int y, int z, const void *descriptor) {
  const auto &a = fixture->world.creatures[0];
  unsigned special = a.block723 || read32(actors.data(), 0x5ec) == 14;
  if (read32(descriptor, 0) != a.height || read32(descriptor, 4) != a.width ||
      read32(descriptor, 8) != read32(actors.data(), 0x104) ||
      read32(descriptor, 12) != a.movementKind ||
      read32(descriptor, 17) != read32(actors.data(), 0xa8) ||
      static_cast<const unsigned char *>(descriptor)[16] ||
      static_cast<const unsigned char *>(descriptor)[21] != special)
    throw std::runtime_error("segment legality ABI");
  record(312, 0, special, {x, y, z});
  return (fixture->number / 128) % 3 != 0;
}
void installSegmentServices() {
  for (auto v : {std::pair<unsigned, void *>{
                     0x464c80, reinterpret_cast<void *>(stopSegmentStub)},
                 {0x4de090, reinterpret_cast<void *>(releaseSegmentStub)},
                 {0x513c20, reinterpret_cast<void *>(fallSegmentStub)},
                 {0x514860, reinterpret_cast<void *>(defenseSegmentStub)},
                 {0x416d80, reinterpret_cast<void *>(scoreSegmentStub)},
                 {0x51a8f0, reinterpret_cast<void *>(environmentSegmentStub)},
                 {0x50f850, reinterpret_cast<void *>(animationSegmentStub)},
                 {0x51aa30, reinterpret_cast<void *>(terrainSegmentStub)},
                 {0x513cf0, reinterpret_cast<void *>(candidateSegmentStub)},
                 {0x4f4900, reinterpret_cast<void *>(legalSegmentStub)},
                 {0x50ab30, reinterpret_cast<void *>(resetSegmentStub)},
                 {0x4f3260, reinterpret_cast<void *>(heightSegmentStub)}})
    redirect(v.first, v.second);
}
void compareSegment(const SegmentFixture &initial,
                    const SegmentFixture &expected, unsigned op,
                    const std::vector<Event> &trace) {
  auto input = initial;
  segment = &input;
  fixture = &input.base;
  events.clear();
  prepare();
  installSegmentServices();
  nodes.assign(4 * 64, 0xa7);
  owners.assign(3 * 64, 0xb7);
  regions.assign(input.environment.regions.size() * 660, 0xc7);
  for (unsigned i = 0; i < input.state.contributionNodes.size(); ++i) {
    const auto &node = input.state.contributionNodes[i];
    write32(nodes.data() + i * 64, 0x39,
            node.parent ? addr(nodes.data() + *node.parent * 64) : 0);
    write32(nodes.data() + i * 64, 5,
            node.owner ? addr(owners.data() + *node.owner * 64) : 0);
  }
  for (unsigned i = 0; i < 3; ++i)
    write32(owners.data() + i * 64, 0x33, input.state.contributionTotals[i]);
  for (unsigned i = 0; i < input.environment.regions.size(); ++i) {
    const auto &r = input.environment.regions[i];
    write32(regions.data() + i * 660, 4, unsigned(r.position.x));
    write32(regions.data() + i * 660, 8, unsigned(r.position.y));
    write32(regions.data() + i * 660, 12, unsigned(r.position.z));
    write32(regions.data() + i * 660, 0x20c, r.parameter);
  }
  for (unsigned i = 0; i < input.base.world.creatures.size(); ++i)
    segmentExtra(actors.data() + i * 0xe4b, input.base.world.creatures[i],
                 input.motion, input.landing, input.state, i == 0);
  terrain.assign(16 * 16 * 4 * 12, 0);
  metadata.assign(4 * 356, 0);
  for (unsigned z = 0; z < 4; ++z)
    for (unsigned y = 0; y < 16; ++y)
      for (unsigned x = 0; x < 16; ++x) {
        unsigned short k = tileKind({int(x), int(y), int(z)});
        std::memcpy(terrain.data() + ((z * 16 + y) * 16 + x) * 12, &k, 2);
      }
  for (unsigned i = 0; i < 4; ++i) {
    global(0x6cb8c2 + i * 4, i * 256);
    write32(metadata.data() + i * 356, 0x94, unsigned(tileHeight(i)));
  }
  for (unsigned i = 0; i < 16; ++i)
    global(0x6cb942 + i * 4, i * 16);
  global(0x6c54dc, addr(terrain.data()));
  global(0x65660c, addr(metadata.data()));
  for (unsigned i = 0; i < 28; ++i)
    global(0x6a5f80 + i * 0x5c9 + 8, input.widths[i]);
  global(0x5e1908, unsigned(input.environment.scoreNumerator));
  global(0x5e1904, unsigned(input.environment.scoreDenominator));
  global(0x6e1f3c, input.environment.regionContinuation);
  global(0x6eaf08, addr(regions.data()));
  global(0x6eaf0c, input.environment.regions.size());
  auto before = actors, nodeBefore = nodes, ownerBefore = owners,
       regionBefore = regions, terrainBefore = terrain, metaBefore = metadata;
  if (op == 0) {
    using F = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<F>(0x512460)(actors.data());
  } else {
    using F =
        void(__attribute__((thiscall)) *)(void *, int, int, int, unsigned);
    const auto &p = input.state.route[input.state.cursor].position;
    reinterpret_cast<F>(0x5070e0)(actors.data(), p.x, p.y, p.z,
                                  input.base.world.creatures[0].ai.type);
  }
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "segment trace n=" << input.base.number << " op=" << op
              << " i=" << i << " actual=" << events.size()
              << " expected=" << trace.size() << '\n';
    if (i < events.size())
      std::cerr << "actual " << events[i].kind << "," << events[i].argument
                << " xyz=" << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected " << trace[i].kind << "," << trace[i].argument
                << " xyz=" << trace[i].position.x << "," << trace[i].position.y
                << "," << trace[i].position.z << '\n';
    throw std::runtime_error("segment effect order");
  }
  for (unsigned i = 0; i < expected.base.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.base.world.creatures[i],
              expected.base.execution[i]);
    segmentExtra(before.data() + i * 0xe4b, expected.base.world.creatures[i],
                 i == 0 ? expected.motion : initial.motion,
                 i == 0 ? expected.landing : initial.landing,
                 i == 0 ? expected.state : initial.state, i == 0);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "segment actor n=" + std::to_string(input.base.number) +
        " op=" + std::to_string(op) + " offset=" + std::to_string(i % 0xe4b));
  }
  for (unsigned i = 0; i < 3; ++i)
    write32(ownerBefore.data() + i * 64, 0x33,
            expected.state.contributionTotals[i]);
  if (nodeBefore != nodes || ownerBefore != owners || regionBefore != regions ||
      terrainBefore != terrain || metaBefore != metadata)
    throw std::runtime_error("segment owned storage guard");
  if (std::memcmp(reinterpret_cast<void *>(0x512460), segmentBody.data(),
                  segmentBody.size()) ||
      std::memcmp(reinterpret_cast<void *>(0x5070e0), snapBody.data(),
                  snapBody.size()))
    throw std::runtime_error("segment/snap original text modified");
}
#endif
unsigned segmentRefusals() {
  unsigned refused = 0;
  for (unsigned kind = 0; kind < 8; ++kind) {
    auto f = generateSegment(1);
    segment = &f;
    fixture = &f.base;
    events.clear();
    f.state.scorePendingFc = true;
    f.state.contributionRoots = {0, std::nullopt};
    switch (kind) {
    case 0:
      f.state.count = 0;
      break;
    case 1:
      f.state.count = 17;
      break;
    case 2:
      f.state.cursor = f.state.count;
      break;
    case 3:
      f.state.contributionRoots[0] = 4;
      break;
    case 4:
      f.state.contributionNodes[0].parent = 0;
      break;
    case 5:
      f.state.contributionNodes[2].owner = 3;
      break;
    case 6:
      f.environment.regionContinuation = f.state.regionPending = true;
      f.state.regionCursor = 50;
      break;
    case 7:
      f.environment.scoreDenominator = 0;
      break;
    }
    bool rejected = false;
    try {
      completeMotionSegment(f.base.world.creatures[0], f.base.execution[0],
                            f.motion, f.landing, f.state, f.environment,
                            segmentServices());
    } catch (const std::invalid_argument &) {
      rejected = kind != 3 && kind != 5;
    } catch (const std::out_of_range &) {
      rejected = kind == 3 || kind == 5;
    }
    if (!rejected)
      throw std::runtime_error("unsupported segment input admitted");
    ++refused;
  }
  return refused;
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE required");
  install(argv[1]);
  segmentBody = Bytes(reinterpret_cast<unsigned char *>(0x512460),
                      reinterpret_cast<unsigned char *>(0x5127f9));
  snapBody = Bytes(reinterpret_cast<unsigned char *>(0x5070e0),
                   reinterpret_cast<unsigned char *>(0x507189));
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  std::map<std::string, unsigned> coverage;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 2; ++op) {
      auto input = generateSegment(n);
      auto expected = input;
      segment = &expected;
      fixture = &expected.base;
      events.clear();
      auto &a = expected.base.world.creatures[0];
      auto &e = expected.base.execution[0];
      if (!op)
        completeMotionSegment(a, e, expected.motion, expected.landing,
                              expected.state, expected.environment,
                              segmentServices());
      else
        snapCreaturePosition(
            a, expected.state.route[expected.state.cursor].position,
            expected.widths[a.ai.type], snapServices());
      auto trace = events;
      for (const auto &event : trace)
        ++coverage["service_" + std::to_string(event.kind)];
      if (expected.state.contributionTotals[2] !=
          input.state.contributionTotals[2])
        ++coverage["leaf_credit"];
      if (expected.state.contributionTotals[0] !=
              input.state.contributionTotals[0] ||
          expected.state.contributionTotals[1] !=
              input.state.contributionTotals[1])
        ++coverage["ancestry_credit"];
      if (expected.state.regionCursor != input.state.regionCursor)
        ++coverage["region_cursor_advance"];
      if (input.state.regionPending && !expected.state.regionPending)
        ++coverage["region_continuation_clear"];
      mix(expected.state.cursor);
      mix(expected.state.regionCursor);
      mix(expected.state.regionPending);
      mix(expected.state.scorePendingFc);
      mix(expected.state.overlayFrame809);
      mix(expected.state.overlayLink805.value_or(0));
      for (auto v : expected.state.contributionTotals)
        mix(v);
      for (const auto &v : expected.base.world.creatures) {
        mix(v.ai.active);
        mix(v.ai.health);
        mix(v.ai.motion);
        mix(v.ai.marker);
        mix(unsigned(v.ai.position.x));
        mix(unsigned(v.ai.position.y));
        mix(unsigned(v.ai.position.z));
        mix(unsigned(v.finePosition.x));
        mix(unsigned(v.finePosition.y));
        mix(unsigned(v.finePosition.z));
        mix(unsigned(v.cooldown));
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
      compareSegment(input, expected, op, trace);
#endif
      ++cases;
    }
  for (const char *branch :
       {"service_300", "service_301", "service_302", "service_303",
        "service_305", "service_306", "service_307", "service_308",
        "service_309", "service_310", "service_311", "service_312",
        "leaf_credit", "ancestry_credit", "region_cursor_advance",
        "region_continuation_clear"})
    if (!coverage[branch])
      throw std::runtime_error(std::string("segment branch absent: ") + branch);
  unsigned refused = segmentRefusals();
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"native_refusal_cases\":" << refused
            << ",\"functions\":2,\"outcome_digest\":" << digest
            << ",\"coverage\":{";
  bool first = true;
  for (const auto &item : coverage) {
    if (!first)
      std::cout << ',';
    first = false;
    std::cout << '\"' << item.first << "\":" << item.second;
  }
  std::cout << "}}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
