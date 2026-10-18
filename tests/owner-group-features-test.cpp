#define main unused_owner_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/owner_group_features.hpp"
namespace {
#ifdef MNM_BRAIN_REFERENCE
template <std::size_t... I>
void invoke(void *a, void *b, void *v, std::index_sequence<I...>) {
  using Call = void(__attribute__((cdecl)) *)(void *, void *, int, void *,
                                              unsigned, ...);
  reinterpret_cast<Call>(0x40b420)(a, b, 123, v, 67,
                                   m::hostileTaskFeatureIds[I]...);
}
std::array<int, 67>
originalGroupFeatures(const std::vector<m::OwnerPlannerActor> &a,
                      const std::vector<m::OwnerPlannerActor> &b) {
  const unsigned count = unsigned(a.size() + b.size());
  Bytes pool(125 * 0xe4b), regions(count * 660), configs(count * 0x5c9);
  std::vector<Bytes> leaves(count, Bytes(0x8d));
  std::array<Bytes, 2> roots{{Bytes(0x8d), Bytes(0x8d)}};
  std::array<std::vector<unsigned>, 2> children;
  global(0x6def58, address(pool.data()));
  global(0x6def5c, 125);
  global(0x6df180, 125);
  global(0x6eaf08, address(regions.data()));
  global(0x6eaf0c, count);
  global(0x6c5494, 32);
  global(0x6c5498, 32);
  global(0x6c549c, 8);
  global(0x6c5c84, 0);
  global(0x5d854c, 3);
  for (unsigned i = 0; i < count; ++i) {
    unsigned side = i < a.size() ? 0 : 1;
    const auto &v = side ? b[i - a.size()] : a[i];
    auto *raw = pool.data() + i * 0xe4b;
    auto *cfg = configs.data() + i * 0x5c9;
    auto *node = leaves[i].data();
    const unsigned type = i + 1;
    put(raw, 0, i);
    put(raw, 4, 1);
    put(raw, 8, v.position.x);
    put(raw, 12, v.position.y);
    put(raw, 16, v.position.z);
    put(raw, 0xde7, i);
    put(raw, 0xa8, type);
    put(raw, 0xac, address(cfg));
    put(raw, 0xe4, 100);
    put(raw, 0xf8, v.physical);
    put(raw, 0x104, v.flag104);
    put(raw, 0x174, side);
    put(raw, 0x84, v.power);
    put(raw, 0x88, v.bravery);
    for (unsigned j = 0; j < 6; ++j)
      put(raw, 0x90a + 4 * j, v.combatValue[j]);
    put(cfg, 0, 1);
    put(cfg, 0x589, v.speed);
    for (auto [off, val] :
         std::array<std::pair<unsigned, int>, 5>{{{0, 1},
                                                  {0x589, v.speed},
                                                  {0x5a9, v.factor5a9},
                                                  {0x5ad, v.factor5ad},
                                                  {0x5c5, v.defense5c5}}})
      global(0x6a5f80 + type * 0x5c9 + off, val);
    regions[i * 660 + 0x284] = v.danger;
    put(node, 0x35, address(raw));
    put(node, 0x39, address(roots[side].data()));
    put(node, 0x89, 3);
    for (unsigned j = 0; j < 8; ++j)
      node[0x41 + j] = 1;
    children[side].push_back(address(node));
  }
  for (unsigned side = 0; side < 2; ++side) {
    auto *root = roots[side].data();
    for (unsigned j = 0; j < 8; ++j)
      root[0x41 + j] = 1;
    put(root, 0x29, address(children[side].data()));
    put(root, 0x2d, address(children[side].data() + children[side].size()));
    put(root, 0x31, address(children[side].data() + children[side].size()));
  }
  std::array<int, 67> out{};
  Bytes vec(16);
  put(vec.data(), 4, address(out.data()));
  put(vec.data(), 8, address(out.data()));
  put(vec.data(), 12, address(out.data() + 67));
  invoke(roots[0].data(), roots[1].data(), vec.data(),
         std::make_index_sequence<67>{});
  need(allocated.empty(), "group features fixed capacity");
  return out;
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "group features RX");
#else
  (void)argc;
  (void)argv;
#endif
  for (unsigned n = 0; n < 2048; ++n) {
    std::vector<m::OwnerPlannerActor> a(1 + n % 3), b(1 + n % 8);
    unsigned i = 0;
    for (auto *g : {&a, &b})
      for (auto &v : *g) {
        for (unsigned j = 0; j < 6; ++j)
          v.combatValue[j] = int((n * 103 + i * 79 + j * 91) % 10000);
        v.position = {int((n * 7 + i * 13) % 32), int((n * 11 + i * 29) % 32),
                      int((n + i) % 8)};
        v.speed = int((n + i * 3) % 80);
        v.power = int((n + i * 7) % 300);
        v.bravery = int((n + i * 13) % 100);
        v.defense5c5 = int((n + i * 19) % 50);
        v.factor5a9 = int((n + i * 7) % 30);
        v.factor5ad = int((n + i * 2) % 100);
        v.physical = (n + i) % 2;
        v.flag104 = (n + i) % 3 == 0;
        v.danger = int((n + i * 31) % 256);
        ++i;
      }
    const auto got = m::hostileTaskGroupFeatures(a, b, {32, 32, 8});
#ifdef MNM_BRAIN_REFERENCE
    const auto expected = originalGroupFeatures(a, b);
#else
    const auto &expected = got;
#endif
    for (unsigned i = 0; i < 67; ++i)
      record(unsigned(got[i]), unsigned(expected[i]),
             ("group feature " + std::to_string(m::hostileTaskFeatureIds[i]) +
              " case " + std::to_string(n))
                 .c_str());
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "group features text guard");
#endif
  std::cout << "{\"success\":true,\"checks\":" << checks
            << ",\"cases\":2048,\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
