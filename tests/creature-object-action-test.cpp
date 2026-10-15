#include "../reconstruction/creature-ai/object_action.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_OBJECT_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousCreature actor;
  ObjectActionWorld world;
  unsigned number = 0;
};
struct Event {
  unsigned kind, value;
  Cell position;
  bool operator==(const Event &o) const {
    return kind == o.kind && value == o.value && position == o.position;
  }
};
Fixture *current;
std::vector<Event> trace;
void record(unsigned kind, unsigned value = 0, Cell p = {}) {
  trace.push_back({kind, value, p});
}
Cell nextPosition() {
  return {int(current->number / 13 % unsigned(current->world.dimensions.x)),
          int(current->number / 17 % unsigned(current->world.dimensions.y)),
          int(current->number % 4)};
}
unsigned nextWidth() { return 1 + current->number / 49 % 2; }
ObjectActionServices services() {
  ObjectActionServices s;
  s.poll = [](auto &, bool overlay) {
    unsigned e = overlay ? current->number / 7 % 7 : current->number % 7;
    record(overlay ? 0 : 1, e);
    return int(e);
  };
  s.behavior = [](auto &a) {
    record(2);
    a.width = nextWidth();
    a.ai.position = nextPosition();
  };
  s.cue = [](unsigned code, Cell p) { record(3, code, p); };
  s.reset = [](auto &a) { record(4, 0, a.ai.position); };
  return s;
}
Fixture generate(unsigned n) {
  Fixture f;
  f.number = n;
  auto &a = f.actor;
  auto &w = f.world;
  w.dimensions = {n % 2 ? 16 : 8, n % 3 ? 16 : 8, 4};
  a.ai.slot = 0;
  a.ai.type = n / 7 % 28;
  a.ai.health = n % 9 ? 400 : 0;
  a.animationKind = n % 4;
  constexpr unsigned widths[] = {0, 1, 2, 3, UINT32_MAX};
  a.width = widths[n % 5];
  a.ai.position = {int(n / 7 % unsigned(w.dimensions.x)),
                   int(n / 11 % unsigned(w.dimensions.y)), int(n % 4)};
  if (n % 17 == 0)
    a.ai.position.x = -1;
  if (n % 19 == 0)
    a.ai.position.y = w.dimensions.y;
  if (n % 31 == 0)
    a.ai.position.z = -1;
  if (n % 37 == 0)
    a.ai.position.x = INT32_MAX;
  if (n % 41 == 0)
    a.ai.position.x = INT32_MIN;
  const unsigned count = n % 29 ? 1 + n % 16 : 0;
  w.objects.resize(count);
  constexpr unsigned kinds[] = {7, 26, 62, 65, 27, 0, 34, UINT32_MAX};
  for (unsigned i = 0; i < count; ++i)
    w.objects[i] = {kinds[(n / 7 + i) % 8],
                    std::uint16_t(i % 4 == 3 ? 65535 : i + 1)};
  unsigned cells = unsigned(w.dimensions.x * w.dimensions.y * w.dimensions.z);
  w.heads.resize(cells);
  for (unsigned i = 0; i < cells; ++i)
    w.heads[i] =
        i % 7 == 0 ? 65535 : std::uint16_t(count ? (i + n) % (count + 1) : 0);
  return f;
}
unsigned refusals() {
  for (unsigned i = 0; i < 3; ++i) {
    auto f = generate(2);
    if (i == 0)
      f.world.heads.pop_back();
    if (i == 1)
      f.world.dimensions.x = 0;
    if (i == 2) {
      f.world.objects = {{0, 0}};
      std::fill(f.world.heads.begin(), f.world.heads.end(), 0);
    }
    current = &f;
    bool refused = false;
    try {
      tickObjectAction(f.actor, f.world, services());
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("object action native refusal missing");
  }
  return 3;
}
#ifdef MNM_OBJECT_REFERENCE
std::array<unsigned char, 0xe4b> actor{};
Bytes configuration, grid, objects;
std::array<unsigned char, 480> body{};
unsigned get(const void *p, unsigned offset) {
  unsigned n;
  std::memcpy(&n, static_cast<const unsigned char *>(p) + offset, 4);
  return n;
}
void put(void *p, unsigned offset, unsigned n) {
  std::memcpy(static_cast<unsigned char *>(p) + offset, &n, 4);
}
void half(void *p, unsigned offset, unsigned n) {
  std::uint16_t word = n;
  std::memcpy(static_cast<unsigned char *>(p) + offset, &word, 2);
}
void pack(void *p, const AutonomousCreature &a) {
  for (auto v : {std::pair<unsigned, unsigned>{0, a.ai.slot},
                 {4, unsigned(a.ai.active)},
                 {8, unsigned(a.ai.position.x)},
                 {12, unsigned(a.ai.position.y)},
                 {16, unsigned(a.ai.position.z)},
                 {0xa8, a.ai.type},
                 {0xe4, a.ai.health},
                 {0x7cd, a.animationKind}})
    put(p, v.first, v.second);
}
extern "C" unsigned __attribute__((thiscall)) poll(void *p) {
  bool overlay = p == actor.data() + 0x7d5;
  if (!overlay && p != actor.data() + 0xb0)
    throw std::runtime_error("object action poll ABI");
  unsigned e = overlay ? current->number / 7 % 7 : current->number % 7;
  record(overlay ? 0 : 1, e);
  return e;
}
extern "C" void __attribute__((thiscall)) behavior(void *p) {
  if (p != actor.data())
    throw std::runtime_error("object action behavior ABI");
  record(2);
  put(configuration.data(), 8, nextWidth());
  auto position = nextPosition();
  put(p, 8, position.x);
  put(p, 12, position.y);
  put(p, 16, position.z);
}
extern "C" void __attribute__((thiscall)) reset(void *p) {
  if (p != actor.data())
    throw std::runtime_error("object action reset ABI");
  record(4, 0, {int(get(p, 8)), int(get(p, 12)), int(get(p, 16))});
}
extern "C" void __attribute__((thiscall)) cue(void *p, unsigned code, int x,
                                              int y, int z, unsigned a,
                                              unsigned b, unsigned c) {
  if (p != reinterpret_cast<void *>(0x658178) || code != 0x62 || a || b || c)
    throw std::runtime_error("object action cue ABI");
  record(3, code, {x, y, z});
}
void redirect(unsigned address, void *p) {
  auto *target = reinterpret_cast<unsigned char *>(address);
  target[0] = 0xe9;
  put(target, 1, reinterpret_cast<std::uintptr_t>(p) - address - 5);
}
void install(const char *p) {
  map_image(read(p));
  std::memcpy(body.data(), reinterpret_cast<void *>(0x50f3b0), body.size());
  redirect(0x464ec0, reinterpret_cast<void *>(poll));
  redirect(0x50ab30, reinterpret_cast<void *>(reset));
  redirect(0x571310, reinterpret_cast<void *>(cue));
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<Event> &events) {
  current = &input;
  trace.clear();
  actor.fill(0x9d);
  configuration.assign(0x5c9, 0xa7);
  pack(actor.data(), input.actor);
  put(actor.data(), 0xac,
      reinterpret_cast<std::uintptr_t>(configuration.data()));
  put(configuration.data(), 8, input.actor.width);
  put(actor.data(), 0x5f4, reinterpret_cast<std::uintptr_t>(behavior));
  put(actor.data(), 0x5f8, 0);
  auto beforeActor = actor;
  auto beforeConfig = configuration;
  const auto extent = input.world.dimensions;
  global(0x6c5494, extent.x);
  global(0x6c5498, extent.y);
  global(0x6c549c, extent.z);
  grid.assign(input.world.heads.size() * 12, 0xd3);
  for (unsigned i = 0; i < input.world.heads.size(); ++i)
    half(grid.data() + i * 12, 2, input.world.heads[i]);
  for (int z = 0; z < extent.z; ++z)
    global(0x6cb8c2 + z * 4, unsigned(z * extent.x * extent.y));
  for (int y = 0; y < extent.y; ++y)
    global(0x6cb942 + y * 4, unsigned(y * extent.x));
  global(0x6c54dc, reinterpret_cast<std::uintptr_t>(grid.data()));
  objects.assign(std::max(std::size_t(1), input.world.objects.size()) * 558,
                 0xe1);
  for (unsigned i = 0; i < input.world.objects.size(); ++i) {
    put(objects.data() + i * 558, 0x28, input.world.objects[i].kind);
    half(objects.data() + i * 558, 0x1a0, input.world.objects[i].next);
  }
  global(0x689498, reinterpret_cast<std::uintptr_t>(objects.data()));
  global(0x68949c, input.world.objects.size());
  const auto beforeGrid = grid;
  const auto beforeObjects = objects;
  using Tick = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<Tick>(0x50f3b0)(actor.data());
  if (trace != events)
    throw std::runtime_error("object action event mismatch case=" +
                             std::to_string(input.number));
  pack(beforeActor.data(), expected.actor);
  put(beforeConfig.data(), 8, expected.actor.width);
  if (actor != beforeActor || configuration != beforeConfig ||
      grid != beforeGrid || objects != beforeObjects ||
      std::memcmp(body.data(), reinterpret_cast<void *>(0x50f3b0), body.size()))
    throw std::runtime_error("object action byte/body guard mismatch case=" +
                             std::to_string(input.number));
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_OBJECT_REFERENCE
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
  std::array<unsigned, 5> coverage{};
  for (unsigned n = 0; n < 12288; ++n) {
    auto input = generate(n);
    auto expected = input;
    current = &expected;
    trace.clear();
    tickObjectAction(expected.actor, expected.world, services());
    const auto events = trace;
    bool resetCalled = false;
    for (const auto &e : events) {
      mix(e.kind);
      mix(e.value);
      mix(unsigned(e.position.x));
      mix(unsigned(e.position.y));
      mix(unsigned(e.position.z));
      if (e.kind == 2)
        ++coverage[0];
      if (e.kind == 3)
        ++coverage[1];
      if (e.kind == 4)
        resetCalled = true;
    }
    ++coverage[resetCalled ? 3 : 2];
    if (expected.actor.width == 2)
      ++coverage[4];
    mix(expected.actor.width);
    mix(unsigned(expected.actor.ai.position.x));
#ifdef MNM_OBJECT_REFERENCE
    compare(input, expected, events);
#endif
  }
  if (std::any_of(coverage.begin(), coverage.end(),
                  [](unsigned n) { return n == 0; }))
    throw std::runtime_error("missing object action branch");
  std::cout << "{\"success\":true,\"cases\":12288,\"functions\":1,\"native_"
               "refusal_cases\":"
            << nativeRefusals << ",\"outcome_digest\":" << digest
            << ",\"coverage\":[";
  for (unsigned i = 0; i < coverage.size(); ++i)
    std::cout << (i ? "," : "") << coverage[i];
  std::cout << "]}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
