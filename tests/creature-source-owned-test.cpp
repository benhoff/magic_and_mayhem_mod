#pragma push_macro("main")
#undef main
#define main unused_source_controller_fixture_main
#include "creature-source-controller-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/source_owned.hpp"
namespace {
struct OwnedSourceFixture {
  Fixture base;
  SourcePendingQueues queues;
};
OwnedSourceFixture generateOwnedSource(unsigned n) {
  OwnedSourceFixture f;
  f.base = generateController(n);
  constexpr unsigned kinds[] = {0, 1, 24, 25, 26, 27, 8, 20};
  for (unsigned i = 0; i < 3; ++i) {
    f.base.world.creatures[i].ai.kind = kinds[(n / 3 + i) % 8];
    f.base.world.creatures[i].block7c = (n + i) % 7 == 0;
  }
  for (unsigned i = 0; i < n % 3; ++i) {
    f.queues.general.push_back(i % 2);
    f.queues.wizard.push_back(i % 2);
    f.queues.other.push_back(i % 2);
  }
  return f;
}
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
Bytes ownedDefinitions;
std::vector<Bytes> ownedQueueBlocks, ownedQueueMaps;
std::vector<std::pair<unsigned, Bytes>> extraBodies;
const std::deque<unsigned> &ownedQueue(const OwnedSourceFixture &f,
                                       unsigned i) {
  return i == 0 ? f.queues.general : i == 1 ? f.queues.wizard : f.queues.other;
}
unsigned ownedQueueOffset(unsigned i) { return 0x2a + i * 0x30; }
void ownedQueueIterator(void *p, unsigned i, unsigned pos) {
  auto *base = ownedQueueBlocks[i].data();
  put(p, 0, reinterpret_cast<std::uintptr_t>(base));
  put(p, 4, reinterpret_cast<std::uintptr_t>(base + 4096));
  put(p, 8, reinterpret_cast<std::uintptr_t>(base + pos));
  put(p, 12, reinterpret_cast<std::uintptr_t>(ownedQueueMaps[i].data()));
}
extern "C" void __attribute__((thiscall)) ownedCapacityStub(void *p) {
  auto d = static_cast<unsigned char *>(p) - manager.data();
  if (d != 0x2a && d != 0x5a && d != 0x8a)
    throw std::runtime_error("owned source capacity receiver");
  if (get(p, 12) < get(p, 4) + 4)
    throw std::runtime_error("owned source supplied capacity");
}
void compareOwnedSource(const OwnedSourceFixture &input,
                        const OwnedSourceFixture &expected,
                        const std::vector<unsigned> &trace) {
  Bytes definitionsBefore;
  std::vector<Bytes> queuesExpected, mapsBefore;
  ControllerCompareExtension extension;
  extension.install = [&] {
    std::memcpy(reinterpret_cast<void *>(0x416fe0),
                originalRegistrationBody.data(),
                originalRegistrationBody.size());
    auto *p = reinterpret_cast<unsigned char *>(0x42afd0);
    p[0] = 0xe9;
    put(p, 1,
        unsigned(reinterpret_cast<std::uintptr_t>(ownedCapacityStub) -
                 0x42afd0 - 5));
    ownedDefinitions.assign(3 * 0x5c9, 0xa7);
    for (unsigned i = 0; i < 3; ++i) {
      auto *d = ownedDefinitions.data() + i * 0x5c9;
      put(d, 0, input.base.world.creatures[i].ai.kind);
      put(actorBase + i * 0xe4b, 0xac, reinterpret_cast<std::uintptr_t>(d));
      put(actorBase + i * 0xe4b, 0x7c, input.base.world.creatures[i].block7c);
    }
    definitionsBefore = ownedDefinitions;
    ownedQueueBlocks.assign(3, Bytes(4096, 0xe7));
    ownedQueueMaps.assign(3, Bytes(4, 0));
    for (unsigned i = 0; i < 3; ++i) {
      auto *q = manager.data() + ownedQueueOffset(i);
      const auto &list = ownedQueue(input, i);
      put(ownedQueueMaps[i].data(), 0,
          reinterpret_cast<std::uintptr_t>(ownedQueueBlocks[i].data()));
      ownedQueueIterator(q + 4, i, 128);
      ownedQueueIterator(q + 20, i, 128 + list.size() * 4);
      put(q, 0x2c, list.size());
      for (unsigned j = 0; j < list.size(); ++j)
        put(ownedQueueBlocks[i].data(), 128 + j * 4, nodePointer(list[j]));
    }
    queuesExpected = ownedQueueBlocks;
    mapsBefore = ownedQueueMaps;
  };
  extension.packNewCursors = [&](Bytes &dst, const Fixture &f,
                                 unsigned before) {
    for (unsigned i = before; i < f.state.graph.size(); ++i) {
      const auto &g = f.state.graph[i];
      auto *p = dst.data() + i * 0x8d;
      for (unsigned j = 0; j < 8; ++j)
        if (!g.free[j] || g.position[j] || g.nested[j]) {
          put(p, 0x49 + j * 4,
              g.children.empty() ? 0 : childPointer(i, g.position[j]));
          put(p, 0x69 + j * 4, g.nested[j]);
        }
    }
  };
  extension.packManager = [&](Bytes &dst) {
    for (unsigned i = 0; i < 3; ++i) {
      const auto &q = ownedQueue(expected, i);
      unsigned pos = 128 - unsigned(q.size() - ownedQueue(input, i).size()) * 4;
      auto *p = dst.data() + ownedQueueOffset(i);
      put(p, 12,
          reinterpret_cast<std::uintptr_t>(ownedQueueBlocks[i].data() + pos));
      put(p, 0x2c, q.size());
      for (unsigned j = 0; j < q.size(); ++j)
        put(queuesExpected[i].data(), pos + j * 4, nodePointer(q[j]));
    }
  };
  extension.verify = [&] {
    if (ownedDefinitions != definitionsBefore ||
        ownedQueueBlocks != queuesExpected || ownedQueueMaps != mapsBefore)
      throw std::runtime_error("owned source profile/queue guards");
    for (const auto &body : extraBodies)
      if (std::memcmp(reinterpret_cast<void *>(body.first), body.second.data(),
                      body.second.size()))
        throw std::runtime_error("owned source extra body changed");
  };
  compare(input.base, expected.base, trace, &extension);
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
  if (argc != 2)
    throw std::runtime_error("pinned PE expected");
  install(argv[1]);
  extraBodies.push_back({0x416fe0, originalRegistrationBody});
  for (auto r : {std::pair<unsigned, unsigned>{0x411e50, 0x411e6e},
                 {0x42d830, 0x42d895},
                 {0x430140, 0x430173}})
    extraBodies.push_back(
        {r.first, Bytes(reinterpret_cast<unsigned char *>(r.first),
                        reinterpret_cast<unsigned char *>(r.second))});
#else
  (void)argc;
  (void)argv;
#endif
  std::uint64_t digest = 1469598103934665603ull;
  auto mix = [&](unsigned v) { digest = (digest ^ v) * 1099511628211ull; };
  unsigned general = 0, wizard = 0, other = 0, groups = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    auto input = generateOwnedSource(n), expected = input;
    events.clear();
    updateCreatureSourceOwned(0, expected.base.world, expected.base.state,
                              expected.base.manager, expected.queues,
                              expected.base.configuration, [](unsigned bytes) {
                                events.insert(events.end(), {0, bytes});
                                return true;
                              });
    auto trace = events;
    general += input.queues.general != expected.queues.general;
    wizard += input.queues.wizard != expected.queues.wizard;
    other += input.queues.other != expected.queues.other;
    for (unsigned i = input.base.state.graph.size();
         i < expected.base.state.graph.size(); ++i)
      groups += !expected.base.state.graph[i].actor;
    for (auto v : trace)
      mix(v);
    for (unsigned i = 0; i < 3; ++i) {
      const auto &q = i == 0   ? expected.queues.general
                      : i == 1 ? expected.queues.wizard
                               : expected.queues.other;
      mix(q.size());
      for (auto v : q)
        mix(v);
    }
    for (const auto &g : expected.base.state.graph) {
      mix(g.activity);
      mix(g.parent.value_or(0xffffffff));
      for (auto p : g.position)
        mix(p);
      for (auto t : g.nested)
        mix(t);
    }
#ifdef MNM_SOURCE_CONTROLLER_REFERENCE
    compareOwnedSource(input, expected, trace);
#endif
  }
  if (!general || !wizard || !other || !groups)
    throw std::runtime_error("empty composed source branch");
  std::cout
      << "{\"success\":true,\"cases\":8192,\"functions\":17,\"outcome_digest\":"
      << digest << ",\"general_admissions\":" << general
      << ",\"wizard_admissions\":" << wizard
      << ",\"other_admissions\":" << other << ",\"groups\":" << groups
      << ",\"registration_redirected\":false,\"allocator_tag_equivalence\":"
         "false}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
