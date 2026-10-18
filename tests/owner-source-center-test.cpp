#define MNM_SOURCE_DECISION_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-source-decision-test.cpp"
#pragma GCC diagnostic pop
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "center RX");
#else
  (void)argc;
  (void)argv;
#endif
  for (unsigned n = 0; n < 2048; ++n) {
    std::vector<m::Cell> points;
    for (unsigned i = 0; i < 3; ++i)
      points.push_back({int((n * 7 + i * 13) % 32), int((n * 11 + i * 19) % 32),
                        int((n + i * 3) % 8)});
    const std::vector<unsigned> order =
        n & 16 ? std::vector<unsigned>{2, 1} : std::vector<unsigned>{1, 2};
    std::vector<m::Cell> right{points[order[0]], points[order[1]]};
    auto center = m::ownerSourceCenter(right, {32, 32, 8});
    int distance = m::ownerSourceDistance({points[0]}, right, {32, 32, 8});
#ifdef MNM_BRAIN_REFERENCE
    m::AutonomousWorld world;
    auto storage = fixture(0, world);
    storage.sources.graph.resize(4);
    storage.sources.information.resize(4);
    storage.manager.nodes = {0, 1, 2, 3};
    storage.pending = {};
    storage.pendingRoots.clear();
    for (unsigned i = 0; i < 4; ++i) {
      auto &g = storage.sources.graph[i];
      g.actor = i < 3 ? std::optional<unsigned>(i) : std::nullopt;
      g.parent.reset();
      g.children.clear();
      g.activity = 3;
    }
    storage.sources.graph[3].children = order;
    for (auto i : order)
      storage.sources.graph[i].parent = 3;
    RawSources raw(storage);
    global(0x6c5494, 32);
    global(0x6c5498, 32);
    global(0x6c549c, 8);
    global(0x5d854c, 3);
    for (unsigned i = 0; i < 3; ++i) {
      auto *p = raw.actors.data() + i * 0xe4b;
      put(p, 8, points[i].x);
      put(p, 12, points[i].y);
      put(p, 16, points[i].z);
    }
    int x = 0, y = 0, z = 0;
    reinterpret_cast<void(__attribute__((fastcall)) *)(
        void *, int *, int *, int *)>(0x40a760)(raw.nodes[3].data(), &x, &y,
                                                &z);
    record(center.x, x, "source center x");
    record(center.y, y, "source center y");
    record(center.z, z, "source center z");
    const auto original = reinterpret_cast<int(__attribute__((thiscall)) *)(
        void *, void *, void *)>(0x429270)(
        raw.brain.data(), raw.nodes[0].data(), raw.nodes[3].data());
    record(distance, original, "source merge distance");
    put(raw.children[3].data(), 0, raw.pointer(2));
    put(raw.children[3].data(), 4, raw.pointer(0));
    put(raw.children[3].data(), 8, raw.pointer(1));
    put(raw.nodes[3].data(), 0x2d, address(raw.children[3].data() + 12));
    reinterpret_cast<void(__attribute__((fastcall)) *)(
        void *, int *, int *, int *)>(0x40a760)(raw.nodes[3].data(), &x, &y,
                                                &z);
    center =
        m::ownerSourceCenter({points[2], points[0], points[1]}, {32, 32, 8});
    record(center.x, x, "three center x");
    record(center.y, y, "three center y");
    record(center.z, z, "three center z");
#else
    for (int v : {center.x, center.y, center.z, distance}) {
      record(unsigned(v), unsigned(v), "native center");
    }
    center =
        m::ownerSourceCenter({points[2], points[0], points[1]}, {32, 32, 8});
    for (int v : {center.x, center.y, center.z})
      record(unsigned(v), unsigned(v), "native three center");
#endif
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "center text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":2048,\"slices\":2048,\"checks\":"
            << checks << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
