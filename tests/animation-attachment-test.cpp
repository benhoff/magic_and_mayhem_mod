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
    std::cout<<"Mode-one selection, visibility, update gates, reset and ownership pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
