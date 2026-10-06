#include "effect_motion_initializer.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Motion initializer assertion failed");}
int main(){
    EffectPlacementRecord r;r.parameters.fill(42);r.sentinels.fill(123);r.initialPosition={5,6,7};r.type=36;r.next=7;r.active=true;
    EffectTransitionState s;s.motion.trajectory.words.fill(99);s.motion.previousUnits={8,9,10};s.previousPosition={3,4,5};s.motion.changes=123;
    std::array<std::uint32_t,3> startup{9,9,9};std::vector<EffectCell> cells(8*4*2);cells[47].terrain=65535;
    initializeEffectMotionRecord(r,s,startup,{255,33,16},{1,33,17},8,4,2,cells);
    require(r.position==std::array<unsigned,3>{7,1,1} && r.cell==47 && s.terrain==65535);
    require(r.units==startup && startup==std::array<std::uint32_t,3>{255,33,16});
    require(s.motion.trajectory.words[3]==0 && s.motion.changes==0);
    for(unsigned i=0;i<12;++i)require(r.sentinels[i]==(i<9?0xffffffffu:123u));
    require(r.parameters[0]==42 && r.initialPosition==std::array<unsigned,3>{5,6,7} && r.type==36 && r.next==7 && r.active);
    require(s.motion.previousUnits==std::array<std::uint32_t,3>{8,9,10} && s.previousPosition==std::array<unsigned,3>{3,4,5});
    const auto before=r.units;const auto trajectory=s.motion.trajectory.words;
    auto refused=[&](unsigned w,unsigned h,unsigned l,std::array<std::uint32_t,3> from,std::vector<EffectCell> input){
        bool failed=false;try{initializeEffectMotionRecord(r,s,startup,from,{0,0,0},w,h,l,input);}catch(const std::invalid_argument&){failed=true;}
        require(failed && r.units==before && s.motion.trajectory.words==trajectory && startup==before && r.sentinels[9]==123 && s.terrain==65535);
    };
    refused(0,4,2,{0,0,0},cells);refused(8,4,2,{256,0,0},cells);refused(8,4,2,{0,128,0},cells);refused(8,4,2,{0,0,32},cells);
    refused(8,4,2,{0xffffffffu,0,0},cells);refused(8,4,2,{0,0,0},{});
}
