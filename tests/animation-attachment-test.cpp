#include "attachment.hpp"
#include "no_cd.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Attachment contract differs");}
template<class F>void rejected(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}require(failed);}
int main()try{
    for(unsigned facing=0;facing<8;++facing){const auto selected=modeOneSelection(40,2,facing);require(selected.assetIndex==2 && selected.sequence==40+facing);}
    for(unsigned mode=0;mode<5;++mode)for(auto health:{-1,0,1}){
        require(modeOneVisible(health,mode)==(health!=0 && mode==1));require(modeOneTicks(mode)==(mode==1));}
    rejected([]{modeOneSelection(40,2,8);});rejected([]{modeOneSelection(0xffffffff,2,1);});
    NoCdAnimationPlayer player({{1,2,{}},{0,17,{}},{6,-1,{}}});player.start();player.tick();const auto snapshot=player.displayedRecord();
    player.requestBreak();player.stop();const auto& s=player.state();
    require(!s.active && !s.displayedRecord && s.pc==0 && s.delay==0 && s.elapsed==0 && s.repeats==0 && s.breakFlag==0 && player.tick()==0);
    require(snapshot && snapshot->argument==17);player.start();require(player.sprite()==17 && player.state().delay==2);
    ModeOneAttachment layer;unsigned resolutions=0;
    ModeOneSequenceSource source=[&](unsigned asset,unsigned sequence){
        require(asset==2 && sequence==45);++resolutions;
        return std::vector<mnm::assets::AnimationRecord>{{0,17,{}},{5,-7,{}},{0,18,{}},{6,0,{}}};
    };
    require(layer.mode()==0 && !layer.controller() && !layer.selection());
    require(!layer.change(0,0xffffffff,0xffffffff,99,{}));
    require(layer.change(1,40,2,5,source) && layer.selection()->sequence==45 && resolutions==1);
    require(layer.visible(-1) && !layer.visible(0) && layer.visible(1));
    require(layer.tick()==0 && layer.tick()==-7); // Health does not gate selected child ticking.
    const auto retained=layer.controller()->state();
    require(!layer.change(1,0xffffffff,0xffffffff,99,{}) && resolutions==1);
    rejected([&]{layer.change(2,40,2,5,source);});
    require(layer.controller()->state().pc==retained.pc && layer.mode()==1);
    require(layer.change(0,0,0,0,{}) && !layer.controller()->state().active && layer.tick()==0);
    require(layer.selection()->assetIndex==2 && layer.selection()->sequence==45 && !layer.visible(1));
    rejected([&]{layer.change(1,40,2,5,{});});
    rejected([&]{layer.change(1,40,2,8,source);});
    rejected([&]{layer.change(1,40,2,5,[](unsigned,unsigned)->std::vector<mnm::assets::AnimationRecord>{throw std::runtime_error("Source unavailable");});});
    require(layer.mode()==0 && layer.selection()->sequence==45 && !layer.controller()->displayedRecord());
    require(layer.change(1,40,2,5,source) && resolutions==2 && layer.controller()->sprite()==17);
    require(layer.tick()==0 && layer.tick()==-7 && layer.tick()==0 && layer.tick()==1);
    require(layer.mode()==1 && !layer.visible(1) && layer.tick()==0 && !layer.change(1,40,2,5,source));
    std::cout<<"Mode-one selection, visibility, update gates, reset and ownership pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
