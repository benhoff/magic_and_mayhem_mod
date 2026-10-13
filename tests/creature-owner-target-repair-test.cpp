#include "../reconstruction/creature-ai/owner_target_repair.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_TARGET_REPAIR_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousWorld world;
  OwnerTargetRepairState state;
  unsigned number = 0;
};
Fixture *current;
bool original = false;
std::vector<unsigned> events;
#ifdef MNM_TARGET_REPAIR_REFERENCE
Bytes actors, relations;
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
#endif
void event(unsigned kind, unsigned id) {
  const auto &a = current->world.creatures.at(id);
  events.insert(events.end(), {kind, id, a.ai.behavior, a.ai.action,
                               a.ai.engaged, unsigned(a.alternateTarget),
                               unsigned(a.ai.target), a.ai.pending});
}
void prepare() {
  event(0, 0);
  if (current->number % 7 == 0)
    current->world.creatures[0].ai.owner = 7;
}
void peer(unsigned id, bool flag) {
  if (flag)
    throw std::runtime_error("owner target engagement flag");
  event(1, id);
  if (current->world.creatures[id].ai.incoming)
    --current->world.creatures[id].ai.incoming;
  if (current->number % 17 == 0)
    current->world.creatures[0].ai.owner =
        (current->world.creatures[0].ai.owner + 1) % 8;
}
void behavior(unsigned id) {
  event(2, id);
  auto &a = current->world.creatures[id];
  a.ai.behavior = current->number % 2 ? 1 : 3;
  a.ai.target = 0;
  a.ai.pending = 3;
  if (current->number % 11 == 0)
    a.ai.health = 0;
  if (current->number % 13 == 0)
    current->state.count =
        std::min(2u, unsigned(current->world.creatures.size()));
  if (id + 1 < current->world.creatures.size() && current->number % 19 == 0)
    current->world.creatures[id + 1].ai.active = false;
}
void reset(unsigned id) {
  event(3, id);
  auto &a = current->world.creatures[id];
  a.ai.behavior = 1;
  if (current->number % 5 == 0)
    a.ai.target = -1;
  a.ai.pending = current->number % 7;
  if (current->number % 23 == 0)
    current->state.count = current->world.creatures.size();
}
OwnerTargetRepairServices services() {
  return {[](auto &) { prepare(); }, [](auto &a) { behavior(a.ai.slot); },
          [](auto &a) { reset(a.ai.slot); },
          [](auto &a, bool f) { peer(a.ai.slot, f); }};
}
Fixture generateRepair(unsigned n) {
  Fixture f;
  f.number = n;
  unsigned count = 1 + n % 128;
  f.world.creatures.resize(count);
  f.state.markers6a4.resize(count);
  f.state.count = n % 11 ? count : 0;
  constexpr unsigned behaviors[] = {1, 3, 8, 9, 10, 11, 12};
  for (unsigned i = 0; i < count; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.owner = int((n + i) % 8);
    a.ai.active = (n + i) % 5 != 0;
    a.ai.health = (n + i) % 7 ? 400 : 0;
    a.ai.behavior = behaviors[(n / 3 + i) % 7];
    a.ai.action = (n + i) % 3 ? 20 : 10;
    a.alternateTarget = (n + i) % 4 ? 0 : -1;
    a.ai.target = (n + i) % 3 ? 0 : -1;
    a.ai.engaged = (n + i) % 2;
    a.ai.pending = (n + i) % 5;
    a.ai.incoming = (n + i) % 7;
    f.state.markers6a4[i] = (n + i) % 13;
  }
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      f.world.relation[i][j] = (n + i + j) % 3;
  return f;
}
unsigned refusals() {
  unsigned count = 0;
  for (unsigned op = 0; op < 2; ++op) {
    auto f = generateRepair(3);
    f.state.count = 4;
    f.world.creatures[0].ai.owner = 0;
    if (op == 0)
      f.state.count = 5;
    else {
      f.world.creatures[1].ai.active = true;
      f.world.creatures[1].ai.health = 400;
      f.world.creatures[1].ai.owner = 8;
    }
    current = &f;
    original = false;
    bool refused = false;
    try {
      repairCreatureOwnerTargets(f.world.creatures[0], f.world, f.state,
                                 services());
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("owner target refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_TARGET_REPAIR_REFERENCE
std::array<unsigned char, 0x16a> body{};
void pack(void *p, const AutonomousCreature &a, unsigned marker) {
  put(p, 0, a.ai.slot);
  put(p, 4, a.ai.active);
  put(p, 0xe4, a.ai.health);
  put(p, 0x174, unsigned(a.ai.owner));
  put(p, 0x5ec, a.ai.behavior);
  put(p, 0x5fc, a.ai.action);
  put(p, 0x5f0, a.ai.pending);
  put(p, 0x664, a.ai.engaged);
  put(p, 0x668, a.ai.incoming);
  put(p, 0x6a8, unsigned(a.alternateTarget));
  put(p, 0x64c, unsigned(a.ai.target));
  put(p, 0x6a4, marker);
}
int word(unsigned v) { return v <= 0x7fffffffu ? int(v) : -1 - int(~v); }
void importState() {
  current->state.count = get(reinterpret_cast<void *>(0x6df180), 0);
  for (unsigned i = 0; i < current->world.creatures.size(); ++i) {
    auto *p = actors.data() + i * 0xe4b;
    auto &a = current->world.creatures[i];
    a.ai.active = get(p, 4);
    a.ai.health = get(p, 0xe4);
    a.ai.owner = word(get(p, 0x174));
    a.ai.behavior = get(p, 0x5ec);
    a.ai.action = get(p, 0x5fc);
    a.ai.pending = get(p, 0x5f0);
    a.ai.engaged = get(p, 0x664);
    a.ai.incoming = get(p, 0x668);
    a.alternateTarget = word(get(p, 0x6a8));
    a.ai.target = word(get(p, 0x64c));
    current->state.markers6a4[i] = get(p, 0x6a4);
  }
}
void syncState() {
  global(0x6df180, current->state.count);
  for (unsigned i = 0; i < current->world.creatures.size(); ++i)
    pack(actors.data() + i * 0xe4b, current->world.creatures[i],
         current->state.markers6a4[i]);
}
unsigned slot(void *p) {
  auto d = static_cast<unsigned char *>(p) - actors.data();
  if (d < 0 || unsigned(d) >= actors.size() || d % 0xe4b)
    throw std::runtime_error("owner target actor ABI");
  return unsigned(d) / 0xe4b;
}
extern "C" void __attribute__((thiscall)) prepareStub(void *p) {
  if (slot(p) != 0)
    throw std::runtime_error("owner target source ABI");
  importState();
  prepare();
  syncState();
}
extern "C" void __attribute__((thiscall)) peerStub(void *p, unsigned flag) {
  auto id = slot(p);
  importState();
  peer(id, flag);
  syncState();
}
extern "C" void __attribute__((thiscall)) behaviorStub(void *p) {
  auto id = slot(p);
  importState();
  behavior(id);
  syncState();
}
extern "C" void __attribute__((thiscall)) resetStub(void *p) {
  auto id = slot(p);
  importState();
  reset(id);
  syncState();
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(body.data(), reinterpret_cast<void *>(0x51edf0), body.size());
  for (auto v : {std::pair<unsigned, void *>{
                     0x5178c0, reinterpret_cast<void *>(prepareStub)},
                 {0x50eb60, reinterpret_cast<void *>(peerStub)},
                 {0x50ab30, reinterpret_cast<void *>(resetStub)}}) {
    auto *p = reinterpret_cast<unsigned char *>(v.first);
    p[0] = 0xe9;
    put(p, 1,
        unsigned(reinterpret_cast<std::uintptr_t>(v.second) - v.first - 5));
  }
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &trace) {
  current = &input;
  original = true;
  events.clear();
  auto count = input.world.creatures.size();
  actors.assign(count * 0xe4b, 0x9d);
  relations.assign(8 * 340, 0xa7);
  for (unsigned i = 0; i < count; ++i) {
    auto *p = actors.data() + i * 0xe4b;
    pack(p, input.world.creatures[i], input.state.markers6a4[i]);
    put(p, 0x5f4, reinterpret_cast<std::uintptr_t>(behaviorStub));
    put(p, 0x5f8, 0);
  }
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j)
      relations[i * 340 + 0x18 + j] = input.world.relation[i][j];
  std::memcpy(reinterpret_cast<void *>(0x643a80), relations.data(),
              relations.size());
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actors.data()));
  global(0x6def5c, count);
  global(0x6df180, input.state.count);
  auto before = actors;
  using F = void(__attribute__((thiscall)) *)(void *);
  reinterpret_cast<F>(0x51edf0)(actors.data());
  for (unsigned i = 0; i < count; ++i)
    pack(before.data() + i * 0xe4b, expected.world.creatures[i],
         expected.state.markers6a4[i]);
  if (actors != before || events != trace ||
      get(reinterpret_cast<void *>(0x6df180), 0) != expected.state.count ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), relations.data(),
                  relations.size()))
    throw std::runtime_error("owner target guards n=" +
                             std::to_string(input.number));
  if (get(reinterpret_cast<void *>(0x6def58), 0) !=
          reinterpret_cast<std::uintptr_t>(actors.data()) ||
      get(reinterpret_cast<void *>(0x6def5c), 0) != count ||
      std::memcmp(reinterpret_cast<void *>(0x51edf0), body.data(), body.size()))
    throw std::runtime_error("owner target fixed globals/body");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_TARGET_REPAIR_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned callbacks = 0, resets = 0, engagements = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generateRepair(n), expected = input;
    current = &expected;
    original = false;
    events.clear();
    repairCreatureOwnerTargets(expected.world.creatures[0], expected.world,
                               expected.state, services());
    auto trace = events;
    for (unsigned i = 0; i < trace.size(); i += 8) {
      callbacks += trace[i] == 2;
      resets += trace[i] == 3;
      engagements += trace[i] == 1;
    }
    for (auto v : trace)
      mix(v);
    mix(expected.state.count);
    for (const auto &a : expected.world.creatures) {
      mix(a.ai.health);
      mix(a.ai.behavior);
      mix(a.ai.pending);
      mix(a.ai.engaged);
      mix(unsigned(a.alternateTarget));
    }
#ifdef MNM_TARGET_REPAIR_REFERENCE
    compare(input, expected, trace);
#endif
  }
  unsigned refused = refusals();
  if (!callbacks || !resets || !engagements)
    throw std::runtime_error("empty owner target branch");
  std::cout << "{\"success\":true,\"cases\":8192,\"functions\":1,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"behavior_callbacks\":" << callbacks
            << ",\"reset_calls\":" << resets
            << ",\"engagement_calls\":" << engagements << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
