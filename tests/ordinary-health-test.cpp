#include "health.hpp"
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#ifdef MNM_HEALTH_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#endif
using namespace mnm::reconstruction::combat;
namespace {
void check(bool b, const char *reason) {
  if (!b)
    throw std::runtime_error(reason);
}
std::uint64_t cases = 0, events = 0;
#ifdef MNM_HEALTH_REFERENCE
const Inputs *inputs = nullptr;
std::vector<Event> trace;
std::array<unsigned char, 0x40> creditA{}, creditB{};
void emit(Effect k, std::uint32_t a = 0, std::uint32_t b = 0,
          std::uint32_t c = 0) {
  trace.push_back({k, a, b, c});
}
extern "C" std::uint32_t __attribute__((thiscall)) status(void *) {
  emit(Effect::status);
  return inputs->status;
}
extern "C" void __attribute__((thiscall)) mitigated(void *, std::uint32_t a) {
  emit(Effect::mitigated, a);
}
extern "C" int __attribute__((thiscall)) response(void *, std::uint32_t a) {
  emit(Effect::response, a);
  return inputs->response;
}
extern "C" void __attribute__((thiscall)) damageNotice(void *,
                                                       std::uint32_t a) {
  emit(Effect::damageNotice, a);
}
extern "C" int __attribute__((thiscall)) scenario(void *, void *) {
  emit(Effect::scenarioQuery);
  return 0;
}
extern "C" void __attribute__((thiscall)) killTracker(void *, void *) {
  emit(Effect::killTracker);
}
extern "C" int __attribute__((thiscall)) definition(void *) {
  emit(Effect::definitionQuery);
  return 0;
}
extern "C" void __attribute__((thiscall)) spatial(void *) {
  emit(Effect::spatialCleanup);
}
extern "C" void __attribute__((thiscall)) detach(void *, std::uint32_t a) {
  emit(Effect::detachActor, a);
}
extern "C" void __attribute__((thiscall)) behavior(void *, std::uint32_t a) {
  emit(Effect::behavior, a);
}
extern "C" void __attribute__((thiscall)) repair(void *) {
  emit(Effect::repair);
}
extern "C" std::uint32_t __attribute__((fastcall))
feedback(std::uint32_t, std::uint32_t, std::uint32_t a, std::uint32_t b,
         std::uint32_t, std::uint32_t d) {
  emit(Effect::feedback, a, b, d);
  return inputs->feedbackValue;
}
extern "C" void *__attribute__((thiscall)) credit(void *self,
                                                  std::uint32_t id) {
  if (reinterpret_cast<std::uintptr_t>(self) == 0x690600) {
    emit(Effect::creditA, id);
    return inputs->creditAvailable[0] ? creditA.data() : nullptr;
  }
  check(reinterpret_cast<std::uintptr_t>(self) == 0x644550, "credit receiver");
  emit(Effect::creditB, id);
  return inputs->creditAvailable[1] ? creditB.data() : nullptr;
}
extern "C" void __attribute__((thiscall))
attribution(void *self, std::uint32_t a, std::uint32_t b) {
  emit(Effect::attribution, a, b,
       (reinterpret_cast<std::uintptr_t>(self) - 0x643a80) / 340);
}
extern "C" void *__attribute__((thiscall)) selected(void *, std::uint32_t a) {
  emit(Effect::selectedQuery, a);
  return nullptr;
}
extern "C" void __attribute__((thiscall)) barSet(void *, std::uint32_t a) {
  emit(Effect::barSet, a);
}
extern "C" void __attribute__((thiscall)) barNotify(void *) {
  emit(Effect::barNotify);
}
extern "C" void __attribute__((thiscall)) pain(void *, void *, unsigned a) {
  emit(Effect::pain, a);
}
struct Redirect {
  std::uint32_t address;
  std::uintptr_t function;
};
void redirect(const Redirect &r) {
  auto *at = reinterpret_cast<unsigned char *>(r.address);
  const auto offset = std::uint32_t(r.function) - r.address - 5;
  at[0] = 0xe9;
  std::memcpy(at + 1, &offset, 4);
}
std::vector<std::pair<std::uint32_t, Bytes>> originalCode;
void initialize(const char *path) {
  map_image(read(path));
  for (auto range :
       std::vector<std::pair<unsigned, unsigned>>{{0x5075f0, 0x5076a9},
                                                  {0x5076f0, 0x507c20},
                                                  {0x514860, 0x514982},
                                                  {0x514990, 0x514c56},
                                                  {0x51f5e0, 0x51fa53}}) {
    auto *p = reinterpret_cast<unsigned char *>(range.first);
    originalCode.push_back(
        {range.first, Bytes(p, p + range.second - range.first)});
  }
  check(*reinterpret_cast<unsigned char *>(0x514860) == 0x56 &&
            *reinterpret_cast<unsigned char *>(0x5076f0) == 0x51 &&
            *reinterpret_cast<unsigned char *>(0x514990) == 0x81,
        "health entry signatures");
  const Redirect endpoints[] = {
      {0x54de10, reinterpret_cast<std::uintptr_t>(&status)},
      {0x51fd60, reinterpret_cast<std::uintptr_t>(&mitigated)},
      {0x513e40, reinterpret_cast<std::uintptr_t>(&response)},
      {0x51fba0, reinterpret_cast<std::uintptr_t>(&damageNotice)},
      {0x56c290, reinterpret_cast<std::uintptr_t>(&scenario)},
      {0x570bd0, reinterpret_cast<std::uintptr_t>(&killTracker)},
      {0x411e50, reinterpret_cast<std::uintptr_t>(&definition)},
      {0x5137d0, reinterpret_cast<std::uintptr_t>(&spatial)},
      {0x4d9ac0, reinterpret_cast<std::uintptr_t>(&detach)},
      {0x50aa30, reinterpret_cast<std::uintptr_t>(&behavior)},
      {0x522340, reinterpret_cast<std::uintptr_t>(&repair)},
      {0x416d80, reinterpret_cast<std::uintptr_t>(&feedback)},
      {0x416c30, reinterpret_cast<std::uintptr_t>(&credit)},
      {0x4656c0, reinterpret_cast<std::uintptr_t>(&attribution)},
      {0x466870, reinterpret_cast<std::uintptr_t>(&selected)},
      {0x58f830, reinterpret_cast<std::uintptr_t>(&barSet)},
      {0x58f890, reinterpret_cast<std::uintptr_t>(&barNotify)},
      {0x4d9890, reinterpret_cast<std::uintptr_t>(&pain)}};
  for (const auto &r : endpoints)
    redirect(r);
}
template <class T> void put(Bytes &object, unsigned at, T v) {
  std::memcpy(object.data() + at, &v, sizeof(v));
}
std::uint32_t get(const Bytes &object, unsigned at) {
  std::uint32_t v;
  std::memcpy(&v, object.data() + at, 4);
  return v;
}
Result original(State state, const Inputs &p, Entry entry, std::int32_t value) {
  inputs = &p;
  trace.clear();
  Bytes object(0xe4b, 0), config(0x5c9, 0);
  put(config, 0, 1U);
  put(config, 0x18, p.baseMaximum);
  put(object, 0, p.slot);
  put(object, 4, 1U);
  put(object, 0xa8, 17U);
  put(object, 0xac,
      std::uint32_t(reinterpret_cast<std::uintptr_t>(config.data())));
  const auto fields = [&](const State &s) {
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
      put(object, item.first, item.second);
    for (unsigned i = 0; i < 8; ++i) {
      put(object, 0x24 + i * 4, ((p.visibleMask >> i) & 1));
      put(object, 0xdeb + i * 4, s.incoming[i]);
      put(object, 0xe0b + i * 4, s.ages[i]);
    }
  };
  fields(state);
  put(object, 0x174, p.owner);
  put(object, 0x90, p.mitigation);
  put(object, 0x81d, p.quarter);
  put(object, 0xa0, p.absorption);
  put(object, 0x8c5, p.blocked);
  const auto before = object;
  global(0x6be2de, p.mitigationPercent);
  global(0x5e1910, p.bonusOne);
  global(0x5e1914, p.bonusTwo);
  *reinterpret_cast<unsigned char *>(0x6e9858) = p.globalGate ? 1 : 0;
  global(0x644520, p.selectedOwner);
  global(0x6df180, 0);
  global(0x6898dc, 0);
  for (unsigned i = 0; i < 8; ++i)
    global(0x643a80 + i * 340 + 12, 0xffffffffU);
  auto pair = p.contributionIds;
  creditA.fill(0);
  creditB.fill(0);
  std::memcpy(creditA.data() + 0x2f, &state.credit[0], 4);
  std::memcpy(creditB.data() + 0x2f, &state.credit[1], 4);
  auto *contribution = p.contributions ? pair.data() : nullptr;
  std::uint32_t returned = 0;
  if (entry == Entry::maximum) {
    using Fn = std::uint32_t(__attribute__((thiscall)) *)(void *);
    returned = reinterpret_cast<Fn>(0x5075f0)(object.data());
  } else if (entry == Entry::signedHealth) {
    using Fn = void(__attribute__((thiscall)) *)(void *, int, int, void *, int);
    reinterpret_cast<Fn>(0x5076f0)(object.data(), value, p.attributionOwner,
                                   contribution, p.notify);
  } else if (entry == Entry::defendedDamage) {
    using Fn = void(__attribute__((thiscall)) *)(void *, int, int, void *);
    reinterpret_cast<Fn>(0x514860)(object.data(), value, p.attributionOwner,
                                   contribution);
  } else {
    using Fn = std::uint32_t(__attribute__((thiscall)) *)(void *, std::uint32_t,
                                                          void *);
    returned = reinterpret_cast<Fn>(0x514990)(
        object.data(), static_cast<std::uint32_t>(value), contribution);
  }
  State result = state;
  result.health = get(object, 0xe4);
  result.bar = get(object, 0x6c);
  result.dirty = get(object, 0x92e);
  result.cooldown = get(object, 0x660);
  result.target = get(object, 0x64c);
  result.gate9c = get(object, 0x9c);
  result.expiry = get(object, 0x778);
  result.timer = get(object, 0x780);
  result.cacheA = get(object, 0x889);
  result.cacheB = get(object, 0x8bd);
  result.marker = get(object, 0xb8b);
  result.flag104 = get(object, 0x104);
  result.rank = get(object, 0x124);
  for (unsigned i = 0; i < 8; ++i) {
    result.incoming[i] = get(object, 0xdeb + i * 4);
    result.ages[i] = get(object, 0xe0b + i * 4);
  }
  std::memcpy(&result.credit[0], creditA.data() + 0x2f, 4);
  std::memcpy(&result.credit[1], creditB.data() + 0x2f, 4);
  const auto after = object;
  object = before;
  fields(result);
  check(object == after, "original wrote outside projected fields");
  check(config[0] == 1 && get(config, 0x18) == p.baseMaximum,
        "original changed config");
  for (const auto &range : originalCode)
    check(std::memcmp(reinterpret_cast<void *>(range.first),
                      range.second.data(), range.second.size()) == 0,
          "original target text changed");
  return {result, trace, returned};
}
#endif
void compare(State state, Inputs p, Entry entry, std::int32_t amount) {
  const auto n = apply(state, p, entry, amount);
#ifdef MNM_HEALTH_REFERENCE
  const auto o = original(state, p, entry, amount);
  if (std::memcmp(&n.state, &o.state, sizeof(State)) || n.events != o.events ||
      n.returned != o.returned) {
    std::cerr << "case " << cases << " entry " << int(entry) << " health "
              << state.health << " amount " << amount << " mitigation "
              << p.mitigation << " percent " << p.mitigationPercent
              << " absorption " << p.absorption << " events " << n.events.size()
              << '/' << o.events.size() << '\n';
    for (const auto &e : n.events)
      std::cerr << "native " << unsigned(e.kind) << ' ' << e.a << ' ' << e.b
                << ' ' << e.c << '\n';
    for (const auto &e : o.events)
      std::cerr << "original " << unsigned(e.kind) << ' ' << e.a << ' ' << e.b
                << ' ' << e.c << '\n';
    throw std::runtime_error("original/native health projection differs");
  }
#endif
  events += n.events.size();
  ++cases;
}
void corpus() {
  // Independent literal checks for the documented ordinary numerical cases;
  // the differential build additionally checks every full projection/event.
  State plain;
  plain.health = 12;
  Inputs policy;
  check(apply(plain, policy, Entry::signedHealth, 5).state.health == 17,
        "ordinary healing");
  check(apply(plain, policy, Entry::signedHealth, 100).state.health == 24,
        "healing saturation");
  plain.health = 24;
  policy.mitigation = 1;
  check(apply(plain, policy, Entry::defendedDamage, 20).state.health == 9,
        "mitigated20 becomes15");
  policy.mitigation = 0;
  policy.quarter = 1;
  check(apply(plain, policy, Entry::defendedDamage, 7).state.health == 23,
        "quarter truncation");
  policy.quarter = 0;
  policy.absorption = 1;
  check(apply(plain, policy, Entry::defendedDamage, 7).state.health == 24,
        "absorption mode1");
  policy.absorption = 2;
  check(apply(plain, policy, Entry::defendedDamage, 7).state.health == 17,
        "absorption mode2 health handoff");
  policy.absorption = 0;
  policy.status = 8;
  auto immune = apply(plain, policy, Entry::defendedDamage, 24);
  check(immune.state.health == 24 && immune.events.size() == 1,
        "status8 refuses damage");
  policy.status = 0;
  policy.globalGate = 1;
  check(apply(plain, policy, Entry::signedHealth, -24).events.empty(),
        "global health refusal");
  policy.globalGate = 0;
  auto dead = apply(plain, policy, Entry::signedHealth, -24);
  check(!dead.state.health && dead.state.target == 0xffffffffU &&
            !dead.state.flag104 && dead.state.dirty == 1,
        "ordinary cleanup projection");
  check(apply(dead.state, policy, Entry::signedHealth, 24).state.health == 0,
        "dead health is not revived");
  const std::array<std::int32_t, 12> values{
      {std::numeric_limits<std::int32_t>::min(), -1000001, -100, -24, -1, 0, 1,
       3, 24, 100, 1000000, std::numeric_limits<std::int32_t>::max()}};
  for (unsigned hp : {0U, 1U, 3U, 24U, 100U, 1000000U})
    for (unsigned cap : {0U, 1U, 24U, 100U, 1000000U})
      for (auto delta : values)
        for (unsigned gate : {0U, 1U})
          for (unsigned notify : {0U, 1U}) {
            State s;
            s.health = hp;
            s.cooldown = (hp % 2) ? 0 : 1;
            Inputs p;
            p.baseMaximum = cap;
            p.globalGate = gate;
            p.notify = notify;
            p.visibleMask = 0x55;
            p.contributions = true;
            p.attributionOwner = 2;
            p.selectedOwner = (hp % 2) ? 1 : 0;
            compare(s, p, Entry::signedHealth, delta);
          }
  for (unsigned hp : {0U, 1U, 24U, 100U})
    for (auto amount : values)
      for (unsigned mit : {0U, 1U})
        for (unsigned percent : {0U, 25U, 99U, 100U, 101U, 0xffffffffU})
          for (unsigned quarter : {0U, 1U})
            for (unsigned absorption : {0U, 1U, 2U})
              for (unsigned response : {0U, 1U}) {
                State s;
                s.health = hp;
                Inputs p;
                p.baseMaximum = 100;
                p.mitigation = mit;
                p.mitigationPercent = percent;
                p.quarter = quarter;
                p.absorption = absorption;
                p.response = response;
                p.attributionOwner = 2;
                p.contributions = true;
                compare(s, p, Entry::defendedDamage, amount);
              }
  for (unsigned status : {0U, 8U, 9U})
    for (unsigned blocked : {0U, 1U})
      for (unsigned hp : {0U, 24U})
        for (auto amount : values) {
          State s;
          s.health = hp;
          Inputs p;
          p.status = status;
          p.blocked = blocked;
          compare(s, p, Entry::defendedDamage, amount);
        }
  for (unsigned hp : {0U, 1U, 24U})
    for (auto value : values) {
      State s;
      s.health = hp;
      Inputs p;
      p.visibleMask = 0xaa;
      compare(s, p, Entry::directDamage, value);
    }
  for (unsigned rank : {0U, 1U, 2U})
    for (unsigned cap : {0U, 1U, 24U, 1000000U, 0xffffffffU})
      for (unsigned percent : {0U, 25U, 100U, 101U, 0xffffffffU}) {
        State s;
        s.rank = rank;
        Inputs p;
        p.baseMaximum = cap;
        p.bonusOne = p.bonusTwo = percent;
        compare(s, p, Entry::maximum, 0);
      }
  for (unsigned mask = 0; mask < 4; ++mask) {
    State s;
    Inputs p;
    p.contributions = true;
    p.contributionIds = {{0xffffffffU, 0xffffffffU}};
    p.creditAvailable = {{bool(mask & 1), bool(mask & 2)}};
    p.slot = mask * 31;
    compare(s, p, Entry::defendedDamage, 3);
    compare(s, p, Entry::defendedDamage, 24);
  }
  State s;
  Inputs p;
  p.baseMaximum = 24;
  for (auto delta : {-3, 5, -1, 0, 100, -24, 5}) {
    compare(s, p, Entry::signedHealth, delta);
    s = apply(s, p, Entry::signedHealth, delta).state;
  }
  check(apply(State{}, Inputs{}, Entry::signedHealth, 100).state.health == 24,
        "heal cap");
}
} // namespace
#ifndef MNM_HEALTH_MAIN
#define MNM_HEALTH_MAIN main
#endif
int MNM_HEALTH_MAIN(int argc, char **argv) try {
#ifdef MNM_HEALTH_REFERENCE
  check(argc == 2, "Expected immutable pinned PE path");
  initialize(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  corpus();
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"events\":" << events << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
