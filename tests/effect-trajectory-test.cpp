#include "effect_trajectory.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Effect trajectory assertion failed");}
int main(){
    EffectTrajectory t;
    std::array<std::uint32_t,3> p{0xffffffffu,10,0xffffffffu};
    std::uint32_t count=0xffffffffu;
    t.words={1,2,0,99,3,4,5,6,8,2,5,4,1,10};
    // Primary Z with XY disabled: XY error is untouched, selected flag is overwritten.
    t.step(p,count);
    require(p==std::array<std::uint32_t,3>{0xffffffffu,10,0} && count==0);
    require(t.words[3]==1 && t.words[11]==3 && t.words[10]==5);
    // Primary Z with XY enabled, positive XY error.
    t.words[2]=1;t.step(p,count);
    require(p==std::array<std::uint32_t,3>{4,16,1} && count==3 && t.words[10]==3);
    // Exactly zero errors take alternate branches, even with zero XY deltas.
    t.words[11]=1;t.words[10]=2;t.words[4]=t.words[5]=0;
    t.step(p,count);
    require(p==std::array<std::uint32_t,3>{4,16,3} && count==7);
    require(t.words[3]==0 && t.words[11]==10 && t.words[10]==8);
    // Signed boundary follows wrapped subtraction; zero Z delta does not count.
    t.words[11]=0x80000000u;t.words[12]=1;t.words[0]=0;t.words[2]=0;
    const auto old=p;t.step(p,count);
    require(p==old && count==7 && t.words[11]==0x7fffffffu && t.words[3]==1);
    t.words[11]=0;t.words[12]=0;t.words[1]=0;t.words[10]=0;t.words[9]=0;
    count=0xffffffffu;t.step(p,count);require(count==2 && t.words[3]==0);
}
