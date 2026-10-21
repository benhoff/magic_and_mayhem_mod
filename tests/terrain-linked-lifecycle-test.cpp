#include "terrain_linked_lifecycle.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool ok) {
  if (!ok)
    throw std::runtime_error("Terrain linked lifecycle test");
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
  std::vector<assets::AnimationRecord> sequence = {
      {1, 2, {}}, {0, 1, {}}, {5, 7, {}}, {0, 2, {}}, {6, 0, {}}};
  reconstruction::AnimationControllerState state;
  state.forward.pc = 5;
  check(reconstruction::terrainLinkedTerminalFrame(sequence, state) == 2);
  state.forward.pc = 3;
  check(reconstruction::terrainLinkedTerminalFrame(sequence, state) == 1);
  state.forward.pc = 1;
  check(reconstruction::terrainLinkedTerminalFrame(sequence, state) == -1);
  state.forward.pc = 0;
  refused([&] { reconstruction::terrainLinkedTerminalFrame(sequence, state); });
  state.reverse = true;
  check(reconstruction::terrainLinkedTerminalFrame(sequence, state) == 1);
  state.forward.pc = 3;
  check(reconstruction::terrainLinkedTerminalFrame(sequence, state) == -1);
  state.forward.pc = 4;
  refused([&] { reconstruction::terrainLinkedTerminalFrame(sequence, state); });
  assets::MapAsset map;
  map.width = map.height = map.layers = 3;
  map.cells.resize(27);
  for (auto &c : map.cells)
    c.references = {0xffff, 0xffff, 0xffff};
  auto &c = map.cells[13];
  c.definition = 1;
  c.references[1] = 7;
  c.flags8 = 0x1234;
  assets::TerrainCatalog catalog;
  catalog.records.resize(3);
  catalog.records[1][0x94] = 7;
  catalog.records[2][0x94] = 16;
  auto slow = map;
  check(!reconstruction::setTerrainLinkedFrame(slow, 13, catalog, 2).accepted);
  auto written = reconstruction::setTerrainAnimationFrame(map, 13, catalog, 2);
  check(written.accepted && !written.redrawDirty && c.definition == 2 &&
        (c.flags10 & 1) && c.flags8 == 0x1234);
  written = reconstruction::setTerrainAnimationFrame(map, 13, catalog, 1);
  check(written.accepted && c.definition == 1 && !(c.flags10 & 1) &&
        c.flags8 == 0x1234);
  written = reconstruction::setTerrainAnimationFrame(map, 13, catalog, 0);
  check(written.accepted && written.redrawDirty && c.definition == 0 &&
        c.flags8 != 0x1234);
  auto before = map.cells[13];
  refused(
      [&] { reconstruction::setTerrainAnimationFrame(map, 13, catalog, 3); });
  check(map.cells[13].definition == before.definition &&
        map.cells[13].flags10 == before.flags10);
  check(
      !reconstruction::setTerrainAnimationFrame(map, 13, catalog, 4).accepted);
  catalog.records[0][0x94] = 16;
  c.definition = 1;
  c.flags8 = 0x1234;
  c.flags10 = 2;
  written = reconstruction::setTerrainAnimationFrame(map, 13, catalog, 0);
  check(written.accepted && !written.redrawDirty && c.flags10 == 3 &&
        c.flags8 == 0x1234);
  catalog.records.resize(65536);
  c.references[1] = 0xffff;
  check(reconstruction::setTerrainAnimationFrame(map, 13, catalog, 65535)
            .accepted &&
        c.definition == 65535);
  check(
      reconstruction::setTerrainLinkedFrame(map, 13, catalog, 65535).accepted &&
      c.definition == 65535);
  refused([&] {
    reconstruction::setTerrainAnimationFrame(map, 13, catalog, 65536);
  });
  refused(
      [&] { reconstruction::setTerrainLinkedFrame(map, 13, catalog, 65536); });
  reconstruction::NoCdAnimationController controller(sequence);
  controller.start();
  std::int32_t event = 0;
  for (unsigned i = 0; i < 32 && event != 1; ++i)
    event = controller.tick();
  check(event == 1 && reconstruction::terrainLinkedTerminalFrame(
                          sequence, controller.state()) == 2);
  std::cout << "Linked fast-writer barriers/classification, zero-only refresh "
               "and terminal frame selection passed\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
