#pragma once
#include "../../assets/map.hpp"
#include "../../assets/terrain_catalog.hpp"
#include "../../reconstruction/creature-ai/regional_maintenance.hpp"
#include "../../reconstruction/pathfinding/route_cell_validity.hpp"
#include <memory>
namespace mnm::scene {
struct MapNavigationConfig {
  int boundary = 0;
  unsigned capacity = 512;
  std::array<reconstruction::combat::RegionalCreatureShape, 28> shapes{};
};
// Actual owned geometry and recovered graph repair. Maps, query arrays and
// resumable probes are owned; callback views never outlive their backing.
class MapNavigationRepair {
  struct Geometry;
  std::shared_ptr<const std::vector<reconstruction::TerrainValidityRecord>>
      terrain_;
  std::shared_ptr<const Geometry> geometry_;
  MapNavigationConfig config_;
  std::vector<reconstruction::combat::NavigationRegion> regions_;
  reconstruction::combat::NavigationMaintenance maintenance_;
  std::uint64_t revision_ = 1;
  static reconstruction::combat::RegionalGraphServices
  services(std::shared_ptr<const Geometry>, int boundary);

public:
  MapNavigationRepair(const assets::MapAsset &, const assets::TerrainCatalog &,
                      MapNavigationConfig);
  // Stage signature comparison and all queue insertions against old/new owned
  // geometry. No observer callback substitutes for graph mutation.
  void update(const assets::MapAsset &);
  void step(bool blocked = false);
  reconstruction::combat::NavigationChangeSignature
      signature(reconstruction::combat::Cell) const;
  const std::vector<reconstruction::combat::NavigationRegion> &regions() const {
    return regions_;
  }
  const reconstruction::combat::NavigationMaintenance &maintenance() const {
    return maintenance_;
  }
  bool pending() const {
    return reconstruction::combat::navigationMaintenancePending(maintenance_);
  }
  std::uint64_t revision() const { return revision_; }
};
} // namespace mnm::scene
