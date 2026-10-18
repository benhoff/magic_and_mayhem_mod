#include "../reconstruction/creature-ai/source_graph.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_SOURCE_GRAPH_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  SourceGraph graph;
  std::array<std::optional<unsigned>, 2> roots;
  std::array<unsigned, 8> ownerFlags{};
  std::array<int, 8> controllers{};
  unsigned number = 0, family = 0;
  int owner = 0, mode = 0;
};
Fixture generateSource(unsigned n) {
  Fixture f;
  f.number = n;
  f.graph.resize(4);
  f.family = n % 2;
  f.owner = int(n % 8);
  f.mode = int(n % 6) - 1;
  f.graph[0].children =
      n % 11 ? std::vector<unsigned>{1, 2} : std::vector<unsigned>{};
  f.graph[1].children = {3};
  f.graph[1].parent = 0;
  f.graph[2].actor = 1;
  f.graph[2].parent = 0;
  f.graph[3].actor = 0;
  f.graph[3].parent = 1;
  constexpr unsigned words[] = {0, 1, 2, 3, 0x7fffffff, 0x80000000, 0xffffffff};
  for (unsigned i = 0; i < 4; ++i) {
    auto &v = f.graph[i];
    v.activity = words[(n + i) % 7];
    v.roundRobin = (n + i) % 8;
    for (unsigned j = 0; j < 8; ++j) {
      v.free[j] = n % 5 ? (n + i + j) % 3 : 0;
      v.position[j] = (n + i + j) % (v.children.size() + 1);
      v.nested[j] = (n + i + j) % 8;
    }
  }
  f.roots = {n % 7 ? std::optional<unsigned>(3) : std::nullopt,
             n % 9 ? std::optional<unsigned>(2) : std::nullopt};
  for (unsigned i = 0; i < 8; ++i) {
    f.ownerFlags[i] = (n + i) % 4;
    f.controllers[i] = (n + i) % 3 ? int((n + i) % 8) : -1;
  }
  return f;
}
std::vector<unsigned> nativeSource(Fixture &f, unsigned op) {
  if (op == 0) {
    unsigned cursor = beginSourceTraversal(f.graph, 0);
    std::vector<unsigned> values = {cursor};
    for (unsigned step = 0; step < 16; ++step) {
      const int mode = f.number % 17 ? f.mode : step % 2 ? 3 : 1;
      auto actor = nextSourceActor(f.graph, 0, cursor, mode);
      values.push_back(actor.value_or(0xffffffff));
      if (!actor)
        return values;
    }
    throw std::runtime_error("unbounded source fixture");
  }
  if (op == 1) {
    assignCreatureSourceRoot(f.owner, f.family,
                             f.number % 5 ? std::optional<unsigned>(3)
                                          : std::nullopt,
                             f.ownerFlags, f.roots);
    return {};
  }
  if (op == 2)
    return {terminalCreatureSourceRoot(f.graph, f.family, f.roots)
                .value_or(0xffffffff)};
  return {unsigned(creatureControllerAbsent(f.owner, f.controllers))};
}
unsigned refusals() {
  unsigned count = 0;
  for (unsigned op = 0; op < 5; ++op) {
    auto f = generateSource(5);
    bool refused = false;
    try {
      if (op == 0) {
        f.graph[0].roundRobin = 8;
        beginSourceTraversal(f.graph, 0);
      }
      if (op == 1) {
        f.graph[0].children = {0};
        beginSourceTraversal(f.graph, 0);
      }
      if (op == 2) {
        f.graph[3].parent = 3;
        f.roots[0] = 3;
        terminalCreatureSourceRoot(f.graph, 0, f.roots);
      }
      if (op == 3)
        assignCreatureSourceRoot(-1, 0, 0, f.ownerFlags, f.roots);
      if (op == 4)
        terminalCreatureSourceRoot(f.graph, 2, f.roots);
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("source graph refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_SOURCE_GRAPH_REFERENCE
Bytes actors, nodes, owners;
std::vector<Bytes> children;
std::vector<std::pair<unsigned, Bytes>> bodies;
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
unsigned nodePointer(std::optional<unsigned> id) {
  return id ? reinterpret_cast<std::uintptr_t>(nodes.data() + *id * 0x8d) : 0;
}
unsigned actorPointer(std::optional<unsigned> id) {
  return id ? reinterpret_cast<std::uintptr_t>(actors.data() + *id * 0xe4b) : 0;
}
unsigned childPointer(unsigned id, unsigned position) {
  return children[id].empty() ? 0
                              : reinterpret_cast<std::uintptr_t>(
                                    children[id].data() + position * 4);
}
void packNodes(Bytes &dst, const Fixture &f) {
  for (unsigned i = 0; i < f.graph.size(); ++i) {
    auto *p = dst.data() + i * 0x8d;
    const auto &v = f.graph[i];
    put(p, 0x35, actorPointer(v.actor));
    put(p, 0x39, nodePointer(v.parent));
    put(p, 0x89, v.activity);
    put(p, 0x3d, v.roundRobin);
    put(p, 0x29, childPointer(i, 0));
    put(p, 0x2d, childPointer(i, v.children.size()));
    put(p, 0x31, childPointer(i, v.children.size()));
    for (unsigned j = 0; j < 8; ++j) {
      p[0x41 + j] = v.free[j];
      put(p, 0x49 + j * 4, childPointer(i, v.position[j]));
      put(p, 0x69 + j * 4, v.nested[j]);
    }
  }
}
void packRoots(Bytes &dst, const Fixture &f) {
  for (unsigned i = 0; i < 2; ++i)
    put(dst.data(), 0x678 + i * 4, nodePointer(f.roots[i]));
}
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x407920, 0x407982},
                 {0x4079f0, 0x407a73},
                 {0x514810, 0x51485f},
                 {0x5183f0, 0x518415},
                 {0x51e790, 0x51e7c7}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
}
void compare(const Fixture &input, const Fixture &expected, unsigned op,
             const std::vector<unsigned> &trace) {
  actors.assign(2 * 0xe4b, 0x9d);
  nodes.assign(input.graph.size() * 0x8d, 0xa7);
  owners.assign(8 * 340, 0xb7);
  children.resize(input.graph.size());
  for (unsigned i = 0; i < input.graph.size(); ++i) {
    children[i].assign(input.graph[i].children.size() * 4, 0xc7);
    for (unsigned j = 0; j < input.graph[i].children.size(); ++j)
      put(children[i].data(), j * 4, nodePointer(input.graph[i].children[j]));
  }
  packNodes(nodes, input);
  packRoots(actors, input);
  put(actors.data(), 0x174, unsigned(input.owner));
  // Traversal admits dead/inactive actors: its only leaf filter is activity89.
  for (unsigned i = 0; i < 2; ++i) {
    put(actors.data() + i * 0xe4b, 4, i % 2);
    put(actors.data() + i * 0xe4b, 0xe4, i ? 0 : 400);
  }
  for (unsigned i = 0; i < 8; ++i) {
    put(owners.data() + i * 340, 4, input.ownerFlags[i]);
    put(owners.data() + i * 340, 12, unsigned(input.controllers[i]));
  }
  std::memcpy(reinterpret_cast<void *>(0x643a80), owners.data(), owners.size());
  auto actorsExpected = actors, nodesExpected = nodes;
  const auto childrenBefore = children;
  std::vector<unsigned> got;
  global(0x5d854c, unsigned(input.mode));
  if (op == 0) {
    using Begin = unsigned(__attribute__((thiscall)) *)(void *);
    using Next = void *(__attribute__((thiscall)) *)(void *, unsigned);
    unsigned cursor = reinterpret_cast<Begin>(0x407920)(nodes.data());
    got.push_back(cursor);
    for (unsigned step = 0; step < 16; ++step) {
      const int mode = input.number % 17 ? input.mode : step % 2 ? 3 : 1;
      global(0x5d854c, unsigned(mode));
      void *p = reinterpret_cast<Next>(0x4079f0)(nodes.data(), cursor);
      if (!p) {
        got.push_back(0xffffffff);
        break;
      }
      const auto d = static_cast<unsigned char *>(p) - actors.data();
      if (d < 0 || unsigned(d) >= actors.size() || d % 0xe4b)
        throw std::runtime_error("source actor pointer");
      got.push_back(unsigned(d) / 0xe4b);
    }
  } else if (op == 1) {
    using F = void(__attribute__((thiscall)) *)(void *, unsigned, void *);
    reinterpret_cast<F>(0x514810)(actors.data(), input.family,
                                  input.number % 5 ? nodes.data() + 3 * 0x8d
                                                   : nullptr);
  } else if (op == 2) {
    using F = void *(__attribute__((thiscall)) *)(void *, unsigned);
    void *p = reinterpret_cast<F>(0x5183f0)(actors.data(), input.family);
    if (!p)
      got.push_back(0xffffffff);
    else {
      auto d = static_cast<unsigned char *>(p) - nodes.data();
      if (d < 0 || unsigned(d) >= nodes.size() || d % 0x8d)
        throw std::runtime_error("source root pointer");
      got.push_back(unsigned(d) / 0x8d);
    }
  } else {
    using F = unsigned(__attribute__((thiscall)) *)(void *);
    got.push_back(reinterpret_cast<F>(0x51e790)(actors.data()));
  }
  packNodes(nodesExpected, expected);
  packRoots(actorsExpected, expected);
  if (nodes != nodesExpected || actors != actorsExpected ||
      children != childrenBefore || got != trace ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), owners.data(),
                  owners.size()))
    throw std::runtime_error(
        "source graph guards n=" + std::to_string(input.number) +
        " op=" + std::to_string(op));
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("source graph body");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_SOURCE_GRAPH_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned actorsReturned = 0, rotations = 0, absent = 0;
  for (unsigned n = 0; n < 8192; ++n)
    for (unsigned op = 0; op < 4; ++op) {
      auto input = generateSource(n), expected = input;
      auto trace = nativeSource(expected, op);
      if (op == 0) {
        actorsReturned += trace.size() - 2;
        rotations += input.graph[0].roundRobin != expected.graph[0].roundRobin;
      }
      if (op == 3)
        absent += trace[0];
      for (auto v : trace)
        mix(v);
      for (const auto &v : expected.graph) {
        mix(v.roundRobin);
        for (auto flag : v.free)
          mix(flag);
        for (auto x : v.nested)
          mix(x);
      }
#ifdef MNM_SOURCE_GRAPH_REFERENCE
      compare(input, expected, op, trace);
#endif
    }
  unsigned refused = refusals();
  if (!actorsReturned || !rotations || !absent)
    throw std::runtime_error("empty source graph branch");
  std::cout << "{\"success\":true,\"cases\":32768,\"functions\":5,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"actors_returned\":" << actorsReturned
            << ",\"rotations\":" << rotations
            << ",\"controllers_absent\":" << absent
            << ",\"helper_redirects\":0}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
