#pragma once
#include "../../assets/mps.hpp"
#include "../../game/simulation/world.hpp"
#include "../../reconstruction/animation/controller.hpp"
#include "../../reconstruction/rendering/terrain_entity_rotation.hpp"
#include "../../reconstruction/rendering/terrain_entity_updates.hpp"
#include "map_navigation.hpp"
#include <map>
#include <memory>
namespace mnm::scene {
enum class CellReference : unsigned { effect = 0, creature = 1, artifact = 2 };
struct MapEntity {
  game::Handle handle;
  CellReference reference;
  std::size_t cell;
  // Explicit authoritative occupied cells, including non-anchor footprint
  // cells.
  std::vector<std::size_t> footprint;
  std::optional<std::size_t> placement;
};
struct LinkedTerrain {
  game::Handle handle;
  std::uint32_t sourceSequence;
  reconstruction::NoCdAnimationController animation;
};
struct EntityMapLimits {
  std::uint32_t entities = 4096, linked = 4096;
  std::uint64_t animationBytes = 64ULL * 1024 * 1024;
  std::uint64_t membershipBytes = 64ULL * 1024 * 1024;
};
// Application-owned runtime map. Source assets/placements remain immutable and
// inspectable. Activation decisions belong to simulation, never rendering RNG.
class EntityMap {
  struct State {
    assets::MapAsset map;
    game::World world;
    std::map<std::size_t, LinkedTerrain> linked;
    std::map<std::uint32_t, MapEntity> entities;
    std::vector<std::uint8_t> occupied;
    std::uint64_t revision = 1, navigationRevision = 1;
    bool redraw = true;
    std::optional<MapNavigationRepair> navigation;
    explicit State(std::uint32_t capacity);
  };
  std::shared_ptr<const assets::MapAsset> source_;
  std::shared_ptr<const assets::TerrainCatalog> catalog_;
  std::shared_ptr<const assets::Animation> animation_;
  assets::MpsAsset placements_;
  EntityMapLimits limits_;
  State state_;
  void rebuild(State &) const;
  void publish(State);
  void reset(State &, std::size_t) const;
  void budget(const State &) const;
  std::vector<assets::AnimationRecord> sequence(std::uint32_t) const;
  void changed(State &, bool navigation) const;

public:
  EntityMap(assets::MapAsset, assets::TerrainCatalog, assets::Animation,
            assets::MpsAsset = {}, EntityMapLimits = {},
            unsigned sectionRotation = 0);
  const assets::MapAsset &source() const { return *source_; }
  const assets::MapAsset &map() const { return state_.map; }
  const assets::TerrainCatalog &catalog() const { return *catalog_; }
  const assets::MpsAsset &placements() const { return placements_; }
  const game::World &world() const { return state_.world; }
  const std::map<std::size_t, LinkedTerrain> &linked() const {
    return state_.linked;
  }
  const std::map<std::uint32_t, MapEntity> &entities() const {
    return state_.entities;
  }
  std::uint64_t revision() const { return state_.revision; }
  // Every map/reference mutation invalidates previously frozen navigation.
  std::uint64_t navigationRevision() const { return state_.navigationRevision; }
  bool redrawPending() const { return state_.redraw; }
  void acknowledgeRedraw(std::uint64_t revision);
  std::vector<game::Handle> references(std::size_t, CellReference) const;
  game::Handle activate(game::Entity, CellReference, std::size_t,
                        std::vector<std::size_t> footprint = {},
                        std::optional<std::size_t> placement = {});
  void relocate(game::Handle, std::size_t,
                std::vector<std::size_t> footprint = {});
  void retire(game::Handle);
  // Explicit selected simulation ticks; no implicit animation/frame timer.
  std::vector<std::pair<std::size_t, std::int32_t>>
  advanceLinked(const std::vector<std::size_t> &);
  void resetCell(std::size_t);
  void enableNavigation(MapNavigationConfig);
  void maintainNavigation(bool blocked = false);
  const MapNavigationRepair *navigation() const {
    return state_.navigation ? &*state_.navigation : nullptr;
  }
};
} // namespace mnm::scene
