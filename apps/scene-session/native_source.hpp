#pragma once
#include "source_snapshot.hpp"
namespace mnm::command_session {
struct Continuation;
struct NativeCreatureProfile;
} // namespace mnm::command_session
namespace mnm::scene {
struct NativeSceneSourceBindings {
  std::array<std::uint32_t, 2> profileStamp{};
  std::vector<assets::ResourceId> animations;
  std::array<unsigned, 28> bodyAssets{}, fallbackBases{};
  std::array<reconstruction::AttachmentPoint, 28> overlayPoints{};
  std::array<std::optional<unsigned>, 28> secondaryAuxiliary{};
  std::array<std::optional<reconstruction::CreatureSpecialView>, 28> special{};
  std::vector<assets::ResourceId> specialCatalog;
  // Branch-specific camera fields require an identified producer. Identity
  // keys bind supplied metadata to the current pool generation.
  std::map<SceneIdentity, SceneEffectView> effectCamera;
  std::map<SceneIdentity, std::optional<SceneActorReference>> linkedEffects;
  reconstruction::TerrainLightRelations relations;
  reconstruction::EffectViewInputs effectViews;
  // Original action-object storage has no retained generation field. Its owner
  // must provide these identities before that storage can become scene
  // entities.
  std::vector<std::uint32_t> objectGenerations;
  std::vector<SceneTypeProfile> objectProfiles;
};
NativeSceneSourceBindings
nativeSceneBindings(const command_session::NativeCreatureProfile &,
                    std::vector<assets::ResourceId>,
                    std::array<std::uint32_t, 2> profileStamp);
SceneSourceSnapshot nativeSceneSnapshot(const command_session::Continuation &,
                                        const NativeSceneSourceBindings &,
                                        std::uint64_t epoch,
                                        std::uint64_t revision,
                                        unsigned sourceView = 0);
} // namespace mnm::scene
