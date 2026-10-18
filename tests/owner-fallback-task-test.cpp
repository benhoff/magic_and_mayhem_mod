#define MNM_SOURCE_DECISION_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-source-decision-test.cpp"
#pragma GCC diagnostic pop
#include "../reconstruction/creature-ai/owner_fallback_task.hpp"
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc seg{};
  seg.entry_number = unsigned(-1);
  seg.base_addr = address(thread.data());
  seg.limit = 0xfffff;
  seg.seg_32bit = seg.limit_in_pages = seg.useable = 1;
  need(!syscall(SYS_set_thread_area, &seg), "fallback FS");
  unsigned short selector = (seg.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&sourceAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&sourceRelease));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "fallback RX");
#else
  (void)argc;
  (void)argv;
#endif
  for (unsigned n = 0; n < 256; ++n) {
    m::OwnerFallbackCandidate s{bool(n & 2), bool(n & 4)};
    bool parent = n & 1;
    m::prepareOwnerFallback(s, parent);
#ifdef MNM_BRAIN_REFERENCE
    Bytes task(0x160), src(0x8d);
    task[0x149] = bool(n & 2);
    task[0x14a] = bool(n & 4);
    put(src.data(), 0x39, parent ? 1 : 0);
    reinterpret_cast<void(__attribute__((thiscall)) *)(
        void *, void *, void *, int)>(0x415640)(task.data(), src.data(),
                                                nullptr, 100);
    record(s.eligible, task[0x149], "fallback preparation");
#else
    record(s.eligible, s.eligible, "fallback preparation");
#endif
    for (unsigned k = 0; k < 3; ++k) {
      auto value = m::nextOwnerFallback(s);
#ifdef MNM_BRAIN_REFERENCE
      float old = 99;
      int b[3] = {0, 0, int(n % 4)};
      const auto emitted =
          reinterpret_cast<unsigned(__attribute__((thiscall)) *)(
              void *, float *, int *, int)>(0x415670)(task.data(), &old, b,
                                                      100) &
          255;
      record(value.has_value(), emitted, "fallback next");
      if (value)
        record(bits(*value), bits(old), "fallback score");
      record(unsigned(n % 4), unsigned(b[2]), "fallback candidate budget");
#else
      record(value.has_value(), value.has_value(), "fallback next");
      if (value)
        record(bits(*value), bits(*value), "fallback score");
      record(n % 4, n % 4, "fallback candidate budget");
#endif
    }
#ifdef MNM_BRAIN_REFERENCE
    record(m::ownerFallbackDuration(),
           reinterpret_cast<unsigned (*)()>(0x4156a0)(), "fallback lifetime");
    record(m::executeOwnerFallback(),
           reinterpret_cast<unsigned(__attribute__((thiscall)) *)(
                   void *, void *, int, int *)>(0x419900)(task.data(), nullptr,
                                                          100, nullptr) &
               255,
           "fallback continuing");
    m::AutonomousWorld world;
    auto storage = fixture(0, world);
    storage.sources.graph[0].parent.reset();
    storage.sources.graph[0].activity = 3;
    RawSources raw(storage);
    global(0x643a80, 1);
    global(0x6def58, address(raw.actors.data()));
    global(0x6def5c, 3);
    global(0x6df180, 3);
    put(raw.actors.data(), 0x922, 7);
    put(raw.actors.data(), 0x926, 3);
    put(task.data(), 0, 0x5c53c0);
    std::array<unsigned, 1> list{{address(task.data())}};
    put(raw.brain.data(), 0x11fe5, address(list.data()));
    put(raw.brain.data(), 0x14, 100 + n);
    put(raw.brain.data(), 0xca, n);
    put(raw.brain.data(), 0x11ddd, bits(-1.f));
    raw.brain[0x11ddc] = n & 1;
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *, void *)>(
        0x4289e0)(raw.brain.data(), raw.nodes[0].data());
    record(100 + n, get(task.data(), 0xc), "fallback installed start");
    record(108 + n, get(task.data(), 0x10), "fallback installed expiry");
    record(n, get(task.data(), 0x1f), "fallback installed serial");
    record(bits(-1.f), get(task.data(), 0x1b), "fallback installed score");
    record(bits(1.75f), get(task.data(), 0x2b), "fallback installed baseline");
    record(n & 1, task[0x1a], "fallback installed weighted");
    record(0, task[0x14] | task[0x15], "fallback installed flags");
#else
    record(m::ownerFallbackDuration(), m::ownerFallbackDuration(),
           "fallback lifetime");
    record(m::executeOwnerFallback(), m::executeOwnerFallback(),
           "fallback continuing");
    for (unsigned v : {100 + n, 108 + n, n, bits(-1.f), bits(1.75f), n & 1, 0u})
      record(v, v, "native fallback install");
#endif
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "fallback unchanged text");
#endif
  std::cout << "{\"success\":true,\"cases\":256,\"slices\":768,\"checks\":"
            << checks << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
