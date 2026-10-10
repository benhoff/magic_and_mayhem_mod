#include "../renderer/minimap/minimap.hpp"
#include "minimap-fixture.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <string>
using namespace mnm::render;
namespace {
MinimapTerrain terrain(const minimap_fixture::Case &c) {
  MinimapTerrain t{c.w, c.h, c.cx, c.cy, minimap_fixture::rootX(c), minimap_fixture::rootY(c), c.view, {}, {}};
  for (unsigned i = 0; i < unsigned(c.w * c.h); ++i) {
    t.colours.push_back(minimap_fixture::colour(c, i));
    if (c.fog) t.hidden.push_back(minimap_fixture::hidden(c, i));
  }
  return t;
}
MinimapPlane plane(const minimap_fixture::Case &c, bool old) {
  return {int(minimap_fixture::width(c)), int(minimap_fixture::height(c)), int(minimap_fixture::stride(c)), minimap_fixture::background(c, old)};
}
void selftest() {
  const minimap_fixture::Case c{4, 4, 3, 2, 0, 1, 17};
  unsigned refused = 0;
  auto reject = [&](const std::function<void(MinimapPlane &, MinimapTerrain &, MinimapPlane &)> &change) {
    auto d = plane(c, false), old = plane(c, true); auto t = terrain(c);
    const auto before = d.words;
    change(d, t, old);
    const auto input = d.words;
    bool failed = false;
    try { drawMinimapTerrain(d, t, &old); } catch (const std::invalid_argument &) { failed = true; }
    if (!failed || d.words != input) throw std::runtime_error("Refusal mutated destination or accepted invalid input");
    (void)before;
    ++refused;
  };
  reject([](auto &, auto &t, auto &) { t.orientation = 4; });
  reject([](auto &, auto &t, auto &) { t.width = 3; });
  reject([](auto &, auto &t, auto &) { t.height = 0; });
  reject([](auto &, auto &t, auto &) { t.centerX = -1; });
  reject([](auto &, auto &t, auto &) { t.centerY = t.height; });
  reject([](auto &, auto &t, auto &) { t.colours.pop_back(); });
  reject([](auto &, auto &t, auto &) { t.hidden.pop_back(); });
  reject([](auto &, auto &t, auto &) { t.hidden.back() = 2; });
  reject([](auto &, auto &t, auto &) { t.rootX = std::numeric_limits<int>::max(); });
  reject([](auto &, auto &t, auto &) { t.rootY = std::numeric_limits<int>::min(); });
  reject([](auto &d, auto &, auto &) { d.stride = d.width - 1; });
  reject([](auto &d, auto &, auto &) { d.words.pop_back(); });
  reject([](auto &, auto &, auto &old) { old.width = 1; });
  auto d = plane(c, false); auto t = terrain(c); const auto before = d.words;
  for (bool alias : {false, true}) {
    bool failed = false;
    try { drawMinimapTerrain(d, t, alias ? &d : nullptr); }
    catch (const std::invalid_argument &) { failed = true; }
    if (!failed || d.words != before) throw std::runtime_error("Missing/aliased history refusal failed");
    ++refused;
  }
  std::cout << refused << " atomic minimap refusals passed\n";
}
}
int main(int argc, char **argv) try {
  if (argc == 2 && std::string(argv[1]) == "--selftest") { selftest(); return 0; }
  if (argc != 3) throw std::runtime_error("Expected cases and output");
  std::ofstream out(argv[2], std::ios::binary);
  for (const auto &c : minimap_fixture::cases(argv[1])) {
    auto d = plane(c, false), old = plane(c, true); const auto prior = old.words;
    drawMinimapTerrain(d, terrain(c), c.fog ? &old : nullptr);
    if (prior != old.words) throw std::runtime_error("Native mutated auxiliary history");
    minimap_fixture::save(out, d.words);
  }
  return 0;
} catch (const std::exception &e) { std::cerr << e.what() << '\n'; return 1; }
