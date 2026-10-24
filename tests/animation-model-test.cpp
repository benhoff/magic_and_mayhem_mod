#include "no_cd.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
template<class F>static void rejected(F f){bool failed=false;try{f();}catch(const std::runtime_error&){failed=true;}require(failed,"Unsafe animation controls accepted");}
int main()try{
    NoCdAnimationPlayer player({{0,17,{}},{1,2,{}},{0,18,{}},{5,-7,{}},{6,-1,{}}});player.start();
    require(player.sprite()==17 && player.tick()==0 && player.sprite()==17,"Initial sprite-selection boundary differs");
    require(player.tick()==0 && player.sprite()==18 && player.state().delay==2,"Delay dispatch changed");
    require(player.tick()==0 && player.tick()==0 && player.sprite()==18,"Delay did not retain current sprite");
    require(player.tick()==-7 && player.sprite()==18,"Event did not retain sprite");
    NoCdAnimationPlayer restored({{0,17,{}},{1,2,{}},{0,18,{}},{5,-7,{}},{6,-1,{}}});
    restored.restore(player.state());require(restored.sprite()==player.sprite(),"Restore lost displayed sprite");
    auto invalid=player.state();invalid.pc=999;
    rejected([&]{restored.restore(invalid);});require(restored.state().pc==player.state().pc,"Rejected restore mutated cursor");
    invalid=player.state();invalid.displayedRecord=1;
    rejected([&]{restored.restore(invalid);});
    auto a=player,b=restored;
    for(unsigned i=0;i<8;++i) require(a.tick()==b.tick() && a.state().pc==b.state().pc && a.state().elapsed==b.state().elapsed,"Restored control flow diverged");
    const auto retained=player.state();
    const auto displayed=player.displayedRecord();
    auto target=std::vector<mnm::assets::AnimationRecord>{{0,27,{}},{1,8,{}},{0,28,{}},{5,9,{}},{6,-1,{}}};
    player.switchSequence(target);
    require(displayed && displayed->argument==18,"Displayed record snapshot borrowed switched storage");
    require(player.sprite()==28 && player.state().pc==retained.pc && player.state().delay==2 && player.state().elapsed==retained.elapsed,
            "Switch restarted or changed delay/display phase");
    rejected([&]{player.switchSequence({{0,99,{}},{6,-1,{}}});});
    require(player.sprite()==28 && player.state().pc==retained.pc,"Rejected switch mutated player");
    auto negativeTarget=target;negativeTarget[2].argument=-1;
    rejected([&]{player.switchSequence(negativeTarget);});
    require(player.sprite()==28 && player.state().pc==retained.pc,"Negative switch published an invalid display");
    player.tick();player.tick();require(player.tick()==1 && !player.sprite() && !player.state().active,"Stop did not hide sprite");
    require(player.tick()==0,"Inactive player advanced");
    rejected([&]{player.switchSequence(target);});
    NoCdAnimationPlayer unstarted(target);rejected([&]{unstarted.switchSequence(target);});
    rejected([]{NoCdAnimationPlayer p({});});
    rejected([]{NoCdAnimationPlayer p({{0,0,{}}});});
    rejected([]{NoCdAnimationPlayer p({{4,-1,{}},{6,-1,{}}});p.start();});
    rejected([]{NoCdAnimationPlayer p({{4,0,{}},{6,-1,{}}});p.start();});
    NoCdAnimationPlayer bad({{0,-1,{}},{6,-1,{}}});
    rejected([&]{bad.start();});require(!bad.displayedRecord(),"Start published a negative sprite record");
    NoCdAnimationPlayer initialControl({{1,2,{}},{0,-1,{}},{6,-1,{}}});
    rejected([&]{initialControl.start();});require(!initialControl.displayedRecord(),"Initial control dispatch published a negative sprite");
    NoCdAnimationPlayer later({{0,5,{}},{0,-1,{}},{6,-1,{}}});later.start();later.tick();
    rejected([&]{later.tick();});
    require(later.sprite()==5 && later.displayedRecord()->argument==5,"Tick published a negative sprite record");
    NoCdAnimationPlayer restart({{0,5,{}},{6,-1,{}}});restart.start();restart.tick();restart.tick();restart.start();require(restart.sprite()==5,"Restart lost selected sequence");
    // Switching retains every control word and owns the replacement sequence.
    const auto same=[](const AnimationState& a,const AnimationState& b){
        return a.pc==b.pc && a.displayedRecord==b.displayedRecord && a.active==b.active &&
            a.delay==b.delay && a.elapsed==b.elapsed && a.repeats==b.repeats && a.breakFlag==b.breakFlag;
    };
    NoCdAnimationPlayer phase(target);AnimationState phaseState{2,0,true,4,2,3,1};phase.restore(phaseState);
    auto replacement=target;replacement[0].argument=117;phase.switchSequence(replacement);replacement[0].argument=999;
    require(same(phase.state(),phaseState) && phase.sprite()==117,"Switch failed full phase/ownership preservation");
    std::vector<std::vector<mnm::assets::AnimationRecord>> refusals{{},{{0,1,{}}},{{0,1,{}},{6,-1,{}}}};
    auto wrong=target;wrong[0].opcode=1;refusals.push_back(wrong);
    wrong=target;wrong[0].argument=-1;refusals.push_back(wrong);
    refusals.push_back(std::vector<mnm::assets::AnimationRecord>(65537,{6,-1,{}}));
    for(const auto& refused:refusals){
        rejected([&]{phase.switchSequence(refused);});
        require(same(phase.state(),phaseState) && phase.sprite()==117,"Refused switch changed phase/owned storage");
    }
    std::cout<<"Forward tick boundaries, events, stop/restart and bounded control failures pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
