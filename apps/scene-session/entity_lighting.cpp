#include "entity_lighting.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>
namespace mnm::scene {
namespace {
const game::Entity &actor(const game::State &world, game::Handle handle) {
  if (handle.slot >= world.slots.size() ||
      world.slots[handle.slot].generation != handle.generation ||
      !world.slots[handle.slot].entity)
    throw std::invalid_argument(
        "Stale entity light health/admission generation");
  return *world.slots[handle.slot].entity;
}
unsigned wrap(std::int32_t x, unsigned n) {
  auto v = x % std::int32_t(n);
  return unsigned(v < 0 ? v + std::int32_t(n) : v);
}
} // namespace
EntityLighting::EntityLighting(unsigned width, unsigned height, unsigned layers,
                               const assets::EffectLightingFields &fields,
                               std::vector<ObjectLightProfile> profiles,
                               reconstruction::TerrainLightingConfig config)
    : profiles_(std::move(profiles)), cycle_(width, height, layers, config),
      width_(width), height_(height), layers_(layers) {
  if (!width || !height || width > 128 || height > 128 || !layers ||
      layers > 32 || profiles_.size() > 4096)
    throw std::invalid_argument("Entity light map/profile limits");
  types_.reload(fields.values);
  std::set<std::pair<unsigned, unsigned>> unique;
  for (const auto &p : profiles_)
    if (unsigned(p.family) > 2 || p.family == game::Family::creature ||
        p.effectLightingType >= 89 ||
        !unique.emplace(unsigned(p.family), p.type).second)
      throw std::invalid_argument("Entity light profile mapping");
}
void EntityLighting::step(const game::State &world,
                          const game::CombatState &health,
                          EntityLightControls controls) {
  std::vector<std::pair<game::Handle, std::uint32_t>> values;
  values.reserve(health.fighters.size());
  for (const auto &fighter : health.fighters)
    values.emplace_back(fighter.actor, fighter.health);
  step(world, values, std::move(controls));
}
void EntityLighting::step(const game::State &world,
                          const game::CreatureEncounter &encounter,
                          EntityLightControls controls) {
  std::vector<std::pair<game::Handle, std::uint32_t>> values;
  values.reserve(encounter.actors.size());
  for (const auto &entry : encounter.actors)
    values.emplace_back(entry.handle, entry.health);
  step(world, values, std::move(controls));
}
void EntityLighting::step(
    const game::State &world,
    const std::vector<std::pair<game::Handle, std::uint32_t>> &health,
    EntityLightControls controls) {
  if (world.slots.size() > 65535 || health.size() > world.slots.size() ||
      controls.readmit.size() > world.slots.size() ||
      controls.camera.span > 256)
    throw std::length_error("Entity light snapshot limits");
  std::map<std::uint32_t, std::uint32_t> living;
  for (const auto &value : health) {
    if (actor(world, value.first).family != game::Family::creature ||
        !living.emplace(value.first.slot, value.second).second)
      throw std::invalid_argument("Duplicate/noncreature entity light health");
  }
  std::set<std::uint32_t> readmit;
  for (auto handle : controls.readmit) {
    const auto &entity = actor(world, handle);
    if (entity.family == game::Family::creature ||
        !readmit.insert(handle.slot).second)
      throw std::invalid_argument("Entity light readmission identity");
  }
  auto admissions = admission_;
  reconstruction::TerrainLightingSnapshot input;
  input.player = controls.player;
  input.changed = controls.changed;
  input.creaturesEnabled = controls.creatures;
  input.objectsEnabled = controls.objects;
  input.relations = controls.relations;
  input.view = {wrap(controls.camera.column, width_),
                wrap(controls.camera.row, height_), controls.camera.span};
  if (input.player == -1)
    input.spectator = reconstruction::TerrainSpectatorLight{input.view.column,
                                                            input.view.row};
  input.creatures.resize(world.slots.size());
  input.objects.resize(world.slots.size());
  input.creatureScan = input.objectScan = unsigned(world.slots.size());
  std::set<std::uint32_t> keep;
  for (std::size_t i = 0; i < world.slots.size(); ++i) {
    const auto &slot = world.slots[i];
    if (!slot.entity || slot.entity->cleaned)
      continue;
    if (!slot.generation)
      throw std::invalid_argument("Zero entity light generation");
    const auto &entity = *slot.entity;
    // Logical cells are authoritative for terrain illumination. Fine motion is
    // a display interpolation, not another terrain-cell/light clock.
    if (entity.z < 0 || unsigned(entity.z) >= layers_)
      throw std::out_of_range("Entity light layer");
    const game::Point position{int(wrap(entity.x, width_)),
                               int(wrap(entity.y, height_)), entity.z};
    if (entity.family == game::Family::creature) {
      auto h = living.find(unsigned(i));
      if (h == living.end())
        throw std::invalid_argument(
            "Creature light requires typed living health");
      const int owner = entity.owner <= 0x7fffffffu
                            ? int(entity.owner)
                            : int(std::int64_t(entity.owner) - 0x100000000ll);
      input.creatures[i] = {true,
                            h->second != 0,
                            owner,
                            unsigned(position.x),
                            unsigned(position.y),
                            unsigned(position.z)};
      continue;
    }
    std::optional<unsigned> type;
    for (const auto &p : profiles_)
      if (p.family == entity.family && p.type == entity.type) {
        type = p.effectLightingType;
        break;
      }
    if (!type && entity.family == game::Family::missileEffect) {
      if (entity.type >= 89)
        throw std::out_of_range("Missing entity effect light profile");
      type = entity.type;
    }
    if (!type)
      continue;
    keep.insert(unsigned(i));
    const game::Handle handle{unsigned(i), slot.generation};
    auto previous = admissions.find(unsigned(i));
    if (previous == admissions.end() || !(previous->second.handle == handle) ||
        readmit.count(unsigned(i)))
      admissions[unsigned(i)] = {handle, position};
    const auto admission = admissions.at(unsigned(i)).position;
    input.objects[i] =
        types_.source(*type, {true, 0, unsigned(position.x),
                              unsigned(position.y), unsigned(position.z),
                              unsigned(admission.x), unsigned(admission.y)});
  }
  for (auto it = admissions.begin(); it != admissions.end();)
    if (!keep.count(it->first))
      it = admissions.erase(it);
    else
      ++it;
  // New/dead/retired/generation-replaced emitters force recount. Ordinary
  // movement retains the recovered incremental cycle and its separate admission
  // position.
  bool changed = input.creatures.size() != last_.creatures.size() ||
                 input.objects.size() != last_.objects.size() ||
                 input.player != last_.player;
  for (std::size_t i = 0; !changed && i < input.creatures.size(); ++i) {
    const auto &a = input.creatures[i];
    const auto &b = last_.creatures[i];
    const auto &x = input.objects[i];
    const auto &y = last_.objects[i];
    changed = a.active != b.active || a.lightEnabled != b.lightEnabled ||
              a.owner != b.owner || x.active != y.active ||
              x.lightIndex != y.lightIndex;
  }
  if (!changed) {
    for (const auto &pair : admissions) {
      auto old = admission_.find(pair.first);
      if (old == admission_.end() ||
          !(old->second.handle == pair.second.handle) ||
          old->second.position != pair.second.position) {
        changed = true;
        break;
      }
    }
  }
  if (changed && !input.changed)
    input.changed = 1;
  auto candidate = cycle_;
  candidate.step(input);
  cycle_ = std::move(candidate);
  admission_ = std::move(admissions);
  last_ = std::move(input);
}
} // namespace mnm::scene
