#include "session.hpp"
#include "../../reconstruction/animation/attachment.hpp"
#include "../../reconstruction/rendering/terrain_traversal.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>
namespace mnm::scene {
namespace {
std::uint32_t capacity(std::size_t n) {
  if (!n || n > 65535)
    throw std::invalid_argument("Scene entity capacity");
  return std::uint32_t(n);
}
int narrow(std::int64_t n) {
  if (n < std::numeric_limits<int>::min() ||
      n > std::numeric_limits<int>::max())
    throw std::out_of_range("Scene projection arithmetic");
  return int(n);
}
std::unique_ptr<assets::AssetFile> open(const assets::AssetStore &store,
                                        const std::string &path) {
  auto input = store.open(path);
  if (auto *e = std::get_if<assets::Error>(&input))
    throw std::runtime_error(path + ": " + e->detail);
  return std::get<std::unique_ptr<assets::AssetFile>>(std::move(input));
}
template <class T, class E> T decoded(std::variant<T, E> result) {
  if (auto *e = std::get_if<E>(&result))
    throw std::runtime_error(e->detail);
  return std::get<T>(std::move(result));
}
std::int64_t wrap(std::int64_t n, std::int64_t period) {
  n %= period;
  return n < 0 ? n + period : n;
}
reconstruction::AnimationOffset project(game::Point p,
                                        const reconstruction::TerrainCamera &c,
                                        const assets::MapAsset &map) {
  const auto sum = narrow(std::int64_t(c.f45) + c.f49),
             difference = narrow(std::int64_t(c.f45) - c.f49);
  std::int64_t x = std::int64_t(c.f11) + c.f51 / 2,
               y = std::int64_t(c.f15) +
                   16 * (std::int64_t(c.f41) - 2 * c.diagonal) + c.f55 / 2 +
                   c.f4d;
  switch (c.view) {
  case 0:
    x -= difference;
    y -= std::uint32_t(sum) >> 1;
    break;
  case 1:
    x -= sum;
    y += difference / 2;
    break;
  case 2:
    x += difference;
    y += std::uint32_t(sum) >> 1;
    break;
  case 3:
    x += sum;
    y -= difference / 2;
    break;
  default:
    throw std::invalid_argument("Scene camera view");
  }
  const auto sx = wrap(c.column + ((c.view == 1 || c.view == 2) ? 1 : -1) *
                                      int(c.diagonal),
                       map.width) *
                  32,
             sy = wrap(c.row + (c.view >= 2 ? 1 : -1) * int(c.diagonal),
                       map.height) *
                  32;
  std::int64_t a = 0, b = 0;
  switch (c.view) {
  case 0:
    a = wrap(std::int64_t(p.y) - sy, map.height * 32);
    b = wrap(std::int64_t(p.x) - sx, map.width * 32);
    break;
  case 1:
    a = wrap(sx + 32 - p.x, map.width * 32);
    b = wrap(std::int64_t(p.y) - sy, map.height * 32);
    break;
  case 2:
    a = wrap(sy + 32 - p.y, map.height * 32);
    b = wrap(sx + 32 - p.x, map.width * 32);
    break;
  case 3:
    a = wrap(std::int64_t(p.x) - sx, map.width * 32);
    b = wrap(sy + 32 - p.y, map.height * 32);
    break;
  }
  return {narrow(x + b - a), narrow(y + (a + b) / 2 - p.z)};
}
const assets::AnimationRecord &record(const assets::VisualResource &resource,
                                      const SceneAnimationSnapshot &pose) {
  const auto &a = *resource.animation;
  if (std::uint64_t(pose.sequence) + 1 >= a.starts.size() ||
      !pose.state.forward.displayedRecord)
    throw std::invalid_argument("Scene ANI display record");
  auto at = std::uint64_t(a.starts[pose.sequence]) +
            *pose.state.forward.displayedRecord;
  if (at >= a.starts[pose.sequence + 1] || at >= a.records.size() ||
      a.records[at].opcode != 0)
    throw std::out_of_range("Scene displayed ANI bounds");
  return a.records[at];
}
bool sameId(const assets::ResourceId &a, const assets::ResourceId &b) {
  return a == b;
}
} // namespace
SceneSession::State::State(const State &other)
    : lights(other.lights), lightSlots(other.lightSlots),
      receivers(other.receivers), visualTicks(other.visualTicks),
      lighting(other.lighting
                   ? std::make_unique<EntityLighting>(*other.lighting)
                   : nullptr),
      epoch(other.epoch), revision(other.revision), serial(other.serial),
      appliedView(other.appliedView), tick(other.tick) {}
void SceneSession::checkThread() const {
  if (thread_ != std::this_thread::get_id())
    throw std::runtime_error("Scene session owner thread");
}
const assets::VisualResource &SceneSession::load(const assets::ResourceId &id) {
  const auto &resource = resources_.load(id);
  if (const auto *sprite = std::get_if<assets::Sprite>(&resource.image);
      sprite && sprite->storage == assets::SpriteStorage::indexed8 &&
      !manifest_.paletteModes.empty()) {
    auto previous = paletteRevisions_.find(id);
    if (previous == paletteRevisions_.end() ||
        previous->second != resource.revision) {
      palettes_.bind(id);
      for (auto mode : manifest_.paletteModes)
        palettes_.append(id, mode, manifest_.shading, manifest_.effects);
      paletteRevisions_[id] = resource.revision;
    }
  }
  return resource;
}
SceneSession::SceneSession(render::GlBlitter &gl, assets::AssetStore store,
                           SceneManifest manifest, SceneSessionLimits limits,
                           std::optional<assets::MapAsset> runtimeMap,
                           SceneMapInputKind mapInputKind)
    : store_(std::move(store)), manifest_(std::move(manifest)), limits_(limits),
      resources_(store_, limits.resources),
      animations_(resources_, limits.animations), palettes_(resources_),
      state_(capacity(limits.entities)), thread_(std::this_thread::get_id()) {
  if (!limits.entities || limits.entities > 65535 || !limits.draws ||
      limits.draws > 65536 || manifest_.assets.empty() ||
      manifest_.assets.size() > limits.resources.bindings ||
      manifest_.viewport.left < 0 || manifest_.viewport.top < 0 ||
      manifest_.viewport.right <= manifest_.viewport.left ||
      manifest_.viewport.bottom <= manifest_.viewport.top ||
      manifest_.viewport.right > 2048 || manifest_.viewport.bottom > 2048 ||
      manifest_.lightPlayer < -1 || manifest_.lightPlayer > 7)
    throw std::invalid_argument("Scene session limits/viewport");
  std::map<assets::ResourceId, assets::ResourceRecipe> bindings;
  for (const auto &b : manifest_.assets) {
    const auto [it, fresh] = bindings.emplace(b.id, b.recipe);
    if (!fresh && !(it->second == b.recipe))
      throw std::invalid_argument("Conflicting scene resource recipes");
  }
  for (const auto &b : bindings)
    resources_.bind(b.first, b.second);
  const auto &terrain = load(manifest_.terrain);
  if (!terrain.frameCount() ||
      !std::holds_alternative<assets::Sprite>(terrain.image) ||
      !terrain.terrainCatalog)
    throw std::invalid_argument("Scene terrain requires paired SPR/TTD");
  assets::MapAsset source;
  if (runtimeMap) {
    for (const auto &c : runtimeMap->cells)
      if ((c.flags10 & 8) && mapInputKind == SceneMapInputKind::runtimeFrames)
        throw std::invalid_argument("Runtime linked map requires an identified "
                                    "sequence/controller provider");
    source = std::move(*runtimeMap);
  } else {
    if (!manifest_.map)
      throw std::invalid_argument(
          "Scene requires installed MAP or owned runtime geometry");
    auto input = open(store_, *manifest_.map);
    source = decoded(assets::loadMap(*input));
  }
  assets::MpsAsset placements;
  if (manifest_.mps) {
    auto input = open(store_, *manifest_.mps);
    placements = decoded(assets::loadMps(*input));
  }
  assets::Animation terrainAnimation;
  if (manifest_.terrainAnimation) {
    auto file = open(store_, *manifest_.terrainAnimation);
    terrainAnimation = decoded(assets::loadAnimation(*file));
  }
  map_ = std::make_unique<EntityMap>(std::move(source), *terrain.terrainCatalog,
                                     std::move(terrainAnimation),
                                     std::move(placements));
  initialMap_ = std::make_unique<const EntityMap>(*map_);
  auto camera = manifest_.camera;
  const auto requestedView = camera.view, requestedCut = camera.cutLevel;
  if (!camera.span) {
    reconstruction::bindTerrainCamera(camera, map_->map().width,
                                      map_->map().height, map_->map().layers);
  }
  camera.view = requestedView;
  camera.cutLevel = requestedCut ? requestedCut : map_->map().layers;
  reconstruction::setTerrainCameraViewport(camera, manifest_.viewport);
  manifest_.camera = camera;
  state_.lighting = std::make_unique<EntityLighting>(
      map_->map().width, map_->map().height, map_->map().layers,
      manifest_.effectLights, manifest_.objectLights, manifest_.lighting);
  render::Image background{
      manifest_.viewport.right, manifest_.viewport.bottom,
      std::vector<std::uint32_t>(std::size_t(manifest_.viewport.right) *
                                     manifest_.viewport.bottom,
                                 0x2124)};
  limits_.rendering.draws = limits_.draws;
  limits_.rendering.cache.preparedOnly = true;
  limits_.rendering.cache.residentOnly = true;
  renderer_ = std::make_unique<render::SceneRenderer>(
      gl, resources_, background, limits_.rendering);
  setCamera(camera);
}
void SceneSession::setCamera(reconstruction::TerrainCamera camera) {
  checkThread();
  const auto &map = map_->map();
  (void)reconstruction::traverseTerrain(
      map.width, map.height, map.layers, camera, manifest_.viewport,
      [](auto, auto, auto) { return std::array<std::uint16_t, 2>{0x80, 0}; });
  manifest_.camera = camera;
}
SceneFramePlan SceneSession::prepare(SceneSourceSnapshot source,
                                     SceneFrameOptions options) {
  checkThread();
  const auto &base = map_->map();
  if (!source.epoch || !source.revision ||
      source.entities.size() > limits_.entities || source.sourceView > 3 ||
      source.dimensions !=
          game::Point{int(base.width), int(base.height), int(base.layers)} ||
      (state_.epoch == source.epoch && source.revision <= state_.revision))
    throw std::invalid_argument("Scene source dimensions/epoch/revision (reuse "
                                "recompose for frozen input)");
  if (state_.serial == std::numeric_limits<std::uint64_t>::max())
    throw std::overflow_error("Scene serial exhausted");
  auto candidate = state_.epoch == source.epoch
                       ? State(state_)
                       : State(std::uint32_t(limits_.entities));
  candidate.serial = state_.serial + 1;
  candidate.epoch = source.epoch;
  candidate.revision = source.revision;
  candidate.appliedView =
      state_.epoch == source.epoch ? state_.appliedView : source.sourceView;
  auto animation = animations_.stage();
  if (state_.epoch != source.epoch)
    animation.clear();
  if (!candidate.lighting)
    candidate.lighting = std::make_unique<EntityLighting>(
        base.width, base.height, base.layers, manifest_.effectLights,
        manifest_.objectLights, manifest_.lighting);
  const auto delta = state_.epoch == source.epoch
                         ? std::uint32_t(source.tick - state_.tick)
                         : 0;
  if (delta > 1)
    throw std::invalid_argument(
        "Scene source clock gap requires a restored presentation state");
  candidate.tick = source.tick;
  std::set<SceneIdentity> active;
  std::map<AnimationReceiverId, Receiver> receivers;
  std::vector<SceneAnimationCommand> commands;
  std::vector<
      std::pair<AnimationReceiverId, reconstruction::AnimationControllerState>>
      restores;
  for (const auto &entity : source.entities) {
    entity.identity.token();
    if (!active.insert(entity.identity).second || !entity.width ||
        entity.width > 128 || entity.cell.x < 0 || entity.cell.y < 0 ||
        entity.cell.z < 0 || unsigned(entity.cell.x) >= base.width ||
        unsigned(entity.cell.y) >= base.height ||
        unsigned(entity.cell.z) >= base.layers || entity.layers.empty() ||
        entity.layers.size() > 16)
      throw std::invalid_argument("Scene entity identity/cell/layer bounds");
    std::set<unsigned> channels;
    for (const auto &layer : entity.layers) {
      if (layer.channel > 15 || !channels.insert(layer.channel).second)
        throw std::invalid_argument("Scene layer channel identity");
      const auto &visual = load(layer.animation.resource);
      const AnimationReceiverId id{entity.identity.token(), layer.channel};
      if (layer.animation.fixedFrame || !visual.animation) {
        if (layer.animation.continuation || layer.animation.pose)
          throw std::invalid_argument(
              "Static image cannot accept ANI continuation");
        if (layer.animation.fixedFrame.value_or(0) >= visual.frameCount())
          throw std::out_of_range("Static scene frame");
        continue;
      }
      const bool fallback =
          layer.animation.fallbackWhenEmpty &&
          (!layer.animation.continuation ||
           !layer.animation.continuation->forward.displayedRecord) &&
          !layer.animation.pose;
      const auto sequence = fallback ? layer.animation.fallbackSequence
                                     : layer.animation.sequence;
      const bool owned =
          fallback || (!layer.animation.continuation && !layer.animation.pose);
      auto old = candidate.receivers.find(id);
      const bool bind =
          old == candidate.receivers.end() ||
          !sameId(old->second.resource, layer.animation.resource) ||
          old->second.revision != visual.revision;
      if (bind)
        commands.push_back({id, SceneAnimationCommand::Operation::bind, 0,
                            layer.animation.resource});
      if (bind || !owned || old->second.sourceSequence != sequence)
        commands.push_back(
            {id, SceneAnimationCommand::Operation::startForward, sequence, {}});
      receivers.emplace(id, Receiver{layer.animation.resource, visual.revision,
                                     owned, sequence, fallback});
      if (!fallback) {
        if (layer.animation.continuation)
          restores.emplace_back(id, *layer.animation.continuation);
        else if (layer.animation.pose) {
          if (layer.animation.pose->sequence != layer.animation.sequence)
            throw std::invalid_argument("Static pose sequence mismatch");
          reconstruction::AnimationControllerState state;
          state.forward.active = true;
          state.forward.pc = layer.animation.pose->displayed;
          state.forward.displayedRecord = layer.animation.pose->displayed;
          restores.emplace_back(id, state);
        }
      }
    }
  }
  for (const auto &old : candidate.receivers)
    if (!receivers.count(old.first)) {
      animation.retireReceiver(old.first);
      candidate.visualTicks.erase(old.first);
    }
  animation.apply(commands);
  animation.restore(restores);
  candidate.receivers = std::move(receivers);
  std::set<SceneIdentity> selected;
  std::vector<AnimationReceiverId> tickIds;
  for (auto actor : options.visualTicks) {
    if (!active.count(actor) || !selected.insert(actor).second || !delta)
      throw std::invalid_argument(
          "Scene visual tick ownership/duplicate/source clock");
    bool found = false;
    for (const auto &entry : candidate.receivers)
      if (entry.first.owner == actor.token() && entry.second.owned) {
        auto prior = candidate.visualTicks.find(entry.first);
        if (prior != candidate.visualTicks.end() &&
            prior->second == source.tick)
          throw std::invalid_argument(
              "Scene visual receiver already advanced at source tick");
        tickIds.push_back(entry.first);
        candidate.visualTicks[entry.first] = source.tick;
        found = true;
      }
    if (!found)
      throw std::invalid_argument(
          "Authoritative source animations cannot receive another clock");
  }
  for (unsigned i = 0; i < delta && !tickIds.empty(); ++i)
    animation.advance(tickIds);
  for (auto it = candidate.lightSlots.begin();
       it != candidate.lightSlots.end();)
    if (!active.count(it->first)) {
      candidate.lights.release(it->second);
      it = candidate.lightSlots.erase(it);
    } else
      ++it;
  auto candidateCamera = manifest_.camera;
  if (manifest_.autoFocus && state_.epoch != source.epoch &&
      !source.entities.empty())
    reconstruction::followTerrainCamera(
        candidateCamera, base.width, base.height, base.layers,
        reconstruction::TerrainCameraFocus{source.entities.front().fine.x,
                                           source.entities.front().fine.y,
                                           source.entities.front().fine.z});
  std::vector<std::pair<game::Handle, std::uint32_t>> health;
  EntityLightControls light;
  light.player = manifest_.lightPlayer;
  light.creatures = manifest_.creatureLights;
  light.objects = manifest_.objectLightsEnabled;
  light.relations = source.relations;
  light.camera = candidateCamera;
  for (const auto &entity : source.entities) {
    auto it = candidate.lightSlots.find(entity.identity);
    game::Handle handle;
    if (it == candidate.lightSlots.end()) {
      game::Entity descriptor;
      descriptor.family = entity.identity.family;
      descriptor.type = entity.type;
      descriptor.owner = std::uint32_t(entity.owner);
      descriptor.x = entity.cell.x;
      descriptor.y = entity.cell.y;
      descriptor.z = entity.cell.z;
      handle = candidate.lights.spawn(std::move(descriptor));
      candidate.lightSlots.emplace(entity.identity, handle);
    } else
      handle = it->second;
    if (entity.identity.family == game::Family::creature)
      health.emplace_back(handle, entity.health);
    if (entity.lightAdmission)
      light.admission.emplace_back(handle, *entity.lightAdmission);
    if (entity.requiresLightAdmission && !entity.lightAdmission &&
        manifest_.useLighting && manifest_.objectLightsEnabled &&
        entity.type < 89) {
      reconstruction::EffectLightingTable table;
      table.reload(manifest_.effectLights.values);
      if (table.lightIndex(entity.type))
        throw std::invalid_argument("Native emitting effect admission awaits "
                                    "identified movement publication");
    }
  }
  auto lightState = candidate.lights.state();
  for (const auto &entity : source.entities) {
    const auto handle = candidate.lightSlots.at(entity.identity);
    auto &descriptor = *lightState.slots[handle.slot].entity;
    descriptor.type = entity.type;
    descriptor.owner = std::uint32_t(entity.owner);
    descriptor.x = entity.cell.x;
    descriptor.y = entity.cell.y;
    descriptor.z = entity.cell.z;
  }
  candidate.lights.restore(std::move(lightState));
  if (manifest_.useLighting && (state_.epoch != source.epoch || delta)) {
    light.changed = state_.epoch != source.epoch || delta > 1 ? 1 : 0;
    candidate.lighting->stepSources(candidate.lights.state(), health, light);
  }
  std::unique_ptr<EntityMap> nextMap;
  if (state_.epoch != source.epoch && state_.epoch)
    nextMap = std::make_unique<EntityMap>(*initialMap_);
  if (!source.terrain && state_.epoch == source.epoch && delta &&
      !map_->linked().empty()) {
    nextMap = std::make_unique<EntityMap>(*map_);
    for (unsigned i = 0; i < delta; ++i) {
      std::vector<std::size_t> cells;
      for (const auto &p : nextMap->linked())
        cells.push_back(p.first);
      nextMap->advanceLinked(cells);
    }
  }
  const auto oldCamera = manifest_.camera;
  manifest_.camera = candidateCamera;
  SceneFramePlan plan;
  try {
    plan = compose(source, candidate, animation, nextMap ? *nextMap : *map_);
  } catch (...) {
    manifest_.camera = oldCamera;
    throw;
  }
  auto retained =
      std::make_shared<const SceneSourceSnapshot>(std::move(source));
  animations_.publish(std::move(animation));
  state_ = std::move(candidate);
  if (nextMap)
    map_ = std::move(nextMap);
  source_ = std::move(retained);
  return plan;
}
SceneFramePlan SceneSession::compose(const SceneSourceSnapshot &source,
                                     State &state,
                                     SceneAnimationBindings &animation,
                                     const EntityMap &owner) {
  const auto &map = source.terrain ? *source.terrain : owner.map();
  if (map.width != owner.map().width || map.height != owner.map().height ||
      map.layers != owner.map().layers ||
      map.cells.size() != owner.map().cells.size())
    throw std::invalid_argument("Scene runtime geometry dimensions");
  for (const auto &c : map.cells)
    if (c.definition >= owner.catalog().records.size())
      throw std::out_of_range("Scene runtime TTD frame");
  std::vector<SceneCreatureView> creatures;
  std::vector<SceneEffectView> effects;
  for (const auto &entity : source.entities) {
    if (entity.creatureView) {
      if (entity.identity.family != game::Family::creature ||
          entity.creatureView->identity != entity.identity.token())
        throw std::invalid_argument("Scene creature camera identity");
      creatures.resize(
          std::max(creatures.size(), std::size_t(entity.identity.slot) + 1));
      creatures[entity.identity.slot] = *entity.creatureView;
      const AnimationReceiverId main{entity.identity.token(), 0};
      auto binding = state.receivers.find(main);
      if (binding != state.receivers.end() && binding->second.fallback)
        creatures[entity.identity.slot].state.mainSequenceBase =
            binding->second.sourceSequence -
            creatures[entity.identity.slot].state.viewDirection;
    }
    if (entity.effectView) {
      if (entity.identity.family != game::Family::missileEffect ||
          entity.effectView->identity != entity.identity.token())
        throw std::invalid_argument("Scene effect camera identity");
      effects.resize(
          std::max(effects.size(), std::size_t(entity.identity.slot) + 1));
      effects[entity.identity.slot] = *entity.effectView;
    }
  }
  const bool changedView = state.appliedView != manifest_.camera.view;
  if (changedView || source.sourceView != manifest_.camera.view) {
    std::map<AnimationReceiverId, SceneAnimationSnapshot> before;
    for (const auto &entry : state.receivers)
      before.emplace(entry.first, animation.snapshot(entry.first));
    std::vector<reconstruction::CameraRefresh> requests;
    for (std::size_t i = 0; i < creatures.size(); ++i)
      if (creatures[i].active)
        requests.push_back({reconstruction::CameraRefresh::Kind::creature, i,
                            manifest_.camera.view});
    for (std::size_t i = 0; i < effects.size(); ++i)
      if (effects[i].active)
        requests.push_back({reconstruction::CameraRefresh::Kind::effect, i,
                            manifest_.camera.view});
    requests.push_back({reconstruction::CameraRefresh::Kind::minimap, 0,
                        manifest_.camera.view});
    SceneMinimapView minimap;
    auto inputs = source.effectViews;
    inputs.width = map.width;
    inputs.height = map.height;
    applyCameraViews(animation, requests, creatures, effects, minimap, inputs,
                     source.specialCatalog);
    if (!changedView) {
      std::vector<std::pair<AnimationReceiverId,
                            reconstruction::AnimationControllerState>>
          restore;
      for (const auto &p : before) {
        auto it = state.receivers.find(p.first);
        if (it == state.receivers.end())
          continue;
        const auto after = animation.snapshot(p.first);
        if (!it->second.owned || (sameId(after.resource, p.second.resource) &&
                                  after.sequence == p.second.sequence))
          restore.emplace_back(p.first, p.second.state);
      }
      animation.restore(restore);
    }
    state.appliedView = manifest_.camera.view;
  }
  SceneFramePlan plan;
  plan.owner_ = identity_;
  plan.serial = state.serial;
  plan.epoch = state.epoch;
  plan.mapRevision = owner.revision();
  std::vector<reconstruction::SpriteQueueEntry> queue;
  std::vector<std::optional<std::size_t>> owners;
  const auto add = [&](SceneFrameEntry entry,
                       std::optional<std::size_t> ownerIndex) {
    if (plan.queue.size() >= limits_.draws)
      throw std::length_error("Scene draw budget");
    plan.resources[entry.draw.resource] =
        resources_.resident(entry.draw.resource).revision;
    queue.push_back({entry.key, std::uint32_t(plan.queue.size())});
    plan.queue.push_back(std::move(entry));
    owners.push_back(ownerIndex);
  };
  const auto &terrain = load(manifest_.terrain);
  const auto &terrainSprite = std::get<assets::Sprite>(terrain.image);
  const auto visits = reconstruction::traverseTerrain(
      map.width, map.height, map.layers, manifest_.camera, manifest_.viewport,
      [&](unsigned x, unsigned y, unsigned z) {
        const auto &c =
            map.cells[(std::size_t(z) * map.height + y) * map.width + x];
        return std::array<std::uint16_t, 2>{c.flags8, c.flags10};
      });
  std::map<std::size_t, std::size_t> cellOwners;
  const render::Rect viewport{manifest_.viewport.left, manifest_.viewport.top,
                              manifest_.viewport.right,
                              manifest_.viewport.bottom};
  for (const auto &visit : visits) {
    const auto &cell = map.cells[visit.cell];
    auto definition = reconstruction::decodeTerrainDefinition(
        owner.catalog().records[cell.definition]);
    auto admission = manifest_.admission;
    admission.view = manifest_.camera.view;
    admission.cutLevel = manifest_.camera.cutLevel;
    admission.mode = manifest_.camera.mode;
    const int shade =
        manifest_.useLighting
            ? state.lighting->cycle().field().at(
                  visit.column % map.width, visit.row % map.height, visit.layer)
            : 0;
    reconstruction::TerrainTile tile{
        int(visit.row), int(visit.column), int(visit.layer),
        visit.anchorX,  visit.anchorY,     visit.priority,
        cell.flags8,    cell.flags10,      std::int8_t(shade)};
    auto found = cellOwners.find(visit.cell);
    std::size_t index;
    if (found == cellOwners.end()) {
      index = plan.owners.size();
      cellOwners.emplace(visit.cell, index);
      const auto identity = [&](unsigned frame) {
        return terrainSprite.frames
            .at(frame < terrainSprite.frames.size() ? frame : 0)
            .sourceOffset;
      };
      plan.owners.push_back({identity(definition.body[admission.view]),
                             identity(definition.first[admission.view]),
                             identity(definition.second[admission.view]),
                             tile.flags8, tile.flags10});
    } else {
      index = found->second;
      tile.flags8 = plan.owners[index].flags8;
      tile.flags10 = plan.owners[index].flags10;
    }
    for (const auto &draw : reconstruction::submitTerrain(
             definition, tile, admission,
             std::uint32_t(terrain.frameCount() - 1))) {
      render::SceneDraw command{manifest_.terrain, draw.frame, draw.anchorX,
                                draw.anchorY};
      command.viewport = viewport;
      if (terrainSprite.storage == assets::SpriteStorage::indexed8 &&
          !manifest_.paletteModes.empty() &&
          !terrainSprite.frames.at(draw.frame).empty())
        palettes_.apply(command,
                        manifest_.paletteOrdinal.value_or(
                            manifest_.paletteModes.size() - 1),
                        draw.shade);
      add({command, draw.key, draw.kind, draw.shade, {}, visit.cell}, index);
    }
    plan.owners[index].flags8 = tile.flags8;
    plan.owners[index].flags10 = tile.flags10;
  }
  for (const auto &entity : source.entities) {
    if (manifest_.lightPlayer >= 0 &&
        !entity.visible.at(unsigned(manifest_.lightPlayer)))
      continue;
    std::optional<assets::AnimationRecord> body;
    const auto base = project(
        {narrow(std::int64_t(entity.fine.x) +
                (entity.identity.family == game::Family::creature ? 16 : 0)),
         narrow(std::int64_t(entity.fine.y) +
                (entity.identity.family == game::Family::creature ? 16 : 0)),
         entity.fine.z},
        manifest_.camera, map);
    for (const auto &layer : entity.layers) {
      if (layer.livingOnly &&
          !reconstruction::modeOneVisible(entity.health ? 1 : 0, 1))
        continue;
      const auto &resource = load(layer.animation.resource);
      std::size_t frame = layer.animation.fixedFrame.value_or(0);
      std::optional<assets::AnimationRecord> displayed;
      if (resource.animation && !layer.animation.fixedFrame) {
        const auto pose =
            animation.snapshot({entity.identity.token(), layer.channel});
        if (!pose.frame)
          continue;
        frame = *pose.frame;
        displayed = record(resource, pose);
      }
      if (frame >= resource.frameCount())
        throw std::out_of_range("Scene entity frame");
      if (!layer.attachment && displayed)
        body = displayed;
      reconstruction::AnimationOffset offset{};
      if (layer.attachment) {
        if (!body)
          continue;
        offset = reconstruction::attachmentOffset(
            *body, *layer.attachment, entity.width, manifest_.camera.view);
      } else if (displayed)
        offset = reconstruction::spriteOffset(*displayed, entity.width,
                                              manifest_.camera.view);
      render::SceneDraw draw{layer.animation.resource, frame,
                             narrow(std::int64_t(base.x) + offset.x),
                             narrow(std::int64_t(base.y) + offset.y)};
      draw.viewport = viewport;
      draw.composite = layer.composite;
      const auto shade =
          manifest_.useLighting && layer.lit
              ? int(state.lighting->cycle().field().at(unsigned(entity.cell.x),
                                                       unsigned(entity.cell.y),
                                                       unsigned(entity.cell.z)))
              : 0;
      if (const auto *sprite = std::get_if<assets::Sprite>(&resource.image);
          sprite && sprite->storage == assets::SpriteStorage::indexed8 &&
          !sprite->frames[frame].empty() && !manifest_.paletteModes.empty())
        palettes_.apply(draw,
                        manifest_.paletteOrdinal.value_or(
                            manifest_.paletteModes.size() - 1),
                        shade);
      const auto key = reconstruction::spriteDepthKey(
          {narrow(std::int64_t(entity.fine.x) +
                  (entity.identity.family == game::Family::creature ? 16 : 0)),
           narrow(std::int64_t(entity.fine.y) +
                  (entity.identity.family == game::Family::creature ? 16 : 0)),
           entity.fine.z, layer.priority},
          manifest_.camera.view);
      add({draw, key, layer.kind, shade, entity.identity, {}}, {});
    }
  }
  reconstruction::sortSpriteQueue(queue);
  std::vector<SceneFrameEntry> ordered;
  std::vector<reconstruction::SpriteVisibilityEntry> visibility;
  ordered.reserve(queue.size());
  for (const auto &item : queue) {
    auto entry = plan.queue[item.payload];
    const auto &resource = resources_.resident(entry.draw.resource);
    std::optional<reconstruction::SpriteVisibilityShape> shape;
    std::uint32_t identity = 0;
    if (const auto *sprite = std::get_if<assets::Sprite>(&resource.image)) {
      const auto &frame = sprite->frames.at(entry.draw.frame);
      shape = reconstruction::decodeSpriteVisibility(
          frame.auxiliaryData[0], frame.auxiliaryData[1], frame.originX,
          frame.originY);
      identity = frame.sourceOffset;
    }
    visibility.push_back({shape, entry.draw.anchorX, entry.draw.anchorY,
                          entry.kind, identity, owners[item.payload]});
    ordered.push_back(std::move(entry));
  }
  if (manifest_.visibility) {
    reconstruction::SpriteVisibilityGrid grid(manifest_.viewport.right > 640 ||
                                              manifest_.viewport.bottom > 512);
    reconstruction::applySpriteVisibility(visibility, grid, plan.owners);
  }
  for (std::size_t i = 0; i < ordered.size(); ++i) {
    ordered[i].kind = visibility[i].kind;
    ordered[i].draw.visible = visibility[i].kind != -2;
  }
  plan.queue = std::move(ordered);
  return plan;
}
SceneFramePlan SceneSession::recompose() {
  checkThread();
  if (!source_)
    throw std::logic_error("Scene has no accepted source snapshot");
  if (state_.serial == std::numeric_limits<std::uint64_t>::max())
    throw std::overflow_error("Scene serial exhausted");
  State candidate(state_);
  ++candidate.serial;
  auto animation = animations_.stage();
  auto plan = compose(*source_, candidate, animation, *map_);
  animations_.publish(std::move(animation));
  state_ = std::move(candidate);
  return plan;
}
void SceneSession::render(const SceneFramePlan &plan,
                          render::SceneBatchBudget budget) {
  checkThread();
  if (plan.owner_.lock() != identity_ || plan.serial != state_.serial ||
      plan.epoch != state_.epoch || plan.mapRevision != map_->revision() ||
      plan.queue.size() > limits_.draws)
    throw std::invalid_argument("Stale or oversized scene frame plan");
  for (const auto &p : plan.resources)
    if (resources_.resident(p.first).revision != p.second)
      throw std::invalid_argument("Scene frame resource revision changed");
  if (!budget.draws || budget.draws > limits_.draws ||
      budget.uploadBytes < 8192 || budget.time.count() <= 0)
    throw std::invalid_argument("Scene frame batch budget");
  std::vector<render::SceneDraw> draws;
  draws.reserve(plan.queue.size());
  for (const auto &p : plan.queue)
    draws.push_back(p.draw);
  renderer_->beginFrame(draws);
  for (std::size_t i = 0; i < 100000; ++i) {
    if (renderer_->drawNext(budget))
      return;
    if (auto need = renderer_->uploadNeed())
      renderer_->supplyUpload(
          *need, render::prepareResourceUpload(
                     resources_.resident(need->request.id), need->request));
  }
  throw std::length_error("Scene frame exceeded bounded submission iterations");
}
render::Image SceneSession::read() {
  checkThread();
  return renderer_->read();
}
QImage SceneSession::present() {
  checkThread();
  return renderer_->present();
}
render::GpuFrame SceneSession::presentGpu() {
  checkThread();
  return renderer_->presentGpu();
}
void SceneSession::retire() {
  checkThread();
  auto map = std::make_unique<EntityMap>(*initialMap_);
  State fresh(capacity(limits_.entities));
  fresh.serial = state_.serial;
  fresh.lighting = std::make_unique<EntityLighting>(
      map->map().width, map->map().height, map->map().layers,
      manifest_.effectLights, manifest_.objectLights, manifest_.lighting);
  animations_.clear();
  palettes_.clear();
  for (const auto &binding : manifest_.assets)
    renderer_->release(binding.id);
  resources_.unloadAll();
  paletteRevisions_.clear();
  map_ = std::move(map);
  state_ = std::move(fresh);
  source_.reset();
}
} // namespace mnm::scene
