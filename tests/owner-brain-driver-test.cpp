#define MNM_SOURCE_DECISION_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-source-decision-test.cpp"
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/owner_brain_driver.hpp"
namespace {
std::vector<unsigned> calls, executed;
#ifdef MNM_BRAIN_REFERENCE
void __attribute__((thiscall)) driverNoop(void *) {}
unsigned __attribute__((thiscall)) driverBody(void *task, void *knowledge,
                                              int tick, int *budget) {
  executed.push_back(get(task, 0x1f));
  return reinterpret_cast<unsigned(__attribute__((thiscall)) *)(
      void *, void *, int, int *)>(0x419900)(task, knowledge, tick, budget);
}
void __attribute__((thiscall)) driverRegister(void *, void *) {}
void __attribute__((thiscall)) driverTeardown(void *) {}
unsigned __attribute__((thiscall)) driverTraining(void *, unsigned, void *) {
  return UINT32_MAX;
}
void __attribute__((thiscall)) driverSight(void *, unsigned) {}
void __attribute__((thiscall)) driverPlan(void *, void *source, int *b) {
  const auto id = currentSources->id(address(source));
  calls.push_back(id);
  b[2] -= 1 + int(id % 3);
}
std::vector<unsigned> rawDeque(RawSources &r, unsigned off) {
  const auto n = get(r.brain.data(), off + 0x2c);
  const auto end = get(r.brain.data(), off + 28);
  std::vector<unsigned> out;
  for (unsigned i = 0; i < n; ++i)
    out.push_back(r.id(get(reinterpret_cast<void *>(end - n * 4 + i * 4), 0)));
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
  need(!syscall(SYS_set_thread_area, &segment), "driver FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  for (unsigned a : {0x4181d0u})
    redirect(a, reinterpret_cast<void *>(&driverSight));
  for (unsigned a : {0x417580u, 0x428e40u, 0x428f80u})
    redirect(a, reinterpret_cast<void *>(&driverNoop));
  for (unsigned a : {0x423590u, 0x421f70u})
    redirect(a, reinterpret_cast<void *>(&driverPlan));
  redirect(0x416fe0, reinterpret_cast<void *>(&driverRegister));
  redirect(0x418050, reinterpret_cast<void *>(&driverTeardown));
  redirect(0x476e30, reinterpret_cast<void *>(&driverTraining));
  redirect(0x597850, reinterpret_cast<void *>(&sourceAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&sourceRelease));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "driver RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, pending = 0;
  for (unsigned n = 0; n < 128; ++n) {
    m::AutonomousWorld world;
    auto v = fixture(0, world);
    v.sources.graph.resize(3);
    v.sources.information.resize(3);
    v.manager.nodes = {0, 1, 2};
    v.pendingRoots.clear();
    v.pending = {};
    for (unsigned i = 0; i < 3; ++i) {
      v.sources.graph[i].parent.reset();
      v.sources.graph[i].activity = 3;
      v.sources.graph[i].children.clear();
    }
    for (unsigned i = 0; i < 3; ++i) {
      if ((n >> i) & 1)
        v.pending.general.push_back(i);
      if ((n >> (i + 2)) & 1)
        v.pending.wizard.push_back(i);
      if ((n >> (i + 4)) & 1)
        v.pending.other.push_back(i);
    }
    m::OwnerBrainDriver state;
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(v);
    global(0x6e1f64, 0);
    global(0x6e1f54, 0);
    global(0x6e1f58, 0);
    raw.brain[0x19] = 1;
#endif
    for (unsigned frame = 0; frame < 32; ++frame) {
      int total = int((frame + n) % 19);
      bool disabled = frame % 17 == 0;
      std::vector<unsigned> expected;
      m::OwnerBrainDriverServices svc{[] {},
                                      [](bool) {},
                                      [&] { return v.manager.nodes; },
                                      [](unsigned) { return false; },
                                      [&](unsigned id, bool, int &b) {
                                        expected.push_back(id);
                                        b -= 1 + int(id % 3);
                                      },
                                      [] { return std::vector<unsigned>{}; },
                                      [](unsigned, int &) {
                                        throw std::runtime_error(
                                            "unexpected execute");
                                      }};
      auto out = m::advanceOwnerBrainDriver(state, v, total, svc, disabled);
      (void)out;
#ifdef MNM_BRAIN_REFERENCE
      calls.clear();
      global(0x6e1f54, disabled);
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *, int)>(
          0x415c40)(raw.brain.data(), total);
      need(expected == calls, "driver planned source order");
      record(state.clock, get(raw.brain.data(), 0x14), "driver clock");
      record(state.phase, get(raw.brain.data(), 0x11dab), "driver phase");
      record(state.current.value_or(UINT32_MAX),
             raw.id(get(raw.brain.data(), 0x11daf)), "driver current");
      need(v.pendingRoots == raw.list(get(raw.brain.data(), 0x11db7),
                                      get(raw.brain.data(), 0x11dbb)),
           "driver remaining roots");
      for (unsigned i = 0; i < 3; ++i) {
        const auto got = rawDeque(raw, 0x2a + i * 0x30);
        const auto &q = i == 0   ? v.pending.general
                        : i == 1 ? v.pending.wizard
                                 : v.pending.other;
        need(std::vector<unsigned>(q.begin(), q.end()) == got,
             "driver pending specialized queue");
      }
#else
      for (unsigned i : {unsigned(state.clock), state.phase,
                         state.current.value_or(UINT32_MAX)})
        record(i, i, "native driver");
#endif
      for (auto id : expected)
        record(id, id, "driver trace");
      ++slices;
      pending += state.phase != 0;
    }
  }
  for (unsigned n = 0; n < 32; ++n) {
    m::AutonomousWorld world;
    auto v = fixture(0, world);
    v.sources.graph.resize(3);
    v.sources.information.resize(3);
    v.manager.nodes = {0, 1, 2};
    v.pendingRoots.clear();
    v.pending = {};
    for (unsigned i = 0; i < 3; ++i) {
      v.sources.graph[i].parent.reset();
      v.sources.graph[i].activity = 3;
      v.sources.graph[i].children.clear();
      v.pending.general.push_back(i);
      m::bindOwnerSourceTask(v, i, i);
    }
    m::OwnerBrainDriver state;
    std::array<bool, 3> alive{{true, true, true}};
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(v);
    raw.extras.resize(64, Bytes(64));
    global(0x6e1f64, 0);
    global(0x6e1f54, 0);
    global(0x6e1f58, 0);
    global(0x6def58, address(raw.actors.data()));
    global(0x6def5c, 3);
    global(0x6df180, 3);
    raw.brain[0x19] = 1;
    // Installation owns the original task map; fixture source bindings are
    // reset before installing these genuine kind0 task objects.
    std::array<unsigned, 7> table{
        {0x415640, 0x419e40, 0x415670, 0x4181c0, 0x4156a0,
         address(reinterpret_cast<void *>(&driverBody)), 0x419910}};
    for (unsigned i = 0; i < 3; ++i) {
      put(raw.nodes[i].data(), 5, 0);
      put(raw.actors.data() + i * 0xe4b, 0xe4, 100);
      auto *task = sourceAllocate(0x14b);
      put(task, 0, address(table.data()));
      put(task, 4, 0);
      put(task, 0x23, UINT32_MAX);
      std::array<unsigned, 1> list{{address(task)}};
      put(raw.brain.data(), 0x11fe5, address(list.data()));
      put(raw.brain.data(), 0x11ddd, bits(-1.f));
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *, void *)>(
          0x4289e0)(raw.brain.data(), raw.nodes[i].data());
    }
#endif
    for (unsigned frame = 0; frame < 64; ++frame) {
      int total = int((n + frame) % 19);
      std::vector<unsigned> expected, expectedBodies;
      auto cleanup = [&] {
        for (unsigned i = 0; i < 3; ++i)
          if (alive[i] && state.clock >= 8) {
            alive[i] = false;
            m::unbindOwnerSourceTask(v, i);
            auto &q = state.execution;
            q.erase(std::remove(q.begin(), q.end(), i), q.end());
          }
      };
      m::OwnerBrainDriverServices svc{cleanup,
                                      [](bool) {},
                                      [&] { return v.manager.nodes; },
                                      [&](unsigned id) { return alive[id]; },
                                      [&](unsigned id, bool, int &b) {
                                        expected.push_back(id);
                                        b -= 1 + int(id % 3);
                                      },
                                      [&] {
                                        std::vector<unsigned> tasks;
                                        for (unsigned i = 0; i < 3; ++i)
                                          if (alive[i])
                                            tasks.push_back(i);
                                        return tasks;
                                      },
                                      [&](unsigned id, int &) {
                                        if (alive[id] && state.clock < 8)
                                          expectedBodies.push_back(id);
                                      }};
      auto out = m::advanceOwnerBrainDriver(state, v, total, svc);
      (void)out;
#ifdef MNM_BRAIN_REFERENCE
      calls.clear();
      executed.clear();
      global(0x6e1f54, 0);
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *, int)>(
          0x415c40)(raw.brain.data(), total);
      need(expected == calls, "task-map driver planning order");
      need(expectedBodies == executed, "task-map driver execution order");
      record(state.clock, get(raw.brain.data(), 0x14), "task-map clock");
      record(state.phase, get(raw.brain.data(), 0x11dab), "task-map phase");
      record(state.current.value_or(UINT32_MAX),
             raw.id(get(raw.brain.data(), 0x11daf)), "task-map current");
      for (unsigned i = 0; i < 3; ++i)
        record(alive[i], get(raw.nodes[i].data(), 5) != 0,
               "fallback expiry retirement");
#else
      for (unsigned x : {unsigned(state.clock), state.phase,
                         state.current.value_or(UINT32_MAX)})
        record(x, x, "task-map state");
      for (bool x : alive)
        record(x, x, "native fallback expiry");
#endif
      for (auto id : expectedBodies)
        record(id, id, "task-map body trace");
      ++slices;
      pending += state.phase != 0;
    }
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "driver unchanged text");
#endif
  need(pending > 0, "driver pending witness");
  std::cout << "{\"success\":true,\"cases\":160,\"slices\":" << slices
            << ",\"checks\":" << checks << ",\"pending\":" << pending
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
