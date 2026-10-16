#define main unused_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_hostile_task.hpp"
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
std::vector<unsigned> oldTask(m::OwnerHostileTask &task,
                              const m::OwnerKnowledge &knowledge, int tick,
                              const m::AutonomousWorld &world, int &budget) {
  Bytes region(660);
  global(0x6eaf08, address(region.data()));
  global(0x6eaf0c, 1);
  Bytes pool(125 * 0xe4b), configs(2 * 0x5c9), source(0x8d), target(0x8d),
      object(0x200), trees(8 * 48), head(0xe68), nil(0xe68), node(0xe68);
  for (unsigned i = 0; i < 2; ++i) {
    const auto &a = world.creatures[i];
    auto *raw = pool.data() + i * 0xe4b;
    auto *cfg = configs.data() + i * 0x5c9;
    put(raw, 0, i);
    put(raw, 4, a.ai.active);
    put(raw, 8, a.ai.position.x);
    put(raw, 12, a.ai.position.y);
    put(raw, 16, a.ai.position.z);
    put(raw, 0xe4, a.ai.health);
    put(raw, 0x174, a.ai.owner);
    put(raw, 0xa8, 17);
    put(raw, 0xac, address(cfg));
    put(raw, 0xf8, a.ai.physical);
    put(raw, 0x91e, 1000);
    put(raw, 0x64c, unsigned(-1));
    put(raw, 0x664, a.ai.engaged);
    put(raw, 0x668, a.ai.incoming);
    put(raw, 0x164, a.ai.readiness);
    put(raw, 0x932, a.ai.dirty932);
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
  for (unsigned i = 0; i < 8; ++i) {
    source[0x41 + i] = 1;
    target[0x41 + i] = 1;
  }
  put(source.data(), 0x35, address(pool.data()));
  put(source.data(), 0x89, 3);
  put(target.data(), 0x35, address(node.data() + 24));
  put(target.data(), 0x89, 3);
  put(target.data(), 9, task.memoryKey);
  put(target.data(), 13, task.position.x);
  put(target.data(), 17, task.position.y);
  put(target.data(), 21, task.position.z);
  put(target.data(), 25, 0);
  put(nil.data(), 0xe64, 1);
  global(0x5faf4c, address(nil.data()));
  put(trees.data(), 4, address(head.data()));
  const auto &records = knowledge.records[0];
  auto r = std::find_if(records.begin(), records.end(), [&](const auto &v) {
    return v.memory.key == task.memoryKey;
  });
  if (r != records.end()) {
    put(head.data(), 0, address(node.data()));
    put(head.data(), 4, address(node.data()));
    put(head.data(), 8, address(node.data()));
    put(node.data(), 0, address(nil.data()));
    put(node.data(), 4, address(head.data()));
    put(node.data(), 8, address(nil.data()));
    put(node.data(), 12, r->memory.key);
    put(node.data(), 20, r->markedTick);
    put(node.data(), 24, r->memory.actor.slot);
  } else {
    put(head.data(), 0, address(head.data()));
    put(head.data(), 4, address(nil.data()));
    put(head.data(), 8, address(head.data()));
  }
  put(object.data(), 4, 6);
  put(object.data(), 8, address(source.data()));
  put(object.data(), 0x27, 0);
  put(object.data(), 0x16f, address(target.data()));
  commands.clear();
  int rawBudget[3] = {0, 0, budget};
  using Execute =
      unsigned(__attribute__((thiscall)) *)(void *, void *, unsigned, int *);
  unsigned result = reinterpret_cast<Execute>(0x432900)(
      object.data(), trees.data(), unsigned(tick), rawBudget);
  budget = rawBudget[2];
  task.position = {int(*reinterpret_cast<unsigned *>(target.data() + 13)),
                   int(*reinterpret_cast<unsigned *>(target.data() + 17)),
                   int(*reinterpret_cast<unsigned *>(target.data() + 21))};
  auto out = commands;
  out.insert(out.begin(), result & 255);
  need(allocated.empty(), "hostile task temporary allocation");
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
  unsigned targetOrders = 0, moves = 0, completed = 0, blocked = 0;
  for (unsigned n = 0; n < 8192; ++n) {
    m::OwnerHostileTask task{5, {int(n % 32), int(n / 3 % 32), 2}};
    m::AutonomousWorld world;
    world.dimensions = {32, 32, 8};
    world.creatures.resize(2);
    for (unsigned i = 0; i < 2; ++i) {
      auto &a = world.creatures[i];
      a.ai.slot = i;
      a.ai.type = 17;
      a.ai.kind = 1;
      a.ai.owner = int(i);
      a.ai.active = true;
      a.ai.health = 100;
      a.ai.position = {int((n / 7 + i * 9) % 32), int((n / 11 + i * 13) % 32),
                       2};
      a.ai.physical = (n + i) % 2;
    }
    world.creatures[0].ai.engaged = n % 7 == 0;
    world.creatures[0].ai.incoming = n % 11 == 0;
    world.creatures[0].ai.readiness = n % 13 == 0;
    world.creatures[0].ai.dirty932 = n % 17 == 0;
    if (n % 19 == 0)
      world.creatures[1].ai.health = 0;
    world.relation[0][1] = n % 23 == 0;
    m::OwnerKnowledge knowledge;
    if (n % 5) {
      m::OwnerMemory r;
      r.memory.key = 5;
      r.memory.actor.slot = 1;
      r.markedTick = n % 3 ? 100 : 99;
      knowledge.records[0].push_back(r);
    }
    int budget = 123;
    auto originalTask = task;
    const auto out =
        m::executeOwnerHostileTask(task, knowledge, 100, 0, 0, world, budget);
    std::vector<unsigned> result{unsigned(out.continuing)};
    if (out.kind == m::OwnerHostileOrderKind::target) {
      result.insert(result.end(), {1, unsigned(out.target)});
      ++targetOrders;
    }
    if (out.kind == m::OwnerHostileOrderKind::move) {
      result.insert(result.end(),
                    {2, unsigned(out.position.x), unsigned(out.position.y),
                     unsigned(out.position.z)});
      ++moves;
    }
    completed += !out.continuing;
    blocked += out.continuing && out.kind == m::OwnerHostileOrderKind::none;
#ifdef MNM_BRAIN_REFERENCE
    int oldBudget = 123;
    const auto expected =
        oldTask(originalTask, knowledge, 100, world, oldBudget);
    record(unsigned(budget), unsigned(oldBudget), "task budget");
    record(unsigned(task.position.x), unsigned(originalTask.position.x),
           "task center x");
    record(unsigned(task.position.y), unsigned(originalTask.position.y),
           "task center y");
    record(unsigned(task.position.z), unsigned(originalTask.position.z),
           "task center z");
    need(result == expected,
         ("task handoff fixture " + std::to_string(n)).c_str());
#else
    (void)originalTask;
    for (unsigned v : {unsigned(budget), unsigned(task.position.x),
                       unsigned(task.position.y), unsigned(task.position.z)})
      record(v, v, "native task");
#endif
    for (unsigned v : result)
      record(v, v, "order");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "hostile task text guard");
#endif
  need(targetOrders && moves && completed && blocked,
       "hostile task branch witnesses");
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":8192,\"targets\":" << targetOrders
            << ",\"moves\":" << moves << ",\"completed\":" << completed
            << ",\"blocked\":" << blocked << ",\"digest\":\"" << digest
            << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
