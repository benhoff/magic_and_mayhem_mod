#include "../reconstruction/creature-ai/owner_transfer.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_TRANSFER_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousCreature actor;
  CreatureExecution execution;
  CreatureTransferState transfer;
  CreatureTransferWorld world;
  std::array<unsigned, 4> rootCallbackCounters{};
  unsigned number = 0;
  int requested = 0;
};
Fixture *current;
bool original = false;
std::vector<unsigned> events;
#ifdef MNM_TRANSFER_REFERENCE
Bytes actor, definition, effects, nodes, displays;
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
void syncActor() {
  put(actor.data(), 0x174, unsigned(current->actor.ai.owner));
  put(actor.data(), 0x5ec, current->actor.ai.behavior);
  put(actor.data(), 0x5f0, current->actor.ai.pending);
  put(actor.data(), 0x7cd, current->actor.animationKind);
  put(definition.data(), 0, current->actor.ai.kind);
}
void importActor() {
  auto word = [](unsigned v) {
    return v <= 0x7fffffffu ? int(v) : -1 - int(~v);
  };
  current->actor.ai.owner = word(get(actor.data(), 0x174));
  current->actor.ai.behavior = get(actor.data(), 0x5ec);
  current->actor.ai.pending = get(actor.data(), 0x5f0);
  current->actor.animationKind = get(actor.data(), 0x7cd);
  current->actor.ai.kind = get(definition.data(), 0);
}
#endif
void event(unsigned kind, unsigned argument = 0) {
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    importActor();
#endif
  events.insert(events.end(),
                {kind, argument, unsigned(current->actor.ai.owner),
                 current->actor.ai.behavior, current->actor.ai.pending});
}
void recountCreatures() {
  event(0);
  if (current->number % 13 == 0)
    current->actor.ai.owner = 7;
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    syncActor();
#endif
}
void refreshVisibility() {
  event(1);
  if (current->number % 7 == 0)
    current->actor.ai.behavior = 8;
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    syncActor();
#endif
}
void reset() {
  event(2);
  current->actor.ai.behavior = 1;
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    syncActor();
#endif
}
void releaseEffect(unsigned id) { event(3, id); }
void clearMarkers() { event(4); }
bool wizard() {
  event(5);
  return current->number % 3 != 0;
}
void unselect() { event(6); }
void color(unsigned key) { event(7, key); }
void repairOwnerTargets() {
  event(8);
  constexpr unsigned behaviors[] = {1, 3, 8, 9, 10, 11, 12};
  current->actor.ai.behavior = behaviors[current->number % 7];
  current->actor.ai.pending = current->number % 5;
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    syncActor();
#endif
}
void clearRootActivity(unsigned id) {
  event(9, id);
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    put(nodes.data() + id * 64, 0, get(nodes.data() + id * 64, 0) - 1);
  else
#endif
    --current->rootCallbackCounters[id];
  // First release changes the second root, testing the parent's per-slot
  // reload.
  if (current->number % 11 == 0) {
    current->transfer.roots[1] = 3;
#ifdef MNM_TRANSFER_REFERENCE
    if (original)
      put(actor.data(), 0x67c,
          reinterpret_cast<std::uintptr_t>(nodes.data() + 3 * 64));
#endif
  }
}
void controlWork(bool wizardController) {
  event(10, wizardController);
  if (current->number % 17 == 0) {
    current->actor.ai.owner = 6;
    current->actor.animationKind = 2;
    current->actor.ai.kind = 24;
  }
#ifdef MNM_TRANSFER_REFERENCE
  if (original)
    syncActor();
#endif
}
CreatureTransferServices services() {
  CreatureTransferServices s;
  s.recountCreatures = [](auto &) { recountCreatures(); };
  s.refreshVisibility = [](auto &) { refreshVisibility(); };
  s.reset = [](auto &) { reset(); };
  s.releaseEffect = releaseEffect;
  s.clearMarkers = [](auto &) { clearMarkers(); };
  s.wizard = [](const auto &) { return wizard(); };
  s.unselectWizard = unselect;
  s.color = [](auto &, unsigned key) { color(key); };
  s.repairOwnerTargets = [](auto &) { repairOwnerTargets(); };
  s.clearRootActivity = clearRootActivity;
  s.controlWork = [](auto &, bool wizardController) { controlWork(wizardController); };
  return s;
}
Fixture generateTransfer(unsigned n) {
  Fixture f;
  f.number = n;
  f.actor.ai.slot = 0;
  f.actor.ai.owner = int(n % 9) - 1;
  f.requested = n % 4 ? int(n / 9 % 9) - 1 : f.actor.ai.owner;
  f.actor.ai.behavior = n / 3 % 16 + 1;
  f.actor.ai.pending = n / 5 % 16;
  constexpr unsigned kinds[] = {0, 1, 23, 24, 25, 26, 27};
  f.actor.ai.kind = kinds[n / 7 % 7];
  f.actor.block7c = n % 5 == 0;
  f.actor.animationKind = n / 11 % 4;
  f.execution.marker68 = n % 3;
  f.execution.routeEffect =
      n % 4 ? std::optional<unsigned>(n % 4) : std::nullopt;
  f.transfer.marker70 = n / 13 % 4;
  f.transfer.marker6e4 = n % 17;
  f.transfer.roots = {n % 4 ? std::optional<unsigned>(n % 4) : std::nullopt,
                      n % 5 ? std::optional<unsigned>(n / 4 % 4)
                            : std::nullopt};
  f.transfer.display = 0;
  f.world.displayOwners = {5};
  f.world.selected = n % 7;
  f.world.selectionPresent = n % 3;
  f.rootCallbackCounters = {7, 8, 9, 10};
  return f;
}
#ifdef MNM_TRANSFER_REFERENCE
std::array<unsigned char, 0x16d> body{};
void checkActor(void *p) {
  if (p != actor.data())
    throw std::runtime_error("transfer actor ABI");
}
extern "C" void __attribute__((thiscall)) recountCreaturesStub(void *p) {
  checkActor(p);
  recountCreatures();
}
extern "C" void __attribute__((thiscall)) routeStub(void *p) {
  checkActor(p);
  refreshVisibility();
}
extern "C" void __attribute__((thiscall)) resetStub(void *p) {
  checkActor(p);
  reset();
}
extern "C" void __attribute__((thiscall)) effectStub(void *p) {
  releaseEffect(
      unsigned((static_cast<unsigned char *>(p) - effects.data()) / 4));
}
extern "C" void __attribute__((thiscall)) clearStub(void *p) {
  checkActor(p);
  clearMarkers();
}
extern "C" unsigned __attribute__((thiscall)) wizardStub(void *p) {
  if (p != definition.data())
    throw std::runtime_error("transfer wizard ABI");
  return wizard();
}
extern "C" void __attribute__((thiscall)) unselectStub(void *p) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6c4e40)
    throw std::runtime_error("transfer unselect ABI");
  unselect();
}
extern "C" void __attribute__((thiscall)) colorStub(void *p, unsigned key) {
  if (p != actor.data() + 0xbf3)
    throw std::runtime_error("transfer color ABI");
  color(key);
}
extern "C" void __attribute__((thiscall)) ownerStub(void *p) {
  checkActor(p);
  repairOwnerTargets();
}
extern "C" void __attribute__((thiscall)) rootStub(void *p) {
  clearRootActivity(
      unsigned((static_cast<unsigned char *>(p) - nodes.data()) / 64));
}
extern "C" void __attribute__((thiscall)) memberStub(void *p, void *a) {
  checkActor(a);
  auto v = reinterpret_cast<std::uintptr_t>(p);
  if (v != 0x690600 && v != 0x644550)
    throw std::runtime_error("transfer controlWork ABI");
  controlWork(v == 0x644550);
}
void redirect(unsigned address, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  put(p, 1, unsigned(reinterpret_cast<std::uintptr_t>(target) - address - 5));
}
void install(const char *path) {
  map_image(read(path));
  std::memcpy(body.data(), reinterpret_cast<void *>(0x51ec80), body.size());
  for (auto v : {std::pair<unsigned, void *>{
                     0x522340, reinterpret_cast<void *>(recountCreaturesStub)},
                 {0x51a880, reinterpret_cast<void *>(routeStub)},
                 {0x50ab30, reinterpret_cast<void *>(resetStub)},
                 {0x4943f0, reinterpret_cast<void *>(effectStub)},
                 {0x512340, reinterpret_cast<void *>(clearStub)},
                 {0x411e50, reinterpret_cast<void *>(wizardStub)},
                 {0x4d3ea0, reinterpret_cast<void *>(unselectStub)},
                 {0x54dad0, reinterpret_cast<void *>(colorStub)},
                 {0x51edf0, reinterpret_cast<void *>(ownerStub)},
                 {0x407b90, reinterpret_cast<void *>(rootStub)},
                 {0x416590, reinterpret_cast<void *>(memberStub)}})
    redirect(v.first, v.second);
}
void pack(void *p, const Fixture &f) {
  put(p, 0x174, unsigned(f.actor.ai.owner));
  put(p, 0x5ec, f.actor.ai.behavior);
  put(p, 0x5f0, f.actor.ai.pending);
  put(p, 0x68, f.execution.marker68);
  put(p, 0x70, f.transfer.marker70);
  put(p, 0x6e4, f.transfer.marker6e4);
  put(p, 0x6e8,
      f.execution.routeEffect
          ? reinterpret_cast<std::uintptr_t>(effects.data() +
                                             *f.execution.routeEffect * 4)
          : 0);
  put(p, 0x7c, f.actor.block7c);
  put(p, 0x7cd, f.actor.animationKind);
  put(p, 0x7d1,
      f.transfer.display ? reinterpret_cast<std::uintptr_t>(
                               displays.data() + *f.transfer.display * 0x220)
                         : 0);
  for (unsigned i = 0; i < 2; ++i)
    put(p, 0x678 + i * 4,
        f.transfer.roots[i] ? reinterpret_cast<std::uintptr_t>(
                                  nodes.data() + *f.transfer.roots[i] * 64)
                            : 0);
}
void compare(Fixture input, const Fixture &expected,
             const std::vector<unsigned> &trace) {
  current = &input;
  original = true;
  events.clear();
  actor.assign(0xe4b, 0x9d);
  definition.assign(0x5c9, 0xa7);
  effects.assign(16, 0xb7);
  nodes.assign(4 * 64, 0xc7);
  displays.assign(0x220, 0xd7);
  pack(actor.data(), input);
  put(actor.data(), 0xac, reinterpret_cast<std::uintptr_t>(definition.data()));
  put(definition.data(), 0, input.actor.ai.kind);
  for (unsigned i = 0; i < 4; ++i)
    put(nodes.data() + i * 64, 0, input.rootCallbackCounters[i]);
  put(displays.data(), 0x212, unsigned(input.world.displayOwners[0]));
  auto before = actor, definitionBefore = definition, effectsBefore = effects,
       nodesExpected = nodes, displaysExpected = displays;
  global(0x6e984c, input.world.selected);
  global(0x6903e8, input.world.selectionPresent);
  using F = void(__attribute__((thiscall)) *)(void *, int);
  reinterpret_cast<F>(0x51ec80)(actor.data(), input.requested);
  pack(before.data(), expected);
  put(definitionBefore.data(), 0, expected.actor.ai.kind);
  for (unsigned i = 0; i < 4; ++i)
    put(nodesExpected.data() + i * 64, 0, expected.rootCallbackCounters[i]);
  put(displaysExpected.data(), 0x212,
      unsigned(expected.world.displayOwners[0]));
  if (actor != before || definition != definitionBefore ||
      effects != effectsBefore || nodes != nodesExpected ||
      displays != displaysExpected || events != trace ||
      get(reinterpret_cast<void *>(0x6e984c), 0) != expected.world.selected ||
      get(reinterpret_cast<void *>(0x6903e8), 0) !=
          expected.world.selectionPresent)
    throw std::runtime_error("transfer actor/service guards case=" +
                             std::to_string(input.number));
  if (std::memcmp(reinterpret_cast<void *>(0x51ec80), body.data(), body.size()))
    throw std::runtime_error("transfer body guard");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_TRANSFER_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  unsigned noops = 0, roots = 0, displayWrites = 0;
  for (unsigned n = 0; n < 16384; ++n) {
    auto input = generateTransfer(n), expected = input;
    current = &expected;
    original = false;
    events.clear();
    transferCreatureOwner(expected.actor, expected.execution, expected.transfer,
                          expected.world, expected.requested, services());
    auto trace = events;
    noops += trace.empty();
    roots += input.rootCallbackCounters != expected.rootCallbackCounters;
    displayWrites += input.world.displayOwners != expected.world.displayOwners;
    for (auto v : trace)
      mix(v);
    mix(unsigned(expected.actor.ai.owner));
    mix(expected.world.selected);
    mix(unsigned(expected.world.displayOwners[0]));
#ifdef MNM_TRANSFER_REFERENCE
    compare(input, expected, trace);
#endif
  }
  unsigned refused = 0;
  for (unsigned op = 0; op < 2; ++op) {
    auto f = generateTransfer(1);
    f.actor.animationKind = 2;
    f.transfer.display = op ? std::optional<unsigned>(99) : std::nullopt;
    current = &f;
    original = false;
    bool caught = false;
    try {
      transferCreatureOwner(f.actor, f.execution, f.transfer, f.world,
                            f.requested, services());
    } catch (const std::logic_error &) {
      caught = true;
    }
    if (!caught)
      throw std::runtime_error("transfer display refusal absent");
    ++refused;
  }
  if (!noops || !roots || !displayWrites)
    throw std::runtime_error("empty transfer branch");
  std::cout
      << "{\"success\":true,\"cases\":16384,\"functions\":1,\"outcome_digest\":"
      << digest << ",\"native_refusal_cases\":" << refused
      << ",\"same_owner_noops\":" << noops
      << ",\"root_callback_cases\":" << roots
      << ",\"display_writes\":" << displayWrites << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
