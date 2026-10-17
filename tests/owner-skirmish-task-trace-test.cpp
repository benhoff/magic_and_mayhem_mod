#define MNM_THREE_MEMBER_TASK_FIXTURE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-three-member-task-test.cpp"
#pragma GCC diagnostic pop
#include "owner-task-trace-wire.hpp"
namespace tr = mnm::test::owner_trace;
namespace {
#ifdef MNM_BRAIN_REFERENCE
const tr::Frame *currentFrame = nullptr;
unsigned __attribute__((thiscall)) recordedRisk(void *actor, int risk) {
  auto id = *static_cast<unsigned *>(actor);
  const auto &f = *currentFrame;
  auto at = std::find(f.members.begin(), f.members.end(), id);
  need(at != f.members.end(), "recorded risk source member");
  auto i = unsigned(at - f.members.begin());
  return std::int64_t(f.percent) * (f.raw[i] - f.net[i]) / 100 <
         f.raw[i] - risk;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 3, "PE TRACE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc seg{};
  seg.entry_number = unsigned(-1);
  seg.base_addr = address(thread.data());
  seg.limit = 0xfffff;
  seg.seg_32bit = seg.limit_in_pages = seg.useable = 1;
  need(!syscall(SYS_set_thread_area, &seg), "trace FS");
  unsigned short selector = (seg.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  redirect(0x50af10, reinterpret_cast<void *>(&targetCommand));
  redirect(0x50fd50, reinterpret_cast<void *>(&moveCommand));
  redirect(0x50fef0, reinterpret_cast<void *>(&stopCommand));
  redirect(0x52a430, reinterpret_cast<void *>(&recordedRisk));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "trace RX");
  const auto path = argv[2];
#else
  need(argc == 2, "TRACE");
  const auto path = argv[1];
#endif
  auto bytes = load(path);
  need(bytes.size() >= 8 && bytes.size() <= 256 * 1024 * 1024 &&
           std::equal(bytes.begin(), bytes.begin() + 8, "MNMOT001"),
       "task trace header/limit");
  tr::Reader input{bytes, 8};
  unsigned cases = 0, members = 0, owners = 0;
  while (input.at < bytes.size()) {
    auto n = input.count(1024 * 1024);
    need(n <= bytes.size() - input.at, "task trace extent");
    std::vector<unsigned char> recordBytes(bytes.begin() + input.at,
                                           bytes.begin() + input.at + n);
    input.at += n;
    tr::Reader reader{recordBytes};
    auto f = reader.frame();
    need(reader.at == recordBytes.size(), "task trace trailing data");
    auto task = f.before;
    int budget = f.initialBudget;
    std::vector<unsigned> observed;
#ifdef MNM_BRAIN_REFERENCE
    currentFrame = &f;
    // Rename the two native factions into the fixture's relative owner0/1.
    auto world = f.world;
    for (auto &a : world.creatures)
      a.ai.owner = a.ai.owner == int(f.owner) ? 0 : 1;
    world.relation[0].fill(0);
    world.relation[0][0] = 1;
    m::OwnerKnowledge knowledge;
    knowledge.records[0] = f.knowledge.records[f.owner];
    m::NavigationRegion region;
    observed = oldGroup(task, knowledge, f.tick, world, region, f.percent,
                        f.threat, f.members, budget);
#else
    auto out = m::executeOwnerSourceGroupTask(
        task, f.knowledge, f.tick, f.owner, f.members, f.world, f.raw, f.net,
        f.percent, f.threat, budget);
    auto copy = f;
    copy.result = std::move(out);
    observed = tr::commands(copy);
#endif
    auto expected = tr::commands(f);
    record(expected.size(), observed.size(), "battle task command count");
    for (unsigned i = 0; i < expected.size(); ++i)
      record(expected[i], observed[i], "battle task command");
    record(unsigned(f.finalBudget), unsigned(budget), "battle task budget");
    need(task.groups.nodes.size() == f.after.groups.nodes.size(),
         "battle task node count");
    for (unsigned i = 0; i < task.groups.nodes.size(); ++i) {
      const auto &a = f.after.groups.nodes[i], &b = task.groups.nodes[i];
      for (auto pair : {std::pair<unsigned, unsigned>{a.activity, b.activity},
                        {unsigned(a.position.x), unsigned(b.position.x)},
                        {unsigned(a.position.y), unsigned(b.position.y)},
                        {unsigned(a.position.z), unsigned(b.position.z)}})
        record(pair.first, pair.second, "battle task refreshed node");
    }
    ++cases;
    members += f.members.size();
    owners |= 1u << f.owner;
  }
  need(cases && owners == 3, "battle task trace witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "battle task text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":" << cases
            << ",\"slices\":" << cases << ",\"checks\":" << checks
            << ",\"members\":" << members << ",\"owners\":" << owners
            << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
