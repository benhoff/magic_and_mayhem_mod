#pragma once
#include "animation_binding.hpp"
#include "../../reconstruction/rendering/camera_lifecycle.hpp"
#include "../../reconstruction/rendering/view_refresh.hpp"
namespace mnm::scene {
struct SceneActorReference {std::size_t ordinal=0;std::uint64_t identity=0;};
struct SceneCreatureView {
 std::uint64_t identity=0;bool active=false;
 reconstruction::CreatureViewState state;
 reconstruction::EffectViewCreature position;
 std::optional<reconstruction::CreatureSpecialView> special;
 std::optional<SceneActorReference> linkedEffect;
};
struct SceneEffectView {
 std::uint64_t identity=0;bool active=false;
 reconstruction::EffectViewState state;
 std::optional<SceneActorReference> target,creator,cachedTarget;
};
struct SceneMinimapView {unsigned view=0;bool dirty=true;};
// Generation identities are caller-owned semantic tokens, not raw pool pointers.
// The application supplies current pool membership and explicit catalog recipes.
void applyCameraViews(SceneAnimationBindings&,
 const std::vector<reconstruction::CameraRefresh>&,
 std::vector<SceneCreatureView>&,std::vector<SceneEffectView>&,
 SceneMinimapView&,reconstruction::EffectViewInputs,
 const std::vector<assets::ResourceId>& specialCatalog);
void rotateSceneCamera(reconstruction::TerrainCameraLifecycle&,reconstruction::CameraTurn,
 SceneAnimationBindings&,std::vector<SceneCreatureView>&,std::vector<SceneEffectView>&,
 SceneMinimapView&,reconstruction::EffectViewInputs,const std::vector<assets::ResourceId>&,
 std::optional<unsigned> distinctPrimaryView={});
void finishSceneCameraFrame(reconstruction::TerrainCameraLifecycle&,SceneAnimationBindings&,
 std::vector<SceneCreatureView>&,std::vector<SceneEffectView>&,SceneMinimapView&,
 reconstruction::EffectViewInputs,const std::vector<assets::ResourceId>&,
 std::optional<unsigned> distinctPrimaryView={});
}
