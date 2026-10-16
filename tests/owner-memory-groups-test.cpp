#define main unused_owner_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_memory_groups.hpp"
namespace {
unsigned salt = 0;
std::vector<unsigned> calls;
bool region(int a, int b, int &middle) {
  calls.insert(calls.end(), {1, unsigned(a), unsigned(b)});
  middle = (a + b + int(salt)) % 16;
  return (unsigned(a * 7 + b * 3) + salt) % 5 != 0;
}
bool line(m::Cell a, m::Cell b) {
  calls.insert(calls.end(), {2, unsigned(a.x), unsigned(a.y), unsigned(a.z),
                             unsigned(b.x), unsigned(b.y), unsigned(b.z)});
  return (unsigned(a.x + b.y) + salt) % 3 != 0;
}
std::vector<unsigned> projection(const m::OwnerMemoryGroups &s) {
  std::vector<unsigned> out;
  std::function<void(unsigned)> visit = [&](unsigned id) {
    const auto &n = s.nodes.at(id);
    out.push_back(unsigned(n.children.size()));
    if (id)
      out.insert(out.end(), {unsigned(n.position.x), unsigned(n.position.y),
                             unsigned(n.position.z), unsigned(n.region),
                             unsigned(n.uncertaintySquared),
                             unsigned(n.flag104), unsigned(n.memoryKey >= 0)});
    if (n.memoryKey >= 0)
      out.push_back(unsigned(n.memoryKey));
    for (auto c : n.children)
      visit(c);
  };
  visit(0);
  return out;
}
#ifdef MNM_BRAIN_REFERENCE
unsigned get(const void *p, unsigned off = 0) {
  unsigned v;
  std::memcpy(&v, static_cast<const char *>(p) + off, 4);
  return v;
}
unsigned __attribute__((thiscall)) regionService(void *, int a, int b,
                                                 const void *p, int *out) {
  const auto *v = static_cast<const unsigned char *>(p);
  need(get(v) == 4 && get(v, 4) == 1 && get(v, 8) == 1 && get(v, 12) == 3,
       "group regional movement");
  return region(a, b, *out);
}
unsigned __attribute__((thiscall)) lineService(void *, int ax, int ay, int az,
                                               int bx, int by, int bz,
                                               const void *) {
  return line({ax, ay, az}, {bx, by, bz});
}
std::vector<unsigned> projectOriginal(void *root) {
  std::vector<unsigned> out;
  std::function<void(void *, bool)> visit = [&](void *p, bool top) {
    unsigned count = (get(p, 0x2d) - get(p, 0x29)) / 4;
    out.push_back(count);
    const bool leaf = get(p, 0x35) != 0;
    if (!top)
      out.insert(out.end(), {get(p, 13), get(p, 17), get(p, 21), get(p, 25),
                             get(p, 29), get(p, 33), unsigned(leaf)});
    if (leaf)
      out.push_back(get(p, 9));
    for (unsigned i = 0; i < count; ++i) {
      auto child = get(reinterpret_cast<void *>(get(p, 0x29)), 4 * i);
      need(get(reinterpret_cast<void *>(child), 0x39) == address(p),
           "original grouping parent");
      visit(reinterpret_cast<void *>(child), false);
    }
  };
  visit(root, true);
  return out;
}
std::array<std::vector<unsigned>, 2>
originalGroups(const m::OwnerKnowledge &k, int tick, bool stale,
               const std::array<std::array<std::uint8_t, 8>, 8> &allied) {
  Bytes ctx(0x100), state(48), head(0xe68), nil(0xe68);
  std::vector<Bytes> nodes(k.records[0].size(), Bytes(0xe68));
  put(ctx.data(), 0x27, 0);
  put(state.data(), 4, address(head.data()));
  put(nil.data(), 0xe64, 1);
  global(0x5faf4c, address(nil.data()));
  const auto h = address(head.data()), n = address(nil.data());
  put(head.data(), 0, nodes.empty() ? h : address(nodes.front().data()));
  put(head.data(), 4, nodes.empty() ? n : address(nodes.front().data()));
  put(head.data(), 8, nodes.empty() ? h : address(nodes.back().data()));
  for (unsigned i = 0; i < nodes.size(); ++i) {
    auto *raw = nodes[i].data();
    const auto &v = k.records[0][i];
    put(raw, 0, n);
    put(raw, 4, i ? address(nodes[i - 1].data()) : h);
    put(raw, 8, i + 1 < nodes.size() ? address(nodes[i + 1].data()) : n);
    put(raw, 12, v.memory.key);
    put(raw, 16, v.memory.observedTick);
    put(raw, 20, v.markedTick);
    put(raw, 24, v.memory.actor.slot);
    put(raw, 32, v.memory.actor.position.x);
    put(raw, 36, v.memory.actor.position.y);
    put(raw, 40, v.memory.actor.position.z);
    put(raw, 0xfc, v.memory.actor.health);
    put(raw, 0x18c, v.memory.actor.owner);
    put(raw, 0x11c, v.memory.actor.flag104);
    put(raw, 0xdff, v.memory.actor.region);
    put(raw, 0xb7b, v.planning->motionB63);
    put(raw, 0xe64, 1);
  }
  for (unsigned i = 0; i < 8; ++i)
    *reinterpret_cast<unsigned char *>(0x643a80 + 24 + i) = allied[0][i];
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  global(0x5d854c, 3);
  using Build =
      void *(__attribute__((thiscall)) *)(void *, void *, int, unsigned);
  auto invoke = reinterpret_cast<Build>(0x413330);
  std::array<std::vector<unsigned>, 2> result;
  calls.clear();
  result[0] = projectOriginal(invoke(ctx.data(), state.data(), tick, stale));
  result[1] = calls;
  for (unsigned i = 0; i < 8; ++i)
    *reinterpret_cast<unsigned char *>(0x643a80 + 24 + i) = 1;
  calls.clear();
  need(projectOriginal(invoke(ctx.data(), state.data(), tick + 1, stale)) ==
               result[0] &&
           calls.empty(),
       "original grouping cache");
  // Fixture owns allocator storage; original cache/clone teardown is excluded.
  while (!allocated.empty())
    release(allocated.begin()->first);
  return result;
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
  need(!syscall(SYS_set_thread_area, &segment), "exception segment");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  redirect(0x542810, reinterpret_cast<void *>(&regionService));
  redirect(0x4f4ba0, reinterpret_cast<void *>(&lineService));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "group RX text");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned merged = 0, cached = 0;
  for (unsigned test = 0; test < 2048; ++test) {
    salt = test;
    m::OwnerKnowledge k;
    std::array<std::array<std::uint8_t, 8>, 8> allied{};
    allied[0][0] = 1;
    allied[0][2] = test % 7 == 0;
    for (unsigned i = 0; i < (test == 2047 ? 125 : test % 17); ++i) {
      m::OwnerMemory v;
      v.memory.key = int(i * 3 + 5);
      v.memory.observedTick = 100 - int((test % 19 == 0  ? 100000
                                         : test % 9 == 0 ? 500
                                                         : test % 20) +
                                        i);
      v.markedTick = (i + test) % 3 ? 100 : 99;
      v.memory.actor.slot = int(i % 4);
      v.memory.actor.health = (i + test) % 11 ? 100 : 0;
      v.memory.actor.owner = 1 + int(i % 2);
      v.memory.actor.flag104 = (i + test) % 3 == 0;
      v.memory.actor.position = {int((i * 7 + test) % 32),
                                 int((i * 11 + test / 3) % 32), 2 + int(i % 3)};
      v.memory.actor.region = (i + test) % 9 ? int(i % 16) : -1;
      v.planning.emplace();
      v.planning->motionB63 = 1 + int(test % 6);
      k.records[0].push_back(v);
    }
    m::OwnerMemoryGroups groups;
    calls.clear();
    m::buildOwnerMemoryGroups(groups, k, 0, 100, test % 2, {32, 32, 8}, allied,
                              {region, line});
    const auto got = projection(groups), trace = calls;
    merged += unsigned(k.records[0].size() > groups.nodes[0].children.size());
#ifdef MNM_BRAIN_REFERENCE
    const auto expected = originalGroups(k, 100, test % 2, allied);
    need(got.size() == expected[0].size(),
         ("group shape case " + std::to_string(test)).c_str());
    for (unsigned i = 0; i < got.size(); ++i)
      record(
          got[i], expected[0][i],
          ("group case " + std::to_string(test) + " field " + std::to_string(i))
              .c_str());
    need(trace == expected[1],
         ("ordered reachability case " + std::to_string(test)).c_str());
#else
    for (auto v : got)
      record(v, v, "native grouping");
#endif
    for (auto v : trace)
      record(v, v, "reachability trace");
    calls.clear();
    for (auto &v : allied[0])
      v = 1;
    m::buildOwnerMemoryGroups(groups, k, 0, 101, test % 2, {32, 32, 8}, allied,
                              {region, line});
    need(projection(groups) == got && calls.empty(), "native grouping cache");
    ++cached;
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "group original text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":2048,\"cache_checks\":" << cached
            << ",\"group_reductions\":" << merged << ",\"digest\":\"" << digest
            << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
