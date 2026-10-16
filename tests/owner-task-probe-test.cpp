#pragma push_macro("main")
#undef main
#define main unused_owner_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/owner_task_probe.hpp"
#include <functional>
namespace {
#ifdef MNM_BRAIN_REFERENCE
unsigned get(const void *p, unsigned off = 0) {
  unsigned v;
  std::memcpy(&v, static_cast<const char *>(p) + off, 4);
  return v;
}
std::vector<int> captured;
void __attribute__((thiscall)) inference(void *, unsigned key, void *v, int n,
                                         float *out) {
  need(key == 6 && n == 3, "probe inference ABI");
  auto begin = reinterpret_cast<int *>(get(v, 4)),
       end = reinterpret_cast<int *>(get(v, 8));
  captured.assign(begin, end);
  out[0] = 0;
  out[1] = 0;
  out[2] = 1;
}
struct OriginalProbe {
  Bytes pool = Bytes(125 * 0xe4b), configs = Bytes(2 * 0x5c9),
        source = Bytes(0x8d), enemy = Bytes(0x8d), root = Bytes(0x8d),
        task = Bytes(0x1b4), regions;
  std::array<unsigned, 1> children{};
  std::array<int, 72> features{};
  OriginalProbe(const std::vector<m::NavigationRegion> &g, int from, int to)
      : regions(g.size() * 660) {
    for (unsigned va : {0x5faf3cu, 0x5faf40u, 0x5faf44u, 0x5faf48u})
      global(va, 0);
    using Ctor = void *(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Ctor>(0x41b010)(task.data() + 0xec);
    for (unsigned i = 0; i < 2; ++i) {
      auto *a = pool.data() + i * 0xe4b, *cfg = configs.data() + i * 0x5c9;
      put(a, 0, i);
      put(a, 4, 1);
      put(a, 8, 2 + i * 4);
      put(a, 12, 2);
      put(a, 16, 2);
      put(a, 0xe4, 100);
      put(a, 0xf8, 1);
      put(a, 0x174, i);
      put(a, 0xa8, 17 + i);
      put(a, 0xac, address(cfg));
      put(a, 0xde7, i ? to : from);
      put(cfg, 0, 1);
      put(cfg, 8, 1);
      put(cfg, 12, 1);
      put(cfg, 0x44, 3);
      put(cfg, 0x589, 24);
      global(0x6a5f80 + (17 + i) * 0x5c9, 1);
      put(pool.data(), 0x936, 100);
    }
    global(0x6def58, address(pool.data()));
    global(0x6def5c, 125);
    global(0x6df180, 125);
    global(0x643a80, 1);
    global(0x643a80 + 12, 0);
    global(0x6c5494, 32);
    global(0x6c5498, 32);
    global(0x6c549c, 8);
    global(0x5e1780, 8);
    global(0x6c5c84, 0);
    global(0x5d854c, 3);
    global(0x6e1f64, 0);
    global(0x6eaf08, address(regions.data()));
    global(0x6eaf0c, g.size());
    for (unsigned i = 0; i < g.size(); ++i) {
      auto *r = regions.data() + 660 * i;
      put(r, 4, g[i].position.x);
      put(r, 8, g[i].position.y);
      put(r, 12, g[i].position.z);
      put(r, 524, g[i].parameter);
      for (unsigned j = 0; j < 18; ++j) {
        auto *e = r + 16 + 28 * j;
        put(e, 0, g[i].connections[j].cost);
        put(e, 4, g[i].connections[j].destination);
        std::memcpy(e + 8, g[i].connections[j].clearance.data(), 20);
      }
      r[644] = g[i].dangerBase;
    }
    for (auto *p : {source.data(), enemy.data(), root.data()})
      for (unsigned j = 0; j < 8; ++j)
        p[0x41 + j] = 1;
    put(source.data(), 0x35, address(pool.data()));
    put(source.data(), 0x89, 3);
    put(enemy.data(), 0x35, address(pool.data() + 0xe4b));
    put(enemy.data(), 0x89, 3);
    put(enemy.data(), 0x19, to);
    put(enemy.data(), 0x39, address(root.data()));
    children[0] = address(enemy.data());
    put(root.data(), 0x29, address(children.data()));
    put(root.data(), 0x2d, address(children.data() + 1));
    put(root.data(), 0x31, address(children.data() + 1));
    root[0x41] = 0;
    put(root.data(), 0x49, address(children.data() + 1));
    put(task.data(), 4, 6);
    put(task.data(), 8, address(source.data()));
    put(task.data(), 0x16, 122);
    put(task.data(), 0x149, 1);
    put(task.data(), 0x14a, address(root.data()));
    put(task.data(), 0x18c, address(enemy.data()));
    put(task.data(), 0x163, address(features.data()));
    put(task.data(), 0x167, address(features.data()));
    put(task.data(), 0x16b, address(features.data() + 72));
    for (unsigned i = 0; i < 3; ++i)
      global(0x6c48fc + 6 * 12 + i * 4, bits(i == 2 ? 1.f : 0.f));
    captured.clear();
  }
  ~OriginalProbe() {
    while (!allocated.empty())
      release(allocated.begin()->first);
  }
  int index(unsigned ptr) {
    if (!ptr)
      return -1;
    need(ptr >= address(regions.data()) &&
             (ptr - address(regions.data())) % 660 == 0,
         "original probe region pointer");
    return int((ptr - address(regions.data())) / 660);
  }
  m::OwnerTaskProbeResult advance(int &budget) {
    int raw[3] = {0, 0, budget};
    float value = 0;
    using Next =
        unsigned(__attribute__((thiscall)) *)(void *, float *, int *, int);
    const auto yes =
        reinterpret_cast<Next>(0x431ce0)(task.data(), &value, raw, 100) & 255;
    budget = raw[2];
    return yes                       ? m::OwnerTaskProbeResult::reached
           : get(task.data(), 0x18c) ? m::OwnerTaskProbeResult::pending
                                     : m::OwnerTaskProbeResult::failed;
  }
  std::vector<int> path() {
    std::vector<int> v;
    for (unsigned p = get(task.data(), 0x135); p != get(task.data(), 0x139);
         p += 32)
      v.push_back(index(get(reinterpret_cast<void *>(p))));
    return v;
  }
  std::vector<unsigned> records() {
    std::vector<unsigned> v;
    auto nil = *reinterpret_cast<unsigned *>(0x5faf44);
    std::function<void(unsigned)> walk = [&](unsigned n) {
      if (n == nil)
        return;
      auto *p = reinterpret_cast<void *>(n);
      walk(get(p));
      v.insert(v.end(), {unsigned(index(get(p, 12))), get(p, 16),
                         unsigned(index(get(p, 20))), get(p, 44), get(p, 48)});
      walk(get(p, 8));
    };
    walk(get(reinterpret_cast<void *>(get(task.data(), 0x115)), 4));
    return v;
  }
  std::vector<unsigned> queue() {
    std::vector<unsigned> v;
    auto nil = *reinterpret_cast<unsigned *>(0x5faf3c);
    std::function<void(unsigned)> walk = [&](unsigned n) {
      if (n == nil)
        return;
      auto *p = reinterpret_cast<void *>(n);
      walk(get(p));
      v.insert(v.end(), {get(p, 12), unsigned(index(get(p, 16)))});
      walk(get(p, 8));
    };
    walk(get(reinterpret_cast<void *>(get(task.data(), 0x105)), 4));
    return v;
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
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  redirect(0x4766b0, reinterpret_cast<void *>(&inference));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "probe RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, reached = 0, failed = 0, yields = 0;
  for (unsigned n = 0; n < 512; ++n) {
    std::vector<m::NavigationRegion> g(4 + n % 93);
    for (unsigned i = 0; i < g.size(); ++i) {
      auto &v = g[i];
      v.position = {int(i % 32), int(i / 32 * 7 % 32), 2 + int(i % 3)};
      v.parameter = 1;
      v.dangerBase = (i * 7 + n) % 20;
      for (unsigned j = 0; j < 3; ++j) {
        auto &e = v.connections[j];
        e.cost = 4 + int((i * 13 + j * 7 + n) % 60);
        e.destination = int((i + (j == 0   ? 1
                                  : j == 1 ? 3
                                           : g.size() - 1)) %
                            g.size());
        e.clearance.fill(8);
        if (n % 7 == 0 && i == g.size() / 2)
          e.cost = 0;
      }
    }
    const int from = 0, to = int(n % g.size());
    m::RegionalQuery q;
    q.movement = {1, 1, 0, 3, 0, 17, 0};
    m::RegionalServices lower;
    lower.danger = [&](int id) { return int(g.at(unsigned(id)).dangerBase); };
    m::OwnerTaskProbe state;
#ifdef MNM_BRAIN_REFERENCE
    OriginalProbe original(g, from, to);
#endif
    for (unsigned slice = 0; slice < 40; ++slice) {
      int budget = slice == 0 ? 0 : int(1 + (slice + n) % 5), other = budget;
      const auto result = m::advanceOwnerTaskProbe(state, g, from, to, q,
                                                   {32, 32, 8}, budget, lower);
      ++slices;
#ifdef MNM_BRAIN_REFERENCE
      const auto expected = original.advance(other);
      record(unsigned(result), unsigned(expected),
             ("probe result case " + std::to_string(n) + " slice " +
              std::to_string(slice))
                 .c_str());
      record(unsigned(budget), unsigned(other), "probe budget");
      record(unsigned(state.remaining), get(original.task.data(), 0x1b0),
             "probe remaining");
      record(state.search.dirty, original.task[0xec], "probe dirty");
      record(unsigned(state.search.best),
             unsigned(original.index(get(original.task.data(), 0xf5))),
             "probe best");
      record(unsigned(state.search.bestHeuristic),
             get(original.task.data(), 0xf1), "probe heuristic");
      need(state.path == original.path(), "probe path");
      auto records = state.search.records;
      std::sort(records.begin(), records.end(),
                [](auto a, auto b) { return a.node < b.node; });
      std::vector<unsigned> r;
      for (const auto &v : records) {
        auto p = mnm::reconstruction::neighbor_movement(v.movement);
        r.insert(r.end(),
                 {unsigned(v.node), unsigned(v.priority),
                  unsigned(v.predecessor), unsigned(p.accumulated_cost),
                  unsigned(p.unknown_18)});
      }
      need(r == original.records(),
           ("probe records case " + std::to_string(n)).c_str());
      auto queue = state.search.open;
      std::stable_sort(queue.begin(), queue.end(),
                       [](auto a, auto b) { return a.priority < b.priority; });
      r.clear();
      for (auto v : queue)
        r.insert(r.end(), {unsigned(v.priority), unsigned(v.node)});
      need(r == original.queue(), "probe queue");
#else
      (void)other;
      for (unsigned v :
           {unsigned(result), unsigned(budget), unsigned(state.remaining),
            unsigned(state.search.dirty), unsigned(state.search.best),
            unsigned(state.search.bestHeuristic)})
        record(v, v, "native probe");
#endif
      for (int p : state.path)
        record(unsigned(p), unsigned(p), "path digest");
      if (result == m::OwnerTaskProbeResult::pending) {
        ++yields;
        continue;
      }
      reached += result == m::OwnerTaskProbeResult::reached;
      failed += result == m::OwnerTaskProbeResult::failed;
      if (result == m::OwnerTaskProbeResult::reached) {
        auto last = std::find_if(state.search.records.begin(),
                                 state.search.records.end(),
                                 [&](const auto &v) { return v.node == to; });
        auto payload = mnm::reconstruction::neighbor_movement(last->movement);
        const auto travel =
            m::ownerTaskTravelTime(payload.accumulated_cost, 24);
        int risk = 0;
        for (auto id : state.path)
          risk = std::max(risk, lower.danger(id));
#ifdef MNM_BRAIN_REFERENCE
        record(unsigned(travel), get(original.task.data(), 0x19c),
               "probe travel");
        record(unsigned(payload.unknown_18), get(original.task.data(), 0x1a0),
               "probe cumulative danger");
        record(unsigned(lower.danger(to)), get(original.task.data(), 0x1a4),
               "probe destination danger");
        record(unsigned(risk), get(original.task.data(), 0x157),
               "probe max risk");
        need(captured.size() == 72 && captured[68] == travel &&
                 captured[69] == payload.unknown_18 &&
                 captured[70] == lower.danger(to),
             "produced suffix");
#else
        for (unsigned v : {unsigned(travel), unsigned(payload.unknown_18),
                           unsigned(lower.danger(to)), unsigned(risk)})
          record(v, v, "native summaries");
#endif
      }
      break;
    }
  }
  need(reached && failed && yields, "probe branch corpus");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "probe text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":512,\"slices\":" << slices
            << ",\"reached\":" << reached << ",\"failed\":" << failed
            << ",\"yields\":" << yields << ",\"digest\":\"" << digest
            << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
