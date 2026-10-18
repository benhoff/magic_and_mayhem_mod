#define main unused_source_probe_fixture_main
#include "owner-task-probe-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_group_features.hpp"
#include "../reconstruction/creature-ai/owner_task_lifecycle.hpp"
#include "../reconstruction/pathfinding/route_neighbors.hpp"
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
  need(!syscall(SYS_set_thread_area, &segment), "source probes FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  redirect(0x4766b0, reinterpret_cast<void *>(&inference));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "source probes RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, yields = 0, reached = 0, failed = 0;
  for (unsigned n = 0; n < 256; ++n) {
    std::vector<m::NavigationRegion> graph(8 + n % 49);
    for (unsigned i = 0; i < graph.size(); ++i) {
      auto &r = graph[i];
      r.position = {int(i % 16) * 2, int(i / 16) * 4, 2};
      r.parameter = 1;
      r.dangerBase = (n + i) % 7;
      for (unsigned j = 0; j < 2; ++j) {
        auto &e = r.connections[j];
        e.destination = int((i + 1 + j * 2) % graph.size());
        e.cost = 8 + int((n + i + j) % 7) * 4;
        e.clearance.fill(8);
        if (n % 11 == 0 && i == graph.size() / 2)
          e.cost = 0;
      }
    }
    const int to = int(graph.size()) - 1;
    const std::array<int, 2> origins{{0, int(n % graph.size())}};
    const std::array<int, 2> speeds{{24, n & 1 ? 48 : 24}};
    auto random = m::seedRandom(n + 17);
    m::OwnerTaskPathTotals totals;
    m::OwnerTaskProbe probe;
    unsigned member = 0;
    m::OwnerTaskProbeResult result = m::OwnerTaskProbeResult::pending;
    int duration = 0;
    std::vector<int> values;
#ifdef MNM_BRAIN_REFERENCE
    OriginalProbe raw(graph, origins[0], to);
    Bytes group(0x8d), leaf(0x8d);
    std::array<unsigned, 2> children{
        {address(raw.source.data()), address(leaf.data())}};
    std::memcpy(raw.pool.data() + 2 * 0xe4b, raw.pool.data(), 0xe4b);
    auto *a = raw.pool.data() + 2 * 0xe4b;
    put(a, 0, 2);
    put(a, 8, graph[origins[1]].position.x);
    put(a, 12, graph[origins[1]].position.y);
    put(a, 16, 2);
    put(a, 0xde7, origins[1]);
    put(a, 0x778, n & 1 ? 1 : 0);
    put(raw.pool.data(), 8, 0);
    put(raw.pool.data(), 12, 0);
    put(raw.pool.data() + 0xe4b, 8, graph[to].position.x);
    put(raw.pool.data() + 0xe4b, 12, graph[to].position.y);
    for (auto *p : {group.data(), leaf.data()})
      for (unsigned j = 0; j < 8; ++j)
        p[0x41 + j] = 1;
    put(group.data(), 0x29, address(children.data()));
    put(group.data(), 0x2d, address(children.data() + 2));
    put(group.data(), 0x31, address(children.data() + 2));
    put(raw.source.data(), 0x39, address(group.data()));
    put(leaf.data(), 0x39, address(group.data()));
    put(leaf.data(), 0x35, address(a));
    put(leaf.data(), 0x89, 3);
    put(raw.task.data(), 8, address(group.data()));
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
#endif
    for (unsigned tick = 0;
         result == m::OwnerTaskProbeResult::pending && tick < 100; ++tick) {
      int budget = 1 + int((n + tick) % 4);
      [[maybe_unused]] const int grant = budget;
      while (member < 2) {
        m::RegionalQuery query;
        query.movement = {1, 1, 0, 3, 0, 17, 0};
        m::RegionalServices services;
        services.danger = [&](int id) {
          return int(graph.at(unsigned(id)).dangerBase);
        };
        auto step =
            m::advanceOwnerTaskProbe(probe, graph, origins[member], to, query,
                                     {32, 32, 8}, budget, services);
        if (step == m::OwnerTaskProbeResult::pending)
          break;
        if (step == m::OwnerTaskProbeResult::failed) {
          result = step;
          break;
        }
        auto at = std::find_if(probe.search.records.begin(),
                               probe.search.records.end(),
                               [&](const auto &r) { return r.node == to; });
        need(at != probe.search.records.end(), "source probe goal record");
        auto path = mnm::reconstruction::neighbor_movement(at->movement);
        int risk = 0;
        for (auto id : probe.path)
          risk = std::max(risk, services.danger(id));
        m::addOwnerTaskPath(totals, path.accumulated_cost, speeds[member],
                            path.unknown_18, services.danger(to), risk);
        ++member;
        probe = {};
      }
      if (member == 2) {
        result = m::OwnerTaskProbeResult::reached;
        duration = m::ownerTask6Duration(random, totals.travel);
        std::vector<m::OwnerPlannerActor> friendly(2), enemy(1);
        for (unsigned i = 0; i < 2; ++i) {
          friendly[i].position = graph[origins[i]].position;
          friendly[i].physical = true;
          friendly[i].speed = speeds[i];
          friendly[i].danger = graph[origins[i]].dangerBase;
        }
        enemy[0].position = graph[to].position;
        enemy[0].physical = true;
        enemy[0].speed = 24;
        enemy[0].danger = graph[to].dangerBase;
        const auto prefix =
            m::hostileTaskGroupFeatures(friendly, enemy, {32, 32, 8});
        values.assign(prefix.begin(), prefix.end());
        values.insert(values.end(), {0, totals.travel, totals.auxiliary / 2,
                                     totals.danger / 2, 0});
      }
      ++slices;
      yields += result == m::OwnerTaskProbeResult::pending;
#ifdef MNM_BRAIN_REFERENCE
      int other = grant;
      const auto original = raw.advance(other);
      record(unsigned(budget), unsigned(other), "two-member probe budget");
      record(unsigned(result), unsigned(original), "two-member probe result");
      if (result == m::OwnerTaskProbeResult::reached) {
        need(values.size() == captured.size(), "member feature count");
        for (unsigned i = 0; i < values.size(); ++i)
          if (values[i] != captured[i])
            throw std::runtime_error(
                "feature case=" + std::to_string(n) + " index=" +
                std::to_string(i) + " native=" + std::to_string(values[i]) +
                " original=" + std::to_string(captured[i]));
        record(unsigned(duration), get(raw.task.data(), 0x15b),
               "two-member lifetime");
        record(unsigned(totals.travel), get(raw.task.data(), 0x19c),
               "maximum member travel");
        record(unsigned(totals.auxiliary), get(raw.task.data(), 0x1a0),
               "member auxiliary sum");
        record(unsigned(totals.danger), get(raw.task.data(), 0x1a4),
               "member danger sum");
        record(unsigned(totals.risk), get(raw.task.data(), 0x157),
               "member maximum path risk");
      }
      record(random.left, *reinterpret_cast<unsigned *>(0x6a29d8),
             "member RNG left");
      record(random.right, *reinterpret_cast<unsigned *>(0x6a29dc),
             "member RNG right");
      for (unsigned i = 0; i < 250; ++i)
        record(random.words[i], reinterpret_cast<unsigned *>(0x6a29e0)[i],
               "member RNG word");
#else
      record(unsigned(budget), unsigned(budget), "native budget");
      record(unsigned(result), unsigned(result), "native result");
      if (result == m::OwnerTaskProbeResult::reached)
        for (auto v : {duration, totals.travel, totals.auxiliary, totals.danger,
                       totals.risk})
          record(unsigned(v), unsigned(v), "native totals");
      record(random.left, random.left, "native RNG left");
      record(random.right, random.right, "native RNG right");
      for (auto v : random.words)
        record(v, v, "native RNG word");
#endif
      if (result == m::OwnerTaskProbeResult::reached)
        for (auto value : values)
          record(unsigned(value), unsigned(value), "produced feature");
    }
    need(result != m::OwnerTaskProbeResult::pending,
         "member probe slice bound");
    reached += result == m::OwnerTaskProbeResult::reached;
    failed += result == m::OwnerTaskProbeResult::failed;
  }
  need(reached && failed && yields, "member probe branches");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "source probes text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":256,\"checks\":" << checks
            << ",\"slices\":" << slices << ",\"yields\":" << yields
            << ",\"reached\":" << reached << ",\"failed\":" << failed
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
