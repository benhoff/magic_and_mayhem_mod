// Reuse the independently compared health endpoints and private PE mapper.
#define MNM_HEALTH_MAIN unused_health_reference_main
#include "ordinary-health-test.cpp"
#undef MNM_HEALTH_MAIN
#include "animation.hpp"
#include "melee.hpp"
#include "no_cd.hpp"
#include <algorithm>
#include <fstream>
using namespace mnm::reconstruction;
namespace {
std::uint64_t meleeCases = 0, meleeDraws = 0, actionCases = 0,
              animationTicks = 0;
bool streamStun = false;
#ifdef MNM_HEALTH_REFERENCE
const MeleeInputs *meleeInputs = nullptr;
std::vector<MeleeEvent> meleeTrace;
extern "C" unsigned __attribute__((thiscall)) meleeResponse(void *, void *) {
  meleeTrace.push_back({MeleeEffect::responseQuery, 0});
  return 0;
}
extern "C" void __attribute__((thiscall)) meleeCredit(void *, int n) {
  meleeTrace.push_back({MeleeEffect::hostileCredit, n});
}
extern "C" unsigned __attribute__((thiscall)) meleeStun(void *, int n,
                                                        unsigned actor) {
  check(actor == 0, "stun source");
  meleeTrace.push_back({MeleeEffect::stunQuery, n});
  if(streamStun){using Next=unsigned(__attribute__((thiscall)) *)(void *);reinterpret_cast<Next>(0x54e200)(reinterpret_cast<void*>(0x6e1ac8));}
  return meleeInputs->stunHandled;
}
extern "C" void __attribute__((thiscall)) meleeHandled(void *) {
  meleeTrace.push_back({MeleeEffect::stunHandled, 0});
}
extern "C" void __attribute__((thiscall))
meleeFallback(void *, int n, unsigned a, unsigned b) {
  check(a == 0 && b == 0, "stun fallback args");
  meleeTrace.push_back({MeleeEffect::stunFallback, n});
}
extern "C" void __attribute__((thiscall)) meleeSound(void *, void *, void *) {
  meleeTrace.push_back({MeleeEffect::attackSound, 0});
}
extern "C" void *__attribute__((thiscall)) meleeEffect(void *) {
  meleeTrace.push_back({MeleeEffect::effectAdmission, 0});
  return nullptr;
}
extern "C" void __attribute__((thiscall)) meleeReady(void *, unsigned action) {
  check(action == 15, "main event1 action");
}
extern "C" void __attribute__((thiscall)) meleeAudio(void *, void *) {}
void initializeMelee(const char *path) {
  initialize(path);
  for (auto range :
       std::vector<std::pair<unsigned, unsigned>>{{0x50c0d0, 0x50c92f},
                                                  {0x50e7a0, 0x50eb24},
                                                  {0x54e170, 0x54e256},
                                                  {0x464cb0, 0x464e20},
                                                  {0x464ec0, 0x46508c}}) {
    auto *p = reinterpret_cast<unsigned char *>(range.first);
    originalCode.push_back(
        {range.first, Bytes(p, p + range.second - range.first)});
  }
  const Redirect endpoints[] = {
      {0x50c930, reinterpret_cast<std::uintptr_t>(&meleeResponse)},
      {0x514e40, reinterpret_cast<std::uintptr_t>(&meleeCredit)},
      {0x521160, reinterpret_cast<std::uintptr_t>(&meleeStun)},
      {0x521420, reinterpret_cast<std::uintptr_t>(&meleeHandled)},
      {0x50d060, reinterpret_cast<std::uintptr_t>(&meleeFallback)},
      {0x570510, reinterpret_cast<std::uintptr_t>(&meleeSound)},
      {0x49cdb0, reinterpret_cast<std::uintptr_t>(&meleeEffect)},
      {0x50db60, reinterpret_cast<std::uintptr_t>(&meleeReady)},
      {0x570480, reinterpret_cast<std::uintptr_t>(&meleeAudio)}};
  for (const auto &r : endpoints)
    redirect(r);
}
void healthFields(Bytes &o, const State &s) {
  for (auto item :
       std::vector<std::pair<unsigned, unsigned>>{{0xe4, s.health},
                                                  {0x6c, s.bar},
                                                  {0x92e, s.dirty},
                                                  {0x660, s.cooldown},
                                                  {0x64c, s.target},
                                                  {0x9c, s.gate9c},
                                                  {0x778, s.expiry},
                                                  {0x780, s.timer},
                                                  {0x889, s.cacheA},
                                                  {0x8bd, s.cacheB},
                                                  {0xb8b, s.marker},
                                                  {0x104, s.flag104},
                                                  {0x124, s.rank}})
    put(o, item.first, item.second);
  for (unsigned i = 0; i < 8; ++i) {
    put(o, 0x24 + i * 4, 1U);
    put(o, 0xdeb + i * 4, s.incoming[i]);
    put(o, 0xe0b + i * 4, s.ages[i]);
  }
}
struct Fixture {
  Bytes pool = Bytes(2 * 0xe4b), config = Bytes(0x5c9);
  Inputs hp;
  State health;
  Bytes targetBefore, sourceBefore;
  Fixture(const MeleeInputs &p, unsigned current) {
    meleeInputs = &p;
    hp.baseMaximum = 360;
    hp.slot = 1;
    hp.attributionOwner = 0;
    hp.contributions = true;
    hp.contributionIds = {{0xffffffffU, 0xffffffffU}};
    hp.creditAvailable = {{false, false}};
    hp.notify = 1;
    health.health = current;
    inputs = &hp;
    trace.clear();
    meleeTrace.clear();
    put(config, 0, 1U);
    put(config, 8, 2U);
    put(config, 0x18, 360U);
    put(config, 0x1a0 + 17 * 4, std::uint32_t(p.base));
    put(config, 0x210 + 17 * 4, std::uint32_t(p.stunThreshold));
    Bytes a(0xe4b), b(0xe4b);
    healthFields(b, health);
    for (auto *o : {&a, &b}) {
      put(*o, 4, 1U);
      put(*o, 0xa8, 17U);
      put(*o, 0xac,
          std::uint32_t(reinterpret_cast<std::uintptr_t>(config.data())));
    }
    put(a, 0, 0U);
    put(b, 0, 1U);
    put(a, 0x64c, 1U);
    put(a, 0x174, 0U);
    put(b, 0x174, 1U);
    put(a, 0xe4, 360U);
    put(a, 0xf8, p.attackerPhysical ? 1U : 0U);
    put(b, 0xf8, p.targetPhysical ? 1U : 0U);
    put(a, 0x8ed, std::uint32_t(p.power));
    put(a, 0x94, p.bloodLust ? 1U : 0U);
    put(a, 0x10, std::uint32_t(p.height));
    put(a, 0x124, p.rank);
    put(a, 0x16c, p.charge ? 1U : 0U);put(a, 0x170, unsigned(p.chargeScalar));
    put(a, 0x81d, p.quarterFlag ? 1U : 0U);
    put(b, 0x6ac, p.targetResponse ? 1U : 0U);
    std::copy(a.begin(), a.end(), pool.begin());
    std::copy(b.begin(), b.end(), pool.begin() + 0xe4b);
    sourceBefore = a;
    targetBefore = b;
    global(0x6def5c, 2);
    global(0x6def58,
           std::uint32_t(reinterpret_cast<std::uintptr_t>(pool.data())));
    global(0x5e1778, std::uint32_t(p.heightBonus));
    global(0x5e177c, std::uint32_t(p.chargeBonus));
    global(0x5e1758, std::uint32_t(p.responseModifier));
    global(0x5e1910, std::uint32_t(p.rankBonus));
    global(0x5e1914, std::uint32_t(p.rankBonus));
    global(0x6e1f30, 0);
    global(0x644520, 0);
    global(0x6df180, 0);
    global(0x6898dc, 0);
    *reinterpret_cast<unsigned char *>(0x6e9858) = 0;
    for (unsigned i = 0; i < 8; ++i)
      global(0x643a80 + i * 340 + 12, 0xffffffffU);
    // Distinct hostile owner relation, no evasion/charge/secondary animation.
    *reinterpret_cast<unsigned char *>(0x643a80 + 0x18 + 1) = 0;
  }
  void verify(const MeleeResult &r, const State &expected, bool action = false,
              const MeleeActionState &s = {}) {
    const Bytes a(pool.begin(), pool.begin() + 0xe4b),
        b(pool.begin() + 0xe4b, pool.end());
    auto expectedTarget = targetBefore;
    healthFields(expectedTarget, expected);
    auto expectedSource = sourceBefore;
    if(meleeInputs->charge&&(!action||!r.draws.empty()))put(expectedSource,0x16c,0U);
    if (action) {
      put(expectedSource, 0x664, s.engaged);
      put(expectedSource, 0x66c, s.cooldown);
      put(expectedTarget, 0x668, s.incoming);
      put(expectedTarget, 0x6a4, s.recentAttacker);
    }
    check(a == expectedSource,
          "original changed source outside melee projection");
    check(b == expectedTarget,
          "original changed target outside melee/health projection");
    auto events = r.events;
    events.erase(std::remove_if(events.begin(), events.end(),
                                [](const MeleeEvent &e) {
                                  return e.kind == MeleeEffect::damage;
                                }),
                 events.end());
    if (events != meleeTrace) {
      for (auto e : events)
        std::cerr << "native " << int(e.kind) << ' ' << e.value << '\n';
      for (auto e : meleeTrace)
        std::cerr << "original " << int(e.kind) << ' ' << e.value << '\n';
      throw std::runtime_error("melee endpoint ordering mismatch");
    }
    for (const auto &range : originalCode)
      check(std::memcmp(reinterpret_cast<void *>(range.first),
                        range.second.data(), range.second.size()) == 0,
            "original text changed");
  }
};
void putRandom(const RandomState &s) {
  global(0x6e1ac8, s.left);
  global(0x6e1acc, s.right);
  std::memcpy(reinterpret_cast<void *>(0x6e1ad0), s.words.data(), 1000);
}
void checkRandom(const RandomState &s) {
  check(*reinterpret_cast<unsigned *>(0x6e1ac8) == s.left &&
            *reinterpret_cast<unsigned *>(0x6e1acc) == s.right &&
            std::memcmp(reinterpret_cast<void *>(0x6e1ad0), s.words.data(),
                        1000) == 0,
        "RNG word/cursor consumption differs");
}
#endif
void formula(const MeleeInputs &p, unsigned seed, unsigned hp, bool stream=false) {
  streamStun=stream;
  auto random = seedRandom(seed);
#ifdef MNM_HEALTH_REFERENCE
  Fixture fixture(p, hp);
  putRandom(random);
#endif
  MeleeDispatch dispatch;std::vector<MeleeEvent> immediate;
  if(stream){dispatch.event=[&](const auto &e){immediate.push_back(e);};dispatch.tryStun=[&](int){nextRandom(random);return p.stunHandled;};}
  auto result = resolveMelee(p, [&] { return nextRandom(random); },dispatch);
  if(stream)check(immediate==result.events,"streamed melee endpoint order");
  State before;
  before.health = hp;
  Inputs h;
  h.baseMaximum = 360;
  h.slot = 1;
  h.attributionOwner = 0;
  h.contributions = true;
  h.contributionIds = {{0xffffffffU, 0xffffffffU}};
  h.creditAvailable = {{false, false}};
  auto expected = result.amount
                      ? apply(before, h, Entry::defendedDamage, result.amount)
                      : Result{before, {}, 0};
#ifdef MNM_HEALTH_REFERENCE
  using Fn = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Fn>(0x50c0d0)(fixture.pool.data());
  fixture.verify(result, expected.state);
  if (trace != expected.events) {
    for (auto e : expected.events)
      std::cerr << "native health " << unsigned(e.kind) << ' ' << e.a << ' '
                << e.b << ' ' << e.c << '\n';
    for (auto e : trace)
      std::cerr << "original health " << unsigned(e.kind) << ' ' << e.a << ' '
                << e.b << ' ' << e.c << '\n';
    throw std::runtime_error("composed original health callbacks differ");
  }
  checkRandom(random);
#endif
  ++meleeCases;
  meleeDraws += result.draws.size();
}
void action(unsigned event, unsigned seed, bool present, bool blocked,
            bool slow, unsigned incoming) {
  auto random = seedRandom(seed);
  MeleeInputs p;
  State before;
  before.health = 360;
  MeleeActionState state;
  state.incoming = incoming;
#ifdef MNM_HEALTH_REFERENCE
  Fixture fixture(p, 360);
  putRandom(random);
  put(fixture.sourceBefore, 0x5fc, 10U);
  put(fixture.sourceBefore, 0x664, state.engaged);
  put(fixture.targetBefore, 0x668, state.incoming);
  put(fixture.sourceBefore, 0x855, blocked ? 1U : 0U);
  put(fixture.sourceBefore, 0x77c, slow ? 1U : 0U);
  if (!present)
    put(fixture.sourceBefore, 0x64c, 0xffffffffU);
  std::copy(fixture.sourceBefore.begin(), fixture.sourceBefore.end(),
            fixture.pool.begin());
  std::copy(fixture.targetBefore.begin(), fixture.targetBefore.end(),
            fixture.pool.begin() + 0xe4b);
  // An authored event packet passed through the UNCHANGED original controller.
  unsigned packet[22] = {};
  packet[0] = 5;
  packet[1] = event;
  packet[11] = 6;
  packet[12] = 0xffffffffU;
  put(fixture.pool, 0xb0 + 8, 1U);
  put(fixture.pool, 0xb0 + 24,
      std::uint32_t(reinterpret_cast<std::uintptr_t>(packet)));
  fixture.sourceBefore =
      Bytes(fixture.pool.begin(), fixture.pool.begin() + 0xe4b);
#endif
  auto classified = meleeEvent(state, event, present, true, blocked, slow, 0,
                               [&] { return nextRandom(random); });
  MeleeResult result;
  if (classified.hit)
    result = resolveMelee(p, [&] { return nextRandom(random); });
  Inputs h;
  h.baseMaximum = 360;
  h.slot = 1;
  h.attributionOwner = 0;
  h.contributions = true;
  h.contributionIds = {{0xffffffffU, 0xffffffffU}};
  h.creditAvailable = {{false, false}};
  const auto expected =
      result.amount ? apply(before, h, Entry::defendedDamage, result.amount)
                    : Result{before, {}, 0};
#ifdef MNM_HEALTH_REFERENCE
  using Fn = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Fn>(0x50e7a0)(fixture.pool.data());
  // Controller advancement is validated independently; retain its actual
  // bytes here so the source guard covers everything else in the action.
  std::copy(fixture.pool.begin() + 0xb0, fixture.pool.begin() + 0xe0,
            fixture.sourceBefore.begin() + 0xb0);
  fixture.verify(result, expected.state, true, classified.state);
  check(trace == expected.events, "main action health callbacks differ");
  checkRandom(random);
#endif
  ++actionCases;
}
void exchange(const std::vector<mnm::assets::AnimationRecord> &records,
              const std::vector<unsigned char> &bytes, unsigned sequence,
              unsigned seed, unsigned hp) {
  MeleeInputs p;
  auto random = seedRandom(seed);
  State health;
  health.health = hp;
  Inputs h;
  h.baseMaximum = 360;
  h.slot = 1;
  h.attributionOwner = 0;
  h.contributions = true;
  h.contributionIds = {{0xffffffffU, 0xffffffffU}};
  h.creditAvailable = {{false, false}};
  NoCdAnimationPlayer player(records);
  player.start();
  MeleeActionState state;
#ifdef MNM_HEALTH_REFERENCE
  Fixture fixture(p, hp);
  putRandom(random);
  put(fixture.sourceBefore, 0x5fc, 10U);
  put(fixture.sourceBefore, 0x664, 1U);
  put(fixture.targetBefore, 0x668, 1U);
  std::copy(fixture.sourceBefore.begin(), fixture.sourceBefore.end(),
            fixture.pool.begin());
  std::copy(fixture.targetBefore.begin(), fixture.targetBefore.end(),
            fixture.pool.begin() + 0xe4b);
  const auto offsets = u32(bytes, 20);
  const auto base = 44 + offsets * 4;
  std::uint32_t asset[3] = {
      std::uint32_t(reinterpret_cast<std::uintptr_t>(bytes.data())),
      std::uint32_t(reinterpret_cast<std::uintptr_t>(bytes.data() + 44)),
      std::uint32_t(reinterpret_cast<std::uintptr_t>(bytes.data() + base))};
  put(fixture.pool, 0xb0 + 28,
      std::uint32_t(reinterpret_cast<std::uintptr_t>(asset)));
  using Start = void(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
  reinterpret_cast<Start>(0x464cb0)(fixture.pool.data() + 0xb0, sequence, 0);
  fixture.sourceBefore =
      Bytes(fixture.pool.begin(), fixture.pool.begin() + 0xe4b);
#else
  (void)bytes;
  (void)sequence;
#endif
  for (unsigned tick = 0; tick < 256 && player.state().active; ++tick) {
    const auto event = player.tick();
    MeleeResult result;
    Result changed{health, {}, 0};
    if (event == 2) {
      result = resolveMelee(p, [&] { return nextRandom(random); });
      if (result.amount)
        changed = apply(health, h, Entry::defendedDamage, result.amount);
    }
    auto action =
        meleeEvent(state, event, true, changed.state.health != 0, false, false,
                   0, [&] { return nextRandom(random); });
    state = action.state;
    health = changed.state;
#ifdef MNM_HEALTH_REFERENCE
    trace.clear();
    meleeTrace.clear();
    using Fn = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Fn>(0x50e7a0)(fixture.pool.data());
    std::copy(fixture.pool.begin() + 0xb0, fixture.pool.begin() + 0xe0,
              fixture.sourceBefore.begin() + 0xb0);
    fixture.verify(result, health, true, state);
    check(trace == changed.events,
          "installed animation -> original melee -> health differs");
    checkRandom(random);
#endif
    ++animationTicks;
  }
  check(state.ready && state.cooldown >= 2 && state.cooldown <= 4,
        "composed attack finish/cooldown");
  ++actionCases;
}
void animation(const char *path) {
  std::ifstream f(path, std::ios::binary);
  check(bool(f), "animation input");
  std::vector<unsigned char> b(std::istreambuf_iterator<char>(f), {});
  auto decoded = mnm::assets::decodeAnimation(b);
  check(std::holds_alternative<mnm::assets::Animation>(decoded), "ANI decode");
  auto a = std::get<mnm::assets::Animation>(decoded);
  for (unsigned sequence = 24; sequence < 32; ++sequence) {
    std::vector<mnm::assets::AnimationRecord> records(
        a.records.begin() + a.starts.at(sequence),
        a.records.begin() + a.starts.at(sequence + 1));
    for (unsigned seed = 0; seed < 32; ++seed)
      for (unsigned hp : {1U, 360U})
        exchange(records, b, sequence, seed, hp);
    NoCdAnimationPlayer player(records);
    player.start();
#ifdef MNM_HEALTH_REFERENCE
    std::uint32_t asset[3] = {
        std::uint32_t(reinterpret_cast<std::uintptr_t>(b.data())),
        std::uint32_t(reinterpret_cast<std::uintptr_t>(b.data() + 44)),
        std::uint32_t(reinterpret_cast<std::uintptr_t>(b.data() + 44 +
                                                       a.starts.size() * 4))};
    std::uint32_t state[12] = {};
    state[7] = std::uint32_t(reinterpret_cast<std::uintptr_t>(asset));
    using Start = void(__attribute__((thiscall)) *)(void *, unsigned, unsigned);
    reinterpret_cast<Start>(0x464cb0)(state, sequence, 0);
    const auto beginning = reinterpret_cast<std::uintptr_t>(
        b.data() + 44 + a.starts.size() * 4 + a.starts[sequence] * 44);
#endif
    unsigned hits = 0;
    for (unsigned tick = 0; tick < 128; ++tick) {
      auto event = player.tick();
      if (event == 2)
        ++hits;
#ifdef MNM_HEALTH_REFERENCE
      using Tick = int(__attribute__((thiscall)) *)(void *);
      auto originalEvent = reinterpret_cast<Tick>(0x464ec0)(state);
      const auto &s = player.state();
      check(event == originalEvent && s.pc == (state[6] - beginning) / 44 &&
                bool(state[2]) == s.active && state[3] == s.delay &&
                state[4] == s.elapsed && state[5] == s.repeats &&
                state[8] == s.breakFlag &&
                (s.displayedRecord ? std::int64_t(*s.displayedRecord) : -1) ==
                    (state[1] ? std::int64_t((state[1] - beginning) / 44) : -1),
            "installed melee ANI continuation differs");
#endif
      // Native checkpoint continuation of the complete controller state.
      NoCdAnimationPlayer copy(records);
      copy.restore(player.state());
      auto future = player;
      for (unsigned n = 0; n < 4; ++n)
        check(copy.tick() == future.tick() &&
                  copy.state().pc == future.state().pc,
              "mid-animation continuation");
      ++animationTicks;
    }
    check(hits == 1 && !player.state().active,
          "one installed hit event and stop required");
  }
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_HEALTH_REFERENCE
  check(argc == 3, "expected immutable PE and installed Crocodile ANI");
  initializeMelee(argv[1]);
#else
  check(argc == 2, "expected installed Crocodile ANI");
#endif
  for (unsigned n = 0; n < 256; ++n) {
    auto random = seedRandom(n * 0x1020304U);
#ifdef MNM_HEALTH_REFERENCE
    using Seed = void(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<Seed>(0x54e170)(reinterpret_cast<void *>(0x6e1ac8),
                                     n * 0x1020304U);
    checkRandom(random);
#endif
    for (unsigned j = 0; j < 1024; ++j) {
      const auto value = nextRandom(random);
#ifdef MNM_HEALTH_REFERENCE
      using Next = unsigned(__attribute__((thiscall)) *)(void *);
      check(value == reinterpret_cast<Next>(0x54e200)(
                         reinterpret_cast<void *>(0x6e1ac8)),
            "original RNG value");
      checkRandom(random);
#else
      (void)value;
#endif
    }
  }
  for (unsigned seed = 0; seed < 128; ++seed)
    for (int base : {-1, 0, 1, 18, 1000000, 0x7fffffff})
      for (unsigned hp : {1U, 18U, 360U}) {
        MeleeInputs p;
        p.base = base;
        formula(p, seed, hp);
      }
  for (unsigned seed = 0; seed < 64; ++seed)
    for (unsigned mask = 0; mask < 32; ++mask)
      for (unsigned rank : {0U, 1U, 2U}) {
        MeleeInputs p;
        p.attackerPhysical = mask & 1;
        p.targetPhysical = mask & 2;
        p.bloodLust = mask & 4;
        p.quarterFlag = mask & 8;
        p.targetResponse = mask & 16;
        p.power = int(seed % 5) * 25 - 25;
        p.height = int(seed % 3) - 1;
        p.heightBonus = 32;
        p.responseModifier = int(seed % 7) * 16 - 32;
        p.rank = rank;
        p.rankBonus = 25;
        p.stunHandled = seed & 1;
        formula(p, seed, 360);
      }
  for(unsigned seed=0;seed<128;++seed)for(int scalar:{-1,0,200,201,499,500,501,749,750,751,999,1000,1001,0x7fffffff}){
    MeleeInputs p;p.charge=true;p.chargeScalar=scalar;p.chargeBonus=seed%2?62:-1000000;p.stunThreshold=99;p.stunHandled=seed&1;formula(p,seed,360,true);
  }
  streamStun=false;
  for (unsigned seed = 0; seed < 64; ++seed)
    for (unsigned event : {0U, 1U, 2U, 3U, 4U})
      for (bool blocked : {false, true})
        for (bool slow : {false, true})
          for (unsigned incoming : {0U, 1U, 2U})
            action(event, seed, true, blocked, slow, incoming);
  for (unsigned seed = 0; seed < 16; ++seed)
    action(2, seed, false, false, false, 1);
  animation(argv[argc - 1]);
  std::cout << "{\"success\":true,\"cases\":" << meleeCases
            << ",\"draws\":" << meleeDraws
            << ",\"action_cases\":" << actionCases
            << ",\"animation_ticks\":" << animationTicks
            << ",\"charge_stream_cases\":1792,\"rng_words\":262144}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << "case " << meleeCases << " action " << actionCases << ": "
            << e.what() << '\n';
  return 1;
}
