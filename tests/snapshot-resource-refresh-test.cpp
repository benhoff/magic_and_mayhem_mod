
#pragma push_macro("main")
#undef main
#define main unused_zero_resource_snapshot_main
#include "observation-snapshot-ownership-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/rendering/actor_visual_lifecycle.hpp"
namespace {
std::set<unsigned> callbacks;
unsigned callbacksReleased = 0;
void actorFree(Arena &a, unsigned p) {
  if (callbacks.erase(p)) {
    record(a.get32(p), 0x5c5c40, "real callback destructor vtable");
    ++callbacksReleased;
  }
  a.free(p);
}
#ifdef MNM_BRAIN_REFERENCE
void __attribute__((cdecl)) resourceRelease(void *p) {
  actorFree(*current, address(p));
}
#endif
struct ActorBackend {
  Arena &a;
  unsigned r(unsigned p) {
    if (p == 0x5c7268)
      return 0x52cbe0;
    return a.get32(p);
  }
  void w(unsigned p, unsigned v) { a.set32(p, v); }
  void w8(unsigned p, unsigned v) { a.set8(p, v); }
  unsigned allocate(unsigned n) { return a.allocate(n); }
  void free(unsigned p) { actorFree(a, p); }
  void arrayFree(unsigned p) { a.freeBuffer(p); }
  unsigned call(unsigned entry, unsigned receiver, unsigned flag) {
    need(entry == 0x52cbe0 && flag == 1, "real callback deleting contract");
    a.set32(receiver, 0x5c5c40);
    free(receiver);
    return receiver;
  }
};
std::vector<unsigned> retainedBuffers;
unsigned resets = 0, liveDestructions = 0;
void resetLive(Arena &a) {
  // Original reset drops histogram backing; preserve that baseline explicitly.
  unsigned oldBuffer = a.get32(Pool + 0x6d0);
  if (oldBuffer)
    retainedBuffers.push_back(oldBuffer);
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *, unsigned)>(
      0x5060a0)(reinterpret_cast<void *>(Pool), 0);
#else
  ActorBackend backend{a};
  mnm::reconstruction::ActorLifecycle<ActorBackend>(backend).reset(Pool, 0);
#endif
  ++resets;
  unsigned callback = a.get32(Pool + 0x708);
  need(callback && callbacks.insert(callback).second,
       "fresh callback generation");
  record(a.get32(Pool + 0xcf3), callback, "live callback alias");
  record(a.get32(callback + 12), Pool, "callback owner");
  if (oldBuffer)
    record(a.live(oldBuffer), 1, "baseline reset retains histogram");
}
void destroyLive(Arena &a) {
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x505f70)(
      reinterpret_cast<void *>(Pool));
#else
  ActorBackend backend{a};
  mnm::reconstruction::ActorLifecycle<ActorBackend>(backend).destruct(Pool);
#endif
  ++liveDestructions;
  record(a.get32(Pool + 0x708), 0, "actor destructor clears callback");
  record(a.get32(Pool + 0x8f9), 0, "actor destructor clears list");
}
void appendOrder(Arena &a, unsigned seed) {
  unsigned list = a.get32(Pool + 0x8f9), head = a.get32(list + 4),
           previous = a.get32(head + 4), node = a.allocate(48);
  a.set32(previous, node);
  a.set32(node + 4, previous);
  a.set32(node, head);
  a.set32(head + 4, node);
  a.set32(node + 8, 0x99200000 + seed);
  a.set32(list + 8, a.get32(list + 8) + 1);
  a.set32(Pool + 0x8fd, node);
}
void populateLive(Arena &a, unsigned seed) {
  resetLive(a);
  for (unsigned j = 0; j < 1 + seed % 4; ++j)
    appendOrder(a, seed + j);
  unsigned histogram = a.allocate(24);
  for (unsigned i = 0; i < 24; ++i)
    a.set8(histogram + i, seed * 11 + i);
  a.set32(Pool + 0x6d0, histogram);
  a.set32(Pool + 0x6d4, 6);
}
std::vector<unsigned> liveResources(Arena &a) {
  std::vector<unsigned> out;
  auto add = [&](unsigned p, unsigned n) {
    out.push_back(p);
    need(a.live(p), "live actor resource survived observation");
    for (unsigned i = 0; i < n; ++i)
      out.push_back(a.get8(p + i));
  };
  unsigned callback = a.get32(Pool + 0x708), list = a.get32(Pool + 0x8f9),
           buffer = a.get32(Pool + 0x6d0);
  if (callback)
    add(callback, 16);
  if (list) {
    add(list, 12);
    unsigned head = a.get32(list + 4);
    add(head, 48);
    unsigned count = 0;
    for (unsigned node = a.get32(head); node != head; node = a.get32(node)) {
      need(++count <= 8, "bounded live list");
      add(node, 48);
    }
    need(count == a.get32(list + 8), "live list count");
  }
  if (buffer)
    add(buffer, 24);
  return out;
}
void compareResources(const std::vector<unsigned> &a,
                      const std::vector<unsigned> &b) {
  record(a.size(), b.size(), "actor resource extent");
  for (unsigned i = 0; i < a.size(); ++i)
    record(a[i], b.at(i), "actor resources unchanged by observation");
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
  redirect(0x597870, reinterpret_cast<void *>(&resourceRelease));
  redirect(0x5979b0, reinterpret_cast<void *>(&bufferFree));
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
           slices = 0;
  for (unsigned episode = 0; episode < 64; ++episode) {
    Arena a;
    current = &a;
#ifdef MNM_BRAIN_REFERENCE
    std::memset(reinterpret_cast<void *>(Heap), 0xa5, Size);
#endif
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
      for (unsigned step = 0; step < 12; ++step) {
        static constexpr unsigned times[] = {
            0, 1, 2, 3, 4, 1028, 1029, 1030, 1032, 1033, 1034, 1035};
        int tick = int(cycle * 1200 + times[step]);
        ++slices;
        bool visible =
            step <= 2 || step == 4 || step == 8 || step == 9 || step == 11;
        if (step == 0 || step == 3 || step == 11)
          populateLive(a, episode + cycle + step);
        if (step == 2)
          appendOrder(a, episode + cycle + 99);
        if (step == 10)
          destroyLive(a);
        auto ownedBefore = liveResources(a);
        unsigned deletedBefore = callbacksReleased;
        std::array<unsigned, 6> kept{};
        unsigned keptOffsets[] = {0x708, 0x8f9, 0xcf3, 0x8fd, 0x6d0, 0x6d4};
        for (unsigned i = 0; i < 6; ++i)
          kept[i] = a.get32(Pool + keptOffsets[i]);
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
                         {0x708, kept[0]},
                         {0x8f9, kept[1]}})
          a.set32(Pool + row.first, row.second);
        for (unsigned i = 0; i < 6; ++i)
          a.set32(Pool + keptOffsets[i], kept[i]);
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
        compareResources(liveResources(a), ownedBefore);
        record(callbacksReleased, deletedBefore,
               "observation never destroys live callback");
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
          record(a.get32(snapshot + 0x708), 0,
                 "stored callback ownership cleared");
          record(a.get32(snapshot + 0x8f9), 0, "stored list ownership cleared");
          unsigned alias = a.get32(snapshot + 0xcf3),
                   cursor = a.get32(snapshot + 0x8fd);
          record(a.live(alias), unsigned((step != 3 && step != 10)),
                 "callback borrowed alias becomes stale across unobserved "
                 "reset/destruction");
          record(a.live(cursor), unsigned((step != 3 && step != 10)),
                 "list cursor borrowed alias becomes stale across unobserved "
                 "reset/destruction");
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
    destroyLive(a);
    for (unsigned p : retainedBuffers) {
      need(a.live(p), "retained reset buffer independently owned");
      a.free(p);
    }
    retainedBuffers.clear();
    cache.close();
    a.free(a.get32(Task + 0x3b));
    a.guards();
    for (auto b : a.blocks)
      record(b.second.live, 0, "all owned storage released");
  }

#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(), reinterpret_cast<void *>(0x401000),
                    text.size()),
       "original resource refresh text unchanged");
#endif
  need(inserted && refreshed && expired && clones && stale && resets &&
           callbacksReleased,
       "resource lifecycle witnesses");
  need(callbacks.empty(), "all live callback generations released");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":64,\"slices\":" << slices
            << ",\"insertions\":" << inserted << ",\"refreshes\":" << refreshed
            << ",\"expiries\":" << expired << ",\"task_clones\":" << clones
            << ",\"stale_snapshot_references\":" << stale
            << ",\"actor_resets\":" << resets
            << ",\"live_destructions\":" << liveDestructions
            << ",\"callbacks_released\":" << callbacksReleased
            << ",\"digest\":\"" << digest << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
