#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#define main unused_wizard_ground_fixture
#include "owner-brain-inference-test.cpp"
#undef main
#pragma GCC diagnostic pop
#include "../assets/animation.hpp"
#include "../reconstruction/motion/creature_profile.hpp"
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 3, "original PE and wizard ANI");
  map_image(load(argv[1]));
  const auto bytes = load(argv[2]);
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "wizard ground RX");
#else
  need(argc == 2, "wizard ANI");
  const auto bytes = load(argv[1]);
#endif
  auto result = mnm::assets::decodeAnimation(bytes);
  need(std::holds_alternative<mnm::assets::Animation>(result),
       "wizard ANI decode");
  const auto &ani = std::get<mnm::assets::Animation>(result);
  unsigned footsteps = 0;
  for (unsigned f = 0; f < 8; ++f)
    for (unsigned i = ani.starts[f]; i < ani.starts[f + 1]; ++i)
      footsteps += ani.records[i].opcode == 5 && ani.records[i].argument == 5;
  need(footsteps > 0, "wizard footstep fixture");
  for (unsigned n = 0; n < 128; ++n) {
    auto a = ani;
    // Translation of every displacement endpoint leaves the original bank
    // invariant; directed edits vary its actual endpoint distance.
    for (auto &r : a.records)
      if (r.opcode == 0)
        r.metadata[0] += n;
    auto raw = bytes;
    const unsigned table = 44 + unsigned(a.starts.size()) * 4;
    for (unsigned i = 0; i < a.records.size(); ++i)
      std::memcpy(raw.data() + table + i * 44 + 8, &a.records[i].metadata[0],
                  4);
    const auto samples = mnm::reconstruction::groundMovementSamples(a);
    const auto maximum = mnm::reconstruction::groundMovementMaximum(samples);
#ifdef MNM_BRAIN_REFERENCE
    Bytes definition(0x5c9);
    put(definition.data(), 0x10, 10);
    put(definition.data(), 0x3c, 0);
    const unsigned asset[3] = {address(raw.data()), address(raw.data() + 44),
                               address(raw.data() + table)};
    std::memcpy(reinterpret_cast<void *>(0x6dbf00), asset, sizeof(asset));
    global(0x6c007d, 720);
    using Build = void(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<Build>(0x505220)(definition.data(), 0);
    for (unsigned i = 0; i < 48; ++i) {
      unsigned actual;
      std::memcpy(&actual, definition.data() + 0xd8 + i * 4, 4);
      record(samples[i], actual, "wizard movement sample");
    }
    unsigned actual;
    std::memcpy(&actual, definition.data() + 0x589, 4);
    record(maximum, actual, "wizard movement maximum");
#else
    for (auto v : samples)
      record(v, v, "wizard movement sample");
    record(maximum, maximum, "wizard movement maximum");
#endif
  }
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "wizard ground unchanged text");
#endif
  std::cout << "{\"success\":true,\"cases\":128,\"checks\":" << checks
            << ",\"footsteps\":" << footsteps << ",\"digest\":\"" << digest
            << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
