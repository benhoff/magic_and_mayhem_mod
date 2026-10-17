#define main unused_task6_composed_main
#include "owner-task6-loop-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_task_scheduler.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
std::array<unsigned, 7> scheduledVtable;
unsigned executionCalls = 0;
int executedBudget = 0;
unsigned suppliedCredit = 34;
unsigned __attribute__((thiscall)) scheduledAccount(void *, unsigned, float,
                                                    float, float) {
  return suppliedCredit;
}
void __attribute__((thiscall)) noMaintenance(void *) {}
void __attribute__((thiscall)) suppliedSightings(void *, unsigned) {}
void __attribute__((thiscall)) suppliedSourceQueue(void *, void *) {}
void *__attribute__((thiscall)) scheduledClone(void *) {
  rawTasks.push_back(std::make_unique<RawTask>(scheduledVtable.data()));
  return rawTasks.back()->data.data();
}
unsigned __attribute__((thiscall)) scheduledExecute(void *task, void *knowledge,
                                                    int tick, int *budget) {
  ++executionCalls;
  using Execute =
      unsigned(__attribute__((thiscall)) *)(void *, void *, int, int *);
  unsigned out =
      reinterpret_cast<Execute>(0x432900)(task, knowledge, tick, budget);
  executedBudget = budget[2];
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
  segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "scheduler FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&guardedAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&loopRelease));
  redirect(0x416d80, reinterpret_cast<void *>(&scheduledAccount));
  redirect(0x418050, reinterpret_cast<void *>(&teardown));
  redirect(0x476e30, reinterpret_cast<void *>(&training));
  redirect(0x50af10, reinterpret_cast<void *>(&targetCommand));
  redirect(0x50fd50, reinterpret_cast<void *>(&moveCommand));
  redirect(0x50fef0, reinterpret_cast<void *>(&stopCommand));
  for (unsigned a : {0x417580u, 0x428e40u, 0x428f80u})
    redirect(a, reinterpret_cast<void *>(&noMaintenance));
  redirect(0x4181d0, reinterpret_cast<void *>(&suppliedSightings));
  redirect(0x416fe0, reinterpret_cast<void *>(&suppliedSourceQueue));
  scheduledVtable = {
      0x431be0, address(reinterpret_cast<void *>(&scheduledClone)),
      0x431ce0, 0x4261b0,
      0x4328d0, address(reinterpret_cast<void *>(&scheduledExecute)),
      0x432fb0};
  taskVtable = {0x431be0, address(reinterpret_cast<void *>(&scheduledClone)),
                0x431ce0, 0x4261b0,
                0x4328d0, address(reinterpret_cast<void *>(&scheduledExecute))};
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "scheduler RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, executionSlices = 0, expiredSkips = 0, completed = 0,
           retired = 0, pending = 0, sameCall = 0, disabled = 0,
           progressRetired = 0;
  unsigned refusals = 0;
  auto refuses = [&](auto operation) {
    bool refused = false;
    try {
      operation();
    } catch (const std::invalid_argument &) {
      refused = true;
    }
    need(refused, "expected native scheduler admission refusal");
    record(refused, refused, "native scheduler refusal");
    ++refusals;
  };
  m::OwnerTaskScheduler empty;
  m::OwnerTaskSchedulerServices valid{[] {}, [](int &) {},
                                      [] { return m::OwnerScheduledTask{}; },
                                      [](int &) { return false; }};
  refuses([&] { m::advanceOwnerTaskScheduler(empty, -1, 4, valid); });
  refuses([&] { m::advanceOwnerTaskScheduler(empty, 0, -1, valid); });
  refuses([&] { m::advanceOwnerTaskScheduler(empty, 0, 1000001, valid); });
  refuses([&] { m::advanceOwnerTaskScheduler(empty, 0, 4, {}); });
  refuses([&] {
    auto bad = empty;
    bad.pendingExecution = 1;
    m::advanceOwnerTaskScheduler(bad, 0, 4, valid);
  });
  refuses([&] {
    auto bad = empty;
    bad.executionComplete = true;
    m::advanceOwnerTaskScheduler(bad, 0, 4, valid);
  });
  refuses([&] { m::retireOwnerTask({}, false, -1, {}); });
  for (unsigned n = 0; n < 64; ++n) {
    const auto brain = model(n);
    std::vector<m::NavigationRegion> graph(8);
    for (unsigned i = 0; i < 8; ++i) {
      auto &r = graph[i];
      r.position = {int(i * 2), 2, 2};
      r.parameter = 1;
      if (i + 1 < 8) {
        r.connections[0].destination = int(i + 1);
        r.connections[0].cost = 8;
        r.connections[0].clearance.fill(8);
      }
    }
    m::OwnerKnowledge knowledge;
    for (unsigned i = 0; i < 2; ++i) {
      m::OwnerMemory r;
      r.memory.key = 5 + int(i * 3);
      r.memory.observedTick = r.markedTick = 100;
      r.memory.actor.slot = 1;
      r.memory.actor.owner = 1;
      r.memory.actor.health = 100;
      r.memory.actor.region = 6 + int(i);
      r.memory.actor.position = graph[6 + i].position;
      r.planning.emplace();
      r.planning->position = r.memory.actor.position;
      r.planning->speed = 24;
      r.planning->physical = true;
      knowledge.records[0].push_back(r);
    }
    NativeLoop native(n * 137 + 17);
    native.selection.weighted = n & 2;
    m::OwnerTaskScheduler scheduler;
    m::AutonomousWorld live;
    live.dimensions = {32, 32, 8};
    live.relation[0][0] = 1;
    live.creatures.resize(2);
    for (unsigned i = 0; i < 2; ++i) {
      auto &a = live.creatures[i].ai;
      a.slot = int(i);
      a.kind = 1;
      a.owner = int(i);
      a.type = 17 + int(i);
      a.active = true;
      a.health = 100;
      a.physical = 1;
      a.position = {i ? 14 : 2, 2, 2};
    }
#ifdef MNM_BRAIN_REFERENCE
    using RCtor = void *(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<RCtor>(0x53a660)(reinterpret_cast<void *>(0x6eaf08), 512);
    RawModel rawModel(brain);
    OriginalLoop old(graph, knowledge, native.random,
                     native.selection.weighted);
    put(rawTasks.back()->data.data(), 0, address(scheduledVtable.data()));
    std::array<unsigned, 1> sourceList{{address(old.world.source.data())}};
    put(old.brain.data(), 0x11ddd, bits(native.selection.value));
    put(old.brain.data(), 0x1e, address(sourceList.data()));
    put(old.brain.data(), 0x22, address(sourceList.data() + 1));
    put(old.brain.data(), 0x26, address(sourceList.data() + 1));
    // Same enabled ordinary direct-root source as native retirement policy.
    old.world.source[4] = 1;
    global(0x6e1f54, 0);
    global(0x6e1f58, 0);
#endif
    int tick = 100;
    bool jumped = false;
    unsigned completions = 0;
    for (unsigned frame = 0; frame < 220; ++frame, ++tick) {
      if (frame == 70 && native.previous) {
        m::addOwnerTaskDamage(nullptr, &*native.previous, 1, 1000000);
#ifdef MNM_BRAIN_REFERENCE
        suppliedCredit = 1000000;
        put(old.world.pool.data(), 0x660, 1);
        put(old.world.pool.data(), 0x678, address(old.world.source.data()));
        using Damage =
            unsigned(__attribute__((thiscall)) *)(void *, unsigned, void *);
        need((reinterpret_cast<Damage>(0x514990)(old.world.pool.data(), 1,
                                                 nullptr) &
              255) == 1,
             "scheduled nonlethal task credit");
#endif
        tick = native.previous->started +
               3 * (native.previous->expires - native.previous->started) / 4;
      }
      if (frame == 100 && native.previous) {
        tick = native.previous->expires;
        jumped = true;
      }
      const int total = frame < 12        ? int(frame % 4)
                        : frame % 11 == 0 ? 0
                                          : int(4 + (frame + n) % 7);
      const bool paused = frame % 37 == 0;
      const bool lost = frame % 13 == 0;
      const int raw = frame > 150 && frame % 17 == 0 ? 50 : 0;
      for (auto &r : knowledge.records[0])
        r.markedTick = tick - int(lost);
      std::vector<unsigned> order;
      const unsigned before = native.serial;
      const bool hadExpired =
          native.previous && tick >= native.previous->expires;
      m::OwnerTaskSchedulerServices services;
      services.cleanup = [&] {
        if (native.previous &&
            m::retireOwnerTask(*native.previous, scheduler.executionComplete,
                               tick, {})) {
          progressRetired +=
              !scheduler.executionComplete && tick < native.previous->expires;
          native.active.reset();
          native.previous.reset();
          scheduler.executionComplete = false;
          scheduler.pendingExecution = 0;
          ++retired;
        }
      };
      services.plan = [&](int &b) {
        if (native.advance(knowledge, graph, brain, tick, b, true)) {
          scheduler.executionComplete = false;
          scheduler.pendingExecution = 0;
        }
      };
      services.active = [&] {
        return m::OwnerScheduledTask{
            native.active ? native.serial : 0,
            native.previous ? native.previous->expires : 0, true};
      };
      services.execute = [&](int &b) {
        auto out = m::executeOwnerGroupTask(*native.active, knowledge, tick, 0,
                                            0, live, raw, raw, 0, {{0, 0}}, b);
        if (out.stop)
          order = {3};
        else if (out.kind == m::OwnerHostileOrderKind::target)
          order = {1, unsigned(out.target)};
        else if (out.kind == m::OwnerHostileOrderKind::move)
          order = {2, unsigned(out.position.x), unsigned(out.position.y),
                   unsigned(out.position.z)};
        return out.continuing;
      };
      const auto result = m::advanceOwnerTaskScheduler(scheduler, tick, total,
                                                       services, paused);
      executionSlices += result.executed;
      pending += scheduler.planningPending;
      expiredSkips += hadExpired && !result.executed;
      sameCall += native.serial > before && result.executed;
      disabled += paused;
      if (result.executed && scheduler.executionComplete) {
        ++completed;
        ++completions;
      }
#ifdef MNM_BRAIN_REFERENCE
      put(old.brain.data(), 0x14, tick - 1);
      for (auto &row : old.memories)
        put(row.data(), 20, tick - int(lost));
      old.world.regions[644] = unsigned(raw);
      global(0x6e1f54, paused);
      commands.clear();
      executionCalls = 0;
      executedBudget = total - total / 2;
      using Driver = void(__attribute__((thiscall)) *)(void *, int);
      reinterpret_cast<Driver>(0x415c40)(old.brain.data(), total);
      need(order == commands,
           ("scheduled command sequence frame=" + std::to_string(frame))
               .c_str());
      record(unsigned(tick), get(old.brain.data(), 0x14),
             "owner clock including disabled call");
      record(result.executed, executionCalls, "scheduled execution calls");
      record(unsigned(result.executionBudget), unsigned(executedBudget),
             "execution budget");
      record(scheduler.planningPending, get(old.brain.data(), 0x11dab) == 4,
             "pending root phase");
      record(bool(scheduler.pendingExecution),
             get(old.brain.data(), 0x11dcf) != 0, "pending execution queue");
      record(scheduler.executionComplete,
             old.active()
                 ? *(reinterpret_cast<unsigned char *>(old.active()) + 0x14)
                 : 0,
             "execution result flag14");
      try {
        old.verify(native, 0, 0);
      } catch (const std::exception &e) {
        throw std::runtime_error(std::string(e.what()) +
                                 " frame=" + std::to_string(frame) +
                                 " episode=" + std::to_string(n));
      }
#else
      record(unsigned(tick), unsigned(tick),
             "owner clock including disabled call");
      record(result.executed, result.executed, "scheduled execution calls");
      record(unsigned(result.executionBudget), unsigned(result.executionBudget),
             "execution budget");
      record(scheduler.planningPending, scheduler.planningPending,
             "pending root phase");
      record(bool(scheduler.pendingExecution), bool(scheduler.pendingExecution),
             "pending execution queue");
      record(scheduler.executionComplete, scheduler.executionComplete,
             "execution result flag14");
      // Same installed-state record order as OriginalLoop::verify.
      record(0, 0, "loop budget");
      record(native.selection.reset, native.selection.reset, "sweep");
      record(bits(native.selection.value), bits(native.selection.value),
             "value");
      record(native.random.left, native.random.left, "RNG");
      record(native.random.right, native.random.right, "RNG");
      for (unsigned v : native.random.words)
        record(v, v, "RNG word");
      record(bool(native.active), bool(native.active), "active");
      record(native.serial, native.serial, "serial");
      if (native.active) {
        record(unsigned(native.activeFeatures.size()),
               unsigned(native.activeFeatures.size()), "features");
        for (int v : native.activeFeatures)
          record(unsigned(v), unsigned(v), "feature");
        const auto &p = *native.previous;
        for (unsigned v :
             {unsigned(p.started), unsigned(p.expires), bits(p.value),
              bits(p.baseline), unsigned(p.successes), unsigned(p.attempts),
              unsigned(p.completed),
              unsigned(
                  native.active->groups.nodes[native.active->root].position.x),
              unsigned(native.active->groups.nodes[native.active->root].region),
              unsigned(native.active->risk)})
          record(v, v, "installed");
      }
#endif
      for (unsigned v : order)
        record(v, v, "order");
      ++slices;
    }
    need(jumped && completions, "expiry/completion witnesses");
#ifdef MNM_BRAIN_REFERENCE
    guardAllocations();
    rawTasks.clear();
#endif
  }
  need(retired && completed && pending && sameCall && expiredSkips &&
           progressRetired,
       "caller witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "scheduler target text changed");
#endif
  std::cout << "{\"success\":true,\"cases\":64,\"checks\":" << checks
            << ",\"slices\":" << slices
            << ",\"execution_slices\":" << executionSlices
            << ",\"completed\":" << completed << ",\"retired\":" << retired
            << ",\"progress_retired\":" << progressRetired
            << ",\"pending\":" << pending
            << ",\"same_call_execution\":" << sameCall
            << ",\"expired_skips\":" << expiredSkips
            << ",\"disabled\":" << disabled << ",\"refusals\":" << refusals
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
