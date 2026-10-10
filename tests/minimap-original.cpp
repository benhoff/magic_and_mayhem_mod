#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "minimap-fixture.hpp"
namespace {
unsigned unlocks;
unsigned __attribute__((stdcall)) unlock(void *, void *) { ++unlocks; return 0; }
}
int main(int argc, char **argv) try {
  if (argc != 4) throw std::runtime_error("Expected pinned PE, cases, output");
  map_image(read(argv[1]));
  const auto entries = minimap_fixture::cases(argv[2]);
  // Replace only the driver Lock adapter in this private mapped image.
  // The runner pins the complete PE hash; check original bytes before mutation.
  auto *lock = reinterpret_cast<unsigned char *>(0x58b660);
  if (std::memcmp(lock, "\x83\xec\x08\x33\xc0", 5))
    throw std::runtime_error("Original lock bytes changed");
  lock[0] = 0xb8; lock[5] = 0xc3;
  if (std::memcmp(reinterpret_cast<void *>(0x553a40), "\x83\xec\x20\x56\x8b\xf1", 6))
    throw std::runtime_error("Original minimap bytes changed");
  std::ofstream out(argv[3], std::ios::binary);
  for (const auto &c : entries) {
    auto pixels = minimap_fixture::background(c, false);
    auto history = minimap_fixture::background(c, true);
    const auto oldHistory = history;
    const auto pointer = std::uint32_t(reinterpret_cast<std::uintptr_t>(history.data()));
    std::memcpy(lock + 1, &pointer, 4);
    std::vector<unsigned char> object(0x6200), cells(c.w * c.h * 16);
    std::vector<signed char> states(c.w * c.h);
    std::uint32_t table[33] = {}, interface[1] = {};
    table[32] = reinterpret_cast<std::uintptr_t>(unlock);
    interface[0] = reinterpret_cast<std::uintptr_t>(table);
    auto field = [&](unsigned offset, unsigned value) { std::memcpy(object.data() + offset, &value, 4); };
    for (unsigned i = 0; i < states.size(); ++i) {
      states[i] = minimap_fixture::hidden(c, i) ? -1 : 1;
      const auto ptr = std::uint32_t(reinterpret_cast<std::uintptr_t>(&states[i]));
      const auto colour = minimap_fixture::colour(c, i);
      std::memcpy(cells.data() + i * 16 + 8, &ptr, 4);
      std::memcpy(cells.data() + i * 16 + 12, &colour, 2);
    }
    field(0x29, reinterpret_cast<std::uintptr_t>(cells.data()));
    field(0xc5, c.view);
    field(0xff, 0x13572468);
    field(0xe7, reinterpret_cast<std::uintptr_t>(pixels.data() + minimap_fixture::rootY(c) * minimap_fixture::stride(c) + minimap_fixture::rootX(c)));
    field(0xeb, minimap_fixture::stride(c));
    field(0x6199, c.w); field(0x619d, c.h); field(0x61a1, c.cx); field(0x61a5, c.cy);
    field(0x61a9 + 4, reinterpret_cast<std::uintptr_t>(interface));
    field(0x61c1, minimap_fixture::stride(c) * 2);
    const auto oldCells = cells, oldObject = object;
    global(0x5e1404, c.fog != 0); global(0x5e18f0, unsigned(-1));
    unlocks = 0;
    using Draw = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Draw>(0x553a40)(object.data());
    auto expectedObject = oldObject;
    const std::uint32_t invalid = 0xfffffffe;
    std::memcpy(expectedObject.data() + 0xff, &invalid, 4);
    if (unlocks != 1 || history != oldHistory || cells != oldCells || object != expectedObject)
      throw std::runtime_error("Original minimap side effects changed");
    minimap_fixture::save(out, pixels);
  }
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
