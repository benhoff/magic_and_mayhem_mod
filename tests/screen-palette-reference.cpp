#define main unused_screen_provider_main
#include "screen-provider-reference.cpp"
#undef main
#include "../reconstruction/rendering/screen_palette_providers.hpp"
static void real(Memory &m, U at, double value) {
  std::uint64_t bits;
  std::memcpy(&bits, &value, 8);
  m.set32(at, U(bits));
  m.set32(at + 4, U(bits >> 32));
}
static std::vector<unsigned char> palette(bool original, U entry, U count,
                                          U variant) {
  Memory m;
  m.original = original;
  m.initialize();
  constexpr U rgb = Base + 0x2000, node = Base + 0x4000;
  for (U i = 0; i < 768; ++i)
    m.set8(rgb + i, (i * 137 + variant * 19) & 255);
  for (U i = 0; i < 4; ++i)
    for (U j = 0; j < 3; ++j)
      m.set8(rgb + 3 * i + j, i == 0             ? 0
                              : i == 1 && j == 2 ? 255
                              : i == 2 && j == 0 ? 255
                              : i == 3 && j == 1 ? 255
                                                 : 0);
  m.set32(0x5f18f8, variant & 1 ? 24 : 30);
  m.set32(0x5f18fc, variant & 1 ? 19 : 40);
  m.set32(0x6e1f88, variant & 2);
  real(m, 0x5f18d8, variant & 4 ? 1.25 : 1);
  real(m, 0x5f1908, variant & 4 ? .75 : 1);
  real(m, 0x5f1910, 0);
  real(m, 0x5f1918, 0);
  const double coefficients[4][3] = {
      {-.4, -.1, .7}, {0, 0, 0}, {.2, -.3, .4}, {-1, 1, -.25}};
  U args[7] = {rgb};
  for (U i = 0; i < 3; ++i) {
    std::uint64_t bits;
    std::memcpy(&bits, &coefficients[variant / 8 % 4][i], 8);
    args[i * 2 + 1] = U(bits);
    args[i * 2 + 2] = U(bits >> 32);
  }
  U result = 0;
#ifdef MNM_ORIGINAL_REFERENCE
  if (original)
    result = actor_services_call(entry, count, node, 7, args, 28);
  else
#endif
    result = *mnm::reconstruction::ScreenPaletteProviders<Memory>(m).tryRun(
        entry, count, node,
        {args[0], args[1], args[2], args[3], args[4], args[5], args[6]});
  guardBodies();
  const U size = count * 512 + 32;
  std::vector<unsigned char> bytes(m.at(node - 16, size),
                                   m.at(node - 16, size) + size);
  bytes.push_back(result);
  for (U i = 0; i < 16; ++i)
    bytes.push_back(m.get8(0x5f1910 + i));
  return bytes;
}
int main(int argc, char **argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
  check(argc == 2, "pinned executable");
  map_image(read(argv[1]));
  check(mmap((void *)Base, Size, PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1,
             0) != MAP_FAILED,
        "palette fixture map");
#else
  (void)argc;
  (void)argv;
#endif
  U cases = 0;
  std::uint64_t checksum = 1469598103934665603ULL;
  for (U entry : {0x584430u, 0x589280u})
    for (U count : {0u, 1u, 2u, 3u, 4u, 8u, 16u, 32u, 64u, 128u, 256u})
      for (U variant = 0; variant < 32; ++variant) {
        const auto native = palette(false, entry, count, variant);
#ifdef MNM_ORIGINAL_REFERENCE
        const auto original = palette(true, entry, count, variant);
        if (native != original) {
          std::cerr << "palette mismatch " << std::hex << entry << std::dec
                    << ' ' << count << ' ' << variant << '\n';
          for (U i = 0; i < native.size(); ++i)
            if (native[i] != original[i]) {
              std::cerr << "byte " << i << ' ' << U(native[i]) << ' '
                        << U(original[i]) << '\n';
              break;
            }
          return 1;
        }
#endif
        for (U value : native)
          checksum = (checksum ^ value) * 1099511628211ULL;
        ++cases;
      }
  std::cout << "{\"cases\":" << cases << ",\"checksum\":" << checksum << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
