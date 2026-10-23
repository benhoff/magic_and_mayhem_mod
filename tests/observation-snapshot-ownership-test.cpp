#pragma push_macro("main")
#undef main
#define main unused_observation_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/owner_knowledge_cache.hpp"
#include "../reconstruction/creature-ai/owner_observation_snapshots.hpp"
#include <set>
namespace {
constexpr unsigned Heap = 0x12000000, Size = 0x1000000, Brain = Heap + 0x10000,
                   Pool = Heap + 0x30000, Config = Heap + 0x40000,
                   Heads = Heap + 0x50000, Nil = Heap + 0x60000,
                   Task = Heap + 0x70000, Scratch = Heap + 0x80000;
struct Arena {
  struct Block {
    unsigned n;
    bool live;
  };
  Bytes data = Bytes(Size, 0xa5);
  std::map<unsigned, Block> blocks;
  unsigned mode = 3, cursor = 0x100000, freed = 0, objects = 0, buffers = 0;
  unsigned char *at(unsigned p, unsigned n = 4) {
    need(p >= Heap && p - Heap + n <= Size, "snapshot arena extent");
    if (p >= Heap + 0x100000) {
      auto it = blocks.upper_bound(p);
      need(it != blocks.begin(), "snapshot allocation");
      --it;
      need(it->second.live && p - it->first + n <= it->second.n,
           "snapshot allocation lifetime");
    }
#ifdef MNM_BRAIN_REFERENCE
    return reinterpret_cast<unsigned char *>(p);
#else
    return data.data() + p - Heap;
#endif
  }
  unsigned get32(unsigned p) {
    if (p == 0x5d854c)
      return mode;
    unsigned v;
    std::memcpy(&v, at(p), 4);
    return v;
  }
  unsigned get8(unsigned p) { return *at(p, 1); }
  void set32(unsigned p, unsigned v) {
    if (p == 0x5d854c) {
      need(v == 3 || v == 0, "source mode");
      mode = v;
      return;
    }
    std::memcpy(at(p), &v, 4);
  }
  void set8(unsigned p, unsigned v) {
    *at(p, 1) = static_cast<unsigned char>(v);
  }
  void zero(unsigned p, unsigned n) {
    for (unsigned i = 0; i < n; ++i)
      set8(p + i, 0);
  }
  unsigned allocate(unsigned n) {
    need(n < 65536 && cursor + n + 64 < Size, "bounded snapshot allocation");
    unsigned p = Heap + cursor;
    cursor += ((n + 15) & ~15u) + 64;
    blocks.emplace(p, Block{n, true});
    return p;
  }
  unsigned allocateCacheSource() { return allocate(0x8d); }
  void free(unsigned p) {
    if (!p)
      return;
    auto &b = blocks.at(p);
    need(b.live, "snapshot double free");
    std::memset(at(p, b.n), 0xdd, b.n);
    b.live = false;
    ++freed;
  }
  void freeBuffer(unsigned p) {
    ++buffers;
    free(p);
  }
  void releaseObject(unsigned p, unsigned flag) {
    need(flag == 1, "snapshot deleting flag");
    ++objects;
    free(p);
  }
  void destroySource(unsigned p) {
    m::OwnerTaskSources<Arena>(*this).destroy(p);
  }
  bool live(unsigned p) {
    auto it = blocks.upper_bound(p);
    if (it == blocks.begin())
      return false;
    --it;
    return it->second.live && p - it->first < it->second.n;
  }
  void guards() {
    for (auto row : blocks) {
      unsigned p = row.first, n = row.second.n;
#ifdef MNM_BRAIN_REFERENCE
      auto *b = reinterpret_cast<unsigned char *>(p);
#else
      auto *b = data.data() + p - Heap;
#endif
      for (unsigned i = 0; i < 16; ++i)
        need(b[n + i] == 0xa5, "snapshot suffix guard");
    }
  }
};
Arena *current;
#ifdef MNM_BRAIN_REFERENCE
void *__attribute__((cdecl)) ownedAllocate(unsigned n) {
  return reinterpret_cast<void *>(current->allocate(n));
}
void __attribute__((cdecl)) ownedFree(void *p) { current->free(address(p)); }
void __attribute__((cdecl)) bufferFree(void *p) {
  current->freeBuffer(address(p));
}
void __attribute__((thiscall)) deleteObject(void *p, unsigned flag) {
  current->releaseObject(address(p), flag);
}
#endif
void snapshotCall(Arena &a, unsigned target, unsigned source, bool construct) {
#ifdef MNM_BRAIN_REFERENCE
  using Copy = void(__attribute__((thiscall)) *)(void *, void *);
  reinterpret_cast<Copy>(construct ? 0x418b60 : 0x418d50)(
      reinterpret_cast<void *>(target), reinterpret_cast<void *>(source));
#else
  m::OwnerObservationSnapshots<Arena> snapshots(a);
  if (construct)
    snapshots.construct(target, source);
  else
    snapshots.assign(target, source);
#endif
  (void)a;
}
void destruction(Arena &a, unsigned p) {
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x505f70)(
      reinterpret_cast<void *>(p));
#else
  m::OwnerObservationSnapshots<Arena>(a).destroy(p);
#endif
  (void)a;
}
void releaseTask(Arena &a) {
  (void)a;
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x40a340)(
      reinterpret_cast<void *>(Task));
#else
  for (unsigned p = a.get32(Task + 0x3b); p != a.get32(Task + 0x3f); p += 4) {
    unsigned node = a.get32(p);
    a.destroySource(node);
    a.free(node);
  }
  a.set32(Task + 0x3f, a.get32(Task + 0x3b));
#endif
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  need(mmap(reinterpret_cast<void *>(Heap), Size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
            0) != MAP_FAILED,
       "observation heap mapping");
  std::array<unsigned char, 4096> thread{};
  user_desc seg{};
  seg.entry_number = unsigned(-1);
  seg.base_addr = address(thread.data());
  seg.limit = 0xfffff;
  seg.seg_32bit = seg.limit_in_pages = seg.useable = 1;
  need(!syscall(SYS_set_thread_area, &seg), "FS");
  unsigned short selector = (seg.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&ownedAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&ownedFree));
  redirect(0x5979b0, reinterpret_cast<void *>(&bufferFree));
  // A synthetic owned object's virtual deleting destructor is an explicit
  // service.
  put(reinterpret_cast<void *>(0x6e1f00), 4,
      address(reinterpret_cast<void *>(&deleteObject)));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "observation RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned inserted = 0, refreshed = 0, expired = 0, clones = 0, stale = 0,
           resources = 0, slices = 0;
  for (unsigned episode = 0; episode < 64; ++episode) {
    Arena a;
    current = &a;
#ifdef MNM_BRAIN_REFERENCE
    std::memset(reinterpret_cast<void *>(Heap), 0xa5, Size);
#endif
    // Full packed copy with all bytes independently authored, including
    // pointers.
    for (unsigned i = 0; i < 0xe4b; ++i)
      a.set8(Scratch + i, (i * 73 + episode * 19) & 255);
    for (bool construct : {false, true}) {
      a.zero(Scratch + 0x2000, 0xe4b);
      snapshotCall(a, Scratch + 0x2000, Scratch, construct);
      for (unsigned i = 0; i < 0xe4b; ++i) {
        bool zero = (i >= 0x6d0 && i < 0x6d8) ||
                    (construct &&
                     ((i >= 0x708 && i < 0x70c) || (i >= 0x8f9 && i < 0x8fd)));
        record(a.get8(Scratch + 0x2000 + i), zero ? 0 : a.get8(Scratch + i),
               "full packed snapshot copy");
      }
    }
    // Populated normal destructor: object, intrusive list nodes/head, buffer.
    a.zero(Scratch, 0xe4b);
    unsigned object = a.allocate(8), list = a.allocate(12),
             head = a.allocate(12), buffer = a.allocate(24);
    a.set32(object, 0x6e1f00);
    a.set32(list + 4, head);
    a.set32(list + 8, episode % 5);
    a.set32(head, head);
    a.set32(head + 4, head);
    unsigned previous = head;
    for (unsigned i = 0; i < episode % 5; ++i) {
      unsigned node = a.allocate(12);
      a.set32(previous, node);
      a.set32(node + 4, previous);
      a.set32(node, head);
      a.set32(head + 4, node);
      a.set32(node + 8, 0x99100000 + i);
      previous = node;
    }
    a.set32(Scratch + 0x708, object);
    a.set32(Scratch + 0x8f9, list);
    a.set32(Scratch + 0x6d0, buffer);
    a.set32(Scratch + 0x6d4, 24);
    destruction(a, Scratch);
    for (unsigned off : {0x708u, 0x8f9u, 0x6d0u, 0x6d4u}) {
      record(a.get32(Scratch + off), 0, "resource cleared");
    }
    record(a.objects, 1, "virtual owned release");
    record(a.buffers, 1, "buffer owned release");
    resources += 2 + episode % 5;
    destruction(a, Scratch);
    record(a.objects, 1, "second destruction idempotent");
    // Native admission policy is exercised in every architecture; this does
    // not assert safe original behavior for resource-bearing mode2 refresh.
    {
      m::OwnerKnowledge guarded;
      m::OwnerMemory row;
      row.memory.key = 0;
      row.memory.observedTick = 1;
      row.memory.actor.slot = 0;
      guarded.records[0].push_back(row);
      m::OwnerObservationSnapshots<Arena> owned(a);
      a.zero(Pool, 0xe4b);
      a.set32(Pool + 0x708, 0x99112233);
      owned.reconcile(guarded, 1, [](int) { return Pool; });
      guarded.records[0][0].memory.observedTick = 2;
      bool rejected = false;
      try {
        owned.reconcile(guarded, 2, [](int) { return Pool; });
      } catch (const std::invalid_argument &) {
        rejected = true;
      }
      record(rejected, 1, "native resource refresh admission");
      record(a.get32(owned.snapshot(0, 0) + 0x708), 0,
             "refusal preserves constructed snapshot");
      owned.clear();
    }
    // One enabled owner; nonempty task/notification lists remain outside mode2.
    a.zero(Brain, 0x12100);
    a.zero(Pool, 0xe4b);
    a.zero(Config, 0x5c9);
    a.zero(Task, 512);
    a.zero(Nil, 0xe68);
    a.set32(Nil + 0xe64, 1);
    a.set8(Brain + 0x11fed, 1);
    a.set32(Brain + 0x11fee, ~0u);
    for (unsigned owner = 0; owner < 8; ++owner) {
      unsigned h = Heads + owner * 0xe68;
      a.zero(h, 0xe68);
      a.set32(h, h);
      a.set32(h + 4, Nil);
      a.set32(h + 8, h);
      a.set32(Brain + 0x11c06 + 48 * owner, h);
    }
#ifdef MNM_BRAIN_REFERENCE
    global(0x5faf4c, Nil);
    global(0x6def58, Pool);
    global(0x6def5c, 1);
    global(0x6df180, 1);
    global(0x6c5494, 32);
    global(0x6c5498, 32);
    global(0x6c549c, 8);
    global(0x5d854c, 3);
    global(0x6a5f80 + 17 * 0x5c9 + 0x589, 24);
    for (unsigned owner = 0; owner < 8; ++owner) {
      global(0x643a80 + 340 * owner, owner == 0);
      global(0x643a80 + 340 * owner + 4, 0);
      for (unsigned j = 0; j < 8; ++j)
        *reinterpret_cast<unsigned char *>(0x643a80 + 340 * owner + 24 + j) =
            owner == j;
    }
#endif
    m::OwnerKnowledge knowledge;
    std::array<bool, 8> enabled{};
    enabled[0] = true;
    std::array<std::array<std::uint8_t, 8>, 8> relation{};
    for (unsigned i = 0; i < 8; ++i)
      relation[i][i] = 1;
    m::OwnerObservationSnapshots<Arena> store(a);
    m::OwnerKnowledgeCache<Arena> cache(a);
    // Original cache uses its real brain frame; allocation above balances the
    // native cache adapter header, which is a host representation only.
    unsigned clone = 0, remembered = 0, oldKey = 0;
    std::map<int, Bytes> expected;
    for (unsigned cycle = 0; cycle < 3; ++cycle) {
      for (unsigned step = 0; step < 9; ++step) {
        int tick = int(cycle * 1200 + (step < 4    ? step
                                       : step == 4 ? 1024
                                       : step == 5 ? 1027
                                       : step == 6 ? 1028
                                       : step == 7 ? 1030
                                                   : 1031));
        ++slices;
        bool visible = step < 4 || step >= 7;
        m::OwnerObservation observation;
        auto &actor = observation.actor;
        actor.slot = 0;
        actor.active = true;
        actor.owner = 1;
        actor.type = 17;
        actor.health = 100 + episode;
        actor.position = {2, 3, 2};
        actor.region = -1;
        actor.aging = 24;
        observation.speed = 24;
        observation.finePosition = {actor.position.x * 32, 96, 256};
        observation.visible.fill(visible);
        m::OwnerPlannerActor planner;
        planner.motionB63 = 24;
        observation.planning = planner;
        for (unsigned i = 0; i < 0xe4b; ++i)
          a.set8(Pool + i,
                 (i * 37 + episode * 11 + step * 7 + cycle * 13) & 255);
        // Explicit raw actor fields consumed by4181d0, cache production and
        // cleanup.
        for (auto row : {std::pair<unsigned, unsigned>{0, 0},
                         {4, 1},
                         {8, unsigned(actor.position.x)},
                         {12, 3},
                         {16, 2},
                         {20, unsigned(observation.finePosition.x)},
                         {24, 96},
                         {28, 256},
                         {0xa8, 17},
                         {0xac, Config},
                         {0xe4, unsigned(actor.health)},
                         {0x174, 1},
                         {0xb63, 24},
                         {0xde7, ~0u},
                         {0x708, 0},
                         {0x8f9, 0}})
          a.set32(Pool + row.first, row.second);
        for (unsigned i = 0; i < 8; ++i) {
          a.set32(Pool + 0x24 + 4 * i, visible);
          a.set32(Pool + 0x48 + 4 * i, 0);
        }
        a.set32(Config, 1);
        a.set32(Config + 0x589, 24);
        m::markOwnerKnowledge(knowledge, tick);
        cache.invalidate();
#ifdef MNM_BRAIN_REFERENCE
        a.set32(Brain + 0x14, unsigned(tick));
        reinterpret_cast<void(__attribute__((thiscall)) *)(void *, int)>(
            0x4181d0)(reinterpret_cast<void *>(Brain), 1);
#endif
        m::updateOwnerKnowledge(knowledge, tick, {32, 32, 8}, enabled, relation,
                                {observation});
#ifndef MNM_BRAIN_REFERENCE
        unsigned beforeI = store.insertions, beforeR = store.refreshes,
                 beforeE = store.expiries;
        store.reconcile(knowledge, tick, [](int) { return Pool; });
        inserted += store.insertions - beforeI;
        refreshed += store.refreshes - beforeR;
        expired += store.expiries - beforeE;
#else
        unsigned oldCount = a.get32(Brain + 0x11c0e),
                 oldNext = a.get32(Brain + 0x11d82);
        reinterpret_cast<void(__attribute__((thiscall)) *)(void *, int)>(
            0x4181d0)(reinterpret_cast<void *>(Brain), 2);
        unsigned newCount = a.get32(Brain + 0x11c0e),
                 newNext = a.get32(Brain + 0x11d82);
        inserted += newNext - oldNext;
        expired += oldCount + (newNext - oldNext) - newCount;
#endif
        record(knowledge.records[0].size(),
#ifdef MNM_BRAIN_REFERENCE
               a.get32(Brain + 0x11c0e),
#else
               store.records[0].size(),
#endif
               "owned observation count");
        for (const auto &row : knowledge.records[0]) {
          unsigned snapshot;
#ifdef MNM_BRAIN_REFERENCE
          unsigned node = a.get32(Heads);
          need(node != Heads, "original observation exists");
          record(a.get32(node + 12), unsigned(row.memory.key),
                 "owned observation key");
          record(a.get32(node + 16), unsigned(row.memory.observedTick),
                 "owned observation seen");
          record(a.get32(node + 20), unsigned(row.markedTick),
                 "owned observation marker");
          snapshot = node + 24;
          if (row.memory.observedTick == tick && oldCount && newNext == oldNext)
            ++refreshed;
#else
          unsigned node = store.records[0].at(row.memory.key);
          record(a.get32(node + 12), unsigned(row.memory.key),
                 "owned observation key");
          record(a.get32(node + 16), unsigned(row.memory.observedTick),
                 "owned observation seen");
          record(a.get32(node + 20), unsigned(row.markedTick),
                 "owned observation marker");
          snapshot = node + 24;
#endif
          // Compare every raw byte when freshly copied, including inventory,
          // form, status, resource values and explicit borrowed CFG identity.
          if (row.memory.observedTick == tick) {
            Bytes bytes(0xe4b);
            for (unsigned i = 0; i < 0xe4b; ++i) {
              bool zero = (i >= 0x6d0 && i < 0x6d8) ||
                          (i >= 0x708 && i < 0x70c) ||
                          (i >= 0x8f9 && i < 0x8fd);
              bytes[i] =
                  static_cast<unsigned char>(zero ? 0 : a.get8(Pool + i));
            }
            expected[row.memory.key] = std::move(bytes);
          }
          for (unsigned i = 0; i < 0xe4b; ++i)
            record(a.get8(snapshot + i), expected.at(row.memory.key)[i],
                   "mode2 packed owned snapshot retained through hidden ticks");
          if (step == 1) {
            remembered = snapshot;
            oldKey = unsigned(row.memory.key);
#ifdef MNM_BRAIN_REFERENCE
            clone = reinterpret_cast<unsigned(__attribute__((thiscall)) *)(
                void *, void *, int, unsigned)>(0x413330)(
                reinterpret_cast<void *>(Task),
                reinterpret_cast<void *>(Brain + 0x11c02), tick, 1);
#else
            unsigned root =
                cache.prepare(1, knowledge, 0, tick, {32, 32, 8}, relation,
                              {[](int, int, int &) { return false; },
                               [](auto, auto) { return false; }},
                              [&](int key) { return store.snapshot(0, key); });
            clone = m::OwnerTaskSources<Arena>(a).clone(Task, root);
#endif
            need(clone && a.get32(clone + 0x29) != a.get32(clone + 0x2d),
                 "task retained clone");
            ++clones;
          }
        }
        if (clone) {
          unsigned group = a.get32(a.get32(clone + 0x29)),
                   leaf = a.get32(a.get32(group + 0x29));
          record(a.get32(leaf + 9), oldKey, "retained anonymous key");
          record(a.get32(leaf + 0x35) == remembered, 1,
                 "retained snapshot address");
          record(a.live(remembered),
                 !knowledge.records[0].empty() &&
                     knowledge.records[0].front().memory.key == int(oldKey),
                 "snapshot lifetime after update");
          if (!a.live(remembered)) {
            ++stale;
            record(a.get32(leaf + 0x89), 3,
                   "expiry alone retains copied source activity");
          }
        }
      }
      need(clone, "task before retirement");
      releaseTask(a);
      clone = 0;
    }
#ifdef MNM_BRAIN_REFERENCE
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x42f2e0)(
        reinterpret_cast<void *>(Brain + 0x11c02));
    a.free(a.get32(Brain + 0x11c26));
#else
    store.clear();
#endif
    cache.close();
    a.free(a.get32(Task + 0x3b));
    a.guards();
    for (auto b : a.blocks)
      record(b.second.live, 0, "all owned storage released");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(), reinterpret_cast<void *>(0x401000),
                    text.size()),
       "original text unchanged");
#endif
  need(inserted && refreshed && expired && clones && stale,
       "ownership witnesses");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":64,\"slices\":" << slices
            << ",\"insertions\":" << inserted << ",\"refreshes\":" << refreshed
            << ",\"expiries\":" << expired << ",\"task_clones\":" << clones
            << ",\"stale_snapshot_references\":" << stale
            << ",\"native_admission_refusals\":64,\"resource_releases\":"
            << resources << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
