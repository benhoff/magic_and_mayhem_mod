#include "body_placement.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Body fallback differs");}
template<class F>void rejected(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}require(failed);}
int main()try{
    NoCdAnimationPlayer player({{0,7,{}},{6,0,{}}});
    unsigned calls=0;
    BodySequenceSource source=[&](unsigned sequence){
        require(sequence==293);++calls;
        mnm::assets::AnimationRecord sprite{0,9,{}};sprite.metadata[0]=0x7fffffff;sprite.metadata[1]=0x80000000;
        return std::vector<mnm::assets::AnimationRecord>{{1,2,{}},sprite,{6,0,{}}};
    };
    const auto result=bodyPlacement(player,5,3,source,2,1);
    require(calls==1 && result.startedSequence==293 && result.sprite==0);
    require(std::uint32_t(result.offset.x)==0x8000001f && std::uint32_t(result.offset.y)==0x7ffffff0);
    require(player.sprite()==9 && player.state().delay==2 && player.state().pc==2);
    auto previous=player.state();
    const auto retained=bodyPlacement(player,0xffffffff,20,{},2,1);
    require(!retained.startedSequence && retained.sprite==9 && calls==1 && player.state().pc==previous.pc);
    player.stop();
    rejected([&]{bodyPlacement(player,8,3,source);});
    rejected([&]{bodyPlacement(player,0,3,{});});
    rejected([&]{bodyPlacement(player,0,3,[](unsigned)->std::vector<mnm::assets::AnimationRecord>{throw std::runtime_error("Unavailable sequence");});});
    rejected([&]{bodyPlacement(player,0,3,[](unsigned){return std::vector<mnm::assets::AnimationRecord>{{6,0,{}}};});});
    rejected([&]{bodyPlacement(player,0,3,[](unsigned){return std::vector<mnm::assets::AnimationRecord>{{0,-1,{}},{6,0,{}}};});});
    require(!player.state().active && !player.displayedRecord() && calls==1);
    player.start();require(player.sprite()==9); // Rejected starts retained the old owned sequence.
    std::cout<<"Owned body fallback, inclusive SPR bound, signed offsets and atomic refusal pass\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
