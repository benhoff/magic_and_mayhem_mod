#pragma once
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>
namespace minimap_fixture {
struct Case { int w, h, cx, cy; unsigned view, fog, seed; };
inline std::vector<Case> cases(const char *path) {
  std::ifstream in(path);
  std::vector<Case> result;
  Case c{};
  while (in >> c.w >> c.h >> c.cx >> c.cy >> c.view >> c.fog >> c.seed) {
    if (c.w < 2 || c.w > 256 || c.w % 2 || c.h < 1 || c.h > 256 ||
        c.cx < 0 || c.cx >= c.w || c.cy < 0 || c.cy >= c.h || c.view > 3 || c.fog > 3)
      throw std::runtime_error("Invalid minimap fixture case");
    result.push_back(c);
  }
  if (!in.eof() || result.empty()) throw std::runtime_error("Invalid fixture file");
  return result;
}
inline unsigned width(const Case &c) { return 2 * (c.w + c.h) + 8; }
inline unsigned height(const Case &c) { return c.w + c.h + 8; }
inline unsigned stride(const Case &c) { return width(c) + 7; }
inline int rootX(const Case &c) { return c.w + c.h + 2; }
inline int rootY(const Case &c) { return c.h + 2; }
inline std::vector<std::uint16_t> background(const Case &c, bool history) {
  std::vector<std::uint16_t> p(stride(c) * height(c));
  for (unsigned i = 0; i < p.size(); ++i)
    p[i] = std::uint16_t(i * (history ? 2137 : 1273) + c.seed * 197 + (history ? 941 : 113));
  return p;
}
inline std::uint16_t colour(const Case &c, unsigned i) { return std::uint16_t(i * 3137 + c.seed * 211); }
inline bool hidden(const Case &c, unsigned i) { return c.fog == 2 || (c.fog == 1 && (i + c.seed) % 3 == 0); }
inline void save(std::ofstream &out, const std::vector<std::uint16_t> &p) {
  for (auto v : p) { out.put(char(v)); out.put(char(v >> 8)); }
  if (!out) throw std::runtime_error("Fixture output failed");
}
}
