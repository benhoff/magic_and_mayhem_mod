#pragma push_macro("main")
#undef main
#define main unused_snapshot_fixture_main
#include "observation-snapshot-ownership-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/owner_brain_teardown.hpp"

namespace {
constexpr unsigned SecondBrain = Brain + 0x13000, Actors = Heap + 0x40000,
                   Builder = Heap + 0xc0000;
const std::vector<unsigned> Globals = {
    0x5faf20, 0x5faf24, 0x5faf2c, 0x5faf30, 0x5faf34, 0x5faf38, 0x5faf3c,
    0x5faf40, 0x5faf44, 0x5faf48, 0x5faf4c, 0x5faf50, 0x5faf54, 0x5faf58,
    0x6e2040, 0x6e2044, 0x6e2048, 0x6e204c, 0x5d854c};
struct BrainArena : Arena {
  std::map<unsigned, unsigned> globals;
  std::vector<unsigned> releases;
  unsigned get32(unsigned p) {
    if (p < Heap) {
      need(globals.count(p), "declared brain global");
#ifdef MNM_BRAIN_REFERENCE
      unsigned v;
      std::memcpy(&v, reinterpret_cast<void *>(p), 4);
      return v;
#else
      return globals.at(p);
#endif
    }
    return Arena::get32(p);
  }
  void set32(unsigned p, unsigned v) {
    if (p < Heap) {
      need(globals.count(p), "declared brain global write");
      globals[p] = v;
#ifdef MNM_BRAIN_REFERENCE
      std::memcpy(reinterpret_cast<void *>(p), &v, 4);
#endif
    } else
      Arena::set32(p, v);
  }
  void free(unsigned p) {
    if (p)
      releases.push_back(p);
    Arena::free(p);
  }
  void freeBuffer(unsigned p) {
    ++buffers;
    free(p);
  }
  void destroySource(unsigned p) {
    m::OwnerTaskSources<BrainArena>(*this).destroy(p);
  }
  unsigned actorPool() { return Actors; }
};
BrainArena *brainArena;
#ifdef MNM_BRAIN_REFERENCE
void *__attribute__((cdecl)) brainAllocate(unsigned n) {
  return reinterpret_cast<void *>(brainArena->allocate(n));
}
void __attribute__((cdecl)) brainFree(void *p) { brainArena->free(address(p)); }
void __attribute__((cdecl)) brainBufferFree(void *p) {
  brainArena->freeBuffer(address(p));
}
#endif
void vectorAt(BrainArena &a, unsigned h, const std::vector<unsigned> &values) {
  unsigned p = a.allocate(unsigned(values.size() + 2) * 4);
  for (unsigned i = 0; i < values.size(); ++i)
    a.set32(p + 4 * i, values[i]);
  a.set32(h + 4, p);
  a.set32(h + 8, p + unsigned(values.size()) * 4);
  a.set32(h + 12, p + unsigned(values.size() + 2) * 4);
}
unsigned treeAt(BrainArena &a, unsigned h, unsigned global, unsigned counter,
                unsigned size, unsigned count, bool observation = false,
                const std::vector<unsigned> &values = {}) {
  unsigned nil = a.get32(global);
  if (!nil) {
    nil = a.allocate(size);
    a.zero(nil, size);
    a.set32(nil + size - 4, 1);
    a.set32(global, nil);
  }
  a.set32(counter, a.get32(counter) + 1);
  unsigned head = a.allocate(size);
  a.zero(head, size);
  a.set32(head, head);
  a.set32(head + 4, nil);
  a.set32(head + 8, head);
  a.set32(h + 4, head);
  a.set32(h + 12, count);
  std::vector<unsigned> nodes;
  for (unsigned i = 0; i < count; ++i) {
    unsigned p = a.allocate(size);
    a.zero(p, size);
    nodes.push_back(p);
    a.set32(p, nil);
    a.set32(p + 8, nil);
    a.set32(p + 12, i);
    if (observation) {
      for (unsigned j = 0; j < 0xe4b; ++j)
        a.set8(p + 24 + j, (j * 37 + i * 13) & 255);
      for (unsigned off : {0x708u, 0x8f9u, 0x6d0u, 0x6d4u})
        a.set32(p + 24 + off, 0);
      // Borrowed identity retained by snapshot, never released by destruction.
      a.set32(p + 24 + 0xcf3, Actors);
      if (i & 1) {
        unsigned buffer = a.allocate(32);
        a.set32(p + 24 + 0x6d0, buffer);
        a.set32(p + 24 + 0x6d4, 8);
      }
    } else if (size >= 24)
      a.set32(p + 16, values.empty() ? 0xfeed0000 + i : values.at(i));
  }
  if (count) {
    unsigned root = nodes[count / 2];
    a.set32(head, nodes.front());
    a.set32(head + 4, root);
    a.set32(head + 8, nodes.back());
    a.set32(root + 4, head);
    // Match the normal RB invariant as well as the destructor's topology:
    // black root, red children, black shared nil. No rebalancing is supplied.
    a.set32(root + size - 4, 1);
    if (count == 3) {
      a.set32(root, nodes[0]);
      a.set32(root + 8, nodes[2]);
      a.set32(nodes[0] + 4, root);
      a.set32(nodes[2] + 4, root);
    }
  }
  need(count == 0 || count == 1 || count == 3, "authored balanced tree domain");
  return count ? nodes.front() : 0;
}
std::vector<unsigned> forest(BrainArena &a, unsigned borrowed) {
  a.zero(Builder, 512);
  m::OwnerTaskSources<BrainArena> s(a);
  unsigned root = s.anonymous(Builder), leaf = s.actor(Builder, borrowed);
  s.attach(root, leaf);
  std::vector<unsigned> result{root, leaf};
  a.free(a.get32(Builder + 0x3b));
  a.zero(Builder, 512);
  return result;
}
unsigned makeTask(BrainArena &a, unsigned borrowed) {
  m::OwnerTaskStorage<BrainArena> objects(a);
  unsigned p = objects.fallback();
  m::OwnerTaskSources<BrainArena> s(a);
  unsigned root = s.anonymous(p), leaf = s.actor(p, borrowed);
  s.attach(root, leaf);
  vectorAt(a, p + 0xc4, {11, 22});
  vectorAt(a, p + 0xd4, {});
  vectorAt(a, p + 0x121, {33});
  vectorAt(a, p + 0x131, {44, 55});
  return p;
}
void dequeAt(BrainArena &a, unsigned h, unsigned count) {
  if (!count)
    return;
  unsigned n = (count + 1023) / 1024, map = a.allocate((n + 2) * 4);
  std::vector<unsigned> blocks;
  for (unsigned i = 0; i < n; ++i) {
    unsigned p = a.allocate(4096);
    blocks.push_back(p);
    a.set32(map + 4 * i, p);
    for (unsigned j = 0; j < 1024; ++j)
      a.set32(p + 4 * j, 0xdead0000 + j);
  }
  a.set32(h + 4, blocks.front());
  a.set32(h + 8, blocks.front() + 4096);
  a.set32(h + 12, blocks.front());
  a.set32(h + 16, map);
  a.set32(h + 20, blocks.back());
  a.set32(h + 24, blocks.back() + 4096);
  a.set32(h + 28, blocks.back() + ((count - 1) % 1024 + 1) * 4);
  a.set32(h + 32, map + (n - 1) * 4);
  a.set32(h + 36, map);
  a.set32(h + 40, n + 2);
  a.set32(h + 44, count);
}
void setup(BrainArena &a, unsigned brain, unsigned id, unsigned episode) {
  a.zero(brain, 0x12100);
  a.set32(brain, id);
  unsigned snapshot = Actors;
  for (unsigned owner = 0; owner < 8; ++owner) {
    unsigned h = brain + 0x11c02 + 48 * owner;
    unsigned count = (episode + owner) % 3 == 0   ? 0
                     : (episode + owner) % 3 == 1 ? 1
                                                  : 3;
    unsigned node = treeAt(a, h, 0x5faf4c, 0x6e204c, 0xe68, count, true);
    if (node)
      snapshot = node + 24;
    for (unsigned bank = 0; bank < 2; ++bank) {
      auto nodes = forest(a, snapshot);
      nodes.insert(nodes.begin() + 1, 0);
      vectorAt(a, h + 16 + bank * 16, nodes);
    }
  }
  auto primary = forest(a, snapshot), extra = forest(a, snapshot);
  vectorAt(a, brain + 0x1a, {primary[0], 0, primary[1]});
  vectorAt(a, brain + 0x11e05, {primary[1], extra[0], 0, extra[1]});
  vectorAt(a, brain + 4, {makeTask(a, snapshot), 0, makeTask(a, snapshot)});
  unsigned pending = makeTask(a, snapshot), later = makeTask(a, snapshot);
  treeAt(a, brain + 0xba, 0x5faf50, 0x6e2040, 24, 3, false,
         {pending, 0, later});
  treeAt(a, brain + 0x11dc3, 0x5faf50, 0x6e2040, 24, 3, false,
         {pending, later, pending});
  treeAt(a, brain + 0x11df5, 0x5faf54, 0x6e2044, 24, episode % 2 ? 3 : 0);
  treeAt(a, brain + 0x11e15, 0x5faf58, 0x6e2048, 24, episode % 2 ? 1 : 0);
  treeAt(a, brain + 0x12088, 0x5faf20, 0x5faf24, 20, episode % 2 ? 3 : 0);
  vectorAt(a, brain + 0x11de5, {snapshot});
  vectorAt(a, brain + 0x11db3, primary);
  // Two inline sources own their child buffers, borrowing separate live actors.
  for (unsigned off : {0x11f33u, 0x11ea6u}) {
    unsigned p = brain + off;
    a.zero(p, 0x8d);
    a.set32(p + 0x41, 0x01010101);
    a.set32(p + 0x45, 0x01010101);
    a.set32(p + 0x35, Actors);
    a.set32(p + 0x89, 3);
    unsigned backing = a.allocate(16);
    a.set32(p + 0x29, backing);
    a.set32(p + 0x2d, backing);
    a.set32(p + 0x31, backing + 16);
  }
  dequeAt(a, brain + 0x2a, episode % 2 ? 1025 : 0);
  dequeAt(a, brain + 0x5a, episode % 2 ? 1024 : 1);
  dequeAt(a, brain + 0x8a, episode % 2 ? 1 : 1025);
}
} // namespace

int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  need(mmap(reinterpret_cast<void *>(Heap), Size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
            0) != MAP_FAILED,
       "brain heap");
  std::array<unsigned char, 4096> thread{};
  user_desc seg{};
  seg.entry_number = unsigned(-1);
  seg.base_addr = address(thread.data());
  seg.limit = 0xfffff;
  seg.seg_32bit = seg.limit_in_pages = seg.useable = 1;
  need(!syscall(SYS_set_thread_area, &seg), "brain FS");
  unsigned short selector = (seg.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&brainAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&brainFree));
  redirect(0x5979b0, reinterpret_cast<void *>(&brainBufferFree));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "brain RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned totalReleases = 0, totalBuffers = 0;
  for (unsigned episode = 0; episode < 16; ++episode) {
    BrainArena a;
    brainArena = &a;
#ifdef MNM_BRAIN_REFERENCE
    std::memset(reinterpret_cast<void *>(Heap), 0xa5, Size);
    global(0x6def58, Actors);
    global(0x6def5c, 125);
    global(0x643a84, 0);
#endif
    for (unsigned p : Globals)
      a.globals[p] = 0;
    for (unsigned p : Globals)
      a.set32(p, p == 0x5d854c ? 3 : 0);
    for (unsigned i = 0; i < 125 * 0xe4b; ++i)
      a.set8(Actors + i, (i * 53 + episode * 11) & 255);
    for (unsigned i = 0; i < 125; ++i)
      a.set32(Actors + i * 0xe4b + 0x174, 0);
    setup(a, Brain, 0, episode);
    setup(a, SecondBrain, 1, episode);
    // This refusal is a native admission policy, executed equally in both
    // builds.
    a.set32(Brain, 2);
    unsigned before = a.freed;
    bool refused = false;
    try {
      m::OwnerBrainTeardown<BrainArena>(a).destroy(Brain);
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    record(refused, 1, "invalid brain owner refused");
    record(a.freed, before, "refusal has no releases");
    a.set32(Brain, 0);
    a.releases.clear();
    Bytes survivor(0x12100);
    std::memcpy(survivor.data(), a.at(SecondBrain, unsigned(survivor.size())),
                survivor.size());
    for (unsigned brain : {Brain, SecondBrain}) {
#ifdef MNM_BRAIN_REFERENCE
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x4c0920)(
          reinterpret_cast<void *>(brain));
#else
      m::OwnerBrainTeardown<BrainArena>(a).destroy(brain);
#endif
      unsigned alive = brain == Brain;
      for (unsigned p : {0x5faf24u, 0x6e2044u, 0x6e2048u})
        record(a.get32(p), alive, "shared nil references");
      record(a.get32(0x6e2040), 2 * alive, "task map shared references");
      record(a.get32(0x6e204c), 8 * alive, "snapshot shared references");
      for (unsigned h : {4u, 0x1au, 0x11db3u, 0x11de5u, 0x11e05u})
        for (unsigned o : {4u, 8u, 12u})
          record(a.get32(brain + h + o), 0, "final vector cleared");
      for (unsigned owner = 0; owner < 8; ++owner) {
        unsigned h = brain + 0x11c02 + 48 * owner;
        record(a.get32(h + 4), 0, "snapshot head cleared");
        record(a.get32(h + 12), 0, "snapshot count cleared");
        for (unsigned off : {20u, 24u, 28u, 36u, 40u, 44u})
          record(a.get32(h + off), 0, "cache vector cleared");
      }
      // Full final brain image and exact release events participate in the
      // independent original/native digest, including retained stale fields.
      for (unsigned i = 0; i < 0x12100; ++i)
        record(a.get8(brain + i), a.get8(brain + i), "brain image");
      if (brain == Brain) {
        for (unsigned i = 0; i < survivor.size(); ++i)
          record(a.get8(SecondBrain + i), survivor[i], "other brain unchanged");
        for (unsigned i = 0; i < 125; ++i) {
          unsigned p = Actors + i * 0xe4b + 0x67c, expected = 0;
          for (unsigned j = 0; j < 4; ++j)
            expected |= (((i * 0xe4b + 0x67c + j) * 53 + episode * 11) & 255)
                        << (8 * j);
          record(a.get32(p), expected, "other actor assignment unchanged");
        }
      }
      record(a.get32(0x5d854c), 3, "source mode restored");
    }
    for (unsigned i = 0; i < 125; ++i) {
      record(a.get32(Actors + i * 0xe4b + 0x678), 0,
             "brain0 actor assignment cleared");
      record(a.get32(Actors + i * 0xe4b + 0x67c), 0,
             "brain1 actor assignment cleared");
    }
    for (unsigned p : a.releases)
      record(p, p, "exact allocation release sequence");
    for (auto b : a.blocks)
      record(b.second.live, 0, "all owned allocations released");
    a.guards();
    totalReleases += unsigned(a.releases.size());
    totalBuffers += a.buffers;
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(), reinterpret_cast<void *>(0x401000),
                    text.size()),
       "brain original text unchanged");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":16,\"brains\":32,\"owner_rows\":256,\"releases\":"
            << totalReleases << ",\"snapshot_buffers\":" << totalBuffers
            << ",\"digest\":\"" << digest << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
