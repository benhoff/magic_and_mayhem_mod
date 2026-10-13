#include "../reconstruction/creature-ai/source_pending.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_SOURCE_PENDING_REFERENCE
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousWorld world;
  SourceControllerState state;
  SourcePendingQueues queues;
  unsigned number = 0, root = 0;
};
Fixture generatePending(unsigned n) {
  Fixture f;
  f.number = n;
  f.root = n % 3;
  f.world.creatures.resize(2);
  f.state.graph.resize(3);
  f.state.information.resize(3);
  f.state.mode = n % 3 ? 1 : 3;
  f.state.graph[0].children = {1, 2};
  f.state.graph[1].actor = 0;
  f.state.graph[1].parent = 0;
  f.state.graph[2].actor = 1;
  f.state.graph[2].parent = 0;
  for (unsigned i = 0; i < 3; ++i) {
    auto &g = f.state.graph[i];
    g.activity = (n + i) % 4;
    g.roundRobin = (n + i) % 8;
    for (unsigned j = 0; j < 8; ++j) {
      g.free[j] = n % 5 ? (n + i + j) % 3 : 0;
      g.position[j] = 0;
      g.nested[j] = (n + i + j) % 8;
    }
  }
  if (n % 7 == 0)
    f.state.graph[0].children.clear();
  if (n / 4 % 4 == 0)
    f.state.information[f.root].recipient = 0;
  constexpr unsigned kinds[] = {0, 1, 24, 25, 26, 27, 8, 20};
  for (unsigned i = 0; i < 2; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.kind = kinds[(n / 5 + i) % 8];
    a.block7c = (n + i) % 5 == 0;
    a.ai.active = (n + i) % 2;
    a.ai.health = (n + i) % 3 ? 0 : 400;
  }
  for (unsigned i = 0; i < n % 4; ++i) {
    f.queues.general.push_back((n + i) % 3);
    f.queues.wizard.push_back((n / 3 + i) % 3);
    f.queues.other.push_back((n / 5 + i) % 3);
  }
  return f;
}
unsigned refusals() {
  unsigned count = 0;
  for (unsigned op = 0; op < 2; ++op) {
    auto f = generatePending(2);
    bool refused = false;
    if (op == 0)
      f.root = 99;
    else {
      f.root = 1;
      f.state.graph[1].actor = 99;
      f.state.graph[1].activity = 3;
    }
    try {
      registerSourceRoot(f.world, f.state, f.queues, f.root);
    } catch (const std::logic_error &) {
      refused = true;
    }
    if (!refused)
      throw std::runtime_error("pending source refusal absent");
    ++count;
  }
  return count;
}
#ifdef MNM_SOURCE_PENDING_REFERENCE
Bytes actors, definitions, nodes, manager, recipient;
std::vector<Bytes> children, queueBlocks, queueMaps;
std::vector<std::pair<unsigned, Bytes>> bodies;
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
unsigned pointer(unsigned node) {
  return reinterpret_cast<std::uintptr_t>(nodes.data() + node * 0x8d);
}
unsigned childPointer(unsigned node, unsigned pos) {
  return children[node].empty() ? 0
                                : reinterpret_cast<std::uintptr_t>(
                                      children[node].data() + pos * 4);
}
unsigned queueOffset(unsigned which) { return 0x2a + which * 0x30; }
extern "C" void __attribute__((thiscall)) capacityStub(void *p) {
  auto *q = static_cast<unsigned char *>(p);
  auto d = q - manager.data();
  if (d != 0x2a && d != 0x5a && d != 0x8a)
    throw std::runtime_error("pending queue capacity receiver");
  if (get(q, 12) < get(q, 4) + 4)
    throw std::runtime_error("pending queue supplied capacity");
}
void install(const char *path) {
  map_image(read(path));
  for (auto r : {std::pair<unsigned, unsigned>{0x416fe0, 0x4172f7},
                 {0x407920, 0x407982},
                 {0x4079f0, 0x407a73},
                 {0x411e50, 0x411e6e},
                 {0x42d830, 0x42d895},
                 {0x430140, 0x430173}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  auto *p = reinterpret_cast<unsigned char *>(0x42afd0);
  p[0] = 0xe9;
  put(p, 1,
      unsigned(reinterpret_cast<std::uintptr_t>(capacityStub) - 0x42afd0 - 5));
}
void packGraph(Bytes &dst, const Fixture &f) {
  for (unsigned i = 0; i < 3; ++i) {
    auto *p = dst.data() + i * 0x8d;
    const auto &g = f.state.graph[i];
    put(p, 5,
        f.state.information[i].recipient
            ? reinterpret_cast<std::uintptr_t>(recipient.data())
            : 0);
    put(p, 0x29, childPointer(i, 0));
    put(p, 0x2d, childPointer(i, g.children.size()));
    put(p, 0x31, childPointer(i, g.children.size()));
    put(p, 0x35,
        g.actor
            ? reinterpret_cast<std::uintptr_t>(actors.data() + *g.actor * 0xe4b)
            : 0);
    put(p, 0x39, g.parent ? pointer(*g.parent) : 0);
    put(p, 0x89, g.activity);
    put(p, 0x3d, g.roundRobin);
    for (unsigned j = 0; j < 8; ++j) {
      p[0x41 + j] = g.free[j];
      put(p, 0x49 + j * 4, childPointer(i, g.position[j]));
      put(p, 0x69 + j * 4, g.nested[j]);
    }
  }
}
const std::deque<unsigned> &queue(const Fixture &f, unsigned i) {
  return i == 0 ? f.queues.general : i == 1 ? f.queues.wizard : f.queues.other;
}
void queueIterator(void *p, unsigned which, unsigned pos) {
  auto *base = queueBlocks[which].data();
  put(p, 0, reinterpret_cast<std::uintptr_t>(base));
  put(p, 4, reinterpret_cast<std::uintptr_t>(base + 4096));
  put(p, 8, reinterpret_cast<std::uintptr_t>(base + pos));
  put(p, 12, reinterpret_cast<std::uintptr_t>(queueMaps[which].data()));
}
void compare(const Fixture &input, const Fixture &expected) {
  actors.assign(2 * 0xe4b, 0x9d);
  definitions.assign(2 * 0x5c9, 0xa7);
  nodes.assign(3 * 0x8d, 0xb7);
  manager.assign(0x100, 0xc7);
  recipient.assign(64, 0xd7);
  children.resize(3);
  queueBlocks.assign(3, Bytes(4096, 0xe7));
  queueMaps.assign(3, Bytes(4, 0));
  for (unsigned i = 0; i < 3; ++i) {
    children[i].assign(input.state.graph[i].children.size() * 4, 0);
    for (unsigned j = 0; j < input.state.graph[i].children.size(); ++j)
      put(children[i].data(), j * 4, pointer(input.state.graph[i].children[j]));
  }
  packGraph(nodes, input);
  for (unsigned i = 0; i < 2; ++i) {
    auto *p = actors.data() + i * 0xe4b;
    auto *d = definitions.data() + i * 0x5c9;
    put(p, 0xac, reinterpret_cast<std::uintptr_t>(d));
    put(p, 0x7c, input.world.creatures[i].block7c);
    put(p, 4, input.world.creatures[i].ai.active);
    put(p, 0xe4, input.world.creatures[i].ai.health);
    put(d, 0, input.world.creatures[i].ai.kind);
  }
  for (unsigned i = 0; i < 3; ++i) {
    auto *p = manager.data() + queueOffset(i);
    const auto &q = queue(input, i);
    put(queueMaps[i].data(), 0,
        reinterpret_cast<std::uintptr_t>(queueBlocks[i].data()));
    queueIterator(p + 4, i, 128);
    queueIterator(p + 20, i, 128 + q.size() * 4);
    put(p, 0x2c, q.size());
    for (unsigned j = 0; j < q.size(); ++j)
      put(queueBlocks[i].data(), 128 + j * 4, pointer(q[j]));
  }
  auto nodesExpected = nodes, managerExpected = manager;
  auto queuesExpected = queueBlocks;
  const auto actorsBefore = actors, defsBefore = definitions;
  const auto childrenBefore = children, mapsBefore = queueMaps;
  const auto recipientBefore = recipient;
  global(0x5d854c, unsigned(input.state.mode));
  using F = void(__attribute__((thiscall)) *)(void *, void *);
  reinterpret_cast<F>(0x416fe0)(manager.data(),
                                nodes.data() + input.root * 0x8d);
  packGraph(nodesExpected, expected);
  for (unsigned i = 0; i < 3; ++i) {
    auto *p = managerExpected.data() + queueOffset(i);
    const auto &q = queue(expected, i);
    unsigned pos = 128 - unsigned(q.size() - queue(input, i).size()) * 4;
    put(p, 12, reinterpret_cast<std::uintptr_t>(queueBlocks[i].data() + pos));
    put(p, 0x2c, q.size());
    for (unsigned j = 0; j < q.size(); ++j)
      put(queuesExpected[i].data(), pos + j * 4, pointer(q[j]));
  }
  if (nodes != nodesExpected || manager != managerExpected ||
      queueBlocks != queuesExpected || actors != actorsBefore ||
      definitions != defsBefore || children != childrenBefore ||
      queueMaps != mapsBefore || recipient != recipientBefore ||
      get(reinterpret_cast<void *>(0x5d854c), 0) !=
          unsigned(input.state.mode)) {
    unsigned off = 0;
    while (off < manager.size() && manager[off] == managerExpected[off])
      ++off;
    throw std::runtime_error(
        "pending source guards n=" + std::to_string(input.number) +
        " manager-offset=" + std::to_string(off));
  }
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("pending source text guard");
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_SOURCE_PENDING_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned general = 0, wizard = 0, other = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generatePending(n), expected = input;
    registerSourceRoot(expected.world, expected.state, expected.queues,
                       expected.root);
    general += input.queues.general != expected.queues.general;
    wizard += input.queues.wizard != expected.queues.wizard;
    other += input.queues.other != expected.queues.other;
    for (unsigned i = 0; i < 3; ++i) {
      const auto &q = i == 0   ? expected.queues.general
                      : i == 1 ? expected.queues.wizard
                               : expected.queues.other;
      mix(q.size());
      for (auto v : q)
        mix(v);
    }
    for (const auto &v : expected.state.graph) {
      mix(v.roundRobin);
      for (auto f : v.free)
        mix(f);
      for (auto p : v.position)
        mix(p);
      for (auto t : v.nested)
        mix(t);
    }
#ifdef MNM_SOURCE_PENDING_REFERENCE
    compare(input, expected);
#endif
  }
  unsigned refused = refusals();
  if (!general || !wizard || !other)
    throw std::runtime_error("empty source pending queue");
  std::cout << "{\"success\":true,\"cases\":8192,\"functions\":6,\"native_"
               "refusal_cases\":"
            << refused << ",\"outcome_digest\":" << digest
            << ",\"general_admissions\":" << general
            << ",\"wizard_admissions\":" << wizard
            << ",\"other_admissions\":" << other << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
