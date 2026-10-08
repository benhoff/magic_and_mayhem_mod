#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
static unsigned __attribute__((stdcall)) copy_surface(void *, void *, void *, void *, unsigned, void *) { return 0; }
static unsigned __attribute__((stdcall)) fast_surface(void *, unsigned, unsigned, void *, void *, unsigned) { return 0; }
static unsigned __attribute__((stdcall)) unlock_surface(void *, void *) {
  return 0;
}
int main(int argc, char **argv) try {
  if (argc != 4)
    throw std::runtime_error("Expected pinned PE, panel kind, output");
  map_image(read(argv[1]));
  const unsigned kind = std::stoul(argv[2]);
  if ((kind < 15 || kind > 17) && (kind < 21 || kind > 26))
    throw std::runtime_error("Panel fixture kind");
  std::vector<std::uint16_t> pixels(16 * 16);
  for (unsigned i = 0; i < pixels.size(); ++i)
    pixels[i] = std::uint16_t(i * 1273 + 113);
  // Adapter boundaries only: replace the lock entry in this private PE mapping
  // after checking its original bytes; the recovered panel routines stay
  // intact.
  auto *lock = reinterpret_cast<unsigned char *>(0x58b660);
  if (std::memcmp(lock, "\x83\xec\x08\x33\xc0", 5))
    throw std::runtime_error("Original lock bytes changed");
  lock[0] = 0xb8;
  const auto pointer =
      std::uint32_t(reinterpret_cast<std::uintptr_t>(pixels.data()));
  std::memcpy(lock + 1, &pointer, 4);
  lock[5] = 0xc3;
  std::uint32_t table[33] = {}, interface[1] = {}, surface[12] = {};
  table[5] = reinterpret_cast<std::uintptr_t>(copy_surface);
  table[7] = reinterpret_cast<std::uintptr_t>(fast_surface);
  table[32] = reinterpret_cast<std::uintptr_t>(unlock_surface);
  interface[0] = reinterpret_cast<std::uintptr_t>(table);
  surface[2] = reinterpret_cast<std::uintptr_t>(interface);
  surface[1] = reinterpret_cast<std::uintptr_t>(interface);
  surface[4] = 16;
  surface[5] = 16;
  surface[6] = 32;
  global(0x6e1f88, 0);
  *reinterpret_cast<std::uint16_t *>(0x6e1f7c) = 0xf800;
  *reinterpret_cast<std::uint16_t *>(0x6e1f7e) = 0x07e0;
  *reinterpret_cast<std::uint16_t *>(0x6e1f80) = 0x001f;
  int rectangle[4] = {3, 4, 10, 11};
  if (kind >= 23 && kind <= 25) {
    global(0x6f68d8, 1);
    unsigned address = kind == 23 ? 0x58c6a0 : kind == 24 ? 0x58cd50 : 0x58cf20;
    using Copy = unsigned(__attribute__((thiscall)) *)(void *, void *, void *, int, int);
    reinterpret_cast<Copy>(address)(surface, surface, rectangle, 0, 0);
  } else if (kind == 22 || kind == 26) {
    std::vector<unsigned char> object(0x6200), cells(16 * 16);
    auto field = [&](unsigned offset, unsigned value) { std::memcpy(object.data() + offset, &value, 4); };
    for (unsigned i = 0; i < 16; ++i) { unsigned short colour = i * 3137 + 211; std::memcpy(cells.data() + i * 16 + 12, &colour, 2); }
    field(0x29, reinterpret_cast<std::uintptr_t>(cells.data()));
    field(0xe7, pointer + (3 * 16 + 8) * 2); field(0xeb, 16);
    field(0x6199, 4); field(0x619d, 4); field(0x61a1, 3); field(0x61a5, 2);
    field(0x61a9 + 4, reinterpret_cast<std::uintptr_t>(interface)); field(0x61c1, 32);
    std::vector<signed char> states(16, 1);
    for (unsigned i = 0; i < 16; ++i) {
      states[i] = i % 3 == 0 ? -1 : 1;
      const unsigned ptr = reinterpret_cast<std::uintptr_t>(&states[i]);
      std::memcpy(cells.data() + i * 16 + 8, &ptr, 4);
    }
    global(0x5e1404, kind == 26 ? 1 : 0); global(0x5e18f0, unsigned(-1));
    using Draw = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Draw>(0x553a40)(object.data());
  } else if (kind == 15) {
    using Draw =
        void(__attribute__((thiscall)) *)(void *, void *, int, int, int);
    reinterpret_cast<Draw>(0x58ddf0)(surface, rectangle, 40, 1, 1);
  } else if (kind == 21) {
    using Draw = void(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Draw>(0x58ed80)(surface);
  } else {
    using Draw = void(__attribute__((thiscall)) *)(void *, void *);
    reinterpret_cast<Draw>(kind == 16 ? 0x58d8c0 : 0x58e2c0)(surface,
                                                             rectangle);
  }
  save(argv[3], pixels);
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
