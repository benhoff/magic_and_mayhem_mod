#include "native_source.hpp"
#include "../command-session/native_providers.hpp"
#include <set>
#include <stdexcept>
namespace mnm::scene {
namespace {
namespace model = reconstruction::combat;
game::Point point(model::Cell p) { return {p.x, p.y, p.z}; }
int signedWord(unsigned n) {
  return n <= 0x7fffffffu ? int(n) : int(std::int64_t(n) - 0x100000000ll);
}
SceneLayerSource layer(const command_session::NativeAnimationChannel &c,
                       unsigned channel, const NativeSceneSourceBindings &b) {
  if (c.asset >= b.animations.size())
    throw std::out_of_range("Native scene ANI asset registry");
  SceneLayerSource result;
  result.channel = channel;
  result.animation.resource = b.animations[c.asset];
  result.animation.sequence = c.sequence;
  if (c.bound)
    result.animation.continuation = c.state;
  return result;
}
} // namespace
NativeSceneSourceBindings
nativeSceneBindings(const command_session::NativeCreatureProfile &profile,
                    std::vector<assets::ResourceId> resources,
                    std::array<std::uint32_t, 2> stamp) {
  if (resources.size() != profile.animation.size() || resources.empty() ||
      resources.size() > 1024)
    throw std::invalid_argument(
        "Native scene animation registry/profile mismatch");
  NativeSceneSourceBindings result;
  result.profileStamp = stamp;
  result.animations = std::move(resources);
  for (unsigned i = 0; i < 28; ++i) {
    result.bodyAssets[i] = profile.types[i].bodyAsset;
    result.fallbackBases[i] = 288;
    result.overlayPoints[i] = reconstruction::AttachmentPoint::first;
  }
  return result;
}
SceneSourceSnapshot nativeSceneSnapshot(const command_session::Continuation &c,
                                        const NativeSceneSourceBindings &b,
                                        std::uint64_t epoch,
                                        std::uint64_t revision,
                                        unsigned sourceView) {
  const auto count = c.world.creatures.size();
  const auto dimensions = c.world.dimensions;
  if (!epoch || !revision || !c.native.present ||
      b.profileStamp != c.native.profileStamp || count > 65536 ||
      c.generations.size() != count || c.native.animation.size() != count ||
      c.execution.size() != count || c.actions.motion.size() != count ||
      sourceView > 3 || dimensions.x < 1 || dimensions.x > 128 ||
      dimensions.y < 1 || dimensions.y > 128 || dimensions.z < 1 ||
      dimensions.z > 32 ||
      c.native.cells.size() !=
          std::size_t(dimensions.x) * dimensions.y * dimensions.z)
    throw std::invalid_argument(
        "Native scene continuation shape/profile ownership");
  SceneSourceSnapshot result;
  result.epoch = epoch;
  result.revision = revision;
  result.tick = c.native.tick;
  result.sourceView = sourceView;
  result.dimensions = {dimensions.x, dimensions.y, dimensions.z};
  result.relations = b.relations;
  result.effectViews = b.effectViews;
  result.effectViews.width = unsigned(dimensions.x);
  result.effectViews.height = unsigned(dimensions.y);
  result.specialCatalog = b.specialCatalog;
  assets::MapAsset map;
  map.width = unsigned(dimensions.x);
  map.height = unsigned(dimensions.y);
  map.layers = unsigned(dimensions.z);
  map.cells.resize(c.native.cells.size());
  for (std::size_t i = 0; i < map.cells.size(); ++i) {
    const auto &source = c.native.cells[i];
    auto &cell = map.cells[i];
    cell.definition = source.kind;
    cell.flags8 = source.links;
    cell.flags10 = source.flags;
    cell.references = {0xffff, 0xffff, 0xffff};
  }
  result.terrain = std::move(map);
  for (unsigned i = 0; i < count; ++i) {
    const auto &a = c.world.creatures[i];
    if (!a.ai.active)
      continue;
    if (a.ai.slot != i || a.ai.type >= 28 || !c.generations[i])
      throw std::invalid_argument("Native scene creature generation/type");
    const auto &animation = c.native.animation[i];
    const auto &execution = c.execution[i];
    SceneEntitySource entity;
    entity.identity = {game::Family::creature, i, c.generations[i]};
    entity.type = a.ai.type;
    entity.owner = a.ai.owner;
    entity.health = a.ai.health;
    entity.cell = point(a.ai.position);
    entity.fine = point(a.finePosition);
    entity.width = a.width;
    entity.visible = a.ai.visible;
    auto body = animation.body;
    if (!body.bound)
      body.asset = b.bodyAssets.at(a.ai.type);
    auto main = layer(body, 0, b);
    main.animation.fallbackWhenEmpty = true;
    main.animation.fallbackSequence =
        b.fallbackBases.at(a.ai.type) + execution.animationFacing;
    if (!body.bound)
      main.animation.sequence = main.animation.fallbackSequence;
    entity.layers.push_back(std::move(main));
    if (animation.overlay.bound) {
      auto overlay = layer(animation.overlay, 4, b);
      overlay.attachment = b.overlayPoints.at(a.ai.type);
      overlay.priority = 7;
      overlay.livingOnly = a.animationKind == 1;
      entity.layers.push_back(std::move(overlay));
    }
    SceneCreatureView view;
    view.identity = entity.identity.token();
    view.active = true;
    view.position = {a.ai.position.x, a.ai.position.y,
                     execution.animationFacing};
    view.state = {c.actions.motion[i].direction,
                  execution.animationFacing,
                  a.animationKind,
                  execution.animationOffset,
                  execution.block722 != 0,
                  false,
                  execution.block855 != 0};
    if (a.animationKind == 3) {
      const auto special = b.special.at(a.ai.type);
      if (!special || special->catalogIndex >= b.specialCatalog.size())
        throw std::invalid_argument(
            "Native special ANI producer is unclassified");
      view.special = special;
      SceneLayerSource visual;
      visual.channel = 1;
      visual.animation.resource = b.specialCatalog[special->catalogIndex];
      visual.animation.sequence =
          special->sequenceBase + execution.animationFacing;
      visual.attachment = reconstruction::AttachmentPoint::first;
      visual.livingOnly = true;
      visual.priority = 7;
      entity.layers.push_back(std::move(visual));
    }
    if (view.state.secondary) {
      const auto index = b.secondaryAuxiliary.at(a.ai.type);
      if (!index || *index >= animation.auxiliary.size() ||
          !animation.auxiliary[*index].bound)
        throw std::invalid_argument(
            "Native secondary ANI producer is unclassified");
      auto visual = layer(animation.auxiliary[*index], 2, b);
      visual.attachment = reconstruction::AttachmentPoint::second;
      visual.priority = 7;
      entity.layers.push_back(std::move(visual));
    }
    if (auto linked = b.linkedEffects.find(entity.identity);
        linked != b.linkedEffects.end())
      view.linkedEffect = linked->second;
    else if (a.animationKind == 2)
      throw std::invalid_argument(
          "Native kind2 linked effect requires identified producer (including "
          "explicit absence)");
    entity.creatureView = view;
    result.entities.push_back(std::move(entity));
  }
  if (c.native.wizards) {
    if (c.native.wizards->effects.size() > 65536)
      throw std::length_error("Native scene effect pool bound");
    for (unsigned i = 0; i < c.native.wizards->effects.size(); ++i) {
      const auto &effect = c.native.wizards->effects[i];
      if (!effect.active)
        continue;
      if (!effect.generation || effect.animationAsset >= b.animations.size())
        throw std::invalid_argument("Native scene effect generation/asset");
      SceneEntitySource entity;
      entity.identity = {game::Family::missileEffect, i, effect.generation};
      entity.type = effect.placement.type;
      entity.owner = -1;
      entity.requiresLightAdmission = true;
      entity.cell = {int(effect.placement.position[0]),
                     int(effect.placement.position[1]),
                     int(effect.placement.position[2])};
      entity.fine = {signedWord(effect.placement.units[0]),
                     signedWord(effect.placement.units[1]),
                     signedWord(effect.placement.units[2])};
      SceneLayerSource visual;
      visual.channel = 3;
      visual.kind = 0;
      visual.priority = 6;
      visual.animation.resource = b.animations[effect.animationAsset];
      visual.animation.sequence = effect.animationSequence;
      visual.animation.continuation = effect.animation;
      entity.layers.push_back(std::move(visual));
      if (effect.placement.initialized) {
        auto x = effect.placement.sentinels[6],
             y = effect.placement.sentinels[7];
        if (x >= unsigned(dimensions.x) || y >= unsigned(dimensions.y))
          throw std::out_of_range("Native effect light admission coordinates");
        entity.lightAdmission = game::Point{int(x), int(y), entity.cell.z};
      }
      SceneEffectView view;
      if (auto supplied = b.effectCamera.find(entity.identity);
          supplied != b.effectCamera.end()) {
        view = supplied->second;
        if (view.identity != entity.identity.token() || !view.active ||
            view.state.type != entity.type)
          throw std::invalid_argument(
              "Native effect camera producer identity/type");
      } else if (entity.type == 0 || entity.type == 4 || entity.type == 7 ||
                 entity.type == 20 || entity.type == 63 || entity.type == 81 ||
                 entity.type == 51 || entity.type == 52)
        throw std::invalid_argument("Native effect camera branch requires "
                                    "identified metadata producer");
      view.identity = entity.identity.token();
      view.active = true;
      view.state.type = entity.type;
      view.state.column = entity.cell.x;
      view.state.row = entity.cell.y;
      entity.effectView = view;
      result.entities.push_back(std::move(entity));
    }
  }
  const auto &objects = c.actions.objects;
  if (!objects.objects.empty()) {
    if (objects.objects.size() > 65536 ||
        b.objectGenerations.size() != objects.objects.size() ||
        objects.heads.size() != c.native.cells.size())
      throw std::invalid_argument(
          "Native action-object generation provider is unclassified");
    std::set<unsigned> emitted;
    for (std::size_t at = 0; at < objects.heads.size(); ++at) {
      std::set<unsigned> chain;
      auto id = objects.heads[at];
      while (id != 0xffff) {
        if (id >= objects.objects.size() || !chain.insert(id).second)
          throw std::invalid_argument(
              "Native action-object chain bounds/cycle");
        const auto &object = objects.objects[id];
        if (emitted.insert(id).second) {
          const SceneTypeProfile *profile = nullptr;
          for (const auto &p : b.objectProfiles)
            if (p.family == game::Family::mapLinked && p.type == object.kind &&
                !p.owner) {
              if (profile)
                throw std::invalid_argument(
                    "Duplicate native object scene profile");
              profile = &p;
            }
          if (!profile || !b.objectGenerations[id])
            throw std::invalid_argument(
                "Native action-object resource/generation provider");
          SceneEntitySource entity;
          entity.identity = {game::Family::mapLinked, id,
                             b.objectGenerations[id]};
          entity.type = object.kind;
          entity.owner = -1;
          entity.cell = {
              int(at % unsigned(dimensions.x)),
              int(at / unsigned(dimensions.x) % unsigned(dimensions.y)),
              int(at / unsigned(dimensions.x * dimensions.y))};
          entity.fine = {entity.cell.x * 32, entity.cell.y * 32,
                         entity.cell.z * 16};
          entity.width = profile->width;
          entity.layers = profile->layers;
          result.entities.push_back(std::move(entity));
        }
        id = object.next;
      }
    }
  }
  return result;
}
} // namespace mnm::scene
