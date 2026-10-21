#include "entity_map.hpp"
#include "../../reconstruction/rendering/terrain_linked_lifecycle.hpp"
#include "../../reconstruction/rendering/terrain_map.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>
namespace mnm::scene {
namespace {
std::uint32_t word(const assets::TerrainCatalog &t, unsigned d, unsigned o) {
  const auto &r = t.records.at(d);
  return std::uint32_t(r[o]) | (std::uint32_t(r[o + 1]) << 8) |
         (std::uint32_t(r[o + 2]) << 16) | (std::uint32_t(r[o + 3]) << 24);
}
game::Point point(const assets::MapAsset &m, std::size_t i) {
  return {int(i % m.width), int(i / m.width % m.height),
          int(i / (m.width * m.height))};
}
void validCell(const assets::MapAsset &m, std::size_t i) {
  if (i >= m.cells.size())
    throw std::out_of_range("Runtime map cell");
}
unsigned pool(CellReference r) {
  auto p = unsigned(r);
  if (p > 2)
    throw std::invalid_argument("Runtime map reference family");
  return p;
}
} // namespace
EntityMap::State::State(std::uint32_t capacity)
    : world(capacity, game::Limits{capacity, capacity, 64ULL * 1024 * 1024}) {}
std::vector<assets::AnimationRecord>
EntityMap::sequence(std::uint32_t n) const {
  const auto &a = *animation_;
  if (n >= a.starts.size() || a.starts.size() - n < 2 ||
      a.starts[n] >= a.starts[n + 1] || a.starts[n + 1] > a.records.size())
    throw std::out_of_range("Runtime map animation sequence");
  return {a.records.begin() + a.starts[n], a.records.begin() + a.starts[n + 1]};
}
void EntityMap::budget(const State &s) const {
  if (s.linked.size() > limits_.linked)
    throw std::length_error("Runtime linked entity limit");
  std::uint64_t memberships = 0;
  for (const auto &pair : s.entities) {
    const auto count = pair.second.footprint.capacity();
    const auto charge =
        std::uint64_t(count) * sizeof(std::size_t) + sizeof(MapEntity);
    if (charge > limits_.membershipBytes - memberships)
      throw std::length_error("Runtime map membership budget");
    memberships += charge;
  }
  std::uint64_t bytes = 0;
  for (const auto &pair : s.linked) {
    auto charge = pair.second.animation.retainedRecordBytes();
    if (charge > limits_.animationBytes - bytes)
      throw std::length_error("Runtime linked animation budget");
    bytes += charge;
  }
}
void EntityMap::changed(State &s, bool navigation) const {
  if (s.revision == std::numeric_limits<std::uint64_t>::max() ||
      (navigation &&
       s.navigationRevision == std::numeric_limits<std::uint64_t>::max()))
    throw std::overflow_error("Runtime map revision exhausted");
  ++s.revision;
  if (navigation)
    ++s.navigationRevision;
  s.redraw = true;
}
void EntityMap::rebuild(State &s) const {
  for (auto &c : s.map.cells)
    c.references = {0xffff, 0xffff, 0xffff};
  s.occupied.assign(s.map.cells.size(), 0);
  for (const auto &pair : s.entities) {
    const auto &e = pair.second;
    const auto *entity = s.world.find(e.handle);
    if (!entity || entity->cleaned)
      throw std::invalid_argument("Stale runtime map generation");
    validCell(s.map, e.cell);
    const auto p = pool(e.reference);
    auto &head = s.map.cells[e.cell].references[p];
    if (p == 1 && head != 0xffff)
      throw std::invalid_argument("Multiple creatures in one runtime map cell");
    if (e.handle.slot >= 0xffff)
      throw std::length_error("Runtime map WORD reference capacity");
    if (head == 0xffff)
      head = std::uint16_t(e.handle.slot);
    if (p == 2) {
      s.occupied[e.cell] = 1;
      for (auto at : e.footprint) {
        validCell(s.map, at);
        s.occupied[at] = 1;
      }
    }
  }
  // Membership changes affect the empty flag even on the highest layer, which
  // the original local surface loop deliberately does not visit.
  for (std::size_t i = 0; i < s.map.cells.size(); ++i) {
    auto &c = s.map.cells[i];
    if (!c.definition &&
        c.references == std::array<std::uint16_t, 3>{0xffff, 0xffff, 0xffff} &&
        !(c.flags10 & 0x2000) && !s.occupied[i])
      c.flags8 |= 0x80;
    else
      c.flags8 &= 0xff7f;
  }
}
EntityMap::EntityMap(assets::MapAsset source, assets::TerrainCatalog catalog,
                     assets::Animation animation, assets::MpsAsset placements,
                     EntityMapLimits limits, unsigned rotation)
    : source_(std::make_shared<const assets::MapAsset>(std::move(source))),
      catalog_(
          std::make_shared<const assets::TerrainCatalog>(std::move(catalog))),
      animation_(
          std::make_shared<const assets::Animation>(std::move(animation))),
      placements_(std::move(placements)), limits_(limits),
      state_(limits.entities) {
  if (!limits.entities || limits.entities > 65535 || !source_->width ||
      source_->width > 128 || !source_->height || source_->height > 128 ||
      !source_->layers || source_->layers > 32 ||
      source_->cells.size() !=
          std::size_t(source_->width) * source_->height * source_->layers ||
      catalog_->records.empty() || catalog_->records.size() > 65536 ||
      rotation > 3 || animation_->records.size() > 65536 ||
      animation_->starts.size() > 65537 ||
      placements_.placements.size() > 65536)
    throw std::invalid_argument("Runtime entity map dimensions/capacity");
  state_.map = *source_;
  for (std::size_t i = 0; i < state_.map.cells.size(); ++i) {
    auto &c = state_.map.cells[i];
    // Persisted WORD references have no native generation/provider. Preserve
    // them in source(), then replace them with owned runtime membership only.
    c.references = {0xffff, 0xffff, 0xffff};
    c.flags10 &= 0xdfff;
    if ((c.flags10 & 8) && rotation) {
      auto result = reconstruction::rotateTerrainLinkedDefinition(
          c, *animation_, *catalog_, rotation);
      if (result.action == reconstruction::TerrainLinkedRotationAction::reset) {
        c.definition = 0;
        c.flags10 &= 0xfe67;
        c.flags8 &= 0x80fc;
      } else if (result.action ==
                 reconstruction::TerrainLinkedRotationAction::replace)
        c.definition = result.definition;
    }
    if (c.flags10 & 8) {
      const auto sequenceNumber = c.definition;
      reconstruction::NoCdAnimationController controller(
          sequence(sequenceNumber));
      controller.start();
      const auto frame = controller.sprite();
      if (!frame || *frame >= catalog_->records.size())
        throw std::out_of_range("Runtime linked initial terrain frame");
      c.definition = std::uint16_t(*frame);
      game::Entity entity;
      entity.family = game::Family::mapLinked;
      entity.type = sequenceNumber;
      auto at = point(state_.map, i);
      entity.x = at.x;
      entity.y = at.y;
      entity.z = at.z;
      auto handle = state_.world.spawn(std::move(entity));
      state_.linked.emplace(
          i, LinkedTerrain{handle, sequenceNumber, std::move(controller)});
    }
    if (c.definition >= catalog_->records.size())
      throw std::out_of_range("Runtime terrain definition");
    if (c.definition && word(*catalog_, c.definition, 0x94) == 16)
      c.flags10 |= 1;
    else
      c.flags10 &= 0xfffe;
  }
  budget(state_);
  reconstruction::deriveTerrainSurfaces(state_.map, *catalog_);
  rebuild(state_);
}
std::vector<game::Handle> EntityMap::references(std::size_t at,
                                                CellReference reference) const {
  validCell(state_.map, at);
  pool(reference);
  std::vector<game::Handle> result;
  for (const auto &pair : state_.entities)
    if (pair.second.cell == at && pair.second.reference == reference)
      result.push_back(pair.second.handle);
  return result;
}
game::Handle EntityMap::activate(game::Entity entity, CellReference reference,
                                 std::size_t at,
                                 std::vector<std::size_t> footprint,
                                 std::optional<std::size_t> placement) {
  validCell(state_.map, at);
  if (footprint.size() > state_.map.cells.size() ||
      (reference != CellReference::artifact && !footprint.empty()))
    throw std::length_error("Runtime map footprint limits");
  const auto p = pool(reference);
  const auto expected = p == 1   ? game::Family::creature
                        : p == 0 ? game::Family::missileEffect
                                 : game::Family::mapLinked;
  if (entity.family != expected || entity.cleaned)
    throw std::invalid_argument("Runtime entity/reference mismatch");
  if (placement && *placement >= placements_.placements.size())
    throw std::out_of_range("Runtime MPS activation identity");
  auto s = state_;
  auto where = point(s.map, at);
  entity.x = where.x;
  entity.y = where.y;
  entity.z = where.z;
  auto handle = s.world.spawn(std::move(entity));
  s.entities.emplace(handle.slot, MapEntity{handle, reference, at,
                                            std::move(footprint), placement});
  rebuild(s);
  budget(s);
  changed(s, true);
  publish(std::move(s));
  return handle;
}
void EntityMap::relocate(game::Handle handle, std::size_t at,
                         std::vector<std::size_t> footprint) {
  validCell(state_.map, at);
  auto s = state_;
  auto it = s.entities.find(handle.slot);
  if (it == s.entities.end() || !(it->second.handle == handle))
    throw std::invalid_argument("Stale runtime entity relocation");
  if (footprint.size() > s.map.cells.size() ||
      (it->second.reference != CellReference::artifact && !footprint.empty()))
    throw std::length_error("Runtime map footprint limits");
  it->second.cell = at;
  it->second.footprint = std::move(footprint);
  auto world = s.world.state();
  auto where = point(s.map, at);
  auto &entity = *world.slots.at(handle.slot).entity;
  entity.x = where.x;
  entity.y = where.y;
  entity.z = where.z;
  s.world.restore(std::move(world));
  rebuild(s);
  budget(s);
  changed(s, true);
  publish(std::move(s));
}
void EntityMap::retire(game::Handle handle) {
  auto s = state_;
  auto it = s.entities.find(handle.slot);
  if (it == s.entities.end() || !(it->second.handle == handle))
    throw std::invalid_argument("Stale runtime entity retirement");
  s.world.release(handle);
  s.entities.erase(it);
  rebuild(s);
  budget(s);
  changed(s, true);
  publish(std::move(s));
}
void EntityMap::reset(State &s, std::size_t at) const {
  validCell(s.map, at);
  auto &c = s.map.cells[at];
  if (word(*catalog_, c.definition, 0x94) == 16)
    c.flags10 &= 0xfffc;
  auto linked = s.linked.find(at);
  if (linked != s.linked.end()) {
    s.world.release(linked->second.handle);
    s.linked.erase(linked);
    c.flags10 &= 0xfff7;
    auto written = reconstruction::setTerrainLinkedFrame(s.map, at, *catalog_,
                                                         0, s.occupied);
    s.redraw |= written.redrawDirty;
  } else {
    if (c.flags10 & 8)
      throw std::logic_error("Linked runtime cell has no owned controller");
    c.definition = 0;
  }
  c.flags10 &= 0xfe6f;
  c.flags8 &= 0x80fc;
  rebuild(s);
  s.redraw |= reconstruction::refreshTerrainCellNeighborhood(
      s.map, at, *catalog_, s.occupied);
  changed(s, true);
}
void EntityMap::resetCell(std::size_t at) {
  auto s = state_;
  reset(s, at);
  publish(std::move(s));
}
std::vector<std::pair<std::size_t, std::int32_t>>
EntityMap::advanceLinked(const std::vector<std::size_t> &cells) {
  if (cells.empty())
    return {};
  std::set<std::size_t> unique;
  auto s = state_;
  std::vector<std::pair<std::size_t, std::int32_t>> events;
  for (auto at : cells) {
    if (!unique.insert(at).second)
      throw std::invalid_argument("Duplicate runtime linked tick");
    auto it = s.linked.find(at);
    if (it == s.linked.end())
      throw std::invalid_argument("Missing runtime linked controller");
    auto event = it->second.animation.tick();
    events.emplace_back(at, event);
    const auto frame =
        event == 1 ? std::uint32_t(reconstruction::terrainLinkedTerminalFrame(
                         sequence(it->second.sourceSequence),
                         it->second.animation.state()))
                   : it->second.animation.sprite().value();
    if (event == 1)
      s.map.cells.at(at).flags10 &= 0xfff7;
    const auto written = reconstruction::setTerrainAnimationFrame(
        s.map, at, *catalog_, frame, s.occupied);
    s.redraw |= written.redrawDirty;
    if (event == 1) {
      s.world.release(it->second.handle);
      s.linked.erase(it);
    }
    changed(s, true);
  }
  budget(s);
  publish(std::move(s));
  return events;
}
void EntityMap::publish(State state) {
  if (state.navigation)
    state.navigation->update(state.map);
  state_ = std::move(state);
}
void EntityMap::enableNavigation(MapNavigationConfig config) {
  auto s = state_;
  s.navigation.emplace(s.map, *catalog_, config);
  changed(s, false);
  publish(std::move(s));
}
void EntityMap::maintainNavigation(bool blocked) {
  if (!state_.navigation)
    throw std::logic_error("Runtime map navigation is not configured");
  auto candidate = *state_.navigation;
  candidate.step(blocked);
  state_.navigation = std::move(candidate);
}
void EntityMap::acknowledgeRedraw(std::uint64_t revision) {
  if (revision != state_.revision)
    throw std::invalid_argument("Stale runtime map redraw acknowledgement");
  state_.redraw = false;
}
} // namespace mnm::scene
