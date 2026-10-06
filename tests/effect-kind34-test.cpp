#include "effect_creature_collision.hpp"
#include <cstdlib>
#include <stdexcept>
using namespace mnm::reconstruction;
static void check(bool b){if(!b)std::abort();}
int main(){
 for(unsigned kind:{0u,34u,68u})for(bool refresh:{false,true})for(unsigned status:{0u,27u}){
  EffectPlacementPool pool(8,8,4,1);std::array<std::uint32_t,63> p{};
  p[0]=p[3]=64;p[1]=p[4]=64;p[2]=p[5]=16;p[6]=NoCreature;p[8]=kind;p[9]=1;
  pool.place(0,13,p);pool.records()[0].parameters[6]=0;
  EffectCreatureTransitionState s;s.movement.terrain=0;s.movement.previousPosition=pool.records()[0].position;
  unsigned dx=refresh?32:0;
  s.movement.motion.trajectory.words={0,0,1,0,dx,0,dx,0,2,1,2,2,1,2};
  EffectCreatureWorld world;world.cells.assign(256,NoEffect);
  const auto target=pool.records()[0].cell+(refresh?1:0);world.cells[target]=0;
  CreatureOccupancy footprint;footprint.rows.fill(0xffff);footprint.height=2;
  world.creatures.push_back({777,status,{56,56,16},footprint});
  bool hit=kind==34 && status!=27;
  check(transitionEffectCreatureWorld(pool,0,s,8,8,4,world)==(hit?0u:3u));
  check(pool.records()[0].parameters[7]==(hit?777u:0u));
  check(pool.records()[0].parameters[6]==0 && pool.records()[0].parameters[8]==kind);
  // Public empty-world admission still rejects kind 34 and creator-bearing records.
  auto before=pool.records()[0].parameters;auto movement=s.movement;
  try{transitionEffectEmptyWorld(pool,0,movement,8,8,4);check(false);}catch(const std::invalid_argument&){}
  check(pool.records()[0].parameters==before);
  pool.records()[0].parameters[8]=35;before=pool.records()[0].parameters;
  try{transitionEffectCreatureWorld(pool,0,s,8,8,4,world);check(false);}catch(const std::invalid_argument&){}
  check(pool.records()[0].parameters==before);
 }
}
