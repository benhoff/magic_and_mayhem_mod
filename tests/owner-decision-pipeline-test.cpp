// Original-first capture: no native decision functions run in the reference main.
#define main unused_scheduler_fixture_main
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-task6-loop-test.cpp"
#include "../reconstruction/creature-ai/owner_task_scheduler.hpp"
#pragma GCC diagnostic pop
#undef main
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

namespace {
constexpr unsigned episodes = 32, frames = 220;
struct Transcript {
  Bytes bytes;
  std::size_t at = 0;
  bool capture;
  explicit Transcript(const char *path, bool write) : capture(write) {
    if (write) {
      std::ifstream exists(path);
      need(!exists, "refuse existing decision capture");
      bytes.assign({'M','N','M','D','P','0','0','1'});
    } else {
      std::ifstream f(path, std::ios::binary | std::ios::ate);
      need(bool(f) && f.tellg() >= 16 && f.tellg() <= 32 * 1024 * 1024,
           "decision transcript size");
      bytes.resize(std::size_t(f.tellg()));
      f.seekg(0);
      need(bool(f.read(reinterpret_cast<char *>(bytes.data()), bytes.size())),
           "decision transcript read");
      need(std::equal(bytes.begin(), bytes.begin() + 8, "MNMDP001"),
           "decision transcript version");
    }
    at = 8;
    check(episodes, "episode count");
    check(frames, "frame count");
  }
  unsigned word(unsigned value = 0) {
    if (capture) {
      for (unsigned i = 0; i < 4; ++i)
        bytes.push_back(static_cast<unsigned char>(value >> (8 * i)));
      at += 4;
      return value;
    }
    need(at <= bytes.size() && bytes.size() - at >= 4,
         "truncated decision transcript");
    unsigned out = 0;
    for (unsigned i = 0; i < 4; ++i)
      out |= unsigned(bytes[at++]) << (8 * i);
    return out;
  }
  void check(unsigned value, const char *field) {
    record(value, word(value), field);
  }
  void finish(const char *path) {
    need(at == bytes.size(), "decision transcript trailing data");
    if (capture) {
      std::ofstream f(path, std::ios::binary);
      need(bool(f.write(reinterpret_cast<const char *>(bytes.data()), bytes.size())),
           "decision transcript write");
    }
  }
};
}
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 3, "PE NEW_CAPTURE");
  const auto path = argv[2];
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "pipeline FS");
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
  scheduledVtable = {0x431be0, address(reinterpret_cast<void *>(&scheduledClone)),
                     0x431ce0, 0x4261b0, 0x4328d0,
                     address(reinterpret_cast<void *>(&scheduledExecute)), 0x432fb0};
  taskVtable = {scheduledVtable[0], scheduledVtable[1], scheduledVtable[2],
                scheduledVtable[3], scheduledVtable[4], scheduledVtable[5]};
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC), "pipeline RX");
  Transcript trace(path, true);
#else
  need(argc == 2, "CAPTURE");
  const auto path = argv[1];
  Transcript trace(path, false);
#endif
  unsigned installed = 0, executed = 0, pending = 0, completed = 0,
           target = 0, move = 0, stop = 0, expired = 0, pausedCount = 0;
  for (unsigned n = 0; n < episodes; ++n) {
    trace.check(n, "fixture id");
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
#ifdef MNM_BRAIN_REFERENCE
    using RCtor = void *(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<RCtor>(0x53a660)(reinterpret_cast<void *>(0x6eaf08), 512);
    RawModel rawModel(brain);
    OriginalLoop old(graph, knowledge, m::seedRandom(n * 137 + 17), n & 2);
    put(rawTasks.back()->data.data(), 0, address(scheduledVtable.data()));
    std::array<unsigned, 1> sourceList{{address(old.world.source.data())}};
    put(old.brain.data(), 0x11ddd, bits(-2.f));
    put(old.brain.data(), 0x1e, address(sourceList.data()));
    put(old.brain.data(), 0x22, address(sourceList.data() + 1));
    put(old.brain.data(), 0x26, address(sourceList.data() + 1));
    old.world.source[4] = 1;
    global(0x6e1f58, 0);
#else
    NativeLoop native(n * 137 + 17);
    native.selection.weighted = n & 2;
    m::OwnerTaskScheduler scheduler;
    m::AutonomousWorld live;
    live.dimensions = {32, 32, 8};
    live.relation[0][0] = 1;
    live.creatures.resize(2);
    for (unsigned i = 0; i < 2; ++i) {
      auto &a = live.creatures[i].ai;
      a.slot = int(i); a.kind = 1; a.owner = int(i); a.type = 17 + int(i);
      a.active = true; a.health = 100; a.physical = 1;
      a.position = {i ? 14 : 2, 2, 2};
    }
#endif
    int tick = 100;
    unsigned lastSerial = 0;
    for (unsigned frame = 0; frame < frames; ++frame, ++tick) {
      trace.check(frame, "frame ordinal");
#ifdef MNM_BRAIN_REFERENCE
      if (old.active() && frame == 70)
        tick = int(get(reinterpret_cast<void *>(old.active()), 0xc) +
                   3 * (get(reinterpret_cast<void *>(old.active()), 0x10) -
                        get(reinterpret_cast<void *>(old.active()), 0xc)) / 4);
      if (old.active() && frame == 100)
        tick = int(get(reinterpret_cast<void *>(old.active()), 0x10));
#endif
      tick = int(trace.word(unsigned(tick)));
      int total = int(trace.word(frame < 12 ? frame % 4 : frame % 11 == 0 ? 0 :
                                4 + (frame + n) % 7));
      const bool paused = trace.word(frame % 37 == 0);
      const bool lost = trace.word(frame % 13 == 0);
      const int raw = int(trace.word(frame > 150 && frame % 17 == 0 ? 50 : 0));
      const bool credit = trace.word(frame == 70);
      need(tick >= 100 && tick < 1000000 && total >= 0 && total <= 10 &&
               (raw == 0 || raw == 50), "decision stimulus domain");
#ifdef MNM_BRAIN_REFERENCE
      const bool hadExpired = old.active() && unsigned(tick) >= get(reinterpret_cast<void *>(old.active()), 0x10);
#else
      const bool hadExpired = native.previous && tick >= native.previous->expires;
#endif
      std::vector<unsigned> state, order;
      auto add = [&](unsigned v) { state.push_back(v); };
#ifdef MNM_BRAIN_REFERENCE
      if (credit && old.active()) {
        suppliedCredit = 1000000;
        put(old.world.pool.data(), 0x660, 1);
        put(old.world.pool.data(), 0x678, address(old.world.source.data()));
        using Damage = unsigned(__attribute__((thiscall)) *)(void *, unsigned, void *);
        need((reinterpret_cast<Damage>(0x514990)(old.world.pool.data(), 1, nullptr) & 255) == 1,
             "original task credit");
      }
      put(old.brain.data(), 0x14, tick - 1);
      for (auto &row : old.memories) put(row.data(), 20, tick - int(lost));
      old.world.regions[644] = unsigned(raw);
      global(0x6e1f54, paused);
      commands.clear(); executionCalls = 0;
      executedBudget = total - total / 2;
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *, int)>(0x415c40)(old.brain.data(), total);
      add(get(old.brain.data(), 0x14));
      add(get(old.brain.data(), 0x11dab));
      add(get(old.brain.data(), 0x11dcf) != 0);
      add(executionCalls); add(unsigned(executedBudget));
      add(old.brain[0x11dd3]); add(get(old.brain.data(), 0x11ddd));
      add(*reinterpret_cast<unsigned *>(0x6a29d8));
      add(*reinterpret_cast<unsigned *>(0x6a29dc));
      for (unsigned i = 0; i < 250; ++i) add(reinterpret_cast<unsigned *>(0x6a29e0)[i]);
      add(bool(old.active())); add(get(old.brain.data(), 0xca));
      if (old.active()) {
        auto *p = reinterpret_cast<unsigned char *>(old.active());
        add(p[0x14]); add(p[0x15]);
        for (unsigned o : {0xcu,0x10u,0x1bu,0x2bu,0x2fu,0x33u}) add(get(p,o));
        auto *node = reinterpret_cast<void *>(get(p,0x16f));
        add(get(node,13)); add(get(node,25)); add(get(p,0x173));
        const unsigned count = (get(p,0x183)-get(p,0x17f))/4;
        need(count == 72, "original saved feature dimension");
        add(count);
        auto *features = reinterpret_cast<unsigned *>(get(p,0x17f));
        for (unsigned i=0; i<count; ++i) add(features[i]);
      }
      order = commands;
      guardAllocations();
#else
      if (credit && native.previous) m::addOwnerTaskDamage(nullptr, &*native.previous, 1, 1000000);
      for (auto &r : knowledge.records[0]) r.markedTick = tick - int(lost);
      m::OwnerTaskSchedulerServices services;
      services.cleanup = [&] {
        if (native.previous && m::retireOwnerTask(*native.previous, scheduler.executionComplete, tick, {})) {
          native.active.reset(); native.previous.reset();
          scheduler.executionComplete = false; scheduler.pendingExecution = 0;
        }
      };
      services.plan = [&](int &b) {
        if (native.advance(knowledge, graph, brain, tick, b, true)) {
          scheduler.executionComplete = false; scheduler.pendingExecution = 0;
        }
      };
      services.active = [&] { return m::OwnerScheduledTask{native.active ? native.serial : 0,
                            native.previous ? native.previous->expires : 0, true}; };
      services.execute = [&](int &b) {
        auto out = m::executeOwnerGroupTask(*native.active, knowledge, tick, 0, 0,
                                           live, raw, raw, 0, {{0,0}}, b);
        if (out.stop) order = {3};
        else if (out.kind == m::OwnerHostileOrderKind::target) order = {1,unsigned(out.target)};
        else if (out.kind == m::OwnerHostileOrderKind::move)
          order = {2,unsigned(out.position.x),unsigned(out.position.y),unsigned(out.position.z)};
        return out.continuing;
      };
      const auto result = m::advanceOwnerTaskScheduler(scheduler, tick, total, services, paused);
      add(unsigned(tick)); add(scheduler.planningPending ? 4 : 0);
      add(bool(scheduler.pendingExecution)); add(result.executed); add(unsigned(result.executionBudget));
      add(native.selection.reset); add(bits(native.selection.value));
      add(native.random.left); add(native.random.right);
      for (auto v : native.random.words) add(v);
      add(bool(native.active)); add(native.serial);
      if (native.active) {
        const auto &p = *native.previous;
        add(scheduler.executionComplete); add(p.completed);
        for (auto v : {unsigned(p.started),unsigned(p.expires),bits(p.value),bits(p.baseline),
                       unsigned(p.successes),unsigned(p.attempts)}) add(v);
        const auto &node = native.active->groups.nodes[native.active->root];
        add(unsigned(node.position.x)); add(unsigned(node.region)); add(unsigned(native.active->risk));
        add(native.activeFeatures.size());
        for (int v : native.activeFeatures) add(unsigned(v));
      }
#endif
      try {
        trace.check(state.size(), "pipeline state extent");
        for (unsigned i = 0; i < state.size(); ++i)
          trace.check(state[i], ("pipeline state word " + std::to_string(i)).c_str());
        trace.check(order.size(), "command extent");
        for (auto v : order) trace.check(v, "command word");
      } catch (const std::exception &e) {
        throw std::runtime_error(std::string(e.what()) + " episode=" + std::to_string(n) +
                                 " frame=" + std::to_string(frame));
      }
      installed += state[260] - lastSerial;
      lastSerial = state[260];
      executed += state[3]; pending += state[1] != 0; pausedCount += paused;
      expired += hadExpired && !state[3];
      if (state[259]) {
        completed += state[261] != 0;
      }
      if (!order.empty()) { target += order[0] == 1; move += order[0] == 2; stop += order[0] == 3; }
    }
#ifdef MNM_BRAIN_REFERENCE
    rawTasks.clear();
#endif
  }
  need(installed && executed && pending && completed && target && move && stop && expired,
       ("composed pipeline witnesses installs="+std::to_string(installed)+" executed="+std::to_string(executed)+" pending="+std::to_string(pending)+" complete="+std::to_string(completed)+" target="+std::to_string(target)+" move="+std::to_string(move)+" stop="+std::to_string(stop)+" expired="+std::to_string(expired)).c_str());
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(), reinterpret_cast<unsigned char *>(0x401000)),
       "pipeline target text changed");
#endif
  trace.finish(path);
  std::cout << "{\"success\":true,\"cases\":" << episodes << ",\"slices\":" << episodes*frames
            << ",\"checks\":" << checks << ",\"installed\":" << installed
            << ",\"executed\":" << executed << ",\"pending\":" << pending
            << ",\"completed\":" << completed << ",\"target\":" << target
            << ",\"move\":" << move << ",\"stop\":" << stop << ",\"expired\":" << expired
            << ",\"disabled\":" << pausedCount << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
