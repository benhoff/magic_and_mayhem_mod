#include "terrain_creature_lighting.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Creature lighting assertion failed");}
int main(){
 TerrainLightRelations r;TerrainLightCreature c{true,true,3,0,0,0};
 // Owner-directed admission requires both nonzero bytes, not symmetry.
 for(int player=0;player<8;++player)for(int owner=0;owner<8;++owner)
  for(unsigned a:{0u,1u,255u})for(unsigned b:{0u,1u,255u}){
   r={};r.first[owner][player]=a;r.second[owner][player]=b;c.owner=owner;
   require(admitsTerrainCreatureLight(c,player,r)==(owner==player || (a && b)));
  }
 c.owner=0;r={};c.active=false;require(!admitsTerrainCreatureLight(c,0,r));c.active=true;c.lightEnabled=false;require(!admitsTerrainCreatureLight(c,0,r));c.lightEnabled=true;
 TerrainLightField f(32,32,5,{-50,7});TerrainCreatureLightCycle cycle;
 std::vector<TerrainLightCreature> creatures(17,c);
 for(unsigned i=0;i<creatures.size();++i){creatures[i].column=i;creatures[i].row=i;}
 cycle.step(f,creatures,20,0,r);require(cycle.state().phase==1 && cycle.state().quota==3 && cycle.state().remaining==14 && cycle.state().cursor==3);require(f.at(0,0,0)==-127);
 for(unsigned i=1;i<8;++i)cycle.step(f,creatures,20,0,r);
 require(cycle.state().phase==0 && cycle.state().remaining==0 && cycle.state().published && f.at(0,0,0)>-127);
 const auto old=f.buffers()[0];creatures[0].column=31;creatures[0].row=31;
 cycle.step(f,creatures,20,0,r,{true,true,false,1});require(cycle.state().phase==0 && !cycle.state().published && f.buffers()[0]==old && f.buffers()[2]==f.buffers()[3]);
 cycle.step(f,creatures,20,0,r,{true,false,false,1});require(cycle.state().published && f.buffers()[0]==f.buffers()[3]);
 // changed=2 restarts enumeration without forcing phase 7.
 cycle.step(f,creatures,20,0,r,{true,false,false,2});require(cycle.state().phase==1 && cycle.state().remaining==14);
 const auto prior=f.buffers();const auto cursor=cycle.state().cursor;
 cycle.step(f,creatures,20,0,r,{false,false,false,1});require(f.buffers()==prior && cycle.state().cursor==cursor && cycle.state().phase==1);
 creatures[1].column=32;bool refused=false;
 try{cycle.step(f,creatures,20,0,r);}catch(const std::invalid_argument&){refused=true;}
 require(refused && f.buffers()==prior && cycle.state().cursor==cursor && cycle.state().phase==1);
 creatures[1].column=1;creatures[1].owner=-1;refused=false;
 try{cycle.step(f,creatures,20,0,r);}catch(const std::invalid_argument&){refused=true;}
 require(refused && f.buffers()==prior);
 require(!admitsTerrainCreatureLight(c,-1,r));
 TerrainCreatureLightCycle spectator;TerrainLightField camera(32,32,5,{-50,7});
 const auto initial=camera.buffers();refused=false;
 try{spectator.step(camera,{},0,-1,r);}catch(const std::invalid_argument&){refused=true;}
 require(refused && camera.buffers()==initial && spectator.state().phase==0);
 spectator.step(camera,{{true,true,-1,99,99,99}},1,-1,r,{false,false,false,0,TerrainSpectatorLight{8,9}});
 require(spectator.state().phase==0 && camera.buffers()!=initial && camera.buffers()[0]==initial[0]);
 const auto stamped=camera.buffers();refused=false;
 try{spectator.step(camera,{},0,-1,r,{false,false,false,0,TerrainSpectatorLight{32,0}});}catch(const std::invalid_argument&){refused=true;}
 require(refused && camera.buffers()==stamped && spectator.state().phase==0);
 for(unsigned i=0;i<8;++i)spectator.step(camera,{},0,-1,r,{true,false,false,0,TerrainSpectatorLight{8,9}});
 require(spectator.state().published && spectator.state().phase==0 && camera.at(8,9,2)>-127);
 TerrainCreatureLightCycle empty;TerrainLightField dark(17,17,1,{-50,7});
 empty.step(dark,{},4,0,r,{true,false,false,1});require(empty.state().phase==1 && !empty.state().published);
 // Switching to spectator mid-cycle skips quota and cursor visitation entirely.
 auto midSpectator=cycle;auto midField=f;const auto midState=cycle.state();
 midSpectator.step(midField,creatures,20,-1,r,{true,false,false,0,TerrainSpectatorLight{8,9}});
 require(midSpectator.state().cursor==midState.cursor && midSpectator.state().quota==midState.quota && midSpectator.state().remaining==midState.remaining);
 // Native ownership: copied cycles and fields can advance independently.
 auto clone=cycle;auto copied=f;creatures[1].owner=0;clone.step(copied,creatures,20,0,r,{true,false,false,1});require(cycle.state().phase==1 && f.buffers()==prior && clone.state().phase==0);
}
