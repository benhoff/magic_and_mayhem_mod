#pragma once
#include "terrain_lighting.hpp"
#include <cstddef>
namespace mnm::reconstruction {
// Owned inputs to 004f27d0. first is owner relation18 (alliance/nonhostility),
// second is share20 (shared sight); both retain raw nonzero admission tests.
// Production/defaults and resource ownership: terrain_light_suppliers.hpp.
// Observed tables remain directed even though the recovered setters mirror pairs.
struct TerrainLightCreature {
 bool active=false,lightEnabled=false;
 int owner=0;
 unsigned column=0,row=0,layer=0;
};
struct TerrainLightRelations {
 std::array<std::array<std::uint8_t,8>,8> first{},second{};
};
bool admitsTerrainCreatureLight(const TerrainLightCreature&,int player,const TerrainLightRelations&);
struct TerrainCreatureLightState {
 unsigned phase=0,remaining=0,quota=0,cursor=0;
 bool published=false;
};
struct TerrainSpectatorLight {unsigned column=0,row=0;};
struct TerrainCreatureLightControls {
 bool enabled=true,transitionActive=false,transitionRequested=false;
 // Nonzero restarts enumeration; exactly 1 forces completion if lights exist.
 int changed=0;
 std::optional<TerrainSpectatorLight> spectator{};
};
class TerrainCreatureLightCycle {
public:
 // Capacity is creatures.size(); scanLimit is the distinct original loop bound.
 // Player -1 uses an explicit camera anchor and admits no creature records.
 // Other players and active, enabled owners must be 0..7.
 // Caller owns/clears changed after all updaters, as in original 004f27a0.
 void step(TerrainLightField&,const std::vector<TerrainLightCreature>&,
           unsigned scanLimit,int player,const TerrainLightRelations&,
           TerrainCreatureLightControls={});
 const TerrainCreatureLightState& state() const{return state_;}
 // Restore an owned replay snapshot. Unsigned remaining/quota wrap is retained;
 // phase and cursor must stay in the native cycle/storage domain.
 void restore(TerrainCreatureLightState);
private:
 TerrainCreatureLightState state_;
};
}
