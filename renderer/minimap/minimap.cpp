#include "minimap.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::render {
namespace {
void plane(const MinimapPlane &p) {
  if (p.width <= 0 || p.width > 2048 || p.height <= 0 || p.height > 2048 ||
      p.stride < p.width || p.stride > 4096 ||
      p.words.size() != std::size_t(p.stride) * p.height)
    throw std::invalid_argument("Invalid owned minimap plane");
}
std::pair<int, int> position(int w, unsigned view, int row, int col) {
  switch (view) {
  case 0: return {-2 * (row / 2) + col, (row + 1) / 2 + col / 2};
  case 1: return {-w + 2 + 2 * (row / 2) + col, w / 2 - 1 + (row + 1) / 2 - col / 2};
  case 2: return {1 + 2 * (row / 2) - col, w - 1 - (row + 1) / 2 - col / 2};
  default: return {w - 1 - 2 * (row / 2) - col, w / 2 - (row + 1) / 2 + col / 2};
  }
}
std::size_t at(const MinimapPlane &p, std::int64_t x, std::int64_t y) {
  if (x < 0 || x >= p.width || y < 0 || y >= p.height)
    throw std::invalid_argument("Minimap terrain outside owned plane");
  return std::size_t(y) * p.stride + std::size_t(x);
}
}
void drawMinimapTerrain(MinimapPlane &destination, const MinimapTerrain &t,
                        const MinimapPlane *previous) {
  plane(destination);
  if (t.width <= 0 || t.width > 256 || t.width % 2 || t.height <= 0 ||
      t.height > 256 || t.orientation > 3 ||
      t.centerX < 0 || t.centerX >= t.width || t.centerY < 0 || t.centerY >= t.height ||
      t.colours.size() != std::size_t(t.width) * t.height ||
      (!t.hidden.empty() && t.hidden.size() != t.colours.size()))
    throw std::invalid_argument("Invalid minimap terrain input");
  if (previous) {
    plane(*previous);
    if (previous == &destination)
      throw std::invalid_argument("Minimap history aliases destination");
  }
  struct Write { std::size_t index; std::uint16_t colour; };
  std::vector<Write> writes;
  writes.reserve(t.colours.size());
  // Odd orientations shift the source start by one column, rather than
  // rotating a finished bitmap. Centres are bounded to the original wrap domain.
  const int sx = (t.centerX - t.width / 2 + int(t.orientation % 2) + t.width) % t.width;
  const int sy = (t.centerY - t.height / 2 + t.height) % t.height;
  for (int row = 0; row < t.height; ++row)
    for (int col = 0; col < t.width; ++col) {
      const auto [x, y] = position(t.width, t.orientation, row, col);
      const auto index = at(destination, std::int64_t(t.rootX) + x, std::int64_t(t.rootY) + y);
      const auto cell = std::size_t((sy + row) % t.height) * t.width + (sx + col) % t.width;
      std::uint16_t colour = t.colours[cell];
      if (!t.hidden.empty()) {
        if (t.hidden[cell] > 1) throw std::invalid_argument("Invalid minimap visibility");
        if (t.hidden[cell]) {
          if (!previous) throw std::invalid_argument("Minimap hidden cell lacks owned history");
          colour = previous->words[at(*previous, t.width - 1 + x, y)];
        }
      }
      writes.push_back({index, colour});
    }
  for (const auto &write : writes) destination.words[write.index] = write.colour;
}
}
