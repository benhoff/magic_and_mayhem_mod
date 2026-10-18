// Reuse the isolated DAT oracle's mapper, allocator and FS fixture.
#define main unused_brain_inference_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_planner_features.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
template <std::size_t... I>
void originalFeatureCall(void *a, void *b, void *v, std::index_sequence<I...>) {
  using Features = void(__attribute__((cdecl)) *)(void *, void *, int, void *,
                                                  unsigned, ...);
  reinterpret_cast<Features>(0x40b420)(a, b, 123, v, 67,
                                       m::hostileTaskFeatureIds[I]...);
}
std::array<int, 67> originalFeatures(const m::OwnerPlannerActor &a,
                                     const m::OwnerPlannerActor &b,
                                     m::Extent d) {
  Bytes region(2 * 660);
  region[0x284] = a.danger;
  region[660 + 0x284] = b.danger;
  global(0x6eaf08, address(region.data()));
  global(0x6eaf0c, 2);
  Bytes pool(125 * 0xe4b);
  global(0x6def58, address(pool.data()));
  global(0x6def5c, 125);
  global(0x6df180, 125);
  std::array<Bytes, 2> configs{{Bytes(0x5c9), Bytes(0x5c9)}},
      nodes{{Bytes(0x8d), Bytes(0x8d)}};
  for (unsigned i = 0; i < 2; ++i) {
    const auto &v = i ? b : a;
    auto *raw = pool.data() + i * 0xe4b;
    auto *cfg = configs[i].data();
    auto *node = nodes[i].data();
    put(raw, 0xde7, i);
    put(raw, 0, i);
    put(raw, 4, 1);
    put(raw, 8, v.position.x);
    put(raw, 12, v.position.y);
    put(raw, 16, v.position.z);
    put(raw, 0xa8, 17 + i);
    put(raw, 0xac, address(cfg));
    put(raw, 0xe4, 100);
    put(raw, 0xf8, v.physical);
    put(raw, 0x104, v.flag104);
    put(raw, 0x174, i);
    put(raw, 0x84, v.power);
    put(raw, 0x88, v.bravery);
    for (unsigned j = 0; j < 6; ++j)
      put(raw, 0x90a + 4 * j, v.combatValue[j]);
    put(cfg, 0, 1);
    put(cfg, 0x589, v.speed);
    for (const auto &[off, value] :
         std::array<std::pair<unsigned, int>, 5>{{{0x589, v.speed},
                                                  {0x5a9, v.factor5a9},
                                                  {0x5ad, v.factor5ad},
                                                  {0x5c5, v.defense5c5},
                                                  {0, 1}}})
      global(0x6a5f80 + (17 + i) * 0x5c9 + off, value);
    put(node, 0x35, address(raw));
    put(node, 0x89, 3);
    for (unsigned j = 0; j < 8; ++j)
      node[0x41 + j] = 1;
  }
  global(0x6c5494, d.x);
  global(0x6c5498, d.y);
  global(0x6c549c, d.z);
  global(0x6c5c84, 0);
  global(0x5d854c, 3);
  Bytes output(16);
  std::array<int, 67> values{};
  put(output.data(), 4, address(values.data()));
  put(output.data(), 8, address(values.data()));
  put(output.data(), 12, address(values.data() + 67));
  originalFeatureCall(nodes[0].data(), nodes[1].data(), output.data(),
                      std::make_index_sequence<67>{});
  need(allocated.empty(), "feature helper escaped fixed vector capacity");
  return values;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  // No original model/heap path is reached: output vector owns exact backing.
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "feature RX text");
#else
  (void)argc;
  (void)argv;
#endif
  for (unsigned n = 0; n < 4096; ++n) {
    m::OwnerPlannerActor a, b;
    for (auto *v : {&a, &b}) {
      const unsigned i = v == &a ? 0 : 1;
      for (unsigned j = 0; j < 6; ++j)
        v->combatValue[j] = int((n * 103 + i * 79 + j * 91) % 10000);
      v->position = {int((n * 7 + i * 13) % 32), int((n * 11 + i * 29) % 32),
                     int((n + i) % 8)};
      v->danger = int((n + i * 31) % 256);
      v->speed = int((n + i * 3) % 80);
      v->power = int((n + i * 7) % 300);
      v->bravery = int((n + i * 13) % 100);
      v->defense5c5 = int((n + i * 19) % 50);
      v->factor5a9 = int((n + i * 7) % 30);
      v->factor5ad = int((n + i * 2) % 100);
      v->physical = (n + i) % 2;
      v->flag104 = (n + i) % 3 == 0;
    }
    const auto result = m::hostileTaskFeatures(a, b, {32, 32, 8});
#ifdef MNM_BRAIN_REFERENCE
    const auto expected = originalFeatures(a, b, {32, 32, 8});
#else
    const auto &expected = result;
#endif
    for (unsigned i = 0; i < 67; ++i)
      record(
          unsigned(result[i]), unsigned(expected[i]),
          ("feature " + std::to_string(m::hostileTaskFeatureIds[i])).c_str());
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "feature original text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":4096,\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
