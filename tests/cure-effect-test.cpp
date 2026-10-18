#include "../reconstruction/casting/cure_effect.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#ifdef MNM_CURE_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#include <asm/ldt.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

using namespace mnm::reconstruction::casting;
namespace {
void require(bool ok, const std::string &why) {
  if (!ok)
    throw std::runtime_error(why);
}
std::uint32_t bits(std::int32_t n) { return std::uint32_t(n); }
std::int32_t signedBits(std::uint32_t n) {
  return n <= 0x7fffffffu ? std::int32_t(n) : -1 - std::int32_t(~n);
}
struct Fixture {
  CureEffect effect;
  std::vector<CureActor> actors;
  CureConfiguration configuration;
  unsigned number = 0;
};
Fixture *current = nullptr;
std::vector<std::uint32_t> trace;
std::array<unsigned, 9> calls{};
void event(unsigned kind, std::initializer_list<std::uint32_t> values) {
  ++calls.at(kind);
  trace.push_back(kind);
  trace.push_back(unsigned(values.size()));
  trace.insert(trace.end(), values);
}
unsigned actorIndex(const CureActor &actor) {
  return unsigned(&actor - current->actors.data());
}
std::uint32_t resultingHealth(std::uint32_t old) {
  switch (current->number % 5) {
  case 0: return old;
  case 1: return old + 17;
  case 2: return old - 1;
  case 3: return 0;
  default: return UINT32_MAX;
  }
}
std::uint32_t cleanseHealth(std::uint32_t old) {
  return current->number % 7 == 0 ? old ^ 0x80000000u : old;
}
CureServices services() {
  CureServices s;
  s.health = [](auto &a, auto magnitude, auto attribution, const auto &credits,
                auto flag) {
    event(1, {actorIndex(a), bits(magnitude), attribution, credits[0],
              credits[1], flag});
    a.health = resultingHealth(a.health);
  };
  s.cleanse = [](auto &a) {
    event(2, {actorIndex(a)});
    a.status = 0;
    a.health = cleanseHealth(a.health);
  };
  s.cue = [](unsigned code, CurePosition p) {
    event(3, {code, bits(p.x), bits(p.y), bits(p.z), 0, 0, 0});
  };
  s.feedback = [](auto &, auto &a) {
    event(4, {actorIndex(a), a.health, a.status, a.ongoingDamage});
  };
  s.transition = [](auto &e, unsigned type) {
    event(5, {type});
    e.type = type;
  };
  s.chargeable = [](const auto &a) {
    event(6, {actorIndex(a)});
    return current->number % 3 != 0;
  };
  s.resource = [](auto &a, unsigned delta) {
    event(7, {actorIndex(a), delta});
    a.resource += delta;
  };
  s.release = [](auto &e) {
    event(8, {});
    e.type = 0; // Supplied release endpoint, not recovered release semantics.
  };
  return s;
}
std::uint32_t next(std::uint32_t &state) {
  state ^= state << 13;
  state ^= state >> 17;
  state ^= state << 5;
  return state;
}
Fixture generate(unsigned n) {
  Fixture f;
  f.number = n;
  auto seed = std::uint32_t(n + 1) * 0x9e3779b9u;
  auto random = [&] { return next(seed); };
  constexpr unsigned types[] = {0, 1, 6, 7, 11, 30, 31, UINT32_MAX};
  constexpr unsigned healths[] = {0, 1, 24, 0x7fffffffu, 0x80000000u,
                                 UINT32_MAX};
  constexpr unsigned references[] = {UINT32_MAX, 0, 1, 2, 3, 17, 0xfffffffeu};
  constexpr int owners[] = {-1, 0, 1, 7, 8, INT32_MIN, INT32_MAX};
  constexpr unsigned modes[] = {0, 1, 4, UINT32_MAX};
  f.actors.resize(3);
  for (auto &a : f.actors) {
    a.health = healths[random() % 6];
    a.blocked = random() % 5 == 0 ? random() | 1 : 0;
    a.status = random() % 3 ? random() : 0;
    a.ongoingDamage = random() % 3 ? random() : 0;
    a.resource = random();
    for (auto &visible : a.visible)
      visible = random() % 3;
  }
  auto &e = f.effect;
  e.type = types[random() % 8];
  e.source = random() % 4;
  if (e.source == 3)
    e.source = UINT32_MAX;
  e.target = references[random() % 7];
  e.deliveryMode = modes[random() % 4];
  e.position = {signedBits(random()), signedBits(random()), signedBits(random())};
  e.attribution = random();
  for (auto &credit : e.contributions)
    credit = random();
  if (random() % 2)
    e.cachedActor = random() % 3;
  f.configuration = {random(), signedBits(random()), owners[random() % 7]};
  return f;
}
void semanticChecks() {
  Fixture f;
  f.actors.resize(2);
  f.actors[1].health = 12;
  f.actors[1].visible[0] = 1;
  f.effect.source = 0;
  f.effect.target = 1;
  f.effect.type = 11;
  f.configuration = {9, 5, 0};
  current = &f;
  trace.clear();
  applyCureEffect(f.effect, f.actors, f.configuration, services());
  require(trace == std::vector<std::uint32_t>({5, 1, 77}),
          "Cure delivery must defer before target health");
  f.effect.target = UINT32_MAX;
  f.effect.type = 11;
  f.effect.deliveryMode = 4;
  trace.clear();
  applyCureEffect(f.effect, f.actors, f.configuration, services());
  require(trace.size() == 11 && trace[0] == 3 && trace[2] == 0x3f3 &&
              trace[9] == 8, "Missing target fails before deferred delivery");
  f.effect.target = 1;
  f.effect.type = 0;
  f.actors[1].blocked = 1;
  trace.clear();
  applyCureEffect(f.effect, f.actors, f.configuration, services());
  require(trace.empty() && f.effect.cachedActor == 1,
          "Blocked living Cure target must remain silent");
  f.actors[1].blocked = 0;
  f.actors[1].ongoingDamage = 7;
  f.number = 5; // Health unchanged; existing ongoing damage still earns feedback.
  trace.clear();
  applyCureEffect(f.effect, f.actors, f.configuration, services());
  require(f.actors[1].health == 12 && f.actors[1].ongoingDamage == 0 &&
              trace[trace.size() - 6] == 4,
          "Cure clears ongoing damage and observes pre-call status for feedback");
}
unsigned refusals() {
  for (unsigned i = 0; i < 4; ++i) {
    auto f = generate(1);
    if (i == 0) f.effect.spell = 42;
    if (i == 1) f.effect.source = 3;
    if (i == 2) f.effect.cachedActor = 3;
    if (i == 3) f.actors.resize(65537);
    current = &f;
    trace.clear();
    bool refused = false;
    try { applyCureEffect(f.effect, f.actors, f.configuration, services()); }
    catch (const std::invalid_argument &) { refused = true; }
    require(refused && trace.empty(), "Cure preflight refusal before services");
  }
  return 4;
}

#ifdef MNM_CURE_REFERENCE
Bytes actorBytes, configBytes;
unsigned char *effectBytes = nullptr;
std::array<unsigned char, 0x3939> testedBody{};
std::array<unsigned char, 0x10b> testedDispatch{};
unsigned load(const void *p, unsigned offset) {
  unsigned n;
  std::memcpy(&n, static_cast<const unsigned char *>(p) + offset, 4);
  return n;
}
void put(void *p, unsigned offset, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + offset, &n, 4);
}
unsigned rawActor(const void *p) {
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto base = reinterpret_cast<std::uintptr_t>(actorBytes.data() + 32);
  require(address >= base && address < base + 3 * 0xe4b &&
              (address - base) % 0xe4b == 0, "Cure actor receiver");
  return unsigned((address - base) / 0xe4b);
}
void requireEffect(const void *p) {
  require(p == effectBytes, "Cure effect receiver");
}
extern "C" char *cureItoa(unsigned value, char *buffer, int radix) {
  require(value == 41 && radix == 10, "Cure unused decimal buffer ABI");
  std::memcpy(buffer, "41", 3);
  return buffer;
}
extern "C" void __attribute__((thiscall))
cureHealth(void *p, int magnitude, unsigned attribution, const void *credits,
           unsigned flag) {
  require(credits == effectBytes + 0x216, "Cure contribution pointer ABI");
  event(1, {rawActor(p), bits(magnitude), attribution, load(credits, 0),
            load(credits, 4), flag});
  put(p, 0xe4, resultingHealth(load(p, 0xe4)));
}
extern "C" void __attribute__((thiscall)) cureCleanse(void *p) {
  event(2, {rawActor(p)});
  put(p, 0x114, 0);
  put(p, 0xe4, cleanseHealth(load(p, 0xe4)));
}
extern "C" void __attribute__((thiscall))
cureCue(void *p, unsigned code, int x, int y, int z, unsigned a, unsigned b,
        unsigned c) {
  require(p == reinterpret_cast<void *>(0x658178), "Cure cue receiver");
  event(3, {code, bits(x), bits(y), bits(z), a, b, c});
}
extern "C" void __attribute__((thiscall)) cureFeedback(void *p, void *actor) {
  requireEffect(p);
  event(4, {rawActor(actor), load(actor, 0xe4), load(actor, 0x114),
            load(actor, 0x8d5)});
}
extern "C" void __attribute__((thiscall)) cureTransition(void *p, unsigned type) {
  requireEffect(p);
  event(5, {type});
  put(p, 0x28, type);
}
extern "C" unsigned __attribute__((thiscall)) cureChargeable(void *p) {
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto base = reinterpret_cast<std::uintptr_t>(configBytes.data() + 32);
  require(address >= base && address < base + 3 * 0x5c9 &&
              (address - base) % 0x5c9 == 0, "Cure configuration receiver");
  event(6, {unsigned((address - base) / 0x5c9)});
  return current->number % 3 != 0;
}
extern "C" void __attribute__((thiscall)) cureResource(void *p, unsigned delta) {
  event(7, {rawActor(p), delta});
  put(p, 0xe8, load(p, 0xe8) + delta);
}
extern "C" void __attribute__((thiscall)) cureRelease(void *p) {
  requireEffect(p);
  event(8, {});
  put(p, 0x28, 0);
}
void redirect(unsigned a, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(a);
  p[0] = 0xe9;
  unsigned delta = reinterpret_cast<std::uintptr_t>(target) - a - 5;
  std::memcpy(p + 1, &delta, 4);
}
struct PrivateFs {
  std::array<unsigned, 8> data{};
  unsigned short saved = 0;
  user_desc descriptor{};
  PrivateFs() {
    data.fill(0xa5a5a5a5);
    data[0] = UINT32_MAX;
    descriptor.entry_number = unsigned(-1);
    descriptor.base_addr = reinterpret_cast<std::uintptr_t>(data.data());
    descriptor.limit = 31;
    descriptor.seg_32bit = 1;
    descriptor.useable = 1;
    require(syscall(SYS_set_thread_area, &descriptor) == 0, "Cure private FS");
    asm volatile("mov %%fs,%0" : "=rm"(saved));
    const unsigned short selector = (descriptor.entry_number << 3) | 3;
    asm volatile("mov %0,%%fs" : : "rm"(selector) : "memory");
  }
  ~PrivateFs() {
    asm volatile("mov %0,%%fs" : : "rm"(saved) : "memory");
    user_desc empty{};
    empty.entry_number = descriptor.entry_number;
    empty.seg_not_present = 1;
    empty.read_exec_only = 1;
    (void)syscall(SYS_set_thread_area, &empty);
  }
  void verify() const {
    require(data[0] == UINT32_MAX &&
                std::all_of(data.begin() + 1, data.end(),
                            [](unsigned n) { return n == 0xa5a5a5a5; }),
            "Cure FS chain/guard changed");
  }
};
void install(const char *path) {
  map_image(read(path));
  std::memcpy(testedBody.data(), reinterpret_cast<void *>(0x48b0f0),
              testedBody.size());
  std::memcpy(testedDispatch.data(), reinterpret_cast<void *>(0x48ea2c),
              testedDispatch.size());
  const auto arm = *reinterpret_cast<const unsigned char *>(0x48ead0 + 41);
  require(load(reinterpret_cast<void *>(0x48ea2c), arm * 4) == 0x48b532,
          "Original spell41 dispatch target");
  for (auto x : {std::pair<unsigned, void *>{0x59de2f, (void *)cureItoa},
                 {0x5076f0, (void *)cureHealth},
                 {0x516150, (void *)cureCleanse},
                 {0x571310, (void *)cureCue},
                 {0x49a350, (void *)cureFeedback},
                 {0x494ab0, (void *)cureTransition},
                 {0x411e50, (void *)cureChargeable},
                 {0x507f00, (void *)cureResource},
                 {0x4943f0, (void *)cureRelease}})
    redirect(x.first, x.second);
}
void packActor(void *p, const CureActor &a) {
  for (auto field : {std::pair<unsigned, unsigned>{0xe4, a.health},
                     {0xe8, a.resource}, {0x7c, a.blocked},
                     {0x114, a.status}, {0x8d5, a.ongoingDamage}})
    put(p, field.first, field.second);
  for (unsigned i = 0; i < 8; ++i)
    put(p, 0x24 + i * 4, a.visible[i]);
}
void packEffect(void *p, const CureEffect &e) {
  for (auto field : {std::pair<unsigned, unsigned>{8, bits(e.position.x)},
                     {12, bits(e.position.y)}, {16, bits(e.position.z)},
                     {0x28, e.type}, {0x44, e.source}, {0x48, e.target},
                     {0x4c, e.spell}, {0x5c, e.deliveryMode},
                     {0x212, e.attribution}, {0x216, e.contributions[0]},
                     {0x21a, e.contributions[1]},
                     {0x198, e.cachedActor ? unsigned(reinterpret_cast<std::uintptr_t>(
                         actorBytes.data() + 32 + *e.cachedActor * 0xe4b)) : 0}})
    put(p, field.first, field.second);
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<std::uint32_t> &expectedTrace) {
  current = &input;
  trace.clear();
  actorBytes.assign(3 * 0xe4b + 64, 0x9d);
  configBytes.assign(3 * 0x5c9 + 64, 0xb1);
  for (unsigned i = 0; i < 3; ++i) {
    auto *p = actorBytes.data() + 32 + i * 0xe4b;
    packActor(p, input.actors[i]);
    put(p, 0xac, reinterpret_cast<std::uintptr_t>(configBytes.data() + 32 + i * 0x5c9));
  }
  auto expectedActors = actorBytes;
  const auto initialConfigs = configBytes;
  Bytes effectStorage(0x22e + 64, 0x85);
  effectBytes = effectStorage.data() + 32;
  packEffect(effectBytes, input.effect);
  auto expectedEffect = effectStorage;
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actorBytes.data() + 32));
  global(0x6def5c, 3);
  global(0x644520, bits(input.configuration.localOwner));
  constexpr unsigned spellAddress = 0x6b1270 + 41 * 721;
  Bytes spells(721, 0xe1);
  put(spells.data(), 0, input.configuration.cost);
  put(spells.data(), 4, bits(input.configuration.magnitude));
  std::memcpy(reinterpret_cast<void *>(spellAddress), spells.data(), spells.size());
  static PrivateFs fs;
  using Dispatch = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Dispatch>(0x48b0f0)(effectBytes);
  fs.verify();
  require(trace == expectedTrace, "Cure call trace differs: " + std::to_string(input.number));
  for (unsigned i = 0; i < 3; ++i)
    packActor(expectedActors.data() + 32 + i * 0xe4b, expected.actors[i]);
  packEffect(expectedEffect.data() + 32, expected.effect);
  require(actorBytes == expectedActors && effectStorage == expectedEffect &&
              configBytes == initialConfigs, "Cure full object guards differ: " + std::to_string(input.number));
  require(!std::memcmp(reinterpret_cast<void *>(spellAddress), spells.data(), spells.size()) &&
              !std::memcmp(testedBody.data(), reinterpret_cast<void *>(0x48b0f0), testedBody.size()) &&
              !std::memcmp(testedDispatch.data(), reinterpret_cast<void *>(0x48ea2c), testedDispatch.size()),
          "Cure read-only spell/body/dispatch guard");
}
#endif

std::uint64_t digest = 1469598103934665603ull;
void hash(unsigned n) { digest = (digest ^ n) * 1099511628211ull; }
void hashOutcome(const Fixture &f) {
  hash(f.effect.type);
  hash(f.effect.cachedActor.value_or(UINT32_MAX));
  for (const auto &a : f.actors) {
    hash(a.health); hash(a.blocked); hash(a.status); hash(a.ongoingDamage);
    hash(a.resource);
    for (auto visible : a.visible) hash(visible);
  }
  for (auto n : trace) hash(n);
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_CURE_REFERENCE
  require(argc == 2, "expected pinned PE input");
  install(argv[1]);
#else
  (void)argv;
  require(argc == 1, "unexpected Cure test argument");
#endif
  semanticChecks();
  const auto nativeRefusals = refusals();
  calls.fill(0);
  for (unsigned n = 0; n < 32768; ++n) {
    auto input = generate(n), expected = input;
    current = &expected;
    trace.clear();
    applyCureEffect(expected.effect, expected.actors, expected.configuration, services());
    hashOutcome(expected);
#ifdef MNM_CURE_REFERENCE
    const auto expectedTrace = trace;
    const auto nativeCalls = calls;
    compare(input, expected, expectedTrace);
    calls = nativeCalls;
#endif
  }
  for (unsigned i = 1; i < calls.size(); ++i)
    require(calls[i] != 0, "Cure service branch unexercised");
  std::cout << "{\"success\":true,\"cases\":32768,\"functions\":1,"
               "\"native_refusal_cases\":" << nativeRefusals << ",\"outcome_digest\":\""
            << std::hex << digest << std::dec << "\",\"service_calls\":[";
  for (unsigned i = 1; i < calls.size(); ++i)
    std::cout << (i == 1 ? "" : ",") << calls[i];
  std::cout << "]}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
