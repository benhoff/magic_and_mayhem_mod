#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "minimap-overlays-fixture.hpp"

int main(int argc, char **argv) try {
  if (argc != 4) throw std::runtime_error("Expected pinned PE, cases, output");
  map_image(read(argv[1]));
  const auto cases = minimap_overlays_fixture::cases(argv[2]);
  // Exclude terrain only from full outline calls. Signature check precedes the
  // private mapped-image adapter mutation; no original file is modified.
  auto *terrain = reinterpret_cast<unsigned char *>(0x553a40);
  if (std::memcmp(terrain, "\x83\xec\x20\x56\x8b\xf1", 6)) throw std::runtime_error("Original terrain bytes changed");
  *terrain = 0xc3;
  const unsigned outlines[] = {0x5527a0, 0x552b50, 0x552f20, 0x5532f0};
  for (auto address : outlines)
    if (std::memcmp(reinterpret_cast<void *>(address), "\x83\xec\x28\x53\x55\x56", 6)) throw std::runtime_error("Original outline bytes changed");
  if (std::memcmp(reinterpret_cast<void *>(0x5536c0), "\x53\x8b\x1d\x94\x54\x6c\x00", 7) ||
      std::memcmp(reinterpret_cast<void *>(0x553850), "\x83\xec\x08\x53\x55", 5)) throw std::runtime_error("Original marker bytes changed");
  std::ofstream out(argv[3], std::ios::binary);
  for (const auto &c : cases) {
    auto pixels = minimap_overlays_fixture::background(c);
    std::vector<unsigned char> object(0x6210);
    auto field = [&](unsigned at, std::uint32_t value) { std::memcpy(object.data() + at, &value, 4); };
    auto get = [&](unsigned at) { std::uint32_t value; std::memcpy(&value, object.data() + at, 4); return value; };
    const auto palette = minimap_overlays_fixture::palette(c);
    std::memcpy(object.data() + 0xcb, palette.data(), palette.size() * 2);
    field(0xc5, c.view); field(0xdf, reinterpret_cast<std::uintptr_t>(pixels.data()));
    field(0xeb, minimap_overlays_fixture::stride(c));
    field(0xf7, minimap_overlays_fixture::originX(c)); field(0xfb, minimap_overlays_fixture::originY(c));
    field(0x103, c.rw); field(0x107, c.rh); field(0x61a1, c.cx); field(0x61a5, c.cy);
    field(0x6141, c.flash); field(0x6207, 1);
    field(0x6199, 0x13572468); field(0x619d, 0x24681357);
    global(0x6c5494, c.w); global(0x6c5498, c.h); global(0x6e1f88, c.rgb555);
    global(0x5e1404, c.fog); global(0x5e18f0, unsigned(-1));
    // Resolve independent owned visibility via the original source lookup.
    const auto markers = minimap_overlays_fixture::markers(c);
    std::vector<std::uint32_t> rawMarkers;
    std::vector<signed char> states(4 * 128 * c.w, 1);
    for (unsigned i = 0; i < 128; ++i) global(0x6cb942 + i * 4, i * c.w);
    for (unsigned i = 0; i < 32; ++i) global(0x6cb8c2 + i * 4, i * 128 * c.w);
    for (unsigned i = 0; i < markers.size(); ++i) {
      const auto &m = markers[i];
      if (m.x < 0 || m.x >= c.w || m.y < 0 || m.y >= c.h) throw std::runtime_error("Invalid original marker lookup");
      rawMarkers.insert(rawMarkers.end(), {unsigned(m.x), unsigned(m.y), i * 2});
      states[i * 128 * c.w + m.y * c.w + m.x] = m.hidden ? -1 : 1;
    }
    field(0x614d, rawMarkers.empty() ? 0 : reinterpret_cast<std::uintptr_t>(rawMarkers.data()));
    field(0x6151, rawMarkers.empty() ? 0 : reinterpret_cast<std::uintptr_t>(rawMarkers.data() + rawMarkers.size()));
    global(0x6c5c5c, reinterpret_cast<std::uintptr_t>(states.data()));
    std::vector<std::uint32_t> rows(minimap_overlays_fixture::height(c));
    for (unsigned y = 0; y < rows.size(); ++y) rows[y] = reinterpret_cast<std::uintptr_t>(pixels.data()) +
        ((minimap_overlays_fixture::originY(c) + y) * minimap_overlays_fixture::stride(c) + minimap_overlays_fixture::originX(c)) * 2;
    field(0xf3, reinterpret_cast<std::uintptr_t>(rows.data()));
    const auto corners = minimap_overlays_fixture::corners(c);
    for (unsigned i = 0; i < 4; ++i) { field(0x6121 + i * 8, corners[i][0]); field(0x6125 + i * 8, corners[i][1]); }
    int rectangle[] = {5, 7, 5 + c.viewportW, 7 + c.viewportH};
    global(0x6c482c, reinterpret_cast<std::uintptr_t>(rectangle));
    const auto beforeObject = object;
    const auto beforeStates = states;
    const auto beforeMarkers = rawMarkers, beforeRows = rows;
    std::array<std::uint32_t, 3> result{};
    if (c.kind == 0) {
      using Draw = std::uintptr_t(__attribute__((thiscall)) *)(void *, int, int, unsigned, unsigned);
      const auto pointer = reinterpret_cast<Draw>(0x5536c0)(object.data(), c.x, c.y, c.index, c.emphasized);
      const auto base = reinterpret_cast<std::uintptr_t>(pixels.data());
      if (pointer < base || (pointer - base) % 2 || (pointer - base) / 2 >= pixels.size()) throw std::runtime_error("Original marker return outside destination");
      result[0] = (pointer - base) / 2;
    } else if (c.kind == 1) {
      using Draw = void(__attribute__((thiscall)) *)(void *);
      reinterpret_cast<Draw>(0x553850)(object.data());
      result = {get(0x6199), get(0x619d), 0};
    } else {
      using Draw = void(__attribute__((thiscall)) *)(void *);
      reinterpret_cast<Draw>(outlines[c.view])(object.data());
      result = {get(0xf7), get(0xfb), 0};
    }
    auto expected = beforeObject;
    if (c.kind == 1) {
      const unsigned w = c.w, h = c.h;
      std::memcpy(expected.data() + 0x6199, &w, 4); std::memcpy(expected.data() + 0x619d, &h, 4);
    } else if (c.kind == 2 && c.view) {
      const unsigned x = minimap_overlays_fixture::stride(c) - (c.view == 3 ? c.rw : c.rh) - 8, y = 6;
      std::memcpy(expected.data() + 0xf7, &x, 4); std::memcpy(expected.data() + 0xfb, &y, 4);
    }
    if (object != expected || states != beforeStates || rawMarkers != beforeMarkers || rows != beforeRows)
      throw std::runtime_error("Unexpected original overlay source/object mutation");
    minimap_overlays_fixture::save(out, pixels, result);
  }
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
