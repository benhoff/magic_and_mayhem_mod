// Exercise the production provider through its existing public orient callback.
// The owned profile/terrain fixture is synthetic; no installed/original input is read.
#include "../apps/command-session/native_providers.hpp"
#include <iostream>
#include <stdexcept>
namespace command_session=mnm::command_session;
static void need(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}

int main() try {
  struct Case {int x,y,tx,ty;unsigned direction;};
  const Case cases[]={
    {8,8,8,7,0},{8,8,9,7,1},{8,8,9,8,2},{8,8,9,9,3},
    {8,8,8,9,4},{8,8,7,9,5},{8,8,7,8,6},{8,8,7,7,7},
    {8,8,11,6,1},{8,8,12,6,2},{8,8,10,5,1},{8,8,10,4,0},
    {8,8,5,10,5},{8,8,4,10,6},{8,8,6,11,5},{8,8,6,12,4},
    {14,8,1,8,2},{1,8,14,8,6},{8,14,8,1,4},{8,1,8,14,0}
  };
  const unsigned camera[4][8]={
    {0,1,2,3,4,5,6,7},{6,7,0,1,2,3,4,5},
    {4,5,6,7,0,1,2,3},{2,3,4,5,6,7,0,1}
  };
  const unsigned reverse[8]={4,5,6,7,0,1,2,3};
  unsigned checked=0;
  for(unsigned turn=0;turn<4;++turn)for(unsigned flipped=0;flipped<2;++flipped){
    command_session::NativeCreatureProfile p;p.cameraTurn=turn;p.animation.resize(1);p.terrain.resize(1);p.terrainPropertyA0.resize(1);
    command_session::Continuation c;c.world.dimensions={16,16,4};c.world.creatures.resize(1);c.execution.resize(1);c.world.creatures[0].ai.active=false;
    c.native.cells.resize(16*16*4);
    command_session::prepareNativeContinuation(c,p);
    auto hooks=command_session::nativeProviders(p,[](auto &){return command_session::NativeConsequences{};})(c);
    auto orient=hooks.execution(0).orient;need(bool(orient),"production orient callback missing");
    auto &a=c.world.creatures[0];auto &motion=c.actions.motion[0];auto &execution=c.execution[0];
    for(const auto &v:cases){
      a.ai.position={v.x,v.y,0};execution.block722=flipped;motion.direction=6;execution.animationFacing=5;
      orient(a,{v.tx*32,v.ty*32,0});
      const auto expected=camera[turn][v.direction];
      need(motion.direction==v.direction,"native wrapped target direction");
      need(execution.animationFacing==(flipped?reverse[expected]:expected),"native camera/reverse facing");
      ++checked;
    }
    a.ai.position={8,8,0};motion.direction=6;execution.animationFacing=5;
    orient(a,{8*32,8*32,0});
    need(motion.direction==6&&execution.animationFacing==5,"zero displacement changed facing");
    ++checked;
  }
  std::cout<<"{\"success\":true,\"native_facing_cases\":"<<checked
           <<",\"synthetic_inputs_only\":true,\"original_comparison\":false}\n";
  return 0;
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
