#pragma push_macro("main")
#undef main
#define main unused_execution_fixture_main
#include "creature-execution-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/full_motion_action.hpp"
#include <map>
namespace {
struct MotionFixture {
  Fixture base;
  MotionActionState motion;
  MovementBehaviorState landing;
  std::vector<int> animationEvents;
  unsigned pollCursor = 0;
  std::optional<unsigned> boundBehavior;
};
MotionFixture *moving;
int animationEvent(bool overlay) {
  if (overlay)
    return int(fixture->number % 5);
  unsigned index = moving->pollCursor++;
  return index < moving->animationEvents.size() ? moving->animationEvents[index]
                                                : 0;
}
unsigned tileSound(const AutonomousCreature &a) {
  unsigned kind = fixture->number % 4;
  if (!kind && a.ai.position.z > 0)
    kind = (fixture->number / 4) % 4;
  return kind ? 70 + kind : 0;
}
MotionActionServices motionServices() {
  MotionActionServices s;
  s.cameraTurn = fixture->number % 4;
  s.poll = [](auto &a, bool overlay) {
    record(200, a.ai.slot, overlay);
    return animationEvent(overlay);
  };
  s.rewind = [](auto &a) { record(201, a.ai.slot); };
  s.completeSegment = [](auto &a) {
    record(202, a.ai.slot);
    a.ai.motion = fixture->number % 2;
    a.ai.position.x = (a.ai.position.x + 3) % fixture->world.dimensions.x;
    if (fixture->number % 17 == 0)
      a.ai.active = false;
  };
  s.runBehavior = [](auto &a) {
    executionServices().behavior(a, a.ai.behavior);
  };
  s.reset = [](auto &a) { record(203, a.ai.slot); };
  s.updateOrders = [](auto &a) { record(204, a.ai.slot); };
  s.visualFacing = [](auto &a) {
    auto face = fixture->execution[a.ai.slot].animationFacing;
    if (a.animationKind == 3) {
      record(205, a.ai.slot, 2);
      record(206, a.ai.slot, 20 + face);
    } else if (a.animationKind == 2 && moving->motion.visualSprite)
      record(207, a.ai.slot, face);
    if (fixture->execution[a.ai.slot].block855)
      record(208, a.ai.slot, face + 8);
  };
  s.behavior = [](auto &a, unsigned kind) {
    record(209, a.ai.slot, kind);
    a.ai.behavior = kind;
    if (fixture->number % 19 == 0)
      a.ai.active = false;
  };
  s.bindBehavior = [](auto &, unsigned kind) { moving->boundBehavior = kind; };
  s.action = executionServices().action;
  s.response = [](auto &a) {
    record(210, a.ai.slot);
    return fixture->number % 7 == 0;
  };
  s.legal = [](const auto &a, Cell p) {
    record(211, a.ai.slot, a.block723 || a.ai.behavior == 14, p);
    return (fixture->number / 128) % 3 != 0;
  };
  s.stun = [](auto &a, unsigned duration, unsigned mode) {
    record(212, a.ai.slot, mode, {int(duration), 0, 0});
  };
  s.effect = [](auto &a, unsigned kind, Cell p) {
    record(213, a.ai.slot, kind, p);
  };
  s.footstep = [](auto &a) {
    if (auto sound = tileSound(a))
      record(214, a.ai.slot, sound, {3 > a.ai.position.z, 0, 0});
  };
  s.releaseEffect = [](unsigned token) { record(215, 0, token); };
  return s;
}
MotionFixture generateMotion(unsigned n, unsigned variant) {
  MotionFixture f;
  f.base = generate(n);
  auto &a = f.base.world.creatures[0];
  auto &e = f.base.execution[0];
  auto &m = f.motion;
  a.ai.type = n % 28;
  a.width = 1 + (n / 7) % 2;
  a.height = 1 + (n / 13) % 4;
  a.movementKind = (n / 23) % 8;
  a.animationKind = (n / 28) % 4;
  a.ai.flying = (n / 112) % 3;
  a.formCapability = (n / 336) % 2;
  a.ai.behavior = variant == 3 ? 14 : variant == 4 ? 13 : 1 + n % 16;
  if (a.ai.behavior == 12)
    a.ai.behavior = 11;
  a.ai.action = n % 3 == 0 ? 10 : n % 3 == 1 ? 20 : 2;
  a.ai.engaged = (n / 3) % 3;
  a.ai.pending = (n / 5) % 17;
  if (a.ai.pending == 3 && a.ai.action != 10 && n % 2)
    a.ai.target = -1;
  a.ai.position.z = 1 + int(n % 3);
  a.finePosition = {42, -17, 9};
  a.targetMovingAttacker = (n / 17) % 3;
  a.block723 = n % 2;
  a.flag104 = n % 7;
  a.cooldown = int(n % 9) - 3;
  a.mode = n % 4;
  e.mode10c = (n / 28) % 4;
  e.moveVariant = variant == 0 ? 0 : unsigned(5 + n % 19);
  e.animationOffset = n % 16;
  e.animationFacing = n % 8;
  e.block855 = (n / 4) % 2;
  e.block722 = (variant == 3 || n % 2) ? 1 : 0;
  e.marker68 = n % 2;
  e.actionRouteCounters = {n % 9, unsigned(-20), 17};
  e.routeEffect = n % 2 ? std::optional<unsigned>(45) : std::nullopt;
  e.orders = {{2, 1, {}, -1, 47, {}, false},
              {n % 2 ? 2u : 3u, 2, {}, -1, 49, {}, false}};
  m.duration = 7;
  m.accumulator = variant == 0 ? -5 : int(n % 7);
  m.progress = variant >= 2 ? 180 : int(n % 90);
  m.travelX = int(n % 41) - 20;
  m.travelY = int(n % 37) - 18;
  m.heightDelta = int(n % 81) - 40;
  m.heightOrigin = 100;
  m.heightStep = 3;
  m.step = 99;
  m.deltaX = 7;
  m.deltaY = -9;
  m.direction = (n / 3) % 10;
  m.vertical = (n / 2) % 2;
  m.sampleCursor = 24;
  m.initialCursor = 20;
  m.initialResidualX = -13;
  m.initialResidualY = 23;
  m.displacementPause = n % 2;
  m.visualSprite = (n / 8) % 2;
  for (unsigned i = 0; i < m.samples.size(); ++i)
    m.samples[i] = variant == 5 ? int(i % 3) - 2 : 2 + int((n + i) % 31);
  if (variant == 6) {
    e.moveVariant = unsigned(-31);
    m.duration = -7;
    m.accumulator = -5;
    constexpr int progress[] = {INT32_MIN, INT32_MAX, -193, 191};
    m.progress = progress[n % 4];
    m.travelX = m.heightOrigin = m.deltaX = INT32_MAX;
    m.travelY = m.deltaY = INT32_MIN;
    m.heightDelta = n % 2 ? INT32_MIN : INT32_MAX;
    a.finePosition.x = INT32_MIN;
    a.finePosition.y = INT32_MAX;
    for (unsigned i = 0; i < m.samples.size(); ++i)
      m.samples[i] = (n + i) % 2 ? INT32_MIN : INT32_MAX;
  }
  constexpr int codes[][8] = {
      {0},    {1, 0},          {2, 0},       {3, 4, 100, 0},    {5, 0},
      {6, 0}, {1, 5, 6, 2, 0}, {2, 1, 6, 0}, {6, 5, 4, 3, 2, 0}};
  f.animationEvents.assign(codes[(n / 7) % 9], codes[(n / 7) % 9] + 8);
  return f;
}
#ifdef MNM_EXECUTION_REFERENCE
Bytes motionBody, sampleBytes, motionNodes, terrainBytes, tileMetadata,
    visualConfig, visualMetadata;
unsigned address(const void *p) {
  return unsigned(reinterpret_cast<std::uintptr_t>(p));
}
unsigned animationActor(void *p) {
  return unsigned(static_cast<unsigned char *>(p) - actors.data()) / 0xe4b;
}
void motionExtra(void *p, const AutonomousCreature &a,
                 const CreatureExecution &e, const MotionActionState &m,
                 const MovementBehaviorState &landing,
                 std::optional<unsigned> binding = {}) {
  for (auto v :
       {std::pair<unsigned, unsigned>{0x14, unsigned(a.finePosition.x)},
        {0x18, unsigned(a.finePosition.y)},
        {0x1c, unsigned(a.finePosition.z)},
        {0x104, a.flag104},
        {0x608, m.direction},
        {0x60c, e.animationFacing},
        {0x7cd, a.animationKind},
        {0x7d1, m.visualSprite ? 0xc001u : 0u},
        {0x855, e.block855},
        {0x977, 0},
        {0x98f, unsigned(m.vertical)},
        {0xb3f, unsigned(m.progress)},
        {0xb43, unsigned(m.step)},
        {0xb47, unsigned(m.travelX)},
        {0xb4b, unsigned(m.travelY)},
        {0xb4f, unsigned(m.deltaX)},
        {0xb53, unsigned(m.deltaY)},
        {0xb57, unsigned(m.heightDelta)},
        {0xb5b, unsigned(m.heightStep)},
        {0xb5f, unsigned(m.heightOrigin)},
        {0xb63, e.moveVariant},
        {0xb67, unsigned(m.duration)},
        {0xb6b, unsigned(m.accumulator)},
        {0xb77, e.actionRouteCounters[0]},
        {0xb7b, e.actionRouteCounters[1]},
        {0xb7f, e.actionRouteCounters[2]},
        {0xb83, unsigned(m.initialResidualX)},
        {0xb87, unsigned(m.initialResidualY)},
        {0xb8f, address(sampleBytes.data() + 16) + m.sampleCursor * 4u},
        {0xb93, address(sampleBytes.data() + 16) + m.initialCursor * 4u},
        {0x6f0, unsigned(m.displacementPause)},
        {0xa4, landing.landingKind},
        {0xe0, e.animationOffset},
        {0x6a4, a.targetMovingAttacker},
        {0x5f4, binding ? behaviorAddresses[*binding - 1]
                        : address(reinterpret_cast<void *>(behaviorStub))},
        {0x5f8, 0}})
    write32(p, v.first, v.second);
}
extern "C" unsigned __attribute__((thiscall)) motionPollStub(void *p) {
  unsigned delta = unsigned(static_cast<unsigned char *>(p) - actors.data());
  bool overlay = delta % 0xe4b == 0x7d5;
  record(200, delta / 0xe4b, overlay);
  return unsigned(animationEvent(overlay));
}
extern "C" void __attribute__((thiscall)) motionRewindStub(void *p) {
  record(201, animationActor(p));
}
extern "C" void __attribute__((thiscall)) motionCompleteStub(void *p) {
  record(202, slot(p));
  write32(p, 0x618, fixture->number % 2);
  write32(p, 8, (read32(p, 8) + 3) % unsigned(fixture->world.dimensions.x));
  if (fixture->number % 17 == 0)
    write32(p, 4, 0);
}
extern "C" void __attribute__((thiscall)) motionResetStub(void *p) {
  record(203, slot(p));
}
extern "C" void __attribute__((thiscall)) motionUpdateStub(void *p) {
  record(204, slot(p));
}
extern "C" void __attribute__((thiscall)) motionBindVisualStub(void *p,
                                                               void *metadata) {
  if (metadata != visualMetadata.data() + 2 * 276)
    throw std::runtime_error("motion overlay metadata");
  record(205, animationActor(p), 2);
}
extern "C" void __attribute__((thiscall))
motionStartVisualStub(void *p, unsigned sequence, unsigned reverse) {
  if (reverse)
    throw std::runtime_error("motion overlay reverse");
  record(206, animationActor(p), sequence);
}
extern "C" void __attribute__((thiscall)) motionSpriteStub(void *p,
                                                           unsigned face) {
  if (address(p) != 0xc001)
    throw std::runtime_error("motion visual token");
  record(207, 0, face);
}
extern "C" void __attribute__((thiscall)) motionAuxStub(void *p,
                                                        unsigned frame) {
  record(208, animationActor(p), frame);
}
extern "C" void __attribute__((thiscall)) motionBehaviorStub(void *p,
                                                             unsigned kind) {
  record(209, slot(p), kind);
  write32(p, 0x5ec, kind);
  if (fixture->number % 19 == 0)
    write32(p, 4, 0);
}
extern "C" unsigned __attribute__((thiscall)) motionResponseStub(void *p) {
  record(210, slot(p));
  return fixture->number % 7 == 0;
}
extern "C" unsigned __attribute__((thiscall))
motionLegalStub(void *, int x, int y, int z, const void *descriptor) {
  const auto &a = fixture->world.creatures[0];
  unsigned special = a.block723 || read32(actors.data(), 0x5ec) == 14;
  if (read32(descriptor, 0) != a.height || read32(descriptor, 4) != a.width ||
      read32(descriptor, 8) != a.flag104 ||
      read32(descriptor, 12) != a.movementKind ||
      static_cast<const unsigned char *>(descriptor)[16] ||
      read32(descriptor, 17) != a.ai.type ||
      static_cast<const unsigned char *>(descriptor)[21] != special)
    throw std::runtime_error("motion descriptor ABI");
  record(211, 0, special, {x, y, z});
  return (fixture->number / 128) % 3 != 0;
}
extern "C" void __attribute__((thiscall))
motionStunStub(void *p, unsigned duration, unsigned mode, unsigned zero) {
  if (zero)
    throw std::runtime_error("motion stun flag");
  record(212, slot(p), mode, {int(duration), 0, 0});
}
extern "C" void __attribute__((thiscall))
motionEffectStub(void *, unsigned kind, int x, int y, int z, unsigned zero,
                 void *ref, unsigned last) {
  if (zero || ref || last)
    throw std::runtime_error("motion effect flags");
  record(213, 0, kind, {x, y, z});
}
extern "C" void __attribute__((thiscall))
motionFootstepStub(void *, void *p, unsigned sound, unsigned above) {
  record(214, slot(p), sound, {int(above), 0, 0});
}
extern "C" void __attribute__((thiscall)) motionReleaseStub(void *p) {
  record(215, 0, address(p));
}
void installMotionServices() {
  for (auto v : {std::pair<unsigned, void *>{
                     0x464ec0, reinterpret_cast<void *>(motionPollStub)},
                 {0x464d70, reinterpret_cast<void *>(motionRewindStub)},
                 {0x512460, reinterpret_cast<void *>(motionCompleteStub)},
                 {0x50aa30, reinterpret_cast<void *>(motionBehaviorStub)},
                 {0x50ab30, reinterpret_cast<void *>(motionResetStub)},
                 {0x50db60, reinterpret_cast<void *>(actionStub)},
                 {0x517580, reinterpret_cast<void *>(motionResponseStub)},
                 {0x50d060, reinterpret_cast<void *>(motionStunStub)},
                 {0x4943f0, reinterpret_cast<void *>(motionReleaseStub)},
                 {0x5120c0, reinterpret_cast<void *>(motionUpdateStub)},
                 {0x4f4900, reinterpret_cast<void *>(motionLegalStub)},
                 {0x464ca0, reinterpret_cast<void *>(motionBindVisualStub)},
                 {0x464cb0, reinterpret_cast<void *>(motionStartVisualStub)},
                 {0x494a50, reinterpret_cast<void *>(motionSpriteStub)},
                 {0x464e20, reinterpret_cast<void *>(motionAuxStub)},
                 {0x570e80, reinterpret_cast<void *>(motionFootstepStub)},
                 {0x571310, reinterpret_cast<void *>(motionEffectStub)}})
    redirect(v.first, v.second);
}
void compareMotion(const MotionFixture &initial, const MotionFixture &expected,
                   const std::vector<Event> &trace) {
  auto input = initial;
  moving = &input;
  fixture = &input.base;
  events.clear();
  prepare();
  installMotionServices();
  sampleBytes.assign(16 + 48 * 4 + 16, 0xa7);
  std::memcpy(sampleBytes.data() + 16, input.motion.samples.data(), 48 * 4);
  auto sampleBefore = sampleBytes;
  motionNodes.assign(48 * 3, 0);
  auto *head = motionNodes.data();
  write32(head, 0, address(head + 48));
  write32(head, 4, address(head + 96));
  for (unsigned i = 0; i < 2; ++i) {
    auto *node = head + (i + 1) * 48;
    write32(node, 0, address(i == 1 ? head : node + 48));
    write32(node, 4, address(i == 0 ? head : node - 48));
    write32(node, 8, input.base.execution[0].orders[i].kind);
    write32(node, 0x28, input.base.execution[0].orders[i].effect.value_or(0));
  }
  write32(lists.data(), 4, address(head));
  write32(lists.data(), 8, 2);
  auto nodeBefore = motionNodes;
  for (unsigned i = 0; i < input.base.world.creatures.size(); ++i)
    motionExtra(actors.data() + i * 0xe4b, input.base.world.creatures[i],
                input.base.execution[i], input.motion, input.landing);
  terrainBytes.assign(16 * 16 * 4 * 12, 0);
  tileMetadata.assign(4 * 356, 0);
  visualConfig.assign(0x170, 0);
  visualMetadata.assign(3 * 276, 0);
  const auto &a = input.base.world.creatures[0];
  unsigned index =
      unsigned((a.ai.position.z * 16 + a.ai.position.y) * 16 + a.ai.position.x);
  unsigned short kind = input.base.number % 4;
  std::memcpy(terrainBytes.data() + index * 12, &kind, 2);
  kind = (input.base.number / 4) % 4;
  std::memcpy(terrainBytes.data() + (index - 256) * 12, &kind, 2);
  for (unsigned i = 0; i < 4; ++i) {
    global(0x6cb8c2 + i * 4, i * 256);
    write32(tileMetadata.data() + i * 356, 0xf0, 70 + i);
  }
  for (unsigned i = 0; i < 16; ++i)
    global(0x6cb942 + i * 4, i * 16);
  global(0x6c54dc, address(terrainBytes.data()));
  global(0x65660c, address(tileMetadata.data()));
  global(0x6c5c80, 3);
  global(0x6e9850, input.base.number % 4);
  write32(visualConfig.data(), 0x160, 20);
  write32(visualConfig.data(), 0x164, 2);
  global(0x6898d0, address(visualConfig.data()));
  global(0x6894a0, address(visualMetadata.data()));
  auto before = actors;
  auto terrainBefore = terrainBytes;
  auto metadataBefore = tileMetadata;
  auto visualBefore = visualConfig;
  auto descriptorsBefore = visualMetadata;
  using F = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<F>(0x5104b0)(actors.data());
  if (events != trace) {
    unsigned i = 0;
    while (i < events.size() && i < trace.size() && events[i] == trace[i])
      ++i;
    std::cerr << "motion trace n=" << input.base.number << " i=" << i
              << " actual=" << events.size() << " expected=" << trace.size()
              << '\n';
    if (i < events.size())
      std::cerr << "actual " << events[i].kind << "," << events[i].argument
                << " xyz=" << events[i].position.x << ","
                << events[i].position.y << "," << events[i].position.z << '\n';
    if (i < trace.size())
      std::cerr << "expected " << trace[i].kind << "," << trace[i].argument
                << " xyz=" << trace[i].position.x << "," << trace[i].position.y
                << "," << trace[i].position.z << '\n';
    throw std::runtime_error("motion ordered effects mismatch");
  }
  for (unsigned i = 0; i < expected.base.world.creatures.size(); ++i) {
    packActor(before.data() + i * 0xe4b, expected.base.world.creatures[i],
              expected.base.execution[i]);
    motionExtra(before.data() + i * 0xe4b, expected.base.world.creatures[i],
                expected.base.execution[i],
                i == 0 ? expected.motion : initial.motion,
                i == 0 ? expected.landing : initial.landing,
                i == 0 ? expected.boundBehavior : std::nullopt);
  }
  if (before != actors) {
    unsigned i = 0;
    while (i < actors.size() && actors[i] == before[i])
      ++i;
    throw std::runtime_error(
        "motion write n=" + std::to_string(input.base.number) +
        " offset=" + std::to_string(i % 0xe4b) + " actual=" +
        std::to_string(read32(actors.data() + i / 0xe4b * 0xe4b, i % 0xe4b)) +
        " expected=" +
        std::to_string(read32(before.data() + i / 0xe4b * 0xe4b, i % 0xe4b)));
  }
  for (unsigned i = 0; i < 2; ++i)
    write32(nodeBefore.data() + (i + 1) * 48, 0x28,
            expected.base.execution[0].orders[i].effect.value_or(0));
  if (nodeBefore != motionNodes || sampleBefore != sampleBytes ||
      terrainBefore != terrainBytes || metadataBefore != tileMetadata ||
      visualBefore != visualConfig || descriptorsBefore != visualMetadata)
    throw std::runtime_error("motion owned buffer guard");
  if (std::memcmp(reinterpret_cast<void *>(0x5104b0), motionBody.data(),
                  motionBody.size()))
    throw std::runtime_error("motion original text changed");
  for (const auto &body : originalBodies)
    if (body.first == 0x50a930 &&
        std::memcmp(reinterpret_cast<void *>(body.first), body.second.data(),
                    body.second.size()))
      throw std::runtime_error("motion resolver text changed");
}
#endif
} // namespace
unsigned nativeRefusals() {
  unsigned refused = 0;
  for (unsigned kind = 0; kind < 6; ++kind) {
    auto f = generateMotion(1, 1);
    moving = &f;
    fixture = &f.base;
    events.clear();
    auto &a = f.base.world.creatures[0];
    auto &e = f.base.execution[0];
    a.ai.flying = 0;
    a.animationKind = 0;
    e.moveVariant = 0;
    f.motion.accumulator = 1;
    f.motion.duration = 1;
    auto services = motionServices();
    switch (kind) {
    case 0:
      f.motion.duration = 0;
      break;
    case 1:
      f.motion.accumulator = INT32_MIN;
      f.motion.duration = -1;
      break;
    case 2:
      f.motion.accumulator = 4097;
      break;
    case 3:
      f.motion.sampleCursor = 48;
      break;
    case 4:
      services.poll = [](auto &, bool) { return 3; };
      break;
    case 5:
      f.base.world.dimensions.x = 0;
      break;
    }
    bool rejected = false;
    try {
      tickMotionAction(a, e, f.motion, f.landing, f.base.world, services);
    } catch (const std::invalid_argument &) {
      rejected = kind != 3;
    } catch (const std::out_of_range &) {
      rejected = kind == 3;
    }
    if (!rejected)
      throw std::runtime_error("motion unsupported input admitted");
    ++refused;
  }
  return refused;
}
int main(int argc, char **argv) try {
#ifdef MNM_EXECUTION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE required");
  install(argv[1]);
  motionBody = Bytes(reinterpret_cast<unsigned char *>(0x5104b0),
                     reinterpret_cast<unsigned char *>(0x510e3a));
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0;
  std::map<std::string, unsigned> coverage;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned variant = 0; variant < 7; ++variant) {
      auto initial = generateMotion(n, variant);
      auto expected = initial;
      moving = &expected;
      fixture = &expected.base;
      events.clear();
      tickMotionAction(expected.base.world.creatures[0],
                       expected.base.execution[0], expected.motion,
                       expected.landing, expected.base.world, motionServices());
      auto trace = events;
      if (variant == 0)
        ++coverage["no_admitted_substep"];
      if (variant == 6)
        ++coverage["signed_arithmetic_extremes"];
      if (initial.motion.direction > 7)
        ++coverage["retained_direction_deltas"];
      if (initial.base.world.creatures[0].ai.target == -1)
        ++coverage["missing_reverse_target"];
      if (expected.boundBehavior)
        ++coverage["bound_behavior_" + std::to_string(*expected.boundBehavior)];
      const auto &beforeActor = initial.base.world.creatures[0];
      const auto &afterActor = expected.base.world.creatures[0];
      if (beforeActor.ai.engaged && !afterActor.ai.engaged)
        ++coverage[beforeActor.ai.action == 10
                       ? "main_engagement_release"
                       : "alternate_engagement_release"];
      for (const auto &event : trace) {
        ++coverage["service_" + std::to_string(event.kind)];
        if (event.kind == 213)
          ++coverage["effect_" + std::to_string(event.argument)];
        if (event.kind == 209 && event.argument == 16)
          ++coverage["movement_failure"];
      }
      const auto &m = expected.motion;
      for (int v : {m.accumulator, m.duration, m.progress, m.travelX, m.travelY,
                    m.heightDelta, m.heightStep, m.heightOrigin, m.step,
                    m.deltaX, m.deltaY})
        mix(unsigned(v));
      mix(m.direction);
      mix(m.sampleCursor);
      mix(m.displacementPause);
      mix(expected.boundBehavior.value_or(0));
      mix(expected.landing.landingKind);
      for (const auto &a : expected.base.world.creatures) {
        mix(a.ai.behavior);
        mix(a.ai.pending);
        mix(a.ai.action);
        mix(a.ai.active);
        mix(a.ai.engaged);
        mix(a.ai.incoming);
        mix(unsigned(a.cooldown));
        mix(unsigned(a.finePosition.x));
        mix(unsigned(a.finePosition.y));
        mix(unsigned(a.finePosition.z));
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
      compareMotion(initial, expected, trace);
#endif
      ++cases;
    }
  unsigned refused = nativeRefusals();
  for (const char *branch :
       {"no_admitted_substep", "signed_arithmetic_extremes",
        "retained_direction_deltas", "missing_reverse_target",
        "bound_behavior_2", "bound_behavior_3", "main_engagement_release",
        "alternate_engagement_release", "movement_failure", "service_205",
        "service_206", "service_207", "service_208", "service_212",
        "service_214", "service_215", "effect_515", "effect_526"})
    if (!coverage[branch])
      throw std::runtime_error(std::string("motion fixture branch absent: ") +
                               branch);
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"functions\":1,\"native_refusal_cases\":" << refused
            << ",\"outcome_digest\":" << digest << ",\"coverage\":{";
  bool first = true;
  for (const auto &entry : coverage) {
    if (!first)
      std::cout << ',';
    first = false;
    std::cout << '\"' << entry.first << "\":" << entry.second;
  }
  std::cout << "}}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
