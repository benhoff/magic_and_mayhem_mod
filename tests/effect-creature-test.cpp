#include "effect_creature_collision.hpp"
#include "creature_footprint.hpp"
#include <cstdlib>
#include <stdexcept>
using namespace mnm::reconstruction;
static void check(bool b){if(!b)std::abort();}
int main(){
 for(unsigned type:{3u,13u})for(unsigned status:{0u,27u})for(unsigned kind:{0u,68u}){
  EffectPlacementPool pool(8,8,4,1);std::array<std::uint32_t,63> p{};
  p[0]=p[3]=64;p[1]=p[4]=64;p[2]=p[5]=16;p[6]=NoCreature;p[8]=kind;p[9]=2;
  pool.place(0,type,p);EffectCreatureTransitionState s;s.movement.terrain=0;s.movement.previousPosition=pool.records()[0].position;
  s.movement.motion.trajectory.words={0,0,1,0,0,0,0,0,2,1,2,2,1,2};
  EffectCreatureWorld world;world.cells.assign(256,NoEffect);world.cells[pool.records()[0].cell]=0;
  world.creatures.push_back({42,status,{56,56,16},initializeCreatureFootprint(2,1)});
  auto result=transitionEffectCreatureWorld(pool,0,s,8,8,4,world);
  bool hit=status!=27 && kind!=68;check(result==(hit?0u:3u));
  check(pool.records()[0].parameters[7]==(hit?42u:0u));
  check(pool.records()[0].parameters[9]==2);
  if(hit)check(s.candidate==0);
  check(s.movement.motion.changes==(hit && type!=3?2u:5u));
  auto before=pool.records()[0].parameters;world.cells.clear();
  try{transitionEffectCreatureWorld(pool,0,s,8,8,4,world);check(false);}catch(const std::invalid_argument&){}
  check(pool.records()[0].parameters==before);
 }
 EffectPlacementPool pool(8,8,4,1);std::array<std::uint32_t,63> p{};
 p[0]=p[3]=64;p[1]=p[4]=64;p[2]=p[5]=16;p[6]=NoCreature;p[9]=1;
 pool.place(0,13,p);auto old=pool.records()[0].cell;
 EffectCreatureTransitionState s;s.movement.terrain=0;s.movement.previousPosition=pool.records()[0].position;
 s.movement.motion.trajectory.words={0,0,1,0,32,0,32,0,2,1,2,2,1,2};
 EffectCreatureWorld world;world.cells.assign(256,NoEffect);world.cells[old]=0;
 world.creatures.push_back({42,27,{64,64,16},initializeCreatureFootprint(2,1)});
 check(transitionEffectCreatureWorld(pool,0,s,8,8,4,world)==3);
 check(pool.cells()[old].head==NoEffect && pool.cells()[old].flags==0);
}
