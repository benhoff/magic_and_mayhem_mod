#include "entity_map.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool ok, int line) {
  if (!ok)
    throw std::runtime_error("Runtime entity map test line " +
                             std::to_string(line));
}
#define check(...) require((__VA_ARGS__), __LINE__)
template <class F> static void refused(F action) {
  bool caught = false;
  try {
    action();
  } catch (const std::exception &) {
    caught = true;
  }
  check(caught);
}
static assets::MapAsset map() {
  assets::MapAsset m;
  m.width = m.height = m.layers = 3;
  m.cells.resize(27);
  for (auto &c : m.cells)
    c.references = {0xffff, 0xffff, 0xffff};
  m.cells[13].flags10 = 8;
  m.cells[13].definition = 1;
  m.cells[0].references[0] = 123;
  return m;
}
static assets::TerrainCatalog catalog() {
  assets::TerrainCatalog t;
  t.records.resize(4);
  t.records[2][0x94] = 16;
  return t;
}
static assets::Animation animation() {
  assets::Animation a;
  a.starts = {0, 2, 5};
  a.records = {{0, 0, {}}, {6, 0, {}}, {0, 2, {}}, {0, 3, {}}, {6, 0, {}}};
  return a;
}
int main() try {
  scene::EntityMap m(map(), catalog(), animation());
  check(m.source().cells[13].definition == 1 &&
        m.map().cells[13].definition == 2 &&
        m.linked().at(13).sourceSequence == 1 &&
        m.source().cells[0].references[0] == 123 &&
        m.map().cells[0].references[0] == 0xffff);
  auto linked = m.linked().at(13).handle;
  check(m.world().find(linked) != nullptr && m.entities().empty());
  const auto revision = m.revision();
  m.acknowledgeRedraw(revision);
  check(!m.redrawPending());
  m.advanceLinked({13});
  check(m.map().cells[13].definition == 2);
  m.advanceLinked({13});
  check(m.map().cells[13].definition == 3 && m.redrawPending() &&
        m.navigationRevision() > 1);
  const auto before = m.revision();
  refused([&] { m.advanceLinked({13, 13}); });
  check(m.revision() == before && m.linked().count(13));
  m.advanceLinked({13});
  check(m.map().cells[13].definition == 3 && !(m.map().cells[13].flags10 & 8) &&
        m.linked().empty() && !m.world().find(linked));
  game::Entity e;
  e.family = game::Family::creature;
  e.type = 10;
  auto creature = m.activate(e, scene::CellReference::creature, 13);
  check(m.map().cells[13].references[1] == creature.slot &&
        m.references(13, scene::CellReference::creature).front() == creature);
  const auto occupied = m.revision();
  refused([&] { m.activate(e, scene::CellReference::creature, 13); });
  check(m.revision() == occupied &&
        m.references(13, scene::CellReference::creature).size() == 1);
  m.relocate(creature, 14);
  check(m.map().cells[13].references[1] == 0xffff &&
        m.map().cells[14].references[1] == creature.slot &&
        m.world().find(creature)->x == 2);
  m.retire(creature);
  auto next = m.activate(e, scene::CellReference::creature, 13);
  check(!(next == creature));
  refused([&] { m.retire(creature); });
  refused([&] { m.relocate(creature, 12); });
  check(m.world().find(next) != nullptr);
  e.family = game::Family::missileEffect;
  auto effect1 = m.activate(e, scene::CellReference::effect, 13);
  auto effect2 = m.activate(e, scene::CellReference::effect, 13);
  check(m.references(13, scene::CellReference::effect).size() == 2 &&
        m.map().cells[13].references[0] == effect1.slot);
  m.retire(effect1);
  check(m.map().cells[13].references[0] == effect2.slot);
  e.family = game::Family::mapLinked;
  auto artifact = m.activate(e, scene::CellReference::artifact, 1, {0, 2});
  check(!(m.map().cells[0].flags8 & 0x80) && !(m.map().cells[2].flags8 & 0x80));
  m.retire(artifact);
  check(m.map().cells[0].flags8 & 0x80);
  const auto saved = m.revision();
  refused([&] { m.activate(e, scene::CellReference::artifact, 1, {999}); });
  check(m.revision() == saved);
  refused([&] { m.acknowledgeRedraw(revision); });
  scene::EntityMap reset(map(), catalog(), animation());
  auto old = reset.linked().at(13).handle;
  reset.resetCell(13);
  check(reset.linked().empty() && !reset.world().find(old) &&
        reset.map().cells[13].definition == 0 &&
        !(reset.map().cells[13].flags10 & 0x198));
  scene::EntityMapLimits limits;
  limits.linked = 0;
  refused([&] {
    scene::EntityMap limited(map(), catalog(), animation(), {}, limits);
  });
  limits.linked = 5;
  limits.animationBytes = 1;
  refused([&] {
    scene::EntityMap limited(map(), catalog(), animation(), {}, limits);
  });
  auto bad = map();
  bad.cells[13].definition = 99;
  refused([&] { scene::EntityMap broken(bad, catalog(), animation()); });
  assets::MpsAsset placements;
  placements.placements.resize(1);
  scene::EntityMap authored(map(), catalog(), animation(), placements);
  check(authored.placements().placements.size() == 1 &&
        authored.entities().empty());
  auto activated =
      authored.activate(e, scene::CellReference::artifact, 1, {}, 0);
  check(authored.entities().at(activated.slot).placement == 0);
  refused(
      [&] { authored.activate(e, scene::CellReference::artifact, 2, {}, 1); });
  assets::MapAsset lastMap;
  lastMap.width = lastMap.height = lastMap.layers = 1;
  lastMap.cells.resize(1);
  lastMap.cells[0].definition = 65535;
  lastMap.cells[0].references = {0xffff, 0xffff, 0xffff};
  assets::TerrainCatalog lastCatalog;
  lastCatalog.records.resize(65536);
  scene::EntityMap last(std::move(lastMap), std::move(lastCatalog), {});
  check(last.map().cells[0].definition == 65535);
  std::cout << "{\"success\":true,\"sequence_frame_ownership\":true,\"real_"
               "controller_retirement\":true,\"typed_generation_membership\":"
               "true,\"footprint_and_navigation_invalidation\":true,\"atomic_"
               "refusal\":true,\"explicit_mps_activation\":true}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
