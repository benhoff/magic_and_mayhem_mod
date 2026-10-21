#pragma once
#include "../../reconstruction/rendering/sprite_visibility.hpp"
#include "../../renderer/scenes/scene_renderer.hpp"
#include "entity_map.hpp"
#include "resource_manifest.hpp"
namespace mnm::scene {
enum class SceneMapInputKind { runtimeFrames, assetSequences };
struct SceneSessionLimits {
  std::size_t entities = 4096, draws = 16384;
  assets::ResourceLimits resources;
  SceneAnimationLimits animations;
  render::SceneLimits rendering;
};
struct SceneFrameOptions {
  std::vector<SceneIdentity> visualTicks;
};
struct SceneFrameEntry {
  render::SceneDraw draw;
  std::int32_t key = 0, kind = 0, shade = 0;
  std::optional<SceneIdentity> actor;
  std::optional<std::size_t> cell;
};
struct SceneFramePlan {
private:
  friend class SceneSession;
  std::weak_ptr<const int> owner_;

public:
  std::uint64_t serial = 0, epoch = 0, mapRevision = 0;
  std::vector<SceneFrameEntry> queue;
  std::map<assets::ResourceId, std::uint64_t> resources;
  std::vector<reconstruction::VisibilityOwner> owners;
};
// Complete application rendering owner. Simulation supplies immutable semantic
// snapshots; preparing/presenting never steps a World or movement controller.
class SceneSession {
  struct Receiver {
    assets::ResourceId resource;
    std::uint64_t revision = 0;
    bool owned = false;
    std::uint32_t sourceSequence = 0;
    bool fallback = false;
  };
  struct State {
    game::World lights;
    std::map<SceneIdentity, game::Handle> lightSlots;
    std::map<AnimationReceiverId, Receiver> receivers;
    std::map<AnimationReceiverId, std::uint32_t> visualTicks;
    std::unique_ptr<EntityLighting> lighting;
    std::uint64_t epoch = 0, revision = 0, serial = 0;
    unsigned appliedView = 0;
    std::uint32_t tick = 0;
    explicit State(std::uint32_t capacity) : lights(capacity) {}
    State(const State &);
    State &operator=(State &&) = default;
  };
  assets::AssetStore store_;
  SceneManifest manifest_;
  SceneSessionLimits limits_;
  assets::ResourceManager resources_;
  SceneAnimationBindings animations_;
  ScenePaletteBindings palettes_;
  std::unique_ptr<EntityMap> map_;
  std::unique_ptr<const EntityMap> initialMap_;
  std::shared_ptr<const int> identity_ = std::make_shared<const int>(0);
  std::unique_ptr<render::SceneRenderer> renderer_;
  std::map<assets::ResourceId, std::uint64_t> paletteRevisions_;
  State state_;
  std::thread::id thread_;
  std::shared_ptr<const SceneSourceSnapshot> source_;
  SceneFramePlan compose(const SceneSourceSnapshot &, State &,
                         SceneAnimationBindings &, const EntityMap &);
  void checkThread() const;
  const assets::VisualResource &load(const assets::ResourceId &);

public:
  SceneSession(render::GlBlitter &, assets::AssetStore, SceneManifest,
               SceneSessionLimits = {},
               std::optional<assets::MapAsset> runtimeMap = {},
               SceneMapInputKind = SceneMapInputKind::runtimeFrames);
  SceneSession(const SceneSession &) = delete;
  SceneSession &operator=(const SceneSession &) = delete;
  SceneFramePlan prepare(SceneSourceSnapshot, SceneFrameOptions = {});
  SceneFramePlan recompose();
  void setCamera(reconstruction::TerrainCamera);
  const reconstruction::TerrainCamera &camera() const {
    return manifest_.camera;
  }
  const EntityMap &map() const { return *map_; }
  const EntityLighting &lighting() const { return *state_.lighting; }
  const assets::ResourceManager &resources() const { return resources_; }
  void render(const SceneFramePlan &, render::SceneBatchBudget = {});
  render::Image read();
  QImage present();
  render::GpuFrame presentGpu();
  SceneAnimationSnapshot animationSnapshot(SceneIdentity id,
                                           unsigned channel) const {
    checkThread();
    return animations_.snapshot({id.token(), channel});
  }
  void retire();
};
} // namespace mnm::scene
