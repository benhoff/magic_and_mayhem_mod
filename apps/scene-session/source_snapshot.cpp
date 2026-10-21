#include "source_snapshot.hpp"
#include <limits>
#include <map>
#include <stdexcept>
namespace mnm::scene {
std::uint64_t SceneIdentity::token() const {
  if (unsigned(family) > 2 || slot > 65535 || !generation)
    throw std::invalid_argument("Scene source identity bounds");
  return (std::uint64_t(generation) << 32) |
         (std::uint64_t(unsigned(family)) << 16) | slot;
}
bool SceneIdentity::operator<(const SceneIdentity &other) const {
  return token() < other.token();
}
bool SceneIdentity::operator==(const SceneIdentity &other) const {
  return family == other.family && slot == other.slot &&
         generation == other.generation;
}
namespace {
const SceneTypeProfile &profile(const std::vector<SceneTypeProfile> &profiles,
                                const game::Entity &actor) {
  const SceneTypeProfile *fallback = nullptr;
  const SceneTypeProfile *exact = nullptr;
  for (const auto &p : profiles)
    if (p.family == actor.family && p.type == actor.type) {
      if (p.owner) {
        if (*p.owner == actor.owner) {
          if (exact)
            throw std::invalid_argument("Duplicate owner scene profile");
          exact = &p;
        }
      } else {
        if (fallback)
          throw std::invalid_argument("Duplicate scene type profile");
        fallback = &p;
      }
    }
  if (exact)
    return *exact;
  if (fallback)
    return *fallback;
  throw std::invalid_argument(
      "Current entity has no declared scene resource profile");
}
int signedOwner(std::uint32_t value) {
  return value <= 0x7fffffffu ? int(value)
                              : int(std::int64_t(value) - 0x100000000ll);
}
SceneSourceSnapshot
snapshot(const game::State &world,
         const std::vector<std::pair<game::Handle, std::uint32_t>> &health,
         const std::vector<SceneTypeProfile> &profiles, std::uint64_t epoch,
         std::uint64_t revision, const SceneFinePosition &position) {
  if (!epoch || !revision || world.slots.size() > 65536 ||
      profiles.size() > 4096 || health.size() > world.slots.size())
    throw std::invalid_argument("World scene source limits");
  if (!world.navigation)
    throw std::invalid_argument(
        "World scene source requires owned map dimensions");
  SceneSourceSnapshot result;
  result.epoch = epoch;
  result.revision = revision;
  result.tick = world.tick;
  result.dimensions = world.navigation->dimensions;
  std::map<unsigned, unsigned> living;
  for (const auto &entry : health) {
    if (entry.first.slot >= world.slots.size())
      throw std::invalid_argument("Scene health slot");
    const auto &slot = world.slots[entry.first.slot];
    if (slot.generation != entry.first.generation || !slot.entity ||
        slot.entity->family != game::Family::creature ||
        !living.emplace(entry.first.slot, entry.second).second)
      throw std::invalid_argument("Scene health generation/family/duplicate");
  }
  for (std::size_t i = 0; i < world.slots.size(); ++i) {
    const auto &slot = world.slots[i];
    if (!slot.entity || slot.entity->cleaned)
      continue;
    const auto &actor = *slot.entity;
    const auto &p = profile(profiles, actor);
    SceneEntitySource entity;
    entity.identity = {actor.family, unsigned(i), slot.generation};
    entity.identity.token();
    entity.type = actor.type;
    entity.owner = signedOwner(actor.owner);
    entity.cell = {actor.x, actor.y, actor.z};
    entity.width = p.width;
    entity.layers = p.layers;
    if (actor.family == game::Family::creature) {
      auto h = living.find(unsigned(i));
      if (h == living.end())
        throw std::invalid_argument(
            "Scene creature requires typed living health");
      entity.health = h->second;
    }
    if (position)
      entity.fine = position(actor);
    else if (actor.motion && actor.motion->fine)
      entity.fine = actor.motion->fine->fine;
    else {
      if (actor.motion && actor.motion->terrainMotion)
        throw std::invalid_argument(
            "Scene terrain pose requires authoritative fine-position provider");
      const auto fine = [](int value, int scale) {
        const auto n = std::int64_t(value) * scale;
        if (n < std::numeric_limits<int>::min() ||
            n > std::numeric_limits<int>::max())
          throw std::out_of_range("Scene fine coordinate");
        return int(n);
      };
      entity.fine = {fine(actor.x, 32), fine(actor.y, 32), fine(actor.z, 16)};
    }
    if (entity.layers.empty() ||
        entity.layers[0].channel !=
            (actor.family == game::Family::missileEffect ? 3u : 0u))
      throw std::invalid_argument("Scene body channel is required");
    if (actor.motion) {
      const auto pose = game::displayedPose(*actor.motion);
      if (pose) {
        entity.layers[0].animation.sequence = pose->sequence;
        entity.layers[0].animation.pose = pose;
        entity.layers[0].animation.fixedFrame.reset();
        const game::AnimationCursor *cursor = nullptr;
        if (actor.motion->fine && actor.motion->fine->animation &&
            actor.motion->fine->animation->displayed)
          cursor = &*actor.motion->fine->animation;
        else if (actor.motion->previous &&
                 actor.motion->previous->motion.animation &&
                 actor.motion->previous->motion.animation->displayed)
          cursor = &*actor.motion->previous->motion.animation;
        if (cursor) {
          reconstruction::AnimationControllerState state;
          state.forward = {cursor->pc,       cursor->displayed, cursor->active,
                           cursor->delay,    cursor->elapsed,   cursor->repeats,
                           cursor->breakFlag};
          entity.layers[0].animation.continuation = state;
        }
      }
    }
    if (actor.family == game::Family::creature && p.directional) {
      auto &source = entity.layers[0].animation;
      const auto direction = source.sequence & 7;
      SceneCreatureView view;
      view.identity = entity.identity.token();
      view.active = true;
      view.position = {entity.cell.x, entity.cell.y, direction};
      view.state.direction = view.state.viewDirection = direction;
      view.state.mainSequenceBase = source.sequence - direction;
      entity.creatureView = view;
    }
    result.entities.push_back(std::move(entity));
  }
  return result;
}
} // namespace
SceneSourceSnapshot
worldSceneSnapshot(const game::State &world, const game::CombatState &health,
                   const std::vector<SceneTypeProfile> &profiles,
                   std::uint64_t epoch, std::uint64_t revision,
                   const SceneFinePosition &position) {
  std::vector<std::pair<game::Handle, std::uint32_t>> values;
  for (const auto &f : health.fighters)
    values.emplace_back(f.actor, f.health);
  return snapshot(world, values, profiles, epoch, revision, position);
}
SceneSourceSnapshot worldSceneSnapshot(
    const game::State &world, const game::CreatureEncounter &health,
    const std::vector<SceneTypeProfile> &profiles, std::uint64_t epoch,
    std::uint64_t revision, const SceneFinePosition &position) {
  std::vector<std::pair<game::Handle, std::uint32_t>> values;
  for (const auto &f : health.actors)
    values.emplace_back(f.handle, f.health);
  return snapshot(world, values, profiles, epoch, revision, position);
}
SceneSourceSnapshot
movementSceneSnapshot(const game::MovementSession &session,
                      const game::CombatState &health,
                      const std::vector<SceneTypeProfile> &profiles,
                      std::uint64_t epoch, std::uint64_t revision) {
  return worldSceneSnapshot(
      session.world().state(), health, profiles, epoch, revision,
      [&](const auto &actor) { return session.finePosition(actor); });
}
} // namespace mnm::scene
