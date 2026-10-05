#pragma once
#include "terrain_lighting.hpp"
namespace mnm::reconstruction {
struct TerrainLightView {unsigned column=0,row=0,extent=0;};
// 004ff700/004ff820: wrapped, inclusive endpoint admission (not general overlap).
bool admitsTerrainStaticLight(unsigned width,unsigned height,TerrainLightView,
                              unsigned column,unsigned row,unsigned index);
struct TerrainLightObject {
 bool active=false;
 unsigned lightIndex=0,column=0,row=0,layer=0;
 // The recount uses these two separate coordinates; visitation uses column/row.
 unsigned admissionColumn=0,admissionRow=0;
};
struct TerrainStaticLightState {
 unsigned phase=0,remaining=0,quota=0,cursor=0;
 bool active=true,requested=true;
};
class TerrainStaticLightCycle {
public:
 // Bounded 004f2ad0: scan count cannot exceed owned capacity (the original
 // recount lacks a capacity check). Kernel indices 2..33 and 0 (no light).
 // published is the creature updater's +642e flag, not a static output flag.
 void step(TerrainLightField&,const std::vector<TerrainLightObject>&,
           unsigned scanCount,TerrainLightView,bool published,int changed=0,bool enabled=true);
 const TerrainStaticLightState& state() const{return state_;}
private:
 TerrainStaticLightState state_;
};
}
