#include "terrain_entity_sections.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
void extent(const assets::MapAsset &m) {
  if (!m.width || m.width > 128 || !m.height || m.height > 128 || !m.layers ||
      m.layers > 32 ||
      m.cells.size() != std::size_t(m.width) * m.height * m.layers)
    throw std::invalid_argument("Entity section map dimensions");
}
std::size_t index(const assets::MapAsset &m, unsigned x, unsigned y,
                  unsigned z) {
  return (std::size_t(z) * m.height + y) * m.width + x;
}
} // namespace
TerrainEntitySectionResult
copyTerrainEntitySection(assets::MapAsset &dst, const assets::MapAsset &src,
                         const assets::TerrainCatalog &t,
                         const assets::Animation &animation, unsigned side,
                         unsigned sx, unsigned sy, unsigned dx, unsigned dy,
                         unsigned rotation) {
  extent(dst);
  extent(src);
  if (!side || rotation > 3 || src.layers > dst.layers || sx > src.width ||
      sy > src.height || side > src.width - sx || side > src.height - sy ||
      dx > dst.width || dy > dst.height || side > dst.width - dx ||
      side > dst.height - dy)
    throw std::out_of_range("Entity section copy extent/rotation");
  struct Patch {
    std::size_t at;
    assets::MapCell cell;
  };
  std::vector<Patch> patches;
  patches.reserve(std::size_t(side) * side * src.layers);
  TerrainEntitySectionResult result;
  for (unsigned z = 0; z < src.layers; ++z)
    for (unsigned y = 0; y < side; ++y)
      for (unsigned x = 0; x < side; ++x) {
        auto c = src.cells[index(src, sx + x, sy + y, z)];
        bool reset = false;
        if (rotation) {
          if (c.flags10 & 8) {
            const auto selection =
                rotateTerrainLinkedDefinition(c, animation, t, rotation);
            if (selection.action == TerrainLinkedRotationAction::replace)
              c.definition = selection.definition;
            else if (selection.action == TerrainLinkedRotationAction::reset) {
              reset = true;
              c.definition = 0;
              c.flags10 &= 0xfe64;
              c.flags8 &= 0x80fc;
            }
          } else {
            const auto &r = t.records.at(c.definition);
            const unsigned at = 0x94 - 4 * rotation;
            c.definition =
                std::uint16_t(r[at]) | (std::uint16_t(r[at + 1]) << 8);
          }
          for (unsigned i = 0; i < rotation; ++i)
            c.flags8 =
                std::uint16_t((c.flags8 & 0xc3ff) | ((c.flags8 & 0x1c00) << 1) |
                              ((c.flags8 & 0x2000) >> 3));
        }
        unsigned tx = x, ty = y;
        if (rotation == 1) {
          tx = side - 1 - y;
          ty = x;
        } else if (rotation == 2) {
          tx = side - 1 - x;
          ty = side - 1 - y;
        } else if (rotation == 3) {
          tx = y;
          ty = side - 1 - x;
        }
        const auto at = index(dst, dx + tx, dy + ty, z);
        patches.push_back({at, c});
        if (reset)
          result.resetCells.push_back(at);
      }
  for (const auto &patch : patches)
    dst.cells[patch.at] = patch.cell;
  return result;
}
assets::MapAsset assembleTerrainEntityRegion(
    unsigned columns, unsigned rows, unsigned side, unsigned layers,
    const std::vector<TerrainSection> &sections,
    const assets::TerrainCatalog &t, const assets::Animation &animation) {
  if (!columns || !rows || !side || side > 128 || columns > 128 / side ||
      rows > 128 / side || !layers || layers > 32 ||
      sections.size() != std::size_t(columns) * rows)
    throw std::invalid_argument("Entity section complete grid dimensions");
  assets::MapAsset map;
  map.width = columns * side;
  map.height = rows * side;
  map.layers = layers;
  map.cells.resize(std::size_t(map.width) * map.height * layers);
  for (auto &c : map.cells)
    c.references = {0xffff, 0xffff, 0xffff};
  std::vector<bool> used(columns * rows);
  for (const auto &s : sections) {
    if (!s.source || s.source->layers > layers || s.column >= columns ||
        s.row >= rows || used[s.row * columns + s.column])
      throw std::invalid_argument("Entity section grid coverage/layers");
    used[s.row * columns + s.column] = true;
    copyTerrainEntitySection(map, *s.source, t, animation, side, s.sourceX,
                             s.sourceY, s.column * side, s.row * side,
                             s.rotation);
  }
  return map;
}
} // namespace mnm::reconstruction
