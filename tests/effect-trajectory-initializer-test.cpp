#include "effect_trajectory_initializer.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Trajectory initializer assertion failed");}
int main(){
    EffectTrajectory t;t.words.fill(0xa5a5a5a5u);std::uint32_t changes=99;
    initializeEffectTrajectory(t,{0,0,0},{0,0,0},{128,128},32,changes);
    require(t.words==std::array<std::uint32_t,14>{1,1,0,0xa5a5a5a5u,1,1,1,0,0,0,0,0,0,0} && changes==0);
    initializeEffectTrajectory(t,{0,0,0},{4,2,1},{128,128},32,changes);
    require(t.words==std::array<std::uint32_t,14>{0,1,1,0xa5a5a5a5u,1,1,1,0,8,4,4,4,2,8});
    std::array<std::uint32_t,3> p{0,0,0};t.step(p,changes);
    require(p==std::array<std::uint32_t,3>{1,1,0} && changes==3);
    initializeEffectTrajectory(t,{1,1,4},{4095,4095,0},{128,128},32,changes);
    require(t.words[4]==0xffffffffu && t.words[5]==0xffffffffu && t.words[0]==0xffffffffu);
    require(t.words[13]==8 && t.words[12]==4 && t.words[2]==0 && changes==0);
    // Exactly half a period retains the original target; XY and Z ties select
    // the >= branch. Equal coordinates still select a positive unit direction.
    initializeEffectTrajectory(t,{0,0,0},{8,8,8},{16,16},1,changes);
    require(t.words[4]==1 && t.words[5]==1 && t.words[6]==1 && t.words[7]==0 && t.words[2]==0 && t.words[0]==1);
}
