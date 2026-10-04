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
    const auto retained=player.state();
    auto target=std::vector<mnm::assets::AnimationRecord>{{0,27,{}},{1,8,{}},{0,28,{}},{5,9,{}},{6,-1,{}}};
    player.switchSequence(target);
    require(player.sprite()==28 && player.state().pc==retained.pc && player.state().delay==2 && player.state().elapsed==retained.elapsed,
            "Switch restarted or changed delay/display phase");
    rejected([&]{player.switchSequence({{0,99,{}},{6,-1,{}}});});
    require(player.sprite()==28 && player.state().pc==retained.pc,"Rejected switch mutated player");
    player.tick();player.tick();require(player.tick()==1 && !player.sprite() && !player.state().active,"Stop did not hide sprite");
    require(player.tick()==0,"Inactive player advanced");
    rejected([&]{player.switchSequence(target);});
    NoCdAnimationPlayer unstarted(target);rejected([&]{unstarted.switchSequence(target);});
    rejected([]{NoCdAnimationPlayer p({});});
    rejected([]{NoCdAnimationPlayer p({{0,0,{}}});});
    rejected([]{NoCdAnimationPlayer p({{4,-1,{}},{6,-1,{}}});p.start();});
    rejected([]{NoCdAnimationPlayer p({{4,0,{}},{6,-1,{}}});p.start();});
    NoCdAnimationPlayer bad({{0,-1,{}},{6,-1,{}}});bad.start();rejected([&]{bad.sprite();});
    NoCdAnimationPlayer restart({{0,5,{}},{6,-1,{}}});restart.start();restart.tick();restart.tick();restart.start();require(restart.sprite()==5,"Restart lost selected sequence");
    std::cout<<"Forward tick boundaries, events, stop/restart and bounded control failures pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
