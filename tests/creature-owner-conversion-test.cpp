#include "../reconstruction/creature-ai/owner_conversion.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_CONVERSION_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousWorld world;
  OwnerConversionState state;
  unsigned number = 0;
};
Fixture *current;
std::vector<unsigned> trace;
bool original = false;
void event(unsigned kind, std::initializer_list<unsigned> args = {}) {
  trace.push_back(kind);
  trace.push_back(unsigned(args.size()));
  trace.insert(trace.end(), args.begin(), args.end());
}
bool subjectPresent() { return current->number % 11 != 0; }
#ifdef MNM_CONVERSION_REFERENCE
Bytes actors;
unsigned get(const void *p, unsigned off) {
  unsigned n;
  std::memcpy(&n, static_cast<const unsigned char *>(p) + off, 4);
  return n;
}
void put(void *p, unsigned off, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &n, 4);
}
unsigned slot(void *p) {
  auto difference = static_cast<unsigned char *>(p) - actors.data();
  if (difference < 0 || unsigned(difference) >= actors.size() ||
      difference % 0xe4b)
    throw std::runtime_error("conversion actor ABI");
  return unsigned(difference) / 0xe4b;
}
#endif
void notify(unsigned id) {
  event(1, {id});
  if (current->number % 13 == 0) {
    auto &owner = current->world.creatures[0].ai.owner;
    owner = (owner + 1) % 8;
#ifdef MNM_CONVERSION_REFERENCE
    if (original)
      put(actors.data(), 0x174, unsigned(owner));
#endif
  }
}
void transfer(unsigned id, int owner) {
  event(2, {id, unsigned(owner)});
#ifdef MNM_CONVERSION_REFERENCE
  if (original) {
    put(actors.data() + id * 0xe4b, 0x174, unsigned(owner));
    if (subjectPresent())
      put(actors.data() + 0xe4b, 0x8f5, get(actors.data() + 0xe4b, 0x8f5) + 1);
    return;
  }
#endif
  current->world.creatures.at(id).ai.owner = owner;
  if (subjectPresent())
    ++current->state.limits[1].current;
}
OwnerConversionServices services() {
  OwnerConversionServices s;
  s.subject = [](int owner) -> std::optional<unsigned> {
    event(0, {unsigned(owner)});
    return subjectPresent() ? std::optional<unsigned>(1) : std::nullopt;
  };
  s.notify = notify;
  s.transfer = [](auto &a, int owner) { transfer(a.ai.slot, owner); };
  return s;
}
Fixture generate(unsigned n) {
  Fixture f;
  f.number = n;
  f.world.dimensions = {16, 16, 4};
  unsigned count = 125 + n % 4;
  f.world.creatures.resize(count);
  f.state.cursor.resize(count);
  f.state.limits.resize(count);
  for (unsigned i = 0; i < count; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.owner = int((n + i) % 8);
    a.ai.type = (n / 7 + i) % 28;
    a.ai.active = (n + i) % 5 != 0;
    a.ai.health = (n + i) % 7 ? 360 : 0;
    a.ai.position = {int((n + i) % 16), int((n / 16 + i) % 16),
                     int((n + i) % 4)};
    f.state.cursor[i] = int((n + i) % 125);
    f.state.limits[i] = {int(n % 4), 1 + int(n / 4 % 5)};
  }
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      f.world.relation[i][j] = i == j ? 1 : ((n + i + j) % 9 == 0);
  auto &source = f.world.creatures[0];
  source.ai.owner = int(n % 8);
  source.ai.type = n / 7 % 28;
  source.ai.position = {int(n / 3 % 16), int(n / 5 % 16), int(n % 4)};
  unsigned first = unsigned(f.state.cursor[0]);
  auto &candidate = f.world.creatures[first];
  candidate.ai.active = true;
  candidate.ai.health = 360;
  candidate.ai.type = source.ai.type;
  if (first) {
    candidate.ai.owner = (source.ai.owner + 1) % 8;
    candidate.ai.position = source.ai.position;
    if (n % 7 >= 4)
      candidate.ai.position.x = (candidate.ai.position.x + 2) % 16;
  }
  return f;
}
unsigned refusals() {
  for (unsigned n = 0; n < 4; ++n) {
    auto f = generate(2);
    f.state.limits[1] = {0, 99};
    f.state.cursor[0] = 2;
    auto &a = f.world.creatures[2];
    a.ai.active = true;
    a.ai.health = 360;
    a.ai.type = f.world.creatures[0].ai.type;
    a.ai.owner = 1;
    f.world.creatures[0].ai.owner = 0;
    f.world.relation[0][1] = 0;
    if (n == 0)
      f.state.cursor[0] = -1;
    if (n == 1) {
      f.state.cursor[0] = 124;
      f.world.creatures.resize(100);
    }
    if (n == 2)
      f.world.creatures[0].ai.owner = 8;
    if (n == 3)
      f.world.creatures[0].ai.position.x = -1;
    current = &f;
    original = false;
    bool refused = false;
    try {
      scanOwnerConversion(f.world.creatures[0], f.world, f.state, services());
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("conversion native refusal missing");
  }
  return 4;
}
#ifdef MNM_CONVERSION_REFERENCE
std::array<unsigned char, 467> body{};
void pack(void *p, const AutonomousCreature &a, int cursor,
          const CreatureControlLimit &limit) {
  for (auto v : {std::pair<unsigned, unsigned>{0, a.ai.slot},
                 {4, unsigned(a.ai.active)},
                 {8, unsigned(a.ai.position.x)},
                 {12, unsigned(a.ai.position.y)},
                 {16, unsigned(a.ai.position.z)},
                 {0xa8, a.ai.type},
                 {0xe4, a.ai.health},
                 {0x174, unsigned(a.ai.owner)},
                 {0x15c, unsigned(cursor)},
                 {0x8f1, unsigned(limit.maximum)},
                 {0x8f5, unsigned(limit.current)}})
    put(p, v.first, v.second);
}
extern "C" void *__attribute__((thiscall)) subject(void *p, int owner) {
  if (p != reinterpret_cast<void *>(0x643a80))
    throw std::runtime_error("conversion subject receiver");
  event(0, {unsigned(owner)});
  return subjectPresent() ? actors.data() + 0xe4b : nullptr;
}
extern "C" void __attribute__((thiscall)) notifyCall(void *p, unsigned id) {
  if (p != reinterpret_cast<void *>(0x6f3a70))
    throw std::runtime_error("conversion notify receiver");
  notify(id);
}
extern "C" void __attribute__((thiscall)) transferCall(void *p, int owner) {
  transfer(slot(p), owner);
}
void redirect(unsigned address, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  put(p, 1, reinterpret_cast<std::uintptr_t>(target) - address - 5);
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(body.data(), reinterpret_cast<void *>(0x514c60), body.size());
  redirect(0x466870, reinterpret_cast<void *>(subject));
  redirect(0x56c650, reinterpret_cast<void *>(notifyCall));
  redirect(0x51ec80, reinterpret_cast<void *>(transferCall));
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &events) {
  current = &input;
  trace.clear();
  original = true;
  actors.assign(input.world.creatures.size() * 0xe4b, 0x9d);
  for (unsigned i = 0; i < input.world.creatures.size(); ++i)
    pack(actors.data() + i * 0xe4b, input.world.creatures[i],
         input.state.cursor[i], input.state.limits[i]);
  auto before = actors;
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actors.data()));
  global(0x6def5c, input.world.creatures.size());
  global(0x6c5494, 16);
  global(0x6c5498, 16);
  global(0x6c549c, 4);
  std::array<unsigned char, 8 * 340> relations{};
  relations.fill(0xa7);
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      relations[i * 340 + 0x18 + j] = input.world.relation[i][j];
  std::memcpy(reinterpret_cast<void *>(0x643a80), relations.data(),
              relations.size());
  using Scan = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Scan>(0x514c60)(actors.data());
  if (trace != events) {
    unsigned i = 0;
    while (i < trace.size() && i < events.size() && trace[i] == events[i])
      ++i;
    std::cerr << "trace " << i
              << " actual=" << (i < trace.size() ? trace[i] : 0)
              << " expected=" << (i < events.size() ? events[i] : 0) << '\n';
    throw std::runtime_error("conversion calls mismatch case=" +
                             std::to_string(input.number));
  }
  for (unsigned i = 0; i < expected.world.creatures.size(); ++i)
    pack(before.data() + i * 0xe4b, expected.world.creatures[i],
         expected.state.cursor[i], expected.state.limits[i]);
  if (actors != before ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), relations.data(),
                  relations.size()) ||
      std::memcmp(body.data(), reinterpret_cast<void *>(0x514c60), body.size()))
    throw std::runtime_error(
        "conversion actor/relation/body guard mismatch case=" +
        std::to_string(input.number));
  original = false;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_CONVERSION_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  unsigned nativeRefusals = refusals();
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  unsigned transfers = 0, rollovers = 0, retained = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generate(n);
    auto expected = input;
    current = &expected;
    trace.clear();
    original = false;
    scanOwnerConversion(expected.world.creatures[0], expected.world,
                        expected.state, services());
    const auto events = trace;
    for (auto v : events)
      mix(v);
    for (const auto &a : expected.world.creatures)
      mix(unsigned(a.ai.owner));
    mix(unsigned(expected.state.cursor[0]));
    mix(unsigned(expected.state.limits[1].current));
    transfers += unsigned(expected.state.limits[1].current -
                          input.state.limits[1].current);
    if (input.state.cursor[0] > 0 && expected.state.cursor[0] == 0)
      ++rollovers;
    if (expected.state.limits[1].current > input.state.limits[1].current &&
        expected.state.cursor[0] == input.state.cursor[0])
      ++retained;
#ifdef MNM_CONVERSION_REFERENCE
    compare(input, expected, events);
#endif
  }
  if (!transfers || !rollovers || !retained)
    throw std::runtime_error("missing conversion branch");
  std::cout << "{\"success\":true,\"cases\":8192,\"functions\":1,\"native_"
               "refusal_cases\":"
            << nativeRefusals << ",\"outcome_digest\":" << digest
            << ",\"transfers\":" << transfers << ",\"rollovers\":" << rollovers
            << ",\"retained_cursor\":" << retained << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
