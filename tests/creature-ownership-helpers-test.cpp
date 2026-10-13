#include "../reconstruction/creature-ai/ownership_helpers.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_OWNERSHIP_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousCreature actor;
  AutonomousWorld world;
  CreatureExecution execution;
  CreatureOwnerVisibility visibility;
  OwnerVisionSharing sharing{};
  unsigned kind = 0, activity = 0, number = 0;
};
Fixture *current;
bool original = false;
std::vector<unsigned> events;
#ifdef MNM_OWNERSHIP_REFERENCE
Bytes actor, definition, container, sentinel, nodes, effects, relations;
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
#endif
Fixture generateHelpers(unsigned n) {
  Fixture f;
  f.number = n;
  f.actor.ai.owner = int(n % 8);
  constexpr unsigned words[] = {0,  1,  2,          3,          24,        25,
                                26, 27, 0x7fffffff, 0x80000000, 0xffffffff};
  f.kind = n % 2 ? words[n / 2 % 11] : n * 2654435761u;
  f.activity = n % 2 ? words[n / 3 % 11] : n * 2246822519u;
  for (unsigned i = 0; i < 8; ++i) {
    f.actor.ai.visible[i] = n * 17 + i;
    f.visibility.secondary[i] = n * 23 + i;
    for (unsigned j = 0; j < 8; ++j) {
      f.world.relation[i][j] = (n + i + j) % 5;
      f.sharing[i][j] = (n / 3 + i + j) % 3;
    }
  }
  f.execution.orders.resize(n % 16);
  for (unsigned i = 0; i < f.execution.orders.size(); ++i) {
    auto &order = f.execution.orders[i];
    order.kind = (n + i) % 4;
    order.effect = (n + i) % 3 ? std::optional<unsigned>(i) : std::nullopt;
  }
  return f;
}
void releaseMarker(unsigned token) {
  events.push_back(token);
  if (current->number % 7)
    return;
  unsigned index = 0;
#ifdef MNM_OWNERSHIP_REFERENCE
  if (original) {
    for (; index < current->execution.orders.size(); ++index)
      if (get(nodes.data() + index * 48, 0x28) ==
          reinterpret_cast<std::uintptr_t>(effects.data() + token * 4))
        break;
    if (index + 1 < current->execution.orders.size())
      put(nodes.data() + (index + 1) * 48, 0x28,
          reinterpret_cast<std::uintptr_t>(effects.data() + (16 + index) * 4));
    return;
  }
#endif
  for (; index < current->execution.orders.size(); ++index)
    if (current->execution.orders[index].effect == token)
      break;
  if (index + 1 < current->execution.orders.size())
    current->execution.orders[index + 1].effect = 16 + index;
}
unsigned refusals() {
  unsigned count = 0;
  for (int owner : {-1, 8}) {
    auto f = generateHelpers(1);
    f.actor.ai.owner = owner;
    bool refused = false;
    try {
      refreshCreatureOwnerVisibility(f.actor, f.world, f.sharing, f.visibility);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("owner visibility refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_OWNERSHIP_REFERENCE
std::vector<std::pair<unsigned, Bytes>> bodies;
extern "C" void __attribute__((thiscall)) releaseStub(void *p) {
  auto d = static_cast<unsigned char *>(p) - effects.data();
  if (d < 0 || unsigned(d) >= effects.size() || d % 4)
    throw std::runtime_error("marker release ABI");
  releaseMarker(unsigned(d) / 4);
}
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x411e50, 0x411e6e},
                 {0x407b90, 0x407bb4},
                 {0x51a880, 0x51a8ec},
                 {0x512340, 0x512383}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  auto *p = reinterpret_cast<unsigned char *>(0x4943f0);
  p[0] = 0xe9;
  put(p, 1,
      unsigned(reinterpret_cast<std::uintptr_t>(releaseStub) - 0x4943f0 - 5));
}
void packActor(void *p, const Fixture &f) {
  put(p, 0x174, unsigned(f.actor.ai.owner));
  put(p, 0x89, f.activity);
  for (unsigned i = 0; i < 8; ++i) {
    put(p, 0x24 + i * 4, f.actor.ai.visible[i]);
    put(p, 0x48 + i * 4, f.visibility.secondary[i]);
  }
}
void packNodes(Bytes &b, const Fixture &f) {
  for (unsigned i = 0; i < f.execution.orders.size(); ++i) {
    put(b.data() + i * 48, 8, f.execution.orders[i].kind);
    const auto token = f.execution.orders[i].effect;
    put(b.data() + i * 48, 0x28,
        token ? reinterpret_cast<std::uintptr_t>(effects.data() + *token * 4)
              : 0);
  }
}
void compare(Fixture input, const Fixture &expected, unsigned op, bool wizard,
             const std::vector<unsigned> &trace) {
  current = &input;
  original = true;
  events.clear();
  actor.assign(0xe4b, 0x9d);
  definition.assign(0x5c9, 0xa7);
  container.assign(16, 0xb7);
  sentinel.assign(48, 0xc7);
  nodes.assign(input.execution.orders.size() * 48, 0xd7);
  effects.assign(32 * 4, 0xe7);
  relations.assign(8 * 340, 0xf7);
  packActor(actor.data(), input);
  put(definition.data(), 0, input.kind);
  put(actor.data(), 0x8f9, reinterpret_cast<std::uintptr_t>(container.data()));
  put(container.data(), 4, reinterpret_cast<std::uintptr_t>(sentinel.data()));
  put(container.data(), 8, input.execution.orders.size());
  put(sentinel.data(), 0,
      reinterpret_cast<std::uintptr_t>(nodes.empty() ? sentinel.data()
                                                     : nodes.data()));
  for (unsigned i = 0; i < input.execution.orders.size(); ++i)
    put(nodes.data() + i * 48, 0,
        reinterpret_cast<std::uintptr_t>(i + 1 == input.execution.orders.size()
                                             ? sentinel.data()
                                             : nodes.data() + (i + 1) * 48));
  packNodes(nodes, input);
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 8; ++j) {
      relations[i * 340 + 0x18 + j] = input.world.relation[i][j];
      relations[i * 340 + 0x20 + j] = input.sharing[i][j];
    }
  std::memcpy(reinterpret_cast<void *>(0x643a80), relations.data(),
              relations.size());
  auto before = actor, definitionBefore = definition,
       containerBefore = container, sentinelBefore = sentinel,
       nodesExpected = nodes, effectsBefore = effects;
  if (op == 0) {
    using F = unsigned(__attribute__((thiscall)) *)(void *);
    if (reinterpret_cast<F>(0x411e50)(definition.data()) != unsigned(wizard))
      throw std::runtime_error("wizard predicate return");
  } else {
    using F = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<F>(op == 1   ? 0x407b90
                        : op == 2 ? 0x51a880
                                  : 0x512340)(actor.data());
  }
  packActor(before.data(), expected);
  packNodes(nodesExpected, expected);
  if (actor != before || definition != definitionBefore ||
      container != containerBefore || sentinel != sentinelBefore ||
      nodes != nodesExpected || effects != effectsBefore || events != trace ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), relations.data(),
                  relations.size()))
    throw std::runtime_error(
        "ownership leaf guards n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("ownership leaf text");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_OWNERSHIP_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned clears = 0, markers = 0, wizards = 0;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 4; ++op) {
      auto input = generateHelpers(n), expected = input;
      current = &expected;
      original = false;
      events.clear();
      bool wizard = false;
      if (op == 0)
        wizard = isWizardKind(expected.kind);
      else if (op == 1)
        clearRootActivityFlag(expected.activity);
      else if (op == 2)
        refreshCreatureOwnerVisibility(expected.actor, expected.world,
                                       expected.sharing, expected.visibility);
      else
        clearQueuedMoveMarkers(expected.execution, releaseMarker);
      auto trace = events;
      clears += expected.activity != input.activity;
      markers += trace.size();
      wizards += wizard;
      mix(wizard);
      mix(expected.activity);
      for (auto v : expected.actor.ai.visible)
        mix(v);
      for (auto v : trace)
        mix(v);
#ifdef MNM_OWNERSHIP_REFERENCE
      compare(input, expected, op, wizard, trace);
#endif
    }
  unsigned refused = refusals();
  if (!clears || !markers || !wizards)
    throw std::runtime_error("empty ownership leaf branch");
  std::cout << "{\"success\":true,\"cases\":32768,\"functions\":4,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"activity_clears\":" << clears
            << ",\"marker_releases\":" << markers
            << ",\"wizard_classes\":" << wizards << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
