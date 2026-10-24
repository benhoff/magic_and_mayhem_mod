#define main unused_snapshot_ownership_main
#include "observation-snapshot-ownership-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_task_credit_services.hpp"
#include "../reconstruction/creature-ai/owner_task_storage.hpp"
#include "../reconstruction/rendering/actor_service_effects.hpp"
namespace {
using U = std::uint32_t;
constexpr U Subject = Pool + 0xe4b, Items = Scratch + 0x1000;
struct Memory : Arena {
  Bytes globals = Bytes(0x140000);
  m::ContributionProfiles profiles;
  unsigned episode = 0;
  bool stop = false, chain = false;
  std::vector<std::vector<U>> calls;
  unsigned char *gp(U p, U n = 4) {
    need(p >= 0x5c0000 && p - 0x5c0000 + n <= globals.size(), "credit globals");
#ifdef MNM_BRAIN_REFERENCE
    return reinterpret_cast<unsigned char *>(p);
#else
    return globals.data() + p - 0x5c0000;
#endif
  }
  U get16(U p) { return get8(p) | (get8(p + 1) << 8); }
  U get32(U p) {
    if (p >= 0x5c0000 && p < 0x700000) {
      U v;
      std::memcpy(&v, gp(p), 4);
      return v;
    }
    return Arena::get32(p);
  }
  U get8(U p) {
    return p >= 0x5c0000 && p < 0x700000 ? *gp(p, 1) : Arena::get8(p);
  }
  void set32(U p, U v) {
    if (p >= 0x5c0000 && p < 0x700000)
      std::memcpy(gp(p), &v, 4);
    else
      Arena::set32(p, v);
  }
  void set8(U p, U v) {
    if (p >= 0x5c0000 && p < 0x700000)
      *gp(p, 1) = static_cast<unsigned char>(v);
    else
      Arena::set8(p, v);
  }
  void clear(U p, U n) {
    for (U i = 0; i < n; ++i)
      set8(p + i, 0);
  }
  void destroySource(U p) { m::OwnerTaskSources<Memory>(*this).destroy(p); }
  U temp = Scratch + 0x5000;
  U temporary(U n) {
    U p = temp;
    temp += (n + 15) & ~15u;
    need(temp < Scratch + 0xe000, "credit temporaries");
    return p;
  }
  U service(U id, U self, std::initializer_list<U> args = {}) {
    std::vector<U> a(args);
    calls.push_back({id, self});
    calls.back().insert(calls.back().end(), a.begin(), a.end());
    if (id == 0x464c10 || id == 0x506220)
      return 1;
    if (id == 0x49cdb0)
      return 0;
    if (id == 0x5437f0)
      return chain ? 1 : 0xffff;
    if (id == 0x5097b0) {
      bool accepted = (episode % 4) != 1;
      if (accepted) {
        if (a[0] <= 4 || a[0] == 6)
          set32(self + 0xe4,
                std::min(get32(self + 0xe4) + a[1] * 4,
                         m::OwnerTaskCreditServices<Memory>(*this, profiles)
                             .maximum(self)));
        if (a[0] == 19)
          set32(self + 0xe8, get32(self + 0xe8) + (a[1] << 8));
      }
      return accepted;
    }
    if (id == 0x543c60)
      set32(self + 4, 0);
    if (id == 0x4d6f30 && episode % 5 == 0) {
      U actor = m::OwnerTaskCreditServices<Memory>(*this, profiles)
                    .associated(int(get32(0x644520)));
      set32(actor + 0xe8, get32(actor + 0xe8) + 256);
      set32(Pool + 0xa8, (get32(Pool + 0xa8) + 1) % 28);
    }
    if (id == 0x5070e0) {
      set32(self + 8, a[0]);
      set32(self + 12, a[1]);
      set32(self + 16, a[2]);
      set32(self + 20, a[0] * 32);
      set32(self + 24, a[1] * 32);
      set32(self + 28, a[2] * 16);
    }
    if (id == 0x51a8f0 && stop)
      set32(self + 4, 0);
    return 0;
  }
  U invoke(U id, U self, std::initializer_list<U> args = {}) {
    U a[8]{};
    std::copy(args.begin(), args.end(), a);
    m::OwnerTaskCreditServices<Memory> c(*this, profiles);
    switch (id) {
    case 0x522770:
      return c.taskForActor(self, a[0]);
    case 0x5075f0:
      return c.maximum(self);
    case 0x411e50: {
      U k = get32(self);
      return k == 0 || k == 24 || k == 25 || k == 26;
    }
    case 0x51e7d0:
      c.setPending(self, a[0]);
      return 0;
    case 0x51e500:
      return c.aggregateResource(self);
    case 0x507fa0:
      c.snapshotResource(self, int(a[0]), a[1]);
      return 0;
    case 0x466870:
      return c.associated(int(a[0]));
    default:
      return service(id, self, args);
    }
  }
  U fastcall(U id, U first, U second, std::initializer_list<U> args = {}) {
    if (id == 0x416d80) {
      U a[4]{};
      std::copy(args.begin(), args.end(), a);
      float f[3];
      for (U i = 0; i < 3; ++i)
        std::memcpy(&f[i], a + i + 1, 4);
      return m::OwnerTaskCreditServices<Memory>(*this, profiles)
          .account(first, second, a[0], f[0], f[1], f[2]);
    }
    return service(id, first, {second});
  }
};
Memory *memory;
#ifdef MNM_BRAIN_REFERENCE
#define S0(name, id)                                                           \
  U __attribute__((thiscall)) name(void *p) {                                  \
    return memory->service(id, address(p));                                    \
  }
#define S1(name, id)                                                           \
  U __attribute__((thiscall)) name(void *p, U a) {                             \
    return memory->service(id, address(p), {a});                               \
  }
#define S2(name, id)                                                           \
  U __attribute__((thiscall)) name(void *p, U a, U b) {                        \
    return memory->service(id, address(p), {a, b});                            \
  }
S1(registerActor, 0x49d120)
S2(painCredit, 0x4d9890)
S1(primaryResource, 0x4d6f30) S1(secondaryResource, 0x4d3a10)
    S1(displayHealth, 0x4d3a30) S1(barValue, 0x58f830) S0(barNotify, 0x58f890)
        S0(responseRepair, 0x4637c0) S0(casterAvailable, 0x506220)
            S0(nextItem, 0x5437f0) S0(effectSlot, 0x49cdb0) S0(removeItem,
                                                               0x543c60)
                S1(itemCue, 0x543ee0) S0(environmentCredit, 0x51a8f0)
                    S0(animationCredit, 0x50f850) S0(terrainCredit, 0x51aa30)
                        S0(resetCredit, 0x50ab30) S1(animationBase, 0x464b30)
                            S2(animationReady, 0x464c10)
                                S0(colorTick, 0x54dd60) U
    __attribute__((thiscall)) noopCredit(void *) {
  return 0;
}
U __attribute__((thiscall)) applyItem(void *p, U a, U b, U c, U d, U e, U f) {
  return memory->service(0x5097b0, address(p), {a, b, c, d, e, f});
}
U __attribute__((thiscall)) scenarioCredit(void *p, U a, U b, U c, U d, U e,
                                           U f) {
  return memory->service(0x564e90, address(p), {a, b, c, d, e, f});
}
U __attribute__((thiscall)) audioCredit(void *p, U a, U b, U c, U d, U e, U f,
                                        U g) {
  return memory->service(0x571310, address(p), {a, b, c, d, e, f, g});
}
U __attribute__((thiscall)) snapCredit(void *p, U a, U b, U c, U d) {
  return memory->service(0x5070e0, address(p), {a, b, c, d});
}
U __attribute__((fastcall)) itemAfter(U p, U q) {
  return memory->service(0x4680f0, p, {q});
}
#endif
U makeTask(Memory &a) {
#ifdef MNM_BRAIN_REFERENCE
  (void)a;
  return reinterpret_cast<U (*)()>(0x419e40)();
#else
  return m::OwnerTaskStorage<Memory>(a).fallback();
#endif
}
void destroyTask(Memory &a, U p) {
#ifdef MNM_BRAIN_REFERENCE
  reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x418050)(
      reinterpret_cast<void *>(p));
#else
  m::OwnerTaskStorage<Memory>(a).destroy(p);
#endif
  a.free(p);
}
// Existing maintenance model supplies all ordinary sibling bookkeeping. Only
// its concrete upkeep provider is new; animation/observation/color/action
// services retain the same authored normal no-op behavior as the original.
[[maybe_unused]] void nativeMaintenance(Memory &a) {
  m::AutonomousWorld world;
  world.creatures.resize(1);
  auto &actor = world.creatures[0];
  actor.ai.slot = 0;
  actor.ai.type = a.get32(Pool + 0xa8);
  actor.ai.owner = int(a.get32(Pool + 0x174));
  actor.ai.kind = a.get32(Config);
  actor.ai.health = a.get32(Pool + 0xe4);
  actor.ai.active = a.get32(Pool + 4);
  actor.ai.gateFC = a.get32(Pool + 0xfc);
  for (U i = 0; i < 8; ++i)
    actor.ai.visible[i] = a.get32(Pool + 0x24 + 4 * i);
  m::CreatureExecution exec;
  exec.dirtyValue = a.get32(Pool + 0x92e);
  m::CreatureMaintenance state;
  state.upkeepDeadline = a.get32(Pool + 0x100);
  state.timer660 = a.get32(Pool + 0x660);
  for (U i = 0; i < 8; ++i)
    state.visibilityAge[i] = a.get32(Pool + 0xe0b + 4 * i);
  m::MaintenanceEnvironment env;
  env.tick = a.get32(0x6c4830);
  env.upkeepInterval = a.get32(0x5e1904);
  env.localOwner = int(a.get32(0x644520));
  m::MaintenanceServices s;
  s.repairAnimation = [&](auto &) {
    a.service(0x464b30, 0, {0});
    a.service(0x464c10, 0x6dbf00 + actor.ai.type * 276, {0, 0});
  };
  s.upkeep = [&](auto &) {
    m::OwnerTaskCreditServices<Memory>(a, a.profiles).periodicResource(Pool);
    exec.dirtyValue = a.get32(Pool + 0x92e);
  };
  s.refresh = [](auto &) {};
  s.action = [](auto &) {};
  s.color = [&](auto &, auto color, U) {
    need(color == m::MaintenanceColor::tick, "ordinary maintenance colors");
    a.service(0x54dd60, Pool + 0xbf3);
  };
  m::maintainCreature(actor, exec, state, world, env, s);
  a.set32(Pool + 0x100, state.upkeepDeadline);
  a.set32(Pool + 0x660, state.timer660);
  a.set32(Pool + 0x92e, exec.dirtyValue);
  for (U i = 0; i < 8; ++i)
    a.set32(Pool + 0xe0b + 4 * i, state.visibilityAge[i]);
}
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  need(mmap(reinterpret_cast<void *>(Heap), Size, PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
            0) != MAP_FAILED,
       "credit heap");
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
  for (auto entry : {std::pair<U, void *>{
                         0x49d120, reinterpret_cast<void *>(&registerActor)},
                     {0x4d9890, reinterpret_cast<void *>(&painCredit)},
                     {0x4d6f30, reinterpret_cast<void *>(&primaryResource)},
                     {0x4d3a10, reinterpret_cast<void *>(&secondaryResource)},
                     {0x4d3a30, reinterpret_cast<void *>(&displayHealth)},
                     {0x58f830, reinterpret_cast<void *>(&barValue)},
                     {0x58f890, reinterpret_cast<void *>(&barNotify)},
                     {0x4637c0, reinterpret_cast<void *>(&responseRepair)},
                     {0x506220, reinterpret_cast<void *>(&casterAvailable)},
                     {0x5437f0, reinterpret_cast<void *>(&nextItem)},
                     {0x49cdb0, reinterpret_cast<void *>(&effectSlot)},
                     {0x543c60, reinterpret_cast<void *>(&removeItem)},
                     {0x543ee0, reinterpret_cast<void *>(&itemCue)},
                     {0x5097b0, reinterpret_cast<void *>(&applyItem)},
                     {0x564e90, reinterpret_cast<void *>(&scenarioCredit)},
                     {0x571310, reinterpret_cast<void *>(&audioCredit)},
                     {0x4680f0, reinterpret_cast<void *>(&itemAfter)},
                     {0x51a8f0, reinterpret_cast<void *>(&environmentCredit)},
                     {0x50f850, reinterpret_cast<void *>(&animationCredit)},
                     {0x51aa30, reinterpret_cast<void *>(&terrainCredit)},
                     {0x50ab30, reinterpret_cast<void *>(&resetCredit)},
                     {0x5070e0, reinterpret_cast<void *>(&snapCredit)},
                     {0x464b30, reinterpret_cast<void *>(&animationBase)},
                     {0x464c10, reinterpret_cast<void *>(&animationReady)},
                     {0x54dd60, reinterpret_cast<void *>(&colorTick)},
                     {0x54dac0, reinterpret_cast<void *>(&noopCredit)}})
    redirect(entry.first, entry.second);
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "credit RX");
#else
  (void)argc;
  (void)argv;
#endif
  U slices = 0, constructed = 0, retired = 0, lookups = 0, rewardChanges = 0,
    costChanges = 0, wraps = 0, itemCalls = 0;
  for (U episode = 0; episode < 128; ++episode) {
    Memory a;
    memory = &a;
    current = &a;
    a.episode = episode;
#ifdef MNM_BRAIN_REFERENCE
    std::memset(reinterpret_cast<void *>(Heap), 0xa5, Size);
#endif
    a.clear(Pool, 0xe4b * 2);
    a.clear(Config, 0x2000);
    a.clear(Items, 0x7e * 3);
    for (U g : {0x5faf2cu, 0x5faf34u, 0x5faf3cu, 0x5faf44u}) {
      a.set32(g, 0);
      a.set32(g + 4, 0);
    }
    a.set32(0x5d854c, 0);
    a.set32(0x6def58, Pool);
    a.set32(0x6def5c, 2);
    a.set32(0x6df180, 2);
    a.set32(0x6c4830, 20);
    a.set32(0x5e1904, 3 + episode % 5);
    a.set32(0x5e1908, 2 + episode % 7);
    a.set32(0x5e1910, 25);
    a.set32(0x5e1914, 50);
    a.set32(0x6e1f3c, 0);
    a.set8(0x6e9858, 0);
    a.set32(0x6c5c84, 0);
    a.set32(0x68991c, 0);
    a.set32(0x6c5c88, 3);
    U owner = episode % 8;
    a.set32(0x644520, episode % 2 ? owner : (owner + 1) % 8);
    for (U i = 0; i < 8; ++i) {
      U p = 0x643a80 + i * 340;
      a.set32(p, 1);
      a.set32(p + 4, 1);
      a.set32(p + 12, episode % 3 == 0 ? 0 : 1);
      a.set32(p + 16, ~0u);
    }
    U type = episode % 28,
      kind = std::array<U, 8>{0, 1, 8, 20, 24, 25, 26, 23}[episode / 16 % 8];
    for (U i = 0; i < 28; ++i) {
      auto &p = a.profiles[i];
      p.kind = i == type ? kind : 1;
      p.capacity = 20 + (episode + i) % 80;
      p.flag58 = (episode + i) % 2;
      for (U j = 0; j < 28; ++j) {
        p.first[j] = 1 + (episode + i + j) % 77;
        p.second[j] = 1 + (episode + i * 3 + j) % 91;
      }
      U at = 0x6a5f80 + i * 0x5c9;
      a.clear(at, 0x5c9);
      a.set32(at, p.kind);
      a.set32(at + 0x18, p.capacity);
      a.set32(at + 0x58, p.flag58);
      for (U j = 0; j < 28; ++j) {
        a.set32(at + 0x280 + j * 4, p.first[j]);
        a.set32(at + 0x1a0 + j * 4, p.second[j]);
      }
      a.set8(0x6a6500 + i * 0x5c9, 2);
    }
    a.set32(Config, kind);
    a.set32(Config + 0x18, 100);
    a.set32(Config + 0x1000, kind);
    a.set32(Config + 0x1018, 100);
    for (U actor : {Pool, Subject}) {
      a.set32(actor, actor == Pool ? 0 : 1);
      a.set32(actor + 4, 1);
      a.set32(actor + 0xac, actor == Pool ? Config : Config + 0x1000);
      a.set32(actor + 0xa8, type);
      a.set32(actor + 0x174, owner);
      a.set32(actor + 0xe4, 30 + episode % 20);
      a.set32(actor + 0x124, episode % 3);
      a.set32(actor + 0x8c1, episode % 2 ? 110 : 0);
      a.set32(actor + 0xec, 100);
      a.set32(actor + 0xe8, (8 + episode % 17) * 256 + 128);
      for (U i = 0; i < 8; ++i) {
        a.set32(actor + 0x24 + 4 * i, (episode + i) % 3 != 0);
        a.set32(actor + 0xe2b + i * 4, 512 + i * 17);
      }
      for (U i = 0; i < 42; ++i) {
        a.set32(actor + 0x17c + i * 24, i % 3 ? 19 : 18);
        a.set32(actor + 0x180 + i * 24, (episode + i) % 7);
        a.set32(actor + 0x190 + i * 24, i % 4 != 0);
      }
    }
    a.set32(Pool + 0x600, 0x54dac0);
    a.set32(Pool + 0x660, episode % 2 ? 0 : 1);
    a.set32(Pool + 0x977, 0);
    a.set32(Pool + 0x97b, 3);
    a.set32(Pool + 0xb8b, 17);
    for (U i = 0; i < 16; ++i) {
      a.set32(Pool + 0x97f + i * 28, 2 + i);
      a.set32(Pool + 0x983 + i * 28, 3);
      a.set32(Pool + 0x987 + i * 28, 1);
    }
    std::vector<U> tasks, roots, leaves;
    for (U i = 0; i < 3; ++i) {
      U t = makeTask(a);
      need(t, "task factory");
      tasks.push_back(t);
      ++constructed;
      a.set32(t + 0x2f, episode % 5 ? i * 7 : 0xfffffff0u);
      a.set32(t + 0x33, episode % 7 ? i * 11 : 0xfffffff8u);
      m::OwnerTaskSources<Memory> sources(a);
      U root = sources.anonymous(t), leaf = sources.actor(t, Pool);
      sources.attach(root, leaf);
      a.set32(root + 5, t);
      roots.push_back(root);
      leaves.push_back(leaf);
      a.set32(t + 8, root);
    }
    U shape = episode % 8;
    a.set32(Pool + 0x678,
            shape == 0 || shape == 2 ? 0 : (shape == 5 ? leaves[0] : roots[0]));
    a.set32(Pool + 0x67c, shape == 0 || shape == 1 ? 0
                          : shape == 4             ? roots[0]
                          : shape == 5             ? leaves[1]
                                                   : roots[1]);
    if (shape == 6)
      a.set32(roots[0] + 5, 0);
    U indexNil = a.allocate(24);
    a.zero(indexNil, 24);
    a.set32(indexNil + 20, 1);
    a.set32(0x5faf50, indexNil);
    std::vector<U> indexOwned{indexNil};
    for (U family = 0; family < 2; ++family) {
      U brain = family ? 0x644550 : 0x690600, head = a.allocate(24);
      a.zero(head, 24);
      indexOwned.push_back(head);
      a.set32(brain + 0xbe, head);
      std::array<U, 3> nodes;
      for (U &n : nodes) {
        n = a.allocate(24);
        a.zero(n, 24);
        indexOwned.push_back(n);
      }
      for (U i = 0; i < 3; ++i) {
        a.set32(nodes[i], indexNil);
        a.set32(nodes[i] + 4, i == 1 ? head : nodes[1]);
        a.set32(nodes[i] + 8, indexNil);
        a.set32(nodes[i] + 12, 10 + i * 10);
        a.set32(nodes[i] + 16, tasks[family ? 2 - i : i]);
      }
      a.set32(nodes[1], nodes[0]);
      a.set32(nodes[1] + 8, nodes[2]);
      a.set32(head, nodes[0]);
      a.set32(head + 4, nodes[1]);
      a.set32(head + 8, nodes[2]);
    }
    a.set32(0x6b1248, Items);
    a.set32(0x6b124c, 3);
    a.set32(0x6b1250, Items + 0x1000);
    a.set32(0x6b1254, 123);
    a.clear(Items + 0x1000, 123 * 32);
    for (U i = 0; i < 7; ++i)
      a.set32(Items + 0x1000 + i * 32 + 20, 2);
    for (U i = 0; i < 3; ++i) {
      a.set32(Items + i * 0x7e + 4, 1);
      a.set32(Items + i * 0x7e + 0x2a, i == 0 ? 61 : i == 1 ? 19 : episode % 7);
      a.set32(Items + i * 0x7e + 0x6a, 2 + episode % 3);
    }
    m::OwnerTaskCreditServices<Memory> c(a, a.profiles);
    a.chain = episode % 4 == 0;
    for (int key : {-1, 0, 9, 10, 15, 20, 30, 31})
      for (U family = 0; family < 2; ++family) {
        U expected = key == 10   ? tasks[family ? 2 : 0]
                     : key == 20 ? tasks[1]
                     : key == 30 ? tasks[family ? 0 : 2]
                                 : 0,
          actual;
#ifdef MNM_BRAIN_REFERENCE
        actual = reinterpret_cast<U(__attribute__((thiscall)) *)(void *, int)>(
            0x416c30)(reinterpret_cast<void *>(family ? 0x644550 : 0x690600),
                      key);
#else
        actual = c.taskById(family ? 0x644550 : 0x690600, key);
#endif
        record(actual, expected, "concrete task ID lookup");
        ++lookups;
      }
    for (U step = 0; step < 11; ++step) {
      a.calls.clear();
      a.temp = Scratch + 0x5000;
      std::array<U, 3> beforeReward{}, beforeCost{};
      for (U i = 0; i < 3; ++i) {
        beforeReward[i] = a.get32(tasks[i] + 0x2f);
        beforeCost[i] = a.get32(tasks[i] + 0x33);
      }
      std::optional<std::array<int, 2>> ids = std::array<int, 2>{
          std::array<int, 8>{-1, 0, 10, 15, 20, 30, 31, 10}[episode % 8],
          std::array<int, 8>{10, 20, 30, 31, -1, 15, 10, 20}[episode % 8]};
      if (episode % 9 == 0)
        ids.reset();
      if (step == 7) {
        // Ranked maximum health was exercised above. The admitted periodic
        // slice keeps bonus-animation/training maintenance siblings inactive.
        a.set32(Pool + 0x124, 0);
        a.set32(0x6c4830, episode % 3 ? a.get32(Pool + 0x100) + 1
                                      : a.get32(Pool + 0x100));
        if (episode % 7 == 0)
          a.set32(0x643a8c + owner * 340, ~0u);
      }
      if (step == 9)
        a.stop = episode % 11 == 0;
#ifdef MNM_BRAIN_REFERENCE
      void *actor = reinterpret_cast<void *>(Pool);
      if (step == 0)
        record(reinterpret_cast<U(__attribute__((thiscall)) *)(void *)>(
                   0x467c80)(actor),
               1, "activation return");
      if (step == 1 || step == 6)
        record(reinterpret_cast<U(__attribute__((thiscall)) *)(
                   void *, U, const int *)>(0x514990)(
                   actor, 1 + episode % 7, ids ? ids->data() : nullptr),
               1, "nonlethal return");
      if (step == 2 || step == 5)
        reinterpret_cast<void(__attribute__((thiscall)) *)(
            void *, int, int, void *, U)>(0x5076f0)(
            actor, step == 2 ? 7 : 1000, -1, nullptr, step == 2 || episode % 2);
      if (step == 3 || step == 4 || step == 8) {
        U item = Items + (step == 3 ? 2 : step == 4 ? 1 : 0) * 0x7e;
        reinterpret_cast<void(__attribute__((fastcall)) *)(U, U)>(0x468470)(
            Pool, item);
        ++itemCalls;
      }
      if (step == 7)
        reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x509e50)(
            actor);
      if (step >= 9)
        reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x512460)(
            actor);
#else
      if (step == 0)
        record(c.activate(Pool), 1, "activation return");
      if (step == 1 || step == 6)
        record(c.damage(Pool, 1 + episode % 7, ids), 1, "nonlethal return");
      if (step == 2 || step == 5)
        c.heal(Pool, step == 2 ? 7 : 1000, step == 2 || episode % 2);
      if (step == 3 || step == 4 || step == 8) {
        U item = Items + (step == 3 ? 2 : step == 4 ? 1 : 0) * 0x7e;
        mnm::reconstruction::actor_services::EffectServices<Memory>(a).run(
            0x468470, Pool, {}, item);
        ++itemCalls;
      }
      if (step == 7)
        nativeMaintenance(a);
      if (step >= 9)
        c.completeSegment(Pool);
#endif
      if (std::getenv("MNM_CREDIT_TRACE") && episode < 8) {
        std::cerr << "TRACE " << episode << " " << step
                  << " health=" << a.get32(Pool + 0xe4)
                  << " bar=" << a.get32(Pool + 0x6c)
                  << " fc=" << a.get32(Pool + 0xfc)
                  << " deadline=" << a.get32(Pool + 0x100)
                  << " cursor=" << a.get32(Pool + 0x977);
        for (const auto &call : a.calls) {
          std::cerr << " [";
          for (U v : call)
            std::cerr << std::hex << v << ",";
          std::cerr << "]" << std::dec;
        }
        std::cerr << "\n";
      }
      for (U i = 0; i < 3; ++i) {
        U reward = a.get32(tasks[i] + 0x2f), cost = a.get32(tasks[i] + 0x33);
        rewardChanges += reward != beforeReward[i];
        costChanges += cost != beforeCost[i];
        wraps += (reward < beforeReward[i]) + (cost < beforeCost[i]);
        record(reward, reward, "shared task reward");
        record(cost, cost, "shared task cost");
      }
      for (U actor : {Pool, Subject})
        for (U i = 0; i < 0xe4b; ++i)
          record(a.get8(actor + i), a.get8(actor + i), "packed actor state");
      record(a.calls.size(), a.calls.size(), "external service count");
      for (const auto &call : a.calls) {
        record(call.size(), call.size(), "external service signature");
        for (U value : call)
          record(value, value, "ordered external service");
      }
      for (U p : {0x6c4830u, 0x6c5c88u})
        record(a.get32(p), a.get32(p), "provider global state");
      ++slices;
    }
    // Caller removes index entries and borrowed actor bindings before owner
    // retirement; this fixture does not claim general task
    // registration/erasure.
    a.set32(Pool + 0x678, 0);
    a.set32(Pool + 0x67c, 0);
    for (U family = 0; family < 2; ++family) {
      U brain = family ? 0x644550 : 0x690600, head = a.get32(brain + 0xbe);
      a.set32(head + 4, indexNil);
      record(c.taskById(brain, 20), 0, "retired task index absent");
    }
    for (U task : tasks) {
      destroyTask(a, task);
      ++retired;
    }
    for (U p : indexOwned)
      a.free(p);
    a.guards();
    for (auto row : a.blocks)
      record(row.second.live, 0, "all owned task/source/index storage retired");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(text.data(), reinterpret_cast<void *>(0x401000),
                    text.size()),
       "original text unchanged");
#endif
  need(rewardChanges && costChanges && wraps && constructed == retired,
       "broader credit witnesses");
  std::cout << "{\"success\":true,\"cases\":128,\"checks\":" << checks
            << ",\"slices\":" << slices << ",\"task_lookups\":" << lookups
            << ",\"task_constructions\":" << constructed
            << ",\"task_retirements\":" << retired
            << ",\"reward_changes\":" << rewardChanges
            << ",\"cost_changes\":" << costChanges
            << ",\"wrapping_changes\":" << wraps
            << ",\"item_calls\":" << itemCalls << ",\"digest\":\"" << digest
            << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
