#define main unused_owner_probe_fixture_main
#include "owner-task-probe-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_group_features.hpp"
#include "../reconstruction/creature-ai/owner_group_task.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
std::vector<unsigned> commands;
void __attribute__((thiscall)) targetCommand(void *, void *target) {
  commands = {1, *reinterpret_cast<unsigned *>(target)};
}
void __attribute__((thiscall)) moveCommand(void *, int x, int y, int z,
                                           unsigned adjust, unsigned option,
                                           unsigned other) {
  need(!adjust && !option && !other, "hostile movement options");
  commands = {2, unsigned(x), unsigned(y), unsigned(z)};
}
void __attribute__((thiscall)) stopCommand(void *, unsigned) { commands = {3}; }
void guardAllocations() {
  for (auto item : allocated) {
    auto *p = static_cast<unsigned char *>(item.first);
    auto n = item.second;
    for (unsigned i = 0; i < 64; ++i)
      need(p[i] == 0xa5 && p[n - 64 + i] == 0xa5,
           "original fixture allocation redzone");
  }
}
void *__attribute__((cdecl)) guardedAllocate(unsigned n) {
  guardAllocations();
  auto *p = static_cast<unsigned char *>(allocate(n + 128));
  std::fill(p, p + n + 128, 0xa5);
  return p + 64;
}
void __attribute__((cdecl)) guardedRelease(void *p) {
  if (!p)
    return;
  guardAllocations();
  release(static_cast<unsigned char *>(p) - 64);
}
struct OriginalMemory {
  Bytes ctx = Bytes(0x100), state = Bytes(48), head = Bytes(0xe68),
        nil = Bytes(0xe68);
  std::vector<Bytes> nodes;
  void *root = nullptr;
  OriginalMemory(const m::OwnerKnowledge &k, OriginalProbe &o)
      : nodes(k.records[0].size(), Bytes(0xe68)) {
    put(state.data(), 4, address(head.data()));
    put(nil.data(), 0xe64, 1);
    global(0x5faf4c, address(nil.data()));
    auto h = address(head.data()), n = address(nil.data());
    put(head.data(), 0, address(nodes.front().data()));
    put(head.data(), 4, address(nodes.front().data()));
    put(head.data(), 8, address(nodes.back().data()));
    for (unsigned i = 0; i < nodes.size(); ++i) {
      auto *raw = nodes[i].data();
      auto &r = k.records[0][i];
      std::memcpy(raw + 24, o.pool.data() + 0xe4b, 0xe4b);
      put(raw, 0, n);
      put(raw, 4, i ? address(nodes[i - 1].data()) : h);
      put(raw, 8, i + 1 < nodes.size() ? address(nodes[i + 1].data()) : n);
      put(raw, 12, r.memory.key);
      put(raw, 16, r.memory.observedTick);
      put(raw, 20, r.markedTick);
      put(raw, 32, r.memory.actor.position.x);
      put(raw, 36, r.memory.actor.position.y);
      put(raw, 40, r.memory.actor.position.z);
      put(raw, 0xdff, r.memory.actor.region);
      put(raw, 0xb7b, r.planning->motionB63);
      put(raw, 0xe64, 1);
    }
    *reinterpret_cast<unsigned char *>(0x643a80 + 24) = 1;
    using Build =
        void *(__attribute__((thiscall)) *)(void *, void *, int, unsigned);
    root = reinterpret_cast<Build>(0x413330)(ctx.data(), state.data(), 100, 1);
    need((get(root, 0x2d) - get(root, 0x29)) / 4 == 1,
         "journey original group count");
    auto target = get(reinterpret_cast<void *>(get(root, 0x29)));
    put(o.task.data(), 0x14a, address(root));
    put(o.task.data(), 0x18c, target);
    put(o.task.data(), 0x1b0, 20);
  }
};
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = 1;
  segment.limit_in_pages = 1;
  segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "probe FS");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&guardedAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&guardedRelease));
  redirect(0x4766b0, reinterpret_cast<void *>(&inference));
  redirect(0x50af10, reinterpret_cast<void *>(&targetCommand));
  redirect(0x50fd50, reinterpret_cast<void *>(&moveCommand));
  redirect(0x50fef0, reinterpret_cast<void *>(&stopCommand));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "probe RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned cases = 0, slices = 0, targets = 0, lost = 0, reacquired = 0,
           stops = 0;
  for (unsigned n = 0; n < 128; ++n) {
    std::vector<m::NavigationRegion> graph(8);
    for (unsigned i = 0; i < 8; ++i) {
      auto &r = graph[i];
      r.position = {int(i * 2), 2, 2};
      r.parameter = 1;
      r.dangerBase = i % 3;
      if (i + 1 < 8) {
        auto &e = r.connections[0];
        e.destination = int(i + 1);
        e.cost = 8;
        e.clearance.fill(8);
      }
    }
    m::OwnerKnowledge knowledge;
    for (unsigned i = 0; i < 2; ++i) {
      m::OwnerMemory r;
      r.memory.key = int(5 + i * 3);
      r.memory.observedTick = 100;
      r.markedTick = 100;
      r.memory.actor.slot = 1;
      r.memory.actor.owner = 1;
      r.memory.actor.health = 100;
      r.memory.actor.region = 6 + int(i);
      r.memory.actor.position = graph[6 + i].position;
      r.planning.emplace();
      r.planning->position = r.memory.actor.position;
      r.planning->speed = 24;
      r.planning->physical = true;
      r.planning->danger = graph[6 + i].dangerBase;
      knowledge.records[0].push_back(r);
    }
    std::array<std::array<std::uint8_t, 8>, 8> allied{};
    allied[0][0] = 1;
    m::OwnerMemoryGroups groups;
    m::buildOwnerMemoryGroups(
        groups, knowledge, 0, 100, true, {32, 32, 8}, allied,
        {[&](int a, int b, int &v) {
           return m::regionalDangerCenter(graph, a, b, {4, 1, 1, 3, 0, 0, 0},
                                          {32, 32, 8}, v);
         },
         [](auto, auto) { return false; }});
    need(groups.nodes[0].children.size() == 1, "native journey group count");
    const auto root = groups.nodes[0].children[0];
    const int to = groups.nodes[root].region;
#ifdef MNM_BRAIN_REFERENCE
    using RegionCtor = void *(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<RegionCtor>(0x53a660)(reinterpret_cast<void *>(0x6eaf08),
                                           512);
    OriginalProbe original(graph, 0, to);
    OriginalMemory memory(knowledge, original);
    const auto target = get(reinterpret_cast<void *>(get(memory.root, 0x29)));
    record(unsigned(groups.nodes[root].position.x),
           get(reinterpret_cast<void *>(target), 13), "journey group center");
    record(unsigned(to), get(reinterpret_cast<void *>(target), 25),
           "journey group region");
#else
    record(unsigned(groups.nodes[root].position.x),
           unsigned(groups.nodes[root].position.x), "journey group center");
    record(unsigned(to), unsigned(to), "journey group region");
#endif
    m::OwnerTaskProbe probe;
    m::RegionalQuery query;
    query.movement = {1, 1, 0, 3, 0, 17, 0};
    m::RegionalServices lower;
    lower.danger = [&](int id) {
      return int(graph.at(unsigned(id)).dangerBase);
    };
    bool reached = false;
    for (unsigned slice = 0; slice < 16; ++slice) {
      int budget = slice ? 1 : 0, other = budget;
      auto result = m::advanceOwnerTaskProbe(probe, graph, 0, to, query,
                                             {32, 32, 8}, budget, lower);
      ++slices;
#ifdef MNM_BRAIN_REFERENCE
      record(unsigned(result), unsigned(original.advance(other)),
             "journey probe outcome");
      record(unsigned(budget), unsigned(other), "journey probe budget");
#else
      (void)other;
      record(unsigned(result), unsigned(result), "journey probe outcome");
      record(unsigned(budget), unsigned(budget), "journey probe budget");
#endif
      if (result != m::OwnerTaskProbeResult::pending) {
        need(result == m::OwnerTaskProbeResult::reached,
             "journey path failure");
        reached = true;
        break;
      }
    }
    need(reached, "journey never completed probe");
    m::OwnerPlannerActor friendly;
    friendly.position = {2, 2, 2};
    friendly.speed = 24;
    friendly.physical = true;
    std::vector<m::OwnerPlannerActor> enemy;
    for (auto &r : knowledge.records[0])
      enemy.push_back(*r.planning);
    auto prefix = m::hostileTaskGroupFeatures({friendly}, enemy, {32, 32, 8});
    std::vector<int> features(prefix.begin(), prefix.end());
    auto end =
        std::find_if(probe.search.records.begin(), probe.search.records.end(),
                     [&](const auto &r) { return r.node == to; });
    auto payload = mnm::reconstruction::neighbor_movement(end->movement);
    int risk = 0;
    for (auto id : probe.path)
      risk = std::max(risk, lower.danger(id));
    features.insert(features.end(),
                    {int(std::sqrt(static_cast<long double>(
                         groups.nodes[root].uncertaintySquared))),
                     m::ownerTaskTravelTime(payload.accumulated_cost, 24),
                     payload.unknown_18, lower.danger(to), 0});
#ifdef MNM_BRAIN_REFERENCE
    need(captured.size() == features.size(), "journey feature size");
    for (unsigned i = 0; i < features.size(); ++i)
      record(features[i], captured[i],
             ("journey feature " + std::to_string(i)).c_str());
#else
    for (auto f : features)
      record(f, f, "journey feature");
#endif
    m::AutonomousWorld world;
    world.dimensions = {32, 32, 8};
    world.relation = allied;
    world.creatures.resize(2);
    for (unsigned i = 0; i < 2; ++i) {
      auto &a = world.creatures[i];
      a.ai.slot = i;
      a.ai.kind = 1;
      a.ai.owner = int(i);
      a.ai.type = 17;
      a.ai.active = true;
      a.ai.health = 100;
      a.ai.physical = 1;
      a.ai.position = {int(2 + i * 12), 2, 2};
    }
    m::OwnerGroupTask task{groups, root, risk};
    for (unsigned phase = 0; phase < 4; ++phase) {
      const int tick = 101 + int(phase);
      for (auto &r : knowledge.records[0])
        r.markedTick = phase == 1 ? 100 : tick;
      world.creatures[1].ai.position.x = int((14 + n % 3 + phase) % 32);
      m::NavigationRegion current;
      current.dangerBase = phase == 3 ? 50 : 0;
      auto old = task;
      int budget = 8, other = budget;
      auto out = m::executeOwnerGroupTask(
          task, knowledge, tick, 0, 0, world, current.dangerBase,
          current.dangerBase, 0, {{1000, 900}}, budget);
      std::vector<unsigned> order;
      order.reserve(5);
      order.push_back(unsigned(out.continuing));
      if (out.stop) {
        order.push_back(3);
        ++stops;
      } else if (out.kind == m::OwnerHostileOrderKind::target) {
        order.insert(order.end(), {1, unsigned(out.target)});
        if (phase == 2)
          ++reacquired;
        else
          ++targets;
      } else if (out.kind == m::OwnerHostileOrderKind::move) {
        order.insert(order.end(),
                     {2, unsigned(out.position.x), unsigned(out.position.y),
                      unsigned(out.position.z)});
        ++lost;
      }
#ifdef MNM_BRAIN_REFERENCE
      for (auto &node : memory.nodes)
        put(node.data(), 20, phase == 1 ? 100 : tick);
      put(original.pool.data() + 0xe4b, 8, world.creatures[1].ai.position.x);
      put(original.pool.data(), 0x91e, 1000);
      put(original.pool.data() + 0xe4b, 0x91e, 900);
      original.regions[644] = current.dangerBase;
      global(0x5faf1c, 0);
      put(original.task.data(), 0x16f, target);
      put(original.task.data(), 0x173, risk);
      commands.clear();
      int rawBudget[3] = {0, 0, other};
      using Execute = unsigned(__attribute__((thiscall)) *)(void *, void *,
                                                            unsigned, int *);
      auto continued = reinterpret_cast<Execute>(0x432900)(original.task.data(),
                                                           memory.state.data(),
                                                           tick, rawBudget) &
                       255;
      auto expected = commands;
      expected.insert(expected.begin(), continued);
      other = rawBudget[2];
      guardAllocations();
      need(order == expected, "journey execution order");
      record(unsigned(budget), unsigned(other), "journey execution budget");
      record(task.groups.nodes[root].position.x,
             get(reinterpret_cast<void *>(target), 13),
             "journey pursuit center");
#else
      (void)old;
      (void)other;
      record(unsigned(budget), unsigned(budget), "journey execution budget");
      record(task.groups.nodes[root].position.x,
             task.groups.nodes[root].position.x, "journey pursuit center");
#endif
      for (auto v : order)
        record(v, v, "journey order");
    }
    ++cases;
  }
  need(targets && lost && reacquired && stops, "journey witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "journey text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":" << cases << ",\"slices\":" << slices
            << ",\"targets\":" << targets << ",\"lost\":" << lost
            << ",\"reacquired\":" << reacquired << ",\"stops\":" << stops
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
