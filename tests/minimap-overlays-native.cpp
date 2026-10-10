#include "../renderer/minimap/overlays.hpp"
#include "minimap-overlays-fixture.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <string>
using namespace mnm::render;
namespace {
MinimapPlane plane(const minimap_overlays_fixture::Case &c) {
  return {minimap_overlays_fixture::width(c), minimap_overlays_fixture::height(c), minimap_overlays_fixture::stride(c), minimap_overlays_fixture::background(c)};
}
MinimapOverlayView view(const minimap_overlays_fixture::Case &c) {
  return {c.w, c.h, c.cx, c.cy, c.rw, c.rh, minimap_overlays_fixture::originX(c), minimap_overlays_fixture::originY(c), unsigned(c.view), c.rgb555 != 0};
}
MinimapCameraOutline outline(const minimap_overlays_fixture::Case &c) {
  MinimapCameraOutline result{c.viewportW, c.viewportH, {}, true};
  const auto corners = minimap_overlays_fixture::corners(c);
  for (unsigned i = 0; i < 4; ++i) result.corners[i] = {corners[i][0], corners[i][1]};
  return result;
}
void selftest() {
  const minimap_overlays_fixture::Case c{0, 16, 16, 3, 2, 16, 16, 0, 0, 5, 6, 2, 1, 1, 1, 3, 256, 128, 17};
  unsigned refusals = 0;
  auto reject = [&](const std::function<void(MinimapPlane &, MinimapOverlayView &)> &run) {
    auto p = plane(c); auto v = view(c); const auto before = p.words;
    bool failed = false;
    try { run(p, v); } catch (const std::invalid_argument &) { failed = true; }
    if (!failed || before != p.words) throw std::runtime_error("Overlay refusal mutated storage or accepted invalid input");
    ++refusals;
  };
  const auto palette = minimap_overlays_fixture::palette(c);
  const MinimapCellMarker m{5, 6, 2, true};
  reject([&](auto &p, auto &v) { v.orientation = 4; drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { v.gridWidth = 0; drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { v.centerX = -1; drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { v.rotationHeight = 257; drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { p.stride = p.width - 1; drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { ++p.height; drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { auto bad = m; bad.paletteIndex = 9; drawMinimapCellMarker(p, v, bad, palette, true); });
  reject([&](auto &p, auto &v) { auto bad = m; bad.x = -4097; drawMinimapCellMarker(p, v, bad, palette, true); });
  reject([&](auto &p, auto &v) { v.originX = std::numeric_limits<int>::max(); drawMinimapCellMarker(p, v, m, palette, true); });
  reject([&](auto &p, auto &v) { v.originY = std::numeric_limits<int>::min(); drawMinimapCreatureMarkers(p, v, {{5,6,false}}, false); });
  reject([&](auto &p, auto &v) { drawMinimapCreatureMarkers(p, v, {{5,6,false},{16,6,false}}, false); });
  reject([&](auto &p, auto &v) { drawMinimapCreatureMarkers(p, v, std::vector<MinimapCreatureMarker>(1025, {5,6,false}), false); });
  reject([&](auto &p, auto &v) { auto c0 = outline(c); c0.skipOuterBorder = false; drawMinimapCameraOutline(p, v, c0); });
  reject([&](auto &p, auto &v) { auto c0 = outline(c); c0.viewportWidth = 8193; drawMinimapCameraOutline(p, v, c0); });
  reject([&](auto &p, auto &v) { auto c0 = outline(c); c0.corners[3].xDirection = 0; drawMinimapCameraOutline(p, v, c0); });
  reject([&](auto &p, auto &v) { auto c0 = outline(c); c0.viewportHeight = 8192; drawMinimapCameraOutline(p, v, c0); });
  std::cout << refusals << " atomic overlay refusals passed\n";
}
}
int main(int argc, char **argv) try {
  if (argc == 2 && std::string(argv[1]) == "--selftest") { selftest(); return 0; }
  if (argc != 3) throw std::runtime_error("Expected overlay cases and output");
  std::ofstream out(argv[2], std::ios::binary);
  for (const auto &c : minimap_overlays_fixture::cases(argv[1])) {
    auto p = plane(c); const auto v = view(c);
    std::array<std::uint32_t, 3> result{};
    if (c.kind == 0) result[0] = drawMinimapCellMarker(p, v, {c.x,c.y,unsigned(c.index),c.emphasized != 0}, minimap_overlays_fixture::palette(c), c.flash != 0);
    else if (c.kind == 1) {
      std::vector<MinimapCreatureMarker> markers;
      for (const auto &m : minimap_overlays_fixture::markers(c)) markers.push_back({m.x,m.y,m.hidden});
      const auto r = drawMinimapCreatureMarkers(p, v, markers, c.fog != 0);
      result = {unsigned(r.gridWidth), unsigned(r.gridHeight), 0};
    } else {
      const auto r = drawMinimapCameraOutline(p, v, outline(c));
      result = {unsigned(r.originX), unsigned(r.originY), 0};
    }
    minimap_overlays_fixture::save(out, p.words, result);
  }
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
