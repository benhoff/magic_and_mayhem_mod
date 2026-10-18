#define main unused_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_group_task.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
std::vector<unsigned> commands;
void __attribute__((thiscall)) targetCommand(void *actor, void *target) {
  commands.insert(commands.end(), {*static_cast<unsigned *>(actor), 1,
                                   *static_cast<unsigned *>(target)});
}
void __attribute__((thiscall)) moveCommand(void *actor, int x, int y, int z,
                                           unsigned adjust, unsigned option,
                                           unsigned other) {
  need(!adjust && !option && !other, "hostile movement options");
  commands.insert(commands.end(), {*static_cast<unsigned *>(actor), 2,
                                   unsigned(x), unsigned(y), unsigned(z)});
}
void __attribute__((thiscall)) stopCommand(void *actor, unsigned) {
  commands.insert(commands.end(), {*static_cast<unsigned *>(actor), 3});
}
std::vector<unsigned>
oldGroup(m::OwnerGroupTask &task, const m::OwnerKnowledge &knowledge, int tick,
         const m::AutonomousWorld &world, const m::NavigationRegion &r,
         int percent, const std::vector<int> &threat,
         const std::vector<unsigned> &members, int &budget) {
  Bytes region(660), pool(125 * 0xe4b), configs(world.creatures.size() * 0x5c9),
      source(0x8d), object(0x200), trees(8 * 48), head(0xe68), nil(0xe68);
  global(0x6eaf08, address(region.data()));
  global(0x6eaf0c, 1);
  region[644] = r.dangerBase;
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 7; ++j) {
      region[532 + i * 7 + j] = r.danger[i][j];
      region[588 + i * 7 + j] = r.deductedDanger[i][j];
    }
  global(0x5faf1c, unsigned(percent));
  for (unsigned i = 0; i < world.creatures.size(); ++i) {
    const auto &a = world.creatures[i];
    auto *raw = pool.data() + i * 0xe4b, *cfg = configs.data() + i * 0x5c9;
    put(raw, 0, i);
    put(raw, 4, 1);
    put(raw, 8, a.ai.position.x);
    put(raw, 12, a.ai.position.y);
    put(raw, 16, a.ai.position.z);
    put(raw, 0xe4, a.ai.health);
    put(raw, 0x174, a.ai.owner);
    put(raw, 0xa8, 17);
    put(raw, 0xac, address(cfg));
    put(raw, 0xf8, a.ai.physical);
    put(raw, 0x91e, threat[i]);
    put(raw, 0x64c, unsigned(-1));
    put(raw, 0x664, a.ai.engaged);
    put(raw, 0x668, a.ai.incoming);
    put(raw, 0x164, a.ai.readiness);
    put(raw, 0x932, a.ai.dirty932);
    put(raw, 0xde7, 0);
    put(cfg, 0, 1);
  }
  global(0x6def58, address(pool.data()));
  global(0x6def5c, 125);
  global(0x6df180, 125);
  global(0x5d854c, 3);
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  global(0x643a80, 1);
  global(0x643a80 + 12, 0);
  for (unsigned i = 0; i < 8; ++i)
    *reinterpret_cast<unsigned char *>(0x643a80 + 24 + i) =
        world.relation[0][i];
  for (unsigned i = 0; i < 8; ++i)
    source[0x41 + i] = 1;
  std::vector<Bytes> sourceLeaves(members.size(), Bytes(0x8d));
  std::vector<unsigned> sourceChildren;
  for (unsigned i = 0; i < members.size(); ++i) {
    auto *leaf = sourceLeaves[i].data();
    std::fill(leaf + 0x41, leaf + 0x49, 1);
    put(leaf, 0x35, address(pool.data() + members[i] * 0xe4b));
    put(leaf, 0x39, address(source.data()));
    put(leaf, 0x89, 3);
    sourceChildren.push_back(address(leaf));
  }
  put(source.data(), 0x29, address(sourceChildren.data()));
  put(source.data(), 0x2d,
      address(sourceChildren.data() + sourceChildren.size()));
  put(source.data(), 0x31,
      address(sourceChildren.data() + sourceChildren.size()));
  put(source.data(), 0x89, 3);
  std::vector<Bytes> nodes(task.groups.nodes.size(), Bytes(0x8d));
  std::vector<std::vector<unsigned>> children(nodes.size());
  for (unsigned i = 0; i < nodes.size(); ++i) {
    auto *p = nodes[i].data();
    auto &n = task.groups.nodes[i];
    for (unsigned j = 0; j < 8; ++j)
      p[0x41 + j] = 1;
    put(p, 9, n.memoryKey);
    put(p, 13, n.position.x);
    put(p, 17, n.position.y);
    put(p, 21, n.position.z);
    put(p, 25, n.region);
    put(p, 0x89, n.activity);
    if (n.memoryKey >= 0)
      put(p, 0x35,
          address(pool.data() +
                  unsigned(3 + ((n.memoryKey - 5) / 3) % 3) * 0xe4b));
    if (n.parent >= 0)
      put(p, 0x39, address(nodes[unsigned(n.parent)].data()));
    for (auto child : n.children)
      children[i].push_back(address(nodes[child].data()));
    if (!children[i].empty()) {
      put(p, 0x29, address(children[i].data()));
      put(p, 0x2d, address(children[i].data() + children[i].size()));
      put(p, 0x31, address(children[i].data() + children[i].size()));
    }
  }
  put(nil.data(), 0xe64, 1);
  global(0x5faf4c, address(nil.data()));
  put(trees.data(), 4, address(head.data()));
  const auto &rows = knowledge.records[0];
  std::vector<Bytes> memories(rows.size(), Bytes(0xe68));
  auto h = address(head.data()), nilptr = address(nil.data());
  put(head.data(), 0, rows.empty() ? h : address(memories.front().data()));
  put(head.data(), 4, rows.empty() ? nilptr : address(memories.front().data()));
  put(head.data(), 8, rows.empty() ? h : address(memories.back().data()));
  for (unsigned i = 0; i < rows.size(); ++i) {
    auto *p = memories[i].data();
    put(p, 0, nilptr);
    put(p, 4, i ? address(memories[i - 1].data()) : h);
    put(p, 8, i + 1 < rows.size() ? address(memories[i + 1].data()) : nilptr);
    put(p, 12, rows[i].memory.key);
    put(p, 20, rows[i].markedTick);
    put(p, 24, rows[i].memory.actor.slot);
    put(p, 0xe64, 1);
  }
  put(object.data(), 4, 6);
  put(object.data(), 8, address(source.data()));
  put(object.data(), 0x27, 0);
  put(object.data(), 0x16f, address(nodes[task.root].data()));
  put(object.data(), 0x173, task.risk);
  commands.clear();
  int rawBudget[3] = {0, 0, budget};
  using Execute =
      unsigned(__attribute__((thiscall)) *)(void *, void *, unsigned, int *);
  auto result = reinterpret_cast<Execute>(0x432900)(object.data(), trees.data(),
                                                    tick, rawBudget) &
                255;
  budget = rawBudget[2];
  for (unsigned i = 0; i < nodes.size(); ++i) {
    auto get = [&](unsigned off) {
      unsigned v;
      std::memcpy(&v, nodes[i].data() + off, 4);
      return v;
    };
    task.groups.nodes[i].position = {int(get(13)), int(get(17)), int(get(21))};
    task.groups.nodes[i].activity = get(0x89);
  }
  auto out = commands;
  out.insert(out.begin(), result);
  need(allocated.empty(), "group execution allocation");
  return out;
}
#endif
} // namespace
#ifndef MNM_THREE_MEMBER_TASK_FIXTURE
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "source group FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  redirect(0x50af10, reinterpret_cast<void *>(&targetCommand));
  redirect(0x50fd50, reinterpret_cast<void *>(&moveCommand));
  redirect(0x50fef0, reinterpret_cast<void *>(&stopCommand));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "source group RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned refusals = 0;
  unsigned targeted = 0, stopped = 0, moved = 0, completed = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    m::AutonomousWorld world;
    world.dimensions = {32, 32, 8};
    world.creatures.resize(6);
    world.relation[0][0] = 1;
    for (unsigned i = 0; i < 6; ++i) {
      auto &a = world.creatures[i].ai;
      a.slot = i;
      a.owner = i >= 3;
      a.kind = 1;
      a.type = 17;
      a.active = true;
      a.health = 1 + (n * 13 + i * 7) % 200;
      a.physical = (n + i) % 2;
      a.position = {int((n / 7 + i * 9) % 32), int((n / 11 + i * 13) % 32),
                    int((n + i) % 8)};
    }
    world.creatures[0].ai.engaged = n % 7 == 0;
    world.creatures[1].ai.incoming = n % 11 == 0;
    world.creatures[0].ai.readiness = n % 13 == 0;
    world.creatures[1].ai.dirty932 = n % 17 == 0;
    if (n % 19 == 0)
      world.creatures[3].ai.health = 0;
    world.relation[0][1] = n % 23 == 0;
    m::OwnerKnowledge knowledge;
    m::OwnerMemoryGroups groups;
    for (unsigned i = 0; i < 1 + n % 7; ++i) {
      m::OwnerMemory row;
      row.memory.key = int(i * 3 + 5);
      row.memory.observedTick = 100;
      row.markedTick = (n + i) % 3 ? 100 : 99;
      row.memory.actor.slot = 3 + int(i % 3);
      row.memory.actor.owner = 1;
      row.memory.actor.health = 100;
      row.memory.actor.region = 0;
      row.memory.actor.position = {int((n + i * 3) % 32),
                                   int((n / 3 + i * 7) % 32), int((n + i) % 8)};
      row.planning.emplace();
      knowledge.records[0].push_back(row);
    }
    auto relation = world.relation;
    relation[0][1] = 0;
    m::buildOwnerMemoryGroups(groups, knowledge, 0, 100, true, world.dimensions,
                              relation,
                              {[](int, int, int &v) {
                                 v = 0;
                                 return true;
                               },
                               [](auto, auto) { return true; }});
    need(groups.nodes.size() > 1, "source group memories");
    m::OwnerGroupTask task{groups, groups.nodes[0].children[0], int(n % 13)};
    // Nested groups/aliases and lost observations are handled by real407c10.
    if (n % 5 == 0)
      knowledge.records[0].erase(knowledge.records[0].begin());
    auto original = task;
    m::NavigationRegion region;
    region.dangerBase = n % 5;
    region.danger[1][1] = (n / 5) % 21;
    region.deductedDanger[0][1] = (n / 7) % 11;
    std::vector<unsigned> members =
        n & 1 ? std::vector<unsigned>{2, 1, 0} : std::vector<unsigned>{0, 1, 2};
    std::vector<int> raw, net;
    for (auto slot : members) {
      raw.push_back(m::regionDanger(world.creatures[slot], region, {0, 0}));
      net.push_back(
          m::regionDanger(world.creatures[slot], region, {0, 0}, true));
    }
    std::vector<int> threat{int(n % 101),        int((n / 3) % 101),
                            int((n / 7) % 101),  int((n / 11) % 101),
                            int((n / 13) % 101), int((n / 17) % 101)};
    const int percent = int(n % 4) * 100;
    int budget = int(n % 16) - 4, other = budget;
    auto out =
        m::executeOwnerSourceGroupTask(task, knowledge, 100, 0, members, world,
                                       raw, net, percent, threat, budget);
    std::vector<unsigned> native{unsigned(out.continuing)};
    completed += !out.continuing;
    for (unsigned i = 0; i < out.orders.size(); ++i) {
      const auto &order = out.orders[i];
      if (order.stop) {
        native.insert(native.end(), {members[i], 3});
        ++stopped;
      } else if (order.kind == m::OwnerHostileOrderKind::target) {
        native.insert(native.end(), {members[i], 1, unsigned(order.target)});
        ++targeted;
      } else if (order.kind == m::OwnerHostileOrderKind::move) {
        native.insert(native.end(),
                      {members[i], 2, unsigned(order.position.x),
                       unsigned(order.position.y), unsigned(order.position.z)});
        ++moved;
      }
    }
#ifdef MNM_BRAIN_REFERENCE
    auto old = oldGroup(original, knowledge, 100, world, region, percent,
                        threat, members, other);
#else
    auto old = native;
    other = budget;
    original = task;
#endif
    if (native != old) {
      std::cerr << "case=" << n << " native=";
      for (auto v : native)
        std::cerr << v << ",";
      std::cerr << " original=";
      for (auto v : old)
        std::cerr << v << ",";
      std::cerr << " center native=" << task.groups.nodes[task.root].position.x
                << "," << task.groups.nodes[task.root].position.y << ","
                << task.groups.nodes[task.root].position.z << " original="
                << original.groups.nodes[original.root].position.x << ","
                << original.groups.nodes[original.root].position.y << ","
                << original.groups.nodes[original.root].position.z << "\n";
    }
    record(native.size(), old.size(), "source group ordered command count");
    for (unsigned i = 0; i < native.size(); ++i)
      record(native[i], old[i], "source group ordered command");
    record(unsigned(budget), unsigned(other), "shared body charge");
    for (unsigned i = 0; i < task.groups.nodes.size(); ++i) {
      const auto &a = task.groups.nodes[i], &b = original.groups.nodes[i];
      record(a.activity, b.activity, "source group refreshed activity");
      record(unsigned(a.position.x), unsigned(b.position.x),
             "source group center x");
      record(unsigned(a.position.y), unsigned(b.position.y),
             "source group center y");
      record(unsigned(a.position.z), unsigned(b.position.z),
             "source group center z");
    }
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "source group original text unchanged");
#endif
  need(targeted && stopped && moved && completed,
       "source group branch coverage");
  std::cout << "{\"success\":true,\"cases\":8192,\"checks\":" << checks
            << ",\"targets\":" << targeted << ",\"refusals\":" << refusals
            << ",\"stops\":" << stopped << ",\"moves\":" << moved
            << ",\"completed\":" << completed << ",\"digest\":" << digest
            << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}

#endif
