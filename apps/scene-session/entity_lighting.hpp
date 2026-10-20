#pragma once
#include "../../assets/effect_lighting.hpp"
#include "../../game/simulation/combat.hpp"
#include "../../game/simulation/creature_encounter.hpp"
#include "../../reconstruction/rendering/effect_lighting.hpp"
#include "../../reconstruction/rendering/terrain_camera.hpp"
#include "../../reconstruction/rendering/terrain_lighting_cycle.hpp"
#include <map>
namespace mnm::scene {
struct ObjectLightProfile {
  game::Family family;
  std::uint32_t type, effectLightingType;
};
struct EntityLightControls {
  int player = 0, changed = 0;
  bool creatures = true, objects = true;
  reconstruction::TerrainLightRelations relations;
  reconstruction::TerrainCamera camera;
  // Explicit recount admission relocation, separate from animated draw
  // position.
  std::vector<game::Handle> readmit;
};
// Typed world/profile producer for the existing recovered light cycle. Living
// health supplies +e4's nonzero test; there is no independent light-enable bit.
class EntityLighting {
  struct Admission {
    game::Handle handle;
    game::Point position;
  };
  reconstruction::EffectLightingTable types_;
  std::vector<ObjectLightProfile> profiles_;
  std::map<std::uint32_t, Admission> admission_;
  reconstruction::TerrainLightingCycle cycle_;
  reconstruction::TerrainLightingSnapshot last_;
  unsigned width_, height_, layers_;
  void step(const game::State &,
            const std::vector<std::pair<game::Handle, std::uint32_t>> &,
            EntityLightControls);

public:
  EntityLighting(unsigned width, unsigned height, unsigned layers,
                 const assets::EffectLightingFields &,
                 std::vector<ObjectLightProfile> = {},
                 reconstruction::TerrainLightingConfig = {});
  void step(const game::State &, const game::CombatState &,
            EntityLightControls = {});
  void step(const game::State &, const game::CreatureEncounter &,
            EntityLightControls = {});
  const reconstruction::TerrainLightingCycle &cycle() const { return cycle_; }
  const reconstruction::TerrainLightingSnapshot &snapshot() const {
    return last_;
  }
};
} // namespace mnm::scene
