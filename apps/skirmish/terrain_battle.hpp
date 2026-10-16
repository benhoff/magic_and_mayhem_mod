#pragma once
#include "simulation/movement.hpp"
#include <array>
namespace mnm::skirmish {
// Owned ordinary Plains crop. Runtime map objects are deliberately projected
// out.
constexpr const char *terrainBattleMap = "native-installed-plains-battle-v1";
struct TerrainBattle {
  game::Bytes bytes;
  std::shared_ptr<const game::Navigation> navigation;
  std::array<game::Point, 2> wizards;
  unsigned plane = 0;
  game::Bytes frozen;
  std::vector<game::Point> standing;
};
TerrainBattle installedTerrainBattle(const std::filesystem::path &);
TerrainBattle decodeTerrainBattle(const game::Bytes &);
// Bind movement CFG/ANI to the exact raw inputs owned by the melee profile.
void validateTerrainCreatureProfile(const TerrainBattle &, const game::Bytes &);
// Same admitted cell/profile geometry used for native projectile terrain
// policy. A set bit is a solid voxel. This is a native conservative projection,
// not TTD fine collision-catalog recovery.
std::vector<bool> terrainBattleSolids(const game::Bytes &);
} // namespace mnm::skirmish
