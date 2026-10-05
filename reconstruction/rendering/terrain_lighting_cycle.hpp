#pragma once
#include "terrain_creature_lighting.hpp"
#include "terrain_static_lighting.hpp"
namespace mnm::reconstruction {
struct TerrainLightingSnapshot {
 int player=0,changed=0;
 bool creaturesEnabled=true,objectsEnabled=true;
 TerrainLightRelations relations;
 TerrainLightView view;
 unsigned creatureScan=0,objectScan=0;
 std::vector<TerrainLightCreature> creatures;
 std::vector<TerrainLightObject> objects;
};
// Owned composition of the selected 004f27a0 creature-then-static updater.
class TerrainLightingCycle {
public:
 TerrainLightingCycle(unsigned width,unsigned height,unsigned layers,TerrainLightingConfig config={}):field_(width,height,layers,config){}
 // Validate/advance a candidate copy, so a refused snapshot leaves this cycle
 // intact. The input changed flag is consumed by both subcycles in order.
 void step(const TerrainLightingSnapshot&);
 const TerrainLightField& field() const{return field_;}
 const TerrainCreatureLightState& creatures() const{return creatures_.state();}
 const TerrainStaticLightState& objects() const{return objects_.state();}
 unsigned ticks() const{return ticks_;}
private:
 TerrainLightField field_;
 TerrainCreatureLightCycle creatures_;
 TerrainStaticLightCycle objects_;
 unsigned ticks_=0;
};
}
