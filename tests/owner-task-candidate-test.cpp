#define main unused_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/combat/melee.hpp"
#include "../reconstruction/creature-ai/owner_planner_features.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
std::vector<int> captured;
std::array<float, 3> probabilities;
void __attribute__((thiscall)) inference(void *, unsigned key, void *input,
                                         int outputs, float *out) {
  need(key == 6 && outputs == 3, "candidate inference ABI");
  const auto *begin = reinterpret_cast<int *>(
      *reinterpret_cast<unsigned *>(static_cast<char *>(input) + 4));
  const auto *end = reinterpret_cast<int *>(
      *reinterpret_cast<unsigned *>(static_cast<char *>(input) + 8));
  captured.assign(begin, end);
  std::copy(probabilities.begin(), probabilities.end(), out);
}
float candidate(unsigned n, int radius, const std::array<int, 3> &path,
                const std::array<float, 3> &p,
                const std::array<float, 3> &weights, int percent,
                m::RandomState &random) {
  Bytes pool(125 * 0xe4b), configs(2 * 0x5c9), source(0x8d), enemy(0x8d),
      object(0x300), region(660);
  for (unsigned i = 0; i < 2; ++i) {
    auto *a = pool.data() + i * 0xe4b;
    auto *cfg = configs.data() + i * 0x5c9;
    put(a, 0, i);
    put(a, 4, 1);
    put(a, 8, 2 + i * 4);
    put(a, 12, 2);
    put(a, 16, 2);
    put(a, 0xe4, 100);
    put(a, 0xa8, 17 + i);
    put(a, 0xac, address(cfg));
    put(a, 0x174, i);
    put(a, 0xf8, 1);
    put(cfg, 0, 1);
    put(cfg, 0x589, 24);
    global(0x6a5f80 + (17 + i) * 0x5c9, 1);
  }
  put(pool.data(), 0x936, percent);
  global(0x643a80, 1);
  global(0x643a80 + 12, 0);
  global(0x6def58, address(pool.data()));
  global(0x6def5c, 125);
  global(0x6df180, 125);
  global(0x5d854c, 3);
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  global(0x6c5c84, 0);
  global(0x6e1f64, 0);
  global(0x6eaf08, address(region.data()));
  global(0x6eaf0c, 1);
  put(region.data(), 0x20c, 1);
  for (unsigned j = 0; j < 8; ++j) {
    source[0x41 + j] = 1;
    enemy[0x41 + j] = 1;
  }
  put(source.data(), 0x35, address(pool.data()));
  put(source.data(), 0x89, 3);
  put(enemy.data(), 0x35, address(pool.data() + 0xe4b));
  put(enemy.data(), 0x89, 3);
  put(enemy.data(), 0x1d, radius);
  put(object.data(), 4, 6);
  put(object.data(), 8, address(source.data()));
  put(object.data(), 0x16, 122);
  put(object.data(), 0x149, 1);
  put(object.data(), 0x18c, address(enemy.data()));
  put(object.data(), 0x1a8, 1);
  put(object.data(), 0x19c, path[0]);
  put(object.data(), 0x1a0, path[1]);
  put(object.data(), 0x1a4, path[2]);
  std::array<int, 72> values{};
  put(object.data(), 0x163, address(values.data()));
  put(object.data(), 0x167, address(values.data()));
  put(object.data(), 0x16b, address(values.data() + 72));
  for (unsigned i = 0; i < 3; ++i)
    global(0x6c48fc + 6 * 12 + 4 * i, bits(weights[i]));
  global(0x6a29d8, random.left);
  global(0x6a29dc, random.right);
  std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
  probabilities = p;
  captured.clear();
  int budget[3] = {0, 0, 100};
  float score = 0;
  using Next =
      unsigned(__attribute__((thiscall)) *)(void *, float *, int *, int);
  need((reinterpret_cast<Next>(0x431ce0)(object.data(), &score, budget, 100) &
        255) == 1,
       "candidate result");
  need(captured.size() == 72, "candidate feature count");
  random.left = *reinterpret_cast<unsigned *>(0x6a29d8);
  random.right = *reinterpret_cast<unsigned *>(0x6a29dc);
  std::memcpy(random.words.data(), reinterpret_cast<void *>(0x6a29e0), 1000);
  need(allocated.empty(), "candidate escaped supplied vector capacity");
  (void)n;
  return score;
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
  need(!syscall(SYS_set_thread_area, &segment), "candidate FS");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x4766b0, reinterpret_cast<void *>(&inference));
  redirect(0x597850, reinterpret_cast<void *>(&allocate));
  redirect(0x597870, reinterpret_cast<void *>(&release));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "candidate RX text");
#else
  (void)argc;
  (void)argv;
#endif
  for (unsigned n = 0; n < 4096; ++n) {
    m::OwnerPlannerActor a, b;
    a.position = {2, 2, 2};
    b.position = {6, 2, 2};
    a.speed = b.speed = 24;
    a.physical = b.physical = true;
    const int radius = int(n % 1024);
    const std::array<int, 3> path{{int(n % 100), int(n % 31), int(n % 47)}};
    const auto values =
        m::hostileTaskCandidateFeatures(a, b, {32, 32, 8}, radius, path, false);
    const std::array<float, 3> probability{
        {float(n % 13) / 16.f, .125f, float(n % 7) / 8.f}};
    const std::array<float, 3> weights{
        {float(n % 5) / 4.f, 0, float(n % 3) / 2.f}};
    const int percent = int(n % 3 == 0 ? 100 : n % 201);
    auto random = m::seedRandom(n + 17), oldRandom = random;
    (void)m::nextRandom(random);
    const float result = m::ownerTaskValue(probability, weights, percent);
#ifdef MNM_BRAIN_REFERENCE
    const float expected =
        candidate(n, radius, path, probability, weights, percent, oldRandom);
    need(captured == values, "candidate ordered feature assembly");
    record(bits(result), bits(expected),
           ("candidate score fixture " + std::to_string(n)).c_str());
    need(random == oldRandom, "candidate strategic RNG continuation");
#else
    (void)oldRandom;
    record(bits(result), bits(result), "native candidate score");
#endif
    for (int value : values)
      record(unsigned(value), unsigned(value), "candidate feature");
    record(random.left, random.left, "candidate RNG");
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "candidate text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":4096,\"digest\":\"" << digest << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
