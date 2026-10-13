#include "../reconstruction/creature-ai/owner_counts.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_COUNTS_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousWorld world;
  std::array<AssociatedOwnerRecord, 8> owners;
  std::vector<CreatureControlLimit> limits;
  unsigned number = 0;
  int local = 0;
};
Fixture *current;
bool original = false;
std::vector<unsigned> events;
#ifdef MNM_COUNTS_REFERENCE
Bytes actors, definitions, owners;
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
#endif
[[maybe_unused]] unsigned subject() {
  return current->owners.at(unsigned(current->world.creatures[0].ai.owner))
      .actor;
}
void display(unsigned slot, unsigned quantity) {
  events.insert(events.end(), {0, slot, quantity});
  if (current->number % 13 == 0) {
#ifdef MNM_COUNTS_REFERENCE
    if (original) {
      put(actors.data() + slot * 0xe4b, 0x8f1, 7);
      put(actors.data() + slot * 0xe4b, 0x8f5, unsigned(-1));
      return;
    }
#endif
    current->limits.at(slot) = {-1, 7};
  } else if (current->number % 17 == 0) {
#ifdef MNM_COUNTS_REFERENCE
    if (original) {
      put(actors.data() + slot * 0xe4b, 0x8f1, 7);
      put(actors.data() + slot * 0xe4b, 0x8f5, 7);
      return;
    }
#endif
    current->limits.at(slot) = {7, 7};
  }
}
OwnerCountServices services() {
  return {[](auto &a, unsigned count) { display(a.ai.slot, count); },
          [](auto &a) { events.insert(events.end(), {1, a.ai.slot, 0}); },
          [](auto &a) { events.insert(events.end(), {2, a.ai.slot, 0}); }};
}
Fixture generateCounts(unsigned n) {
  Fixture f;
  f.number = n;
  auto count = 1 + n % 128;
  f.world.creatures.resize(count);
  f.limits.resize(count);
  constexpr unsigned kinds[] = {0, 1, 24, 25, 26, 27, 8, 20};
  for (unsigned i = 0; i < count; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.owner = int((n + i) % 9) - 1;
    a.ai.active = (n + i) % 5 != 0;
    a.ai.health = (n + i) % 7 ? 400 : 0;
    a.ai.type = (n / 3 + i) % 28;
    a.ai.kind = kinds[(n / 5 + i) % 8];
    a.block7c = (n + i) % 11 == 0;
    f.limits[i] = {int(n % 11) - 2, int(n / 7 % 17) - 2};
    if (n % 13 == 0)
      f.limits[i] = {7, 7};
    else if (n % 17 == 0)
      f.limits[i] = {-1, 7};
  }
  for (unsigned i = 0; i < 8; ++i)
    f.owners[i] = {(n + i) % 3,
                   (n + i) % 5 ? unsigned(i % count) : 0xffffffffu};
  auto owner = f.world.creatures[0].ai.owner;
  const auto i = owner >= 0 ? f.owners[unsigned(owner)].actor : 0;
  f.local =
      n % 3 && i < count ? f.world.creatures[i].ai.owner : int(n / 3 % 9) - 1;
  return f;
}
unsigned refusals() {
  unsigned count = 0;
  for (unsigned op = 0; op < 2; ++op) {
    auto f = generateCounts(1);
    f.world.creatures[0].ai.owner = 0;
    f.owners[0] = {1, 0};
    if (op == 0)
      f.world.creatures[0].ai.owner = 8;
    else
      f.limits.clear();
    current = &f;
    original = false;
    bool refused = false;
    try {
      recountOwnerCreatures(f.world.creatures[0], f.world, f.owners, f.limits,
                            f.local, services());
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("count ownership refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_COUNTS_REFERENCE
std::vector<std::pair<unsigned, Bytes>> bodies;
extern "C" void __attribute__((thiscall)) displayStub(void *p,
                                                      unsigned quantity) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6903f0)
    throw std::runtime_error("count UI receiver");
  display(subject(), quantity);
}
extern "C" void __attribute__((thiscall)) belowStub(void *p) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6a2dd0)
    throw std::runtime_error("count below receiver");
  events.insert(events.end(), {1, subject(), 0});
}
extern "C" void __attribute__((thiscall)) atStub(void *p) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6a2dd0)
    throw std::runtime_error("count at receiver");
  events.insert(events.end(), {2, subject(), 0});
}
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x522340, 0x52243e},
                 {0x466870, 0x4668b0}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  for (auto v : {std::pair<unsigned, void *>{
                     0x4dea00, reinterpret_cast<void *>(displayStub)},
                 {0x4d7a00, reinterpret_cast<void *>(belowStub)},
                 {0x4d78b0, reinterpret_cast<void *>(atStub)}}) {
    auto *p = reinterpret_cast<unsigned char *>(v.first);
    p[0] = 0xe9;
    put(p, 1,
        unsigned(reinterpret_cast<std::uintptr_t>(v.second) - v.first - 5));
  }
}
void pack(void *p, const AutonomousCreature &a,
          const CreatureControlLimit &limit) {
  put(p, 0, a.ai.slot);
  put(p, 4, a.ai.active);
  put(p, 0xe4, a.ai.health);
  put(p, 0xa8, a.ai.type);
  put(p, 0x174, unsigned(a.ai.owner));
  put(p, 0x7c, a.block7c);
  put(p, 0x8f1, unsigned(limit.maximum));
  put(p, 0x8f5, unsigned(limit.current));
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &trace) {
  current = &input;
  original = true;
  events.clear();
  const auto count = input.world.creatures.size();
  actors.assign(count * 0xe4b, 0x9d);
  definitions.assign(count * 0x5c9, 0xa7);
  owners.assign(8 * 340, 0xb7);
  for (unsigned i = 0; i < count; ++i) {
    auto *p = actors.data() + i * 0xe4b;
    auto *d = definitions.data() + i * 0x5c9;
    pack(p, input.world.creatures[i], input.limits[i]);
    put(d, 0, input.world.creatures[i].ai.kind);
    put(p, 0xac, reinterpret_cast<std::uintptr_t>(d));
  }
  for (unsigned i = 0; i < 8; ++i) {
    put(owners.data() + i * 340, 0, input.owners[i].enabled);
    put(owners.data() + i * 340, 12, input.owners[i].actor);
  }
  std::memcpy(reinterpret_cast<void *>(0x643a80), owners.data(), owners.size());
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actors.data()));
  global(0x6def5c, count);
  global(0x6df180, count);
  global(0x644520, unsigned(input.local));
  auto before = actors, definitionsBefore = definitions;
  using F = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<F>(0x522340)(actors.data());
  for (unsigned i = 0; i < count; ++i)
    pack(before.data() + i * 0xe4b, expected.world.creatures[i],
         expected.limits[i]);
  if (actors != before || definitions != definitionsBefore || events != trace ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), owners.data(),
                  owners.size()))
    throw std::runtime_error("count guard/trace n=" +
                             std::to_string(input.number));
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("count body guard");
  if (get(reinterpret_cast<void *>(0x6def58), 0) !=
          reinterpret_cast<std::uintptr_t>(actors.data()) ||
      get(reinterpret_cast<void *>(0x6def5c), 0) != count ||
      get(reinterpret_cast<void *>(0x6df180), 0) != count ||
      get(reinterpret_cast<void *>(0x644520), 0) != unsigned(input.local))
    throw std::runtime_error("count globals guard");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_COUNTS_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned displays = 0, below = 0, at = 0;
  for (unsigned n = 0; n < 16384; ++n) {
    auto input = generateCounts(n), expected = input;
    current = &expected;
    original = false;
    events.clear();
    recountOwnerCreatures(expected.world.creatures[0], expected.world,
                          expected.owners, expected.limits, expected.local,
                          services());
    auto trace = events;
    for (unsigned i = 0; i < trace.size(); i += 3) {
      displays += trace[i] == 0;
      below += trace[i] == 1;
      at += trace[i] == 2;
    }
    for (auto v : trace)
      mix(v);
    for (const auto &v : expected.limits) {
      mix(unsigned(v.current));
      mix(unsigned(v.maximum));
    }
#ifdef MNM_COUNTS_REFERENCE
    compare(input, expected, trace);
#endif
  }
  unsigned refused = refusals();
  if (!displays || !below || !at)
    throw std::runtime_error("empty count UI branch");
  std::cout << "{\"success\":true,\"cases\":16384,\"functions\":2,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"display_calls\":" << displays << ",\"below_calls\":" << below
            << ",\"at_calls\":" << at << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
