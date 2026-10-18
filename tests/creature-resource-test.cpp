#include "../reconstruction/creature-ai/resource.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_RESOURCE_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousCreature actor;
  CreatureResourceState resource;
  std::array<AssociatedOwnerRecord, 8> owners{};
  unsigned number = 0, dirty = 0, delta = 0, capacity = 0;
  int local = 0, lookup = 0, snapshot = 0;
};
Fixture *current;
std::vector<unsigned> trace;
bool original = false;
#ifdef MNM_RESOURCE_REFERENCE
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
void primary(unsigned quantity) {
  trace.insert(trace.end(), {0, quantity});
  if (current->number % 7 == 0) {
    current->resource.current ^= 0xabcdef00u;
#ifdef MNM_RESOURCE_REFERENCE
    if (original)
      put(actors.data(), 0xe8, get(actors.data(), 0xe8) ^ 0xabcdef00u);
#endif
  }
}
void secondary(unsigned quantity) { trace.insert(trace.end(), {1, quantity}); }
Fixture generateResource(unsigned n) {
  Fixture f;
  f.number = n;
  constexpr unsigned words[] = {0,          1,          255,        256,
                                0x7fffffff, 0x80000000, 0xffffff00, 0xffffffff};
  constexpr unsigned kinds[] = {0, 1, 23, 24, 25, 26, 27, 0xffffffff};
  f.actor.ai.kind = kinds[n % 8];
  f.actor.ai.owner = int(n / 8 % 9) - 1;
  f.local = n % 3 ? f.actor.ai.owner : int(n / 16 % 9) - 1;
  f.actor.block7c = n % 5 == 0;
  f.dirty = words[n / 3 % 8];
  f.resource.current = words[n / 5 % 8];
  f.resource.maximum = words[n / 11 % 8];
  f.delta = words[n / 13 % 8];
  for (unsigned i = 0; i < 8; ++i) {
    f.resource.ownerSnapshot[i] = words[(n / 17 + i) % 8];
    f.owners[i] = {(n + i) % 3, words[(n / 19 + i) % 8]};
  }
  for (unsigned i = 0; i < 42; ++i)
    f.actor.inventory[i] = {(n + i) % 3 ? 19u : 18u, words[(n / 23 + i) % 8], 0,
                            (n + i) % 4};
  f.lookup = int(n / 29 % 9) - 1;
  f.snapshot = int(n / 31 % 8);
  f.capacity = n % 129;
  if (f.lookup >= 0 && n % 3)
    f.owners[unsigned(f.lookup)].actor = n % 2 ? 0 : f.capacity;
  return f;
}
unsigned refusals() {
  unsigned count = 0;
  for (int owner : {-2, 8}) {
    auto f = generateResource(1);
    bool refused = false;
    try {
      associatedCreature(owner, f.owners, f.capacity);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("associated refusal absent");
    ++count;
    refused = false;
    try {
      updateCreatureResourceSnapshot(f.actor, f.resource, owner, f.delta);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("snapshot refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_RESOURCE_REFERENCE
std::vector<std::pair<unsigned, Bytes>> bodies;
extern "C" void __attribute__((thiscall)) primaryStub(void *p,
                                                      unsigned quantity) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6a2dd0)
    throw std::runtime_error("primary UI ABI");
  primary(quantity);
}
extern "C" void __attribute__((thiscall)) secondaryStub(void *p,
                                                        unsigned quantity) {
  if (reinterpret_cast<std::uintptr_t>(p) != 0x6c4e40)
    throw std::runtime_error("secondary UI ABI");
  secondary(quantity);
}
void redirect(unsigned address, void *target) {
  auto *p = reinterpret_cast<unsigned char *>(address);
  p[0] = 0xe9;
  put(p, 1, unsigned(reinterpret_cast<std::uintptr_t>(target) - address - 5));
}
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x507f00, 0x507f9c},
                 {0x507fa0, 0x508036},
                 {0x466870, 0x4668b0}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  redirect(0x4d6f30, reinterpret_cast<void *>(primaryStub));
  redirect(0x4d3a10, reinterpret_cast<void *>(secondaryStub));
}
void packResource(void *p, const Fixture &f) {
  put(p, 0xe8, f.resource.current);
  put(p, 0xec, f.resource.maximum);
  put(p, 0x92e, f.dirty);
  put(p, 0x174, unsigned(f.actor.ai.owner));
  put(p, 0x7c, f.actor.block7c);
  for (unsigned i = 0; i < 8; ++i)
    put(p, 0xe2b + i * 4, f.resource.ownerSnapshot[i]);
  for (unsigned i = 0; i < 42; ++i) {
    const auto &v = f.actor.inventory[i];
    put(p, 0x17c + i * 24, v.kind);
    put(p, 0x17c + i * 24 + 4, v.magnitude);
    put(p, 0x17c + i * 24 + 20, v.available);
  }
}
void compare(Fixture input, const Fixture &expected, unsigned op,
             std::optional<unsigned> result,
             const std::vector<unsigned> &expectedTrace) {
  current = &input;
  original = true;
  trace.clear();
  actors.assign(std::max(1u, input.capacity) * 0xe4b, 0x9d);
  // Resolver must admit canonical pointers to inactive/dead slots.
  for (unsigned i = 0; i < std::max(1u, input.capacity); ++i) {
    put(actors.data() + i * 0xe4b, 4, i % 2);
    put(actors.data() + i * 0xe4b, 0xe4, i % 3 ? 0 : 400);
  }
  definitions.assign(0x5c9, 0xa7);
  owners.assign(8 * 340, 0xb7);
  packResource(actors.data(), input);
  put(definitions.data(), 0, input.actor.ai.kind);
  put(actors.data(), 0xac,
      reinterpret_cast<std::uintptr_t>(definitions.data()));
  for (unsigned i = 0; i < 8; ++i) {
    put(owners.data() + i * 340, 0, input.owners[i].enabled);
    put(owners.data() + i * 340, 12, input.owners[i].actor);
  }
  auto before = actors, definitionBefore = definitions, ownerBefore = owners;
  global(0x644520, unsigned(input.local));
  global(0x6def58, reinterpret_cast<std::uintptr_t>(actors.data()));
  global(0x6def5c, input.capacity);
  if (op == 0) {
    using F = void(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<F>(0x507f00)(actors.data(), input.delta);
  } else if (op == 1) {
    using F = void(__attribute__((thiscall)) *)(void *, int, unsigned);
    reinterpret_cast<F>(0x507fa0)(actors.data(), input.snapshot, input.delta);
  } else {
    using F = void *(__attribute__((thiscall)) *)(void *, int);
    void *got = reinterpret_cast<F>(0x466870)(owners.data(), input.lookup);
    void *wanted = result ? actors.data() + *result * 0xe4b : nullptr;
    if (got != wanted)
      throw std::runtime_error("associated pointer mismatch");
  }
  packResource(before.data(), expected);
  if (actors != before || definitions != definitionBefore ||
      owners != ownerBefore || trace != expectedTrace)
    throw std::runtime_error(
        "resource guard/trace n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("resource body guard");
  if (get(reinterpret_cast<void *>(0x644520), 0) != unsigned(input.local) ||
      get(reinterpret_cast<void *>(0x6def58), 0) !=
          reinterpret_cast<std::uintptr_t>(actors.data()) ||
      get(reinterpret_cast<void *>(0x6def5c), 0) != input.capacity)
    throw std::runtime_error("resource globals guard");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_RESOURCE_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned n) { digest = (digest ^ n) * 1099511628211ull; };
  unsigned displays = 0, subjects = 0, ownSnapshots = 0;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 3; ++op) {
      auto input = generateResource(n), expected = input;
      current = &expected;
      original = false;
      trace.clear();
      std::optional<unsigned> result;
      if (op == 0)
        changeCreatureResource(expected.actor, expected.resource,
                               expected.dirty, expected.delta, expected.local,
                               {primary, secondary});
      else if (op == 1)
        updateCreatureResourceSnapshot(expected.actor, expected.resource,
                                       expected.snapshot, expected.delta);
      else
        result = associatedCreature(expected.lookup, expected.owners,
                                    expected.capacity);
      displays += op == 0 && !trace.empty();
      subjects += bool(result);
      ownSnapshots += op == 1 && expected.snapshot == expected.actor.ai.owner;
      auto events = trace;
      for (auto v : events)
        mix(v);
      mix(expected.resource.current);
      mix(expected.dirty);
      for (auto v : expected.resource.ownerSnapshot)
        mix(v);
      mix(result.value_or(0xffffffff));
#ifdef MNM_RESOURCE_REFERENCE
      compare(input, expected, op, result, events);
#endif
    }
  unsigned refused = refusals();
  if (!displays || !subjects || !ownSnapshots)
    throw std::runtime_error("empty resource branch");
  std::cout << "{\"success\":true,\"cases\":24576,\"functions\":3,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"display_cases\":" << displays << ",\"subjects\":" << subjects
            << ",\"own_snapshots\":" << ownSnapshots << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
