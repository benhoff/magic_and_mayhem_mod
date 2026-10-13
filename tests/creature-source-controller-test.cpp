#include "../reconstruction/creature-ai/source_controller.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
#pragma push_macro("main")
#undef main
#define main unused_sprite_reference_main
#include "sprite-binary-reference.cpp"
#undef main
#pragma pop_macro("main")
#endif
using namespace mnm::reconstruction::combat;
namespace {
struct Fixture {
  AutonomousWorld world;
  SourceControllerState state;
  SourceManager manager;
  SourceControllerConfiguration configuration;
  unsigned number = 0;
};
Fixture *current;
std::vector<unsigned> events;
Fixture generateController(unsigned n) {
  Fixture f;
  f.number = n;
  f.world.dimensions = {16, 16, 4};
  f.world.creatures.resize(3);
  f.state.roots.resize(3);
  f.state.actorFlag8e5.resize(3);
  f.state.random = seedRandom(n * 0x1020304u);
  f.state.graph.resize(n % 7 ? 2 : 3);
  f.state.information.resize(f.state.graph.size());
  f.manager.kind = n % 2;
  for (unsigned i = 0; i < 3; ++i) {
    auto &a = f.world.creatures[i];
    a.ai.slot = i;
    a.ai.owner = int(n % 8);
    if (i && n % (i + 2) == 0)
      a.ai.owner = (a.ai.owner + 1) % 8;
    a.ai.position = {int((n + i * 7) % 32) - 8, int((n / 3 + i * 9) % 32) - 8,
                     int((n + i) % 8) - 4};
    a.ai.type = n / 5 % 28;
    a.ai.flag704 = n % 31 == 0;
    f.state.actorFlag8e5[i] = n % 3;
  }
  for (unsigned i = 0; i < 8; ++i) {
    f.state.controllers[i] = (n + i) % 5 ? int(i) : -1;
    f.state.ownerFlag4[i] = (n + i) % 13 == 0;
  }
  for (unsigned i = 0; i < 2; ++i) {
    auto &v = f.state.graph[i];
    v.actor = i + 1;
    v.activity = (n + i) % 3 ? 3 : 1;
    v.free.fill(1);
    f.state.information[i].kind = f.manager.kind;
    f.state.information[i].tag9 = 0xffffffff;
    f.state.roots[i + 1] = {i, i};
    f.manager.nodes.push_back(i);
  }
  f.state.recipientRoot = {0};
  if (n % 11 == 0)
    f.state.information[0].recipient = 0;
  if (f.state.graph.size() == 3) {
    f.state.graph[2].children = {0, 1};
    f.state.graph[2].free.fill(1);
    f.state.graph[0].parent = 2;
    f.state.graph[1].parent = 2;
    f.state.information[2].kind = f.manager.kind;
    f.manager.nodes.push_back(2);
  }
  if (n % 17 == 0)
    f.manager.excluded[0] = 0;
  if (n % 19 == 0)
    f.manager.nodes.clear();
  f.state.mode = n % 2 ? 1 : 3;
  f.configuration.coordinateHint25 = 0x7e; // Distinct representation marker.
  f.configuration.players = n % 5 ? 1 : 2;
  f.configuration.networkOwner = int(n / 3 % 8);
  f.configuration.rootThreshold = int(n / 7 % 4) - 1;
  return f;
}
SourceControllerServices services() {
  return {[](unsigned n) {
            events.insert(events.end(), {0, n});
            return true;
          },
          [](unsigned id) { events.insert(events.end(), {1, id}); }};
}
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
Bytes actorStorage, nodes, owners, manager, rootList, recipients;
unsigned char *actorBase;
std::vector<Bytes> childLists;
unsigned allocated = 0, initialNodes = 0;
std::vector<std::pair<unsigned, Bytes>> bodies;
Bytes originalRegistrationBody;
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
void put(void *p, unsigned off, unsigned v) {
  std::memcpy(static_cast<unsigned char *>(p) + off, &v, 4);
}
unsigned nodePointer(std::optional<unsigned> id) {
  return id ? reinterpret_cast<std::uintptr_t>(nodes.data() + *id * 0x8d) : 0;
}
unsigned childPointer(unsigned id, unsigned pos) {
  return reinterpret_cast<std::uintptr_t>(childLists[id].data() + pos * 4);
}
unsigned nodeId(void *p) {
  auto d = static_cast<unsigned char *>(p) - nodes.data();
  if (d < 0 || unsigned(d) >= nodes.size() || d % 0x8d)
    throw std::runtime_error("controller node ABI");
  return unsigned(d) / 0x8d;
}
extern "C" void *__attribute__((cdecl)) allocateStub(unsigned bytes) {
  if (bytes != 0x8d || allocated >= 16)
    throw std::runtime_error("controller allocation ABI");
  events.insert(events.end(), {0, bytes});
  return nodes.data() + allocated++ * 0x8d;
}
extern "C" void __attribute__((thiscall))
appendStub(void *p, void *where, unsigned count, void **item) {
  if (count != 1 || get(p, 8) != reinterpret_cast<std::uintptr_t>(where))
    throw std::runtime_error("controller append ABI");
  if (!get(p, 4) && !get(p, 8) && !get(p, 12)) {
    auto d = static_cast<unsigned char *>(p) - nodes.data() - 0x25;
    if (d < 0 || unsigned(d) >= nodes.size() || d % 0x8d)
      throw std::runtime_error("controller new vector receiver");
    auto &backing = childLists[unsigned(d) / 0x8d];
    put(p, 4, reinterpret_cast<std::uintptr_t>(backing.data()));
    put(p, 8, reinterpret_cast<std::uintptr_t>(backing.data()));
    put(p, 12,
        reinterpret_cast<std::uintptr_t>(backing.data() + backing.size()));
  }
  unsigned end = get(p, 8), cap = get(p, 12);
  if (end + 4 > cap)
    throw std::runtime_error("controller append capacity");
  put(reinterpret_cast<void *>(end), 0,
      reinterpret_cast<std::uintptr_t>(*item));
  put(p, 8, end + 4);
}
extern "C" void __attribute__((thiscall)) registerStub(void *p, void *node) {
  if (p != manager.data())
    throw std::runtime_error("controller register ABI");
  events.insert(events.end(), {1, nodeId(node)});
}
void install(const char *path) {
  map_image(read(path));
  originalRegistrationBody = Bytes(reinterpret_cast<unsigned char *>(0x416fe0),
                                   reinterpret_cast<unsigned char *>(0x4172f7));
  for (auto r : {std::pair<unsigned, unsigned>{0x416590, 0x416b7e},
                 {0x407920, 0x407982},
                 {0x4079f0, 0x407a73},
                 {0x514810, 0x51485f},
                 {0x5183f0, 0x518415},
                 {0x51e790, 0x51e7c7},
                 {0x54e200, 0x54e22b},
                 {0x40e290, 0x40e2b8},
                 {0x40e8a0, 0x40e8c8},
                 {0x40eeb0, 0x40eebb},
                 {0x40e4e0, 0x40e50d},
                 {0x40eb70, 0x40eb9d},
                 {0x40a6b0, 0x40a704}})
    bodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
  for (auto v : {std::pair<unsigned, void *>{
                     0x597850, reinterpret_cast<void *>(allocateStub)},
                 {0x42a340, reinterpret_cast<void *>(appendStub)},
                 {0x416fe0, reinterpret_cast<void *>(registerStub)}}) {
    auto *p = reinterpret_cast<unsigned char *>(v.first);
    p[0] = 0xe9;
    put(p, 1,
        unsigned(reinterpret_cast<std::uintptr_t>(v.second) - v.first - 5));
  }
}
void packNode(Bytes &dst, unsigned i, const Fixture &f, bool newlyAllocated) {
  auto *p = dst.data() + i * 0x8d;
  const auto &v = f.state.graph[i];
  const auto &info = f.state.information[i];
  put(p, 0, info.kind);
  p[4] = info.kind == 0;
  put(p, 5,
      info.recipient ? reinterpret_cast<std::uintptr_t>(recipients.data() +
                                                        *info.recipient * 64)
                     : 0);
  put(p, 9, info.tag9);
  p[0x25] = info.hint25;
  put(p, 0x35,
      v.actor ? reinterpret_cast<std::uintptr_t>(actorBase + *v.actor * 0xe4b)
              : 0);
  put(p, 0x39, nodePointer(v.parent));
  put(p, 0x3d, v.roundRobin);
  put(p, 0x89, v.activity);
  // Supplied vector storage stays at a fixed preallocated address. Empty new
  // nodes start with null pointers; append establishes the reserved backing.
  bool hasStorage = !newlyAllocated || !v.children.empty();
  put(p, 0x29, hasStorage ? childPointer(i, 0) : 0);
  put(p, 0x2d, hasStorage ? childPointer(i, v.children.size()) : 0);
  put(p, 0x31, hasStorage ? childPointer(i, 16) : 0);
  for (unsigned j = 0; j < 8; ++j) {
    p[0x41 + j] = v.free[j];
    if (!newlyAllocated || !v.free[j]) {
      put(p, 0x49 + j * 4, hasStorage ? childPointer(i, v.position[j]) : 0);
      put(p, 0x69 + j * 4, v.nested[j]);
    }
  }
}
// Capture the real target entry stack without changing any compared body.
extern "C" void __attribute__((naked)) invokeController(void *, void *,
                                                        unsigned *) {
  asm volatile("push %ebp; mov %esp,%ebp; push %ebx; push %esi; push %edi;"
               "mov 8(%ebp),%ecx; push 12(%ebp); lea -4(%esp),%eax;"
               "mov 16(%ebp),%edx; mov %eax,(%edx); mov $0x416590,%eax;"
               "call *%eax; pop %edi; pop %esi; pop %ebx; leave; ret");
}
struct ControllerCompareExtension {
  std::function<void()> install;
  std::function<void(Bytes &, const Fixture &, unsigned)> packNewCursors;
  std::function<void(Bytes &)> packManager;
  std::function<void()> verify;
};
void compare(const Fixture &input, const Fixture &expected,
             const std::vector<unsigned> &trace,
             const ControllerCompareExtension *extension = nullptr) {
  current = const_cast<Fixture *>(&input);
  events.clear();
  initialNodes = input.state.graph.size();
  allocated = initialNodes;
  actorStorage.assign(3 * 0xe4b + 256, 0x9d);
  auto address = reinterpret_cast<std::uintptr_t>(actorStorage.data());
  actorBase = actorStorage.data() + ((256 - (address & 255)) & 255);
  nodes.assign(16 * 0x8d, 0);
  owners.assign(8 * 340, 0xa7);
  manager.assign(0x12000, 0xb7);
  rootList.assign(16 * 4, 0);
  recipients.assign(64, 0xc7);
  childLists.assign(16, Bytes(16 * 4, 0));
  for (unsigned i = 0; i < initialNodes; ++i) {
    packNode(nodes, i, input, false);
    for (unsigned j = 0; j < input.state.graph[i].children.size(); ++j)
      put(childLists[i].data(), j * 4,
          nodePointer(input.state.graph[i].children[j]));
  }
  for (unsigned i = 0; i < 3; ++i) {
    const auto &a = input.world.creatures[i];
    auto *p = actorBase + i * 0xe4b;
    put(p, 0, a.ai.slot);
    put(p, 8, unsigned(a.ai.position.x));
    put(p, 12, unsigned(a.ai.position.y));
    put(p, 16, unsigned(a.ai.position.z));
    put(p, 0x174, unsigned(a.ai.owner));
    put(p, 0xa8, a.ai.type);
    put(p, 0x704, a.ai.flag704);
    put(p, 0x8e5, input.state.actorFlag8e5[i]);
    for (unsigned j = 0; j < 2; ++j)
      put(p, 0x678 + j * 4, nodePointer(input.state.roots[i][j]));
  }
  put(manager.data(), 0, input.manager.kind);
  put(manager.data(), 0x1e, reinterpret_cast<std::uintptr_t>(rootList.data()));
  put(manager.data(), 0x22,
      reinterpret_cast<std::uintptr_t>(rootList.data() +
                                       input.manager.nodes.size() * 4));
  put(manager.data(), 0x26,
      reinterpret_cast<std::uintptr_t>(rootList.data() + rootList.size()));
  for (unsigned i = 0; i < input.manager.nodes.size(); ++i)
    put(rootList.data(), i * 4, nodePointer(input.manager.nodes[i]));
  put(manager.data(), 0x11daf, nodePointer(input.manager.excluded[0]));
  put(manager.data(), 0x11fc8, nodePointer(input.manager.excluded[1]));
  put(recipients.data(), 8, nodePointer(input.state.recipientRoot[0]));
  for (unsigned i = 0; i < 8; ++i) {
    put(owners.data() + i * 340, 4, input.state.ownerFlag4[i]);
    put(owners.data() + i * 340, 12, unsigned(input.state.controllers[i]));
  }
  std::memcpy(reinterpret_cast<void *>(0x643a80), owners.data(), owners.size());
  global(0x6c5494, 16);
  global(0x6c5498, 16);
  global(0x6e1f0c, unsigned(input.configuration.players));
  global(0x656634, unsigned(input.configuration.networkOwner));
  global(0x5d865c, unsigned(input.configuration.rootThreshold));
  global(0x5d854c, unsigned(input.state.mode));
  global(0x6a29d8, input.state.random.left);
  global(0x6a29dc, input.state.random.right);
  std::memcpy(reinterpret_cast<void *>(0x6a29e0),
              input.state.random.words.data(), 1000);
  if (extension)
    extension->install();
  auto actorsExpected = actorStorage, nodesExpected = nodes,
       managerExpected = manager, listExpected = rootList,
       recipientExpected = recipients;
  auto childrenExpected = childLists;
  unsigned entryStack = 0;
  invokeController(manager.data(), actorBase, &entryStack);
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 2; ++j)
      put(actorsExpected.data() + (actorBase - actorStorage.data()) + i * 0xe4b,
          0x678 + j * 4, nodePointer(expected.state.roots[i][j]));
  for (unsigned i = 0; i < expected.state.graph.size(); ++i) {
    packNode(nodesExpected, i, expected, i >= initialNodes);
    // The byte25 allocator tag is representation, not a recovered decision.
    // Coordinate temporary construction reuses the incoming argument slot
    // with the final temporary's stack pointer (entryESP-68). Validate that
    // raw machine byte separately from the native deterministic tag.
    if (expected.state.information[i].hint25 == 0x7e)
      nodesExpected[i * 0x8d + 0x25] = (entryStack - 68) & 255;
    for (unsigned j = 0; j < expected.state.graph[i].children.size(); ++j)
      put(childrenExpected[i].data(), j * 4,
          nodePointer(expected.state.graph[i].children[j]));
  }
  if (extension)
    extension->packNewCursors(nodesExpected, expected, initialNodes);
  for (unsigned i = 0; i < expected.manager.nodes.size(); ++i)
    put(listExpected.data(), i * 4, nodePointer(expected.manager.nodes[i]));
  put(managerExpected.data(), 0x22,
      reinterpret_cast<std::uintptr_t>(rootList.data() +
                                       expected.manager.nodes.size() * 4));
  put(recipientExpected.data(), 8,
      nodePointer(expected.state.recipientRoot[0]));
  if (extension)
    extension->packManager(managerExpected);
  if (allocated != expected.state.graph.size() ||
      actorStorage != actorsExpected || nodes != nodesExpected ||
      manager != managerExpected || rootList != listExpected ||
      recipients != recipientExpected || childLists != childrenExpected ||
      events != trace ||
      std::memcmp(reinterpret_cast<void *>(0x643a80), owners.data(),
                  owners.size())) {
    unsigned off = 0;
    while (off < nodes.size() && nodes[off] == nodesExpected[off])
      ++off;
    throw std::runtime_error(
        "controller guards n=" + std::to_string(input.number) + " node=" +
        std::to_string(off / 0x8d) + " offset=" + std::to_string(off % 0x8d) +
        " actual=" + std::to_string(off < nodes.size() ? nodes[off] : 0) +
        " expected=" +
        std::to_string(off < nodesExpected.size() ? nodesExpected[off] : 0) +
        " events=" + std::to_string(events.size()) + "/" +
        std::to_string(trace.size()));
  }
  if (get(reinterpret_cast<void *>(0x5d854c), 0) !=
          unsigned(expected.state.mode) ||
      get(reinterpret_cast<void *>(0x6a29d8), 0) !=
          expected.state.random.left ||
      get(reinterpret_cast<void *>(0x6a29dc), 0) !=
          expected.state.random.right ||
      std::memcmp(reinterpret_cast<void *>(0x6a29e0),
                  expected.state.random.words.data(), 1000))
    throw std::runtime_error("controller RNG/mode");
  for (const auto &b : bodies)
    if (std::memcmp(reinterpret_cast<void *>(b.first), b.second.data(),
                    b.second.size()))
      throw std::runtime_error("controller target text");
  if (extension)
    extension->verify();
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned leaves = 0, groups = 0, blocked = 0, draws = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generateController(n), expected = input;
    current = &expected;
    events.clear();
    updateCreatureSourceController(0, expected.world, expected.state,
                                   expected.manager, expected.configuration,
                                   services());
    auto trace = events;
    blocked += trace.empty();
    draws += expected.state.random.left != input.state.random.left;
    for (unsigned i = input.state.graph.size(); i < expected.state.graph.size();
         ++i) {
      leaves += bool(expected.state.graph[i].actor);
      groups += !expected.state.graph[i].actor;
    }
    for (auto v : trace)
      mix(v);
    mix(expected.state.random.left);
    for (const auto &v : expected.state.graph) {
      mix(v.activity);
      mix(v.parent.value_or(0xffffffff));
      for (auto c : v.children)
        mix(c);
    }
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
    compare(input, expected, trace);
#endif
  }
  unsigned representationCases = 0;
  for (unsigned n = 1; n <= 32; ++n) {
    auto a = generateController(n), b = a;
    b.configuration.initialHint25 = 200;
    b.configuration.coordinateHint25 = 255;
    events.clear();
    updateCreatureSourceController(0, a.world, a.state, a.manager,
                                   a.configuration, services());
    auto traceA = events;
    events.clear();
    updateCreatureSourceController(0, b.world, b.state, b.manager,
                                   b.configuration, services());
    if (events != traceA || a.state.roots != b.state.roots ||
        a.manager.nodes != b.manager.nodes ||
        a.state.recipientRoot != b.state.recipientRoot ||
        !(a.state.random == b.state.random) ||
        a.state.graph.size() != b.state.graph.size())
      throw std::runtime_error("opaque allocator tag changed decisions");
    for (unsigned i = 0; i < a.state.graph.size(); ++i) {
      const auto &x = a.state.graph[i], &y = b.state.graph[i];
      const auto &ix = a.state.information[i], &iy = b.state.information[i];
      if (x.actor != y.actor || x.parent != y.parent ||
          x.children != y.children || x.activity != y.activity ||
          x.roundRobin != y.roundRobin || x.free != y.free ||
          x.position != y.position || x.nested != y.nested ||
          ix.kind != iy.kind || ix.tag9 != iy.tag9 ||
          ix.recipient != iy.recipient)
        throw std::runtime_error("opaque allocator tag changed graph");
    }
    ++representationCases;
  }
  unsigned refused = 0;
  for (unsigned op = 0; op < 5; ++op) {
    auto f = generateController(1);
    auto callbacks = services();
    if (op == 0)
      f.world.creatures[0].ai.owner = -1;
    if (op == 1)
      f.manager.kind = 2;
    if (op == 2)
      f.state.information.clear();
    if (op == 3)
      callbacks.allocate = [](unsigned) { return false; };
    if (op == 4)
      f.state.mode = 4;
    bool caught = false;
    try {
      updateCreatureSourceController(0, f.world, f.state, f.manager,
                                     f.configuration, callbacks);
    } catch (const std::logic_error &) {
      caught = true;
    }
    if (!caught)
      throw std::runtime_error("controller refusal absent");
    ++refused;
  }
  if (!leaves || !groups || !blocked || !draws)
    throw std::runtime_error("empty controller branch");
  std::cout
      << "{\"success\":true,\"cases\":8192,\"functions\":13,\"outcome_digest\":"
      << digest << ",\"representation_policy_cases\":" << representationCases
      << ",\"allocator_tag_equivalence\":false,\"native_refusal_cases\":"
      << refused << ",\"leaves\":" << leaves << ",\"groups\":" << groups
      << ",\"blocked_cases\":" << blocked << ",\"rng_draws\":" << draws
      << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
