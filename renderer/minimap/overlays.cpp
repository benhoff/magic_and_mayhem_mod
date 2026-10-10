#include "overlays.hpp"
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace mnm::render {
namespace {
void validate(const MinimapPlane &p, const MinimapOverlayView &v) {
  if (p.width <= 0 || p.width > 2048 || p.height <= 0 || p.height > 2048 ||
      p.stride < p.width || p.stride > 4096 ||
      p.words.size() != std::size_t(p.stride) * p.height ||
      v.gridWidth <= 0 || v.gridWidth > 256 || v.gridHeight <= 0 || v.gridHeight > 256 ||
      v.centerX < 0 || v.centerX >= v.gridWidth || v.centerY < 0 || v.centerY >= v.gridHeight ||
      v.rotationWidth < 0 || v.rotationWidth > 256 ||
      v.rotationHeight < 0 || v.rotationHeight > 256 || v.orientation > 3)
    throw std::invalid_argument("Invalid owned minimap overlay view");
}
std::size_t slot(const MinimapPlane &p, std::int64_t x, std::int64_t y) {
  if (x < 0 || x >= p.width || y < 0 || y >= p.height)
    throw std::invalid_argument("Minimap overlay outside owned plane");
  return std::size_t(y) * p.stride + std::size_t(x);
}
int wrap(std::int64_t n, int size) { return int((n % size + size) % size); }
std::pair<int, int> project(const MinimapOverlayView &v, int x, int y) {
  int u = wrap(std::int64_t(x) + v.gridWidth / 2 - v.centerX, v.gridWidth);
  int z = wrap(std::int64_t(y) + v.gridHeight / 2 - v.centerY, v.gridHeight);
  switch (v.orientation) {
  case 1: { const int old = u; u = z; z = v.rotationWidth - old; break; }
  case 2: u = v.rotationWidth - u; z = v.rotationHeight - z; break;
  case 3: { const int old = u; u = v.rotationHeight - z; z = old; break; }
  default: break;
  }
  return {u - z, (u + z) / 2}; // C++ division truncates toward zero, like original.
}
struct Write { std::size_t index; std::uint16_t colour; };
void commit(MinimapPlane &p, const std::vector<Write> &writes) {
  for (const auto &w : writes) p.words[w.index] = w.colour;
}
}
std::size_t drawMinimapCellMarker(MinimapPlane &p, const MinimapOverlayView &v,
    const MinimapCellMarker &m, const std::array<std::uint16_t, 9> &palette, bool flash) {
  validate(p, v);
  if (m.paletteIndex >= palette.size() || m.x < -4096 || m.x > 4096 || m.y < -4096 || m.y > 4096)
    throw std::invalid_argument("Invalid minimap cell marker");
  const auto [x, y] = project(v, m.x, m.y);
  const bool ring = m.emphasized && flash;
  const std::array<std::pair<int, int>, 8> offsets = ring ?
      std::array<std::pair<int, int>, 8>{{{-1,-1},{2,-1},{1,0},{0,0},{0,1},{1,1},{2,2},{-1,2}}} :
      std::array<std::pair<int, int>, 8>{{{0,0},{1,0},{0,1},{1,1},{0,0},{0,0},{0,0},{0,0}}};
  std::vector<Write> writes;
  for (unsigned i = 0; i < (ring ? 8u : 4u); ++i)
    writes.push_back({slot(p, std::int64_t(v.originX) + x + offsets[i].first,
                              std::int64_t(v.originY) + y + offsets[i].second), palette[m.paletteIndex]});
  const auto returned = slot(p, std::int64_t(v.originX) + x + (ring ? 2 : 0),
                               std::int64_t(v.originY) + y + (ring ? 2 : 1));
  commit(p, writes);
  return returned;
}
MinimapCreatureResult drawMinimapCreatureMarkers(MinimapPlane &p,
    const MinimapOverlayView &v, const std::vector<MinimapCreatureMarker> &markers, bool fog) {
  validate(p, v);
  if (markers.size() > 1024) throw std::invalid_argument("Minimap creature count limit");
  const std::array<std::pair<int, int>, 13> offsets = {{{-1,-2},{0,-2},{1,-2},{2,-1},
      {-2,-1},{-2,0},{0,0},{2,0},{2,1},{-2,1},{-1,2},{0,2},{1,2}}};
  std::vector<Write> writes;
  unsigned drawn = 0;
  for (const auto &m : markers) {
    if (m.x < 0 || m.x >= v.gridWidth || m.y < 0 || m.y >= v.gridHeight)
      throw std::invalid_argument("Unnormalized minimap creature coordinate");
    if (fog && m.hidden) continue;
    const auto [x, y] = project(v, m.x, m.y);
    for (const auto &d : offsets)
      writes.push_back({slot(p, std::int64_t(v.originX) + x + d.first,
                               std::int64_t(v.originY) + y + d.second),
                        std::uint16_t(v.rgb555 ? 0x7e00 : 0xfc00)});
    ++drawn;
  }
  commit(p, writes);
  return {drawn, v.gridWidth, v.gridHeight};
}
MinimapOutlineResult drawMinimapCameraOutline(MinimapPlane &p,
    const MinimapOverlayView &v, const MinimapCameraOutline &c) {
  validate(p, v);
  if (!c.skipOuterBorder || c.viewportWidth < 0 || c.viewportWidth > 8192 ||
      c.viewportHeight < 0 || c.viewportHeight > 8192)
    throw std::invalid_argument("Unsupported minimap camera outline");
  const bool swapped = v.orientation == 1 || v.orientation == 2;
  const int a = (swapped ? v.rotationHeight : v.rotationWidth) / 2;
  const int b = (swapped ? v.rotationWidth : v.rotationHeight) / 2;
  const int originX = v.orientation == 0 ? v.originX :
      p.stride - (v.orientation == 3 ? v.rotationWidth : v.rotationHeight) - 8;
  const int originY = v.orientation == 0 ? v.originY : 6;
  const int sw = c.viewportWidth / 128 + 1, sh = c.viewportHeight / 64;
  std::vector<Write> writes;
  for (const auto &corner : c.corners) {
    const int dx = corner.xDirection, dy = corner.yDirection;
    if ((dx != -1 && dx != 1) || (dy != -1 && dy != 1))
      throw std::invalid_argument("Invalid minimap camera corner direction");
    const int x = (a + b) / 2 - dx * sw * 2 - b;
    const int y = (a + b) / 2 - 1 - dy * sh + (dy < 0);
    for (int i = 0; i < 4; ++i) {
      writes.push_back({slot(p, std::int64_t(originX) + x + i * dx, std::int64_t(originY) + y), std::uint16_t(v.rgb555 ? 0x200 : 0x400)});
      writes.push_back({slot(p, std::int64_t(originX) + x, std::int64_t(originY) + y + i * dy), std::uint16_t(v.rgb555 ? 0x200 : 0x400)});
    }
  }
  commit(p, writes);
  return {originX, originY};
}
}
