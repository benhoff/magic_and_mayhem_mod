#include "terrain_entity_sections.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool ok) {
  if (!ok)
    throw std::runtime_error("Terrain entity section test");
}
template <class F> static void refused(F f) {
  bool caught = false;
  try {
    f();
  } catch (const std::exception &) {
    caught = true;
  }
  check(caught);
}
int main() try {
  assets::MapAsset source;
  source.width = source.height = 2;
  source.layers = 1;
  source.cells.resize(4);
  for (unsigned i = 0; i < 4; ++i) {
    source.cells[i].definition = i;
    source.cells[i].references = {0xffff, 0xffff, 0xffff};
    source.cells[i].flags8 = std::uint16_t(0x400u << i);
  }
  source.cells[0].flags10 = 8;
  assets::TerrainCatalog catalog;
  catalog.records.resize(5);
  for (auto &r : catalog.records) {
    r[0x88] = 4;
    r[0x8c] = 4;
    r[0x90] = 4;
  }
  assets::Animation animation;
  animation.starts = {0, 2, 4, 6, 8};
  animation.records = {{0, 1, {}}, {6, 0, {}}, {0, 4, {}}, {6, 0, {}},
                       {0, 2, {}}, {6, 0, {}}, {6, 0, {}}, {6, 0, {}}};
  for (unsigned rotation = 0; rotation < 4; ++rotation) {
    auto dst = source;
    auto result = reconstruction::copyTerrainEntitySection(
        dst, source, catalog, animation, 2, 0, 0, 0, 0, rotation);
    check(result.resetCells.empty());
    unsigned linked = 0;
    for (const auto &c : dst.cells)
      if (c.flags10 & 8) {
        ++linked;
        check(c.definition == (rotation ? 1 : 0));
      }
    check(linked == 1 && source.cells[0].definition == 0);
    if (rotation) {
      auto ordinarySource = source;
      ordinarySource.cells[0].flags10 = 0;
      auto ordinary = ordinarySource;
      reconstruction::copyTerrainSection(ordinary, ordinarySource, catalog, 2,
                                         0, 0, 0, 0, rotation);
      for (unsigned i = 0; i < 4; ++i)
        if (!(dst.cells[i].flags10 & 8))
          check(dst.cells[i].definition == ordinary.cells[i].definition &&
                dst.cells[i].flags8 == ordinary.cells[i].flags8);
    }
  }
  auto alias = source;
  reconstruction::copyTerrainEntitySection(alias, alias, catalog, animation, 2,
                                           0, 0, 0, 0, 1);
  check(alias.cells[1].flags10 & 8);
  source.cells[0].definition = 99;
  auto dst = source;
  auto reset = reconstruction::copyTerrainEntitySection(
      dst, source, catalog, animation, 2, 0, 0, 0, 0, 1);
  check(reset.resetCells == std::vector<std::size_t>{1} &&
        dst.cells[1].definition == 0 && !(dst.cells[1].flags10 & 8));
  auto saved = dst;
  source.cells[3].definition = 99;
  refused([&] {
    reconstruction::copyTerrainEntitySection(dst, source, catalog, animation, 2,
                                             0, 0, 0, 0, 1);
  });
  for (unsigned i = 0; i < 4; ++i)
    check(dst.cells[i].definition == saved.cells[i].definition &&
          dst.cells[i].flags10 == saved.cells[i].flags10);
  source.cells[3].definition = 3;
  auto region = reconstruction::assembleTerrainEntityRegion(
      2, 1, 2, 2, {{&source, 0, 0, 0, 0, 0}, {&source, 0, 0, 1, 0, 1}}, catalog,
      animation);
  check(region.width == 4 && region.height == 2 && region.layers == 2 &&
        region.cells[8].references[1] == 0xffff);
  refused([&] {
    reconstruction::assembleTerrainEntityRegion(
        2, 1, 2, 1, {{&source, 0, 0, 0, 0, 0}, {&source, 0, 0, 0, 0, 1}},
        catalog, animation);
  });
  std::cout << "Linked/ordinary section copy, source aliasing, reset "
               "provenance and atomic failures passed\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
