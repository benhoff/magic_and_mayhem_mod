#include "terrain_static_lighting.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Static light assertion failed");}
int main(){
 // Inclusive endpoint admission intentionally misses full containment.
 require(admitsTerrainStaticLight(64,64,{32,32,10},25,32,4));
 require(!admitsTerrainStaticLight(64,64,{32,32,2},32,32,12));
 require(admitsTerrainStaticLight(64,64,{0,0,8},63,63,6));
 require(!admitsTerrainStaticLight(64,64,{0,0,8},32,32,6));
 require(!admitsTerrainStaticLight(64,64,{0,0,8},0,0,0));
 TerrainLightField f(32,32,3,{-50,7});const auto before=f.buffers();
 f.stampStatic({0,0,0,17});require(f.buffers()[0]==before[0] && f.buffers()[3]==before[3] && f.buffers()[4]==before[4]);
 // Center gets two additions: -127 + 2*(-8+127), saturated to zero.
 require(f.buffers()[1][0]==0);require(f.buffers()[1][32]==f.buffers()[1][31*32]);
 TerrainLightField field(64,64,5,{-50,7});TerrainStaticLightCycle cycle;
 std::vector<TerrainLightObject> objects(3,{true,16,8,8,0,8,8});
 const auto prior=field.buffers();cycle.step(field,objects,3,{8,8,16},true);
 require(cycle.state().phase==1 && cycle.state().remaining==1 && cycle.state().cursor==2 && cycle.state().active && field.buffers()[0]==prior[0]);
 cycle.step(field,objects,3,{8,8,16},true);require(cycle.state().phase==0 && !cycle.state().active && cycle.state().remaining==0 && field.buffers()[0]==field.buffers()[1]);
 const auto saved=field.buffers();cycle.step(field,objects,3,{8,8,16},false,1,false);require(field.buffers()==saved && cycle.state().phase==0);
 objects[0].lightIndex=34;bool rejected=false;
 try{cycle.step(field,objects,3,{8,8,16},true);}catch(const std::invalid_argument&){rejected=true;}require(rejected && field.buffers()==saved && cycle.state().phase==0);
 objects[0].lightIndex=16;cycle.step(field,objects,3,{8,8,16},true,1);require(cycle.state().phase==0 && !cycle.state().active);
 TerrainStaticLightCycle empty;TerrainLightField dark(17,17,1);
 empty.step(dark,{},0,{0,0,0},false);require(empty.state().phase==1 && empty.state().active && !empty.state().requested);
 empty.step(dark,{},0,{0,0,0},false);require(empty.state().phase==0 && !empty.state().active);
 empty.step(dark,{},0,{0,0,0},false);require(empty.state().phase==0);
}
