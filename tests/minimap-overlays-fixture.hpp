#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>
namespace minimap_overlays_fixture {
struct Case {
  int kind, w, h, cx, cy, rw, rh, view, rgb555, x, y, index,
      emphasized, flash, fog, pattern, viewportW, viewportH, seed;
};
inline std::vector<Case> cases(const char *path) {
  std::ifstream in(path); std::vector<Case> result; Case c{};
  while (in >> c.kind >> c.w >> c.h >> c.cx >> c.cy >> c.rw >> c.rh >> c.view >> c.rgb555 >>
         c.x >> c.y >> c.index >> c.emphasized >> c.flash >> c.fog >> c.pattern >> c.viewportW >> c.viewportH >> c.seed) {
    if (c.kind < 0 || c.kind > 2 || c.w < 1 || c.w > 128 || c.h < 1 || c.h > 128 ||
        c.cx < 0 || c.cx >= c.w || c.cy < 0 || c.cy >= c.h || c.rw < 0 || c.rw > 128 || c.rh < 0 || c.rh > 128 ||
        c.view < 0 || c.view > 3 || c.rgb555 < 0 || c.rgb555 > 1 || c.index < 0 || c.index >= 9 ||
        c.emphasized < 0 || c.emphasized > 1 || c.flash < 0 || c.flash > 1 || c.fog < 0 || c.fog > 1 ||
        c.pattern < 0 || c.pattern > 3 || c.viewportW < 0 || c.viewportW > 640 || c.viewportH < 0 || c.viewportH > 256 || c.seed < 0)
      throw std::runtime_error("Invalid overlay fixture case");
    result.push_back(c);
  }
  if (!in.eof() || result.empty()) throw std::runtime_error("Invalid overlay fixture file");
  return result;
}
inline int extent(const Case &c) { return std::max({c.w, c.h, c.rw, c.rh}); }
inline int width(const Case &c) { return 2 * extent(c) + 64; }
inline int height(const Case &c) { return extent(c) + 64; }
inline int stride(const Case &c) { return width(c) + 7; }
inline int originX(const Case &c) { return extent(c) + 24; }
inline int originY(const Case &) { return 12; }
inline std::vector<std::uint16_t> background(const Case &c) {
  std::vector<std::uint16_t> result(stride(c) * height(c));
  for (unsigned i = 0; i < result.size(); ++i) result[i] = std::uint16_t(i * 1273 + c.seed * 197 + 113);
  return result;
}
inline std::array<std::uint16_t, 9> palette(const Case &c) {
  std::array<std::uint16_t, 9> result{};
  for (unsigned i = 0; i < result.size(); ++i) result[i] = std::uint16_t(i * 9137 + c.seed * 307 + 941);
  return result;
}
struct Marker { int x, y; bool hidden; };
inline std::vector<Marker> markers(const Case &c) {
  if (!c.pattern) return {};
  if (c.pattern < 3) return {{c.x, c.y, c.pattern == 2}};
  return {{0, 0, true}, {c.w - 1, c.h - 1, false}, {c.x, c.y, false}, {c.x, c.y, true}};
}
inline std::array<std::array<int, 2>, 4> corners(const Case &c) {
  const std::array<std::array<int, 2>, 4> base = {{{-1,-1},{1,-1},{1,1},{-1,1}}};
  auto result = base;
  for (unsigned i = 0; i < 4; ++i) result[i] = base[(i + c.seed % 4) % 4];
  return result;
}
inline void word(std::ofstream &out, std::uint32_t value, unsigned bytes) {
  for (unsigned i = 0; i < bytes; ++i) out.put(char(value >> (i * 8)));
}
inline void save(std::ofstream &out, const std::vector<std::uint16_t> &pixels,
                 std::array<std::uint32_t, 3> state) {
  for (auto p : pixels) word(out, p, 2);
  for (auto s : state) word(out, s, 4);
  if (!out) throw std::runtime_error("Overlay fixture output failed");
}
}
