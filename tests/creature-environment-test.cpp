#include "../reconstruction/creature-ai/environment_pass.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_ENVIRONMENT_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousWorld world;
  EnvironmentPassState state;
  EnvironmentPassConfiguration configuration;
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
#ifdef MNM_ENVIRONMENT_REFERENCE
Bytes actors, definitions;
std::array<unsigned char, 0x22c> manager{};
unsigned get(const void *p, unsigned o) {
  unsigned n;
  std::memcpy(&n, static_cast<const unsigned char *>(p) + o, 4);
  return n;
}
void put(void *p, unsigned o, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + o, &n, 4);
}
unsigned slot(void *p) {
  auto d = static_cast<unsigned char *>(p) - actors.data();
  if (d < 0 || unsigned(d) >= actors.size() || d % 0xe4b)
    throw std::runtime_error("environment actor ABI");
  return unsigned(d) / 0xe4b;
}
#endif
bool water(unsigned id, Cell p) {
  event(0, {id, unsigned(p.x), unsigned(p.y), unsigned(p.z)});
  return (current->number + id) % 2 != 0;
}
void health(unsigned id, int delta, int owner, bool force) {
  unsigned cursor = current->state.cursor;
#ifdef MNM_ENVIRONMENT_REFERENCE
  if (original)
    cursor = get(manager.data(), 8);
#endif
  event(1, {id, unsigned(delta), unsigned(owner), unsigned(force), cursor});
#ifdef MNM_ENVIRONMENT_REFERENCE
  if (original) {
    auto *a = actors.data() + id * 0xe4b;
    put(a, 0xe4, get(a, 0xe4) + unsigned(delta));
    if (current->number % 17 == 0)
      put(a, 4, 0);
    if (id + 1 < current->world.creatures.size()) {
      auto *next = actors.data() + (id + 1) * 0xe4b;
      auto *definition = definitions.data() + (id + 1) * 0x5c9;
      put(definition, 0x44, (get(definition, 0x44) + 1) % 4);
      if (current->number % 29 == 0)
        put(next, 0xe4, 0);
    }
  } else
#endif
  {
    auto &a = current->world.creatures[id];
    a.ai.health += unsigned(delta);
    if (current->number % 17 == 0)
      a.ai.active = false;
    if (id + 1 < current->world.creatures.size()) {
      auto &next = current->world.creatures[id + 1];
      next.movementKind = (next.movementKind + 1) % 4;
      if (current->number % 29 == 0)
        next.ai.health = 0;
    }
  }
  if (current->number % 19 == 0)
    current->configuration.waterline = 999;
  ++current->configuration.damage;
#ifdef MNM_ENVIRONMENT_REFERENCE
  if (original) {
    global(0x6c5c80, unsigned(current->configuration.waterline));
    global(0x5e1748, current->configuration.damage);
  }
#endif
}
EnvironmentPassServices services() {
  return {[](const auto &a, Cell p) { return water(a.ai.slot, p); },
          [](auto &a, int delta, int owner, bool force) {
            health(a.ai.slot, delta, owner, force);
          }};
}
Fixture generate(unsigned n) {
  Fixture f;
  f.number = n;
  f.world.dimensions = {n % 2 ? 16 : 8, n % 3 ? 16 : 8, 4};
  f.state = {73, EnvironmentFilter::none, 41};
  f.configuration = {n % 11 ? 2 : INT32_MIN,
                     n % 17 == 0 ? 0x80000001u : 1 + n % 31};
  f.world.creatures.resize(n % 129);
  constexpr int z[] = {-1, 0, 1, 2, 3, INT32_MIN, INT32_MAX};
  constexpr unsigned height[] = {0, 1, 2, 4, UINT32_MAX};
  for (unsigned i = 0; i < f.world.creatures.size(); ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.health = (n + i) % 7 ? 400 : 0;
    a.ai.active = (n + i) % 5 != 0;
    a.ai.position = {int((n + i) % unsigned(f.world.dimensions.x)),
                     int((n / 16 + i) % unsigned(f.world.dimensions.y)),
                     z[(n + i) % 7]};
    a.height = height[(n / 7 + i) % 5];
    a.movementKind = (n + i) % 4;
  }
  return f;
}
unsigned refusals() {
  for (unsigned n = 0; n < 3; ++n) {
    auto f = generate(1);
    f.world.creatures[0].ai.active = true;
    f.world.creatures[0].ai.health = 400;
    if (n == 0)
      f.world.creatures[0].movementKind = 4;
    if (n == 1)
      f.world.dimensions.x = 0;
    if (n == 2)
      f.world.dimensions.y = 0;
    current = &f;
    original = false;
    bool refused = false;
    try {
      creatureEnvironmentPass(f.world, f.state, f.configuration, services());
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("environment native refusal missing");
  }
  return 3;
}
#ifdef MNM_ENVIRONMENT_REFERENCE
std::array<unsigned char, 295> body{};
std::array<unsigned char, 14> predicate{};
std::array<unsigned char, 16> dispatch{};
void pack(void *p, const AutonomousCreature &a) {
  for (auto v : {std::pair<unsigned, unsigned>{0, a.ai.slot},
                 {4, unsigned(a.ai.active)},
                 {8, unsigned(a.ai.position.x)},
                 {12, unsigned(a.ai.position.y)},
                 {16, unsigned(a.ai.position.z)},
                 {0xe4, a.ai.health}})
    put(p, v.first, v.second);
}
extern "C" unsigned __attribute__((thiscall)) waterCall(void *p, int x, int y,
                                                        int z) {
  return water(slot(p), {x, y, z});
}
extern "C" unsigned __attribute__((thiscall))
healthCall(void *p, int delta, int owner, unsigned auxiliary, unsigned force) {
  if (auxiliary || force != 1 || owner != -1)
    throw std::runtime_error("environment health ABI");
  health(slot(p), delta, owner, force != 0);
  return 0xabcdef;
}
void redirect(unsigned address, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  put(p, 1, reinterpret_cast<std::uintptr_t>(target) - address - 5);
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(body.data(), reinterpret_cast<void *>(0x52a9f0), body.size());
  std::memcpy(predicate.data(), reinterpret_cast<void *>(0x412aa0),
              predicate.size());
  std::memcpy(dispatch.data(), reinterpret_cast<void *>(0x52ab18),
              dispatch.size());
  redirect(0x414940, reinterpret_cast<void *>(waterCall));
  redirect(0x5076f0, reinterpret_cast<void *>(healthCall));
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &events) {
  current = &input;
  original = true;
  trace.clear();
  unsigned count = input.world.creatures.size();
  actors.assign(std::max(1u, count) * 0xe4b, 0x9d);
  definitions.assign(std::max(1u, count) * 0x5c9, 0xa7);
  for (unsigned i = 0; i < count; ++i) {
    auto *a = actors.data() + i * 0xe4b;
    auto *d = definitions.data() + i * 0x5c9;
    pack(a, input.world.creatures[i]);
    put(a, 0xac, reinterpret_cast<std::uintptr_t>(d));
    put(d, 12, input.world.creatures[i].height);
    put(d, 0x44, input.world.creatures[i].movementKind);
  }
  auto beforeActors = actors;
  auto beforeDefinitions = definitions;
  manager.fill(0xb1);
  put(manager.data(), 0, reinterpret_cast<std::uintptr_t>(actors.data()));
  put(manager.data(), 0x228, count);
  auto beforeManager = manager;
  global(0x6c5494, input.world.dimensions.x);
  global(0x6c5498, input.world.dimensions.y);
  global(0x6c549c, 4);
  global(0x6c5c80, unsigned(input.configuration.waterline));
  global(0x5e1748, input.configuration.damage);
  using Pass = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Pass>(0x52a9f0)(manager.data());
  if (trace != events)
    throw std::runtime_error("environment service/parameters mismatch case=" +
                             std::to_string(input.number));
  for (unsigned i = 0; i < count; ++i) {
    pack(beforeActors.data() + i * 0xe4b, expected.world.creatures[i]);
    put(beforeDefinitions.data() + i * 0x5c9, 0x44,
        expected.world.creatures[i].movementKind);
  }
  put(beforeManager.data(), 8, count);
  put(beforeManager.data(), 12,
      reinterpret_cast<std::uintptr_t>(actors.data() + count * 0xe4b));
  put(beforeManager.data(), 16, 0x412aa0);
  put(beforeManager.data(), 20, 0);
  if (actors != beforeActors || definitions != beforeDefinitions ||
      manager != beforeManager ||
      get(reinterpret_cast<void *>(0x6c5c80), 0) !=
          unsigned(expected.configuration.waterline) ||
      get(reinterpret_cast<void *>(0x5e1748), 0) !=
          expected.configuration.damage ||
      std::memcmp(body.data(), reinterpret_cast<void *>(0x52a9f0),
                  body.size()) ||
      std::memcmp(predicate.data(), reinterpret_cast<void *>(0x412aa0),
                  predicate.size()) ||
      std::memcmp(dispatch.data(), reinterpret_cast<void *>(0x52ab18),
                  dispatch.size()))
    throw std::runtime_error(
        "environment actor/config/iterator/global/body guard mismatch case=" +
        std::to_string(input.number));
  original = false;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_ENVIRONMENT_REFERENCE
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
  unsigned waterCalls = 0, healthCalls = 0, empty = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generate(n);
    auto expected = input;
    current = &expected;
    original = false;
    trace.clear();
    creatureEnvironmentPass(expected.world, expected.state,
                            expected.configuration, services());
    const auto events = trace;
    for (auto v : events)
      mix(v);
    for (const auto &a : expected.world.creatures) {
      mix(a.ai.health);
      mix(unsigned(a.ai.active));
      mix(a.movementKind);
    }
    mix(expected.state.cursor);
    mix(expected.configuration.damage);
    mix(unsigned(expected.configuration.waterline));
    for (std::size_t i = 0; i < events.size();) {
      unsigned kind = events[i], size = events[i + 1];
      waterCalls += kind == 0;
      healthCalls += kind == 1;
      i += size + 2;
    }
    empty += input.world.creatures.empty();
#ifdef MNM_ENVIRONMENT_REFERENCE
    compare(input, expected, events);
#endif
  }
  if (!waterCalls || !healthCalls || !empty)
    throw std::runtime_error("missing environment branch");
  std::cout << "{\"success\":true,\"cases\":8192,\"functions\":1,\"native_"
               "refusal_cases\":"
            << nativeRefusals << ",\"outcome_digest\":" << digest
            << ",\"water_calls\":" << waterCalls
            << ",\"health_calls\":" << healthCalls
            << ",\"empty_worlds\":" << empty << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
