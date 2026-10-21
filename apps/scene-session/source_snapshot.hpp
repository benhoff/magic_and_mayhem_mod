#pragma once
#include "../../assets/map.hpp"
#include "../../game/simulation/combat.hpp"
#include "../../game/simulation/creature_encounter.hpp"
#include "../../reconstruction/animation/placement.hpp"
#include "../../reconstruction/rendering/terrain_lighting_cycle.hpp"
#include "../../renderer/sprites/sprite.hpp"
#include "camera_binding.hpp"
namespace mnm::scene {
// Semantic values, never an original pool pointer or a wire C++ layout.
struct SceneIdentity {
  game::Family family = game::Family::creature;
  std::uint32_t slot = 0, generation = 0;
  std::uint64_t token() const;
  bool operator<(const SceneIdentity &other) const;
  bool operator==(const SceneIdentity &other) const;
};
struct SceneAnimationSource {
  assets::ResourceId resource;
  std::uint32_t sequence = 0;
  std::optional<reconstruction::AnimationControllerState> continuation;
  std::optional<game::DisplayPose> pose;
  std::optional<std::uint32_t> fixedFrame;
  bool fallbackWhenEmpty = false;
  std::uint32_t fallbackSequence = 0;
};
struct SceneLayerSource {
  unsigned channel = 0;
  SceneAnimationSource animation;
  std::optional<reconstruction::AttachmentPoint> attachment;
  int kind = 1, priority = 6;
  bool lit = true, livingOnly = false;
  render::SpriteComposite composite;
};
struct SceneEntitySource {
  SceneIdentity identity;
  std::uint32_t type = 0;
  int owner = 0;
  std::uint32_t health = 0;
  game::Point cell, fine;
  unsigned width = 1;
  std::vector<SceneLayerSource> layers;
  std::optional<SceneCreatureView> creatureView;
  std::optional<SceneEffectView> effectView;
  std::optional<game::Point> lightAdmission;
  bool requiresLightAdmission = false;
  std::array<std::uint32_t, 8> visible{{1, 1, 1, 1, 1, 1, 1, 1}};
};
struct SceneSourceSnapshot {
  std::uint64_t epoch = 1, revision = 1;
  std::uint32_t tick = 0;
  game::Point dimensions;
  unsigned sourceView = 0;
  std::vector<SceneEntitySource> entities;
  reconstruction::TerrainLightRelations relations;
  reconstruction::EffectViewInputs effectViews;
  std::vector<assets::ResourceId> specialCatalog;
  std::optional<assets::MapAsset> terrain;
};
struct SceneTypeProfile {
  game::Family family = game::Family::creature;
  std::uint32_t type = 0;
  std::optional<std::uint32_t> owner;
  unsigned width = 1;
  bool directional = false;
  std::vector<SceneLayerSource> layers;
};
using SceneFinePosition = std::function<game::Point(const game::Entity &)>;
// Current native cursor/pose and living-health values are copied, never ticked.
SceneSourceSnapshot
worldSceneSnapshot(const game::State &, const game::CombatState &,
                   const std::vector<SceneTypeProfile> &, std::uint64_t epoch,
                   std::uint64_t revision, const SceneFinePosition & = {});
SceneSourceSnapshot
worldSceneSnapshot(const game::State &, const game::CreatureEncounter &,
                   const std::vector<SceneTypeProfile> &, std::uint64_t epoch,
                   std::uint64_t revision, const SceneFinePosition & = {});
SceneSourceSnapshot movementSceneSnapshot(const game::MovementSession &,
                                          const game::CombatState &,
                                          const std::vector<SceneTypeProfile> &,
                                          std::uint64_t epoch,
                                          std::uint64_t revision);
} // namespace mnm::scene
