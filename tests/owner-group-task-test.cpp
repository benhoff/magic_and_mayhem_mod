#define main unused_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
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
std::vector<unsigned> oldGroup(m::OwnerGroupTask &task,
                               const m::OwnerKnowledge &knowledge, int tick,
                               const m::AutonomousWorld &world,
                               const m::NavigationRegion &r, int percent,
                               const std::array<int, 2> &threat, int &budget) {
  Bytes region(660), pool(125 * 0xe4b), configs(2 * 0x5c9), source(0x8d),
      object(0x200), trees(8 * 48), head(0xe68), nil(0xe68);
  global(0x6eaf08, address(region.data()));
  global(0x6eaf0c, 1);
  region[644] = r.dangerBase;
  for (unsigned i = 0; i < 8; ++i)
    for (unsigned j = 0; j < 7; ++j) {
      region[532 + i * 7 + j] = r.danger[i][j];
      region[588 + i * 7 + j] = r.deductedDanger[i][j];
    }
  global(0x5faf1c, unsigned(percent));
  for (unsigned i = 0; i < 2; ++i) {
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
  put(source.data(), 0x35, address(pool.data()));
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
      put(p, 0x35, address(pool.data() + 0xe4b));
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
  need(!syscall(SYS_set_thread_area, &segment), "original exception segment");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
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
       "task RX text");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned targets = 0, moves = 0, stops = 0, completed = 0, blocked = 0;
  for (unsigned n = 0; n < 4096; ++n) {
    m::AutonomousWorld w;
    w.dimensions = {32, 32, 8};
    w.creatures.resize(2);
    w.relation[0][0] = 1;
    for (unsigned i = 0; i < 2; ++i) {
      auto &a = w.creatures[i];
      a.ai.slot = i;
      a.ai.type = 17;
      a.ai.kind = 1;
      a.ai.owner = int(i);
      a.ai.active = true;
      a.ai.health = 1 + (n * 13 + i * 7) % 200;
      a.ai.physical = (n + i) % 2;
      a.ai.position = {int((n / 7 + i * 9) % 32), int((n / 11 + i * 13) % 32),
                       int((n + i) % 8)};
    }
    w.creatures[0].ai.engaged = n % 7 == 0;
    w.creatures[0].ai.incoming = n % 11 == 0;
    w.creatures[0].ai.readiness = n % 13 == 0;
    w.creatures[0].ai.dirty932 = n % 17 == 0;
    if (n % 19 == 0)
      w.creatures[1].ai.health = 0;
    w.relation[0][1] = n % 23 == 0;
    m::OwnerKnowledge k;
    m::OwnerMemoryGroups groups;
    for (unsigned i = 0; i < 1 + n % 7; ++i) {
      m::OwnerMemory r;
      r.memory.key = int(i * 3 + 5);
      r.memory.observedTick = 100;
      r.markedTick = (n + i) % 3 ? 100 : 99;
      r.memory.actor.slot = 1;
      r.memory.actor.owner = 1;
      r.memory.actor.health = 100;
      r.memory.actor.region = 0;
      r.memory.actor.position = {int((n + i) % 32), int((n / 3 + i) % 32),
                                 int((n + i) % 8)};
      r.planning.emplace();
      k.records[0].push_back(r);
    }
    m::buildOwnerMemoryGroups(groups, k, 0, 100, true, w.dimensions, w.relation,
                              {[](int, int, int &middle) {
                                 middle = 0;
                                 return true;
                               },
                               [](auto, auto) { return true; }});
    // Allied memory rejection can leave no candidate; use the same preserved
    // pre-alliance group to exercise live relation changes.
    if (groups.nodes[0].children.empty()) {
      auto rel = w.relation;
      rel[0][1] = 0;
      groups = {};
      m::buildOwnerMemoryGroups(groups, k, 0, 100, true, w.dimensions, rel,
                                {[](int, int, int &v) {
                                   v = 0;
                                   return true;
                                 },
                                 [](auto, auto) { return true; }});
    }
    m::OwnerGroupTask task{groups, groups.nodes[0].children.front(),
                           int(n % 30)};
    if (n % 5 == 0)
      k.records[0].clear();
    m::NavigationRegion region;
    region.dangerBase = n % 13;
    region.danger[0][0] = n % 17;
    region.deductedDanger[0][0] = n % 7;
    const auto raw = m::regionDanger(w.creatures[0], region, {}),
               net = m::regionDanger(w.creatures[0], region, {}, true);
    const int percent = int(n % 5) * 50;
    std::array<int, 2> threat{{1000, int(n % 10000)}};
    int budget = 123, other = budget;
    auto oldTask = task;
    const auto out = m::executeOwnerGroupTask(task, k, 100, 0, 0, w, raw, net,
                                              percent, threat, budget);
    std::vector<unsigned> result{unsigned(out.continuing)};
    if (out.stop) {
      result.push_back(3);
      ++stops;
    } else if (out.kind == m::OwnerHostileOrderKind::target) {
      result.insert(result.end(), {1, unsigned(out.target)});
      ++targets;
    } else if (out.kind == m::OwnerHostileOrderKind::move) {
      result.insert(result.end(),
                    {2, unsigned(out.position.x), unsigned(out.position.y),
                     unsigned(out.position.z)});
      ++moves;
    }
    completed += !out.continuing;
    blocked += out.continuing && out.kind == m::OwnerHostileOrderKind::none;
#ifdef MNM_BRAIN_REFERENCE
    const auto expected =
        oldGroup(oldTask, k, 100, w, region, percent, threat, other);
    need(result == expected, ("group task case " + std::to_string(n)).c_str());
    record(unsigned(budget), unsigned(other), "group task budget");
    for (unsigned i = 0; i < task.groups.nodes.size(); ++i) {
      auto &a = task.groups.nodes[i], &b = oldTask.groups.nodes[i];
      record(a.position.x, b.position.x, "group center x");
      record(a.position.y, b.position.y, "group center y");
      record(a.position.z, b.position.z, "group center z");
      record(a.activity, b.activity, "group activity");
    }
#else
    (void)other;
    (void)oldTask;
    record(unsigned(budget), unsigned(budget), "group task budget");
    for (auto a : task.groups.nodes) {
      record(a.position.x, a.position.x, "group center x");
      record(a.position.y, a.position.y, "group center y");
      record(a.position.z, a.position.z, "group center z");
      record(a.activity, a.activity, "group activity");
    }
#endif
    for (auto v : result)
      record(v, v, "group task order");
  }
  need(targets && moves && stops && completed && blocked,
       "group task witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "group task text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":4096,\"targets\":" << targets
            << ",\"moves\":" << moves << ",\"stops\":" << stops
            << ",\"completed\":" << completed << ",\"blocked\":" << blocked
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
