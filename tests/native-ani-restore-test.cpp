#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
static void check(bool v){if(!v) throw std::runtime_error("ANI in-place restoration mismatch");}
int main(int argc,char** argv) try {
    check(argc==3);auto animation=mnm::sandbox::loadMovementAnimation(argv[2],8);
    auto navigation=mnm::sandbox::loadFrozenNavigation(argv[1],animation);
    World world(8);auto state=world.state();state.map=argv[1];state.navigation=navigation->binding();state.animation=animation;world.restore(state);
    MovementSession session(std::move(world),navigation);Entity e;e.type=navigation->creatureType();e.x=e.y=e.z=1;
    auto actor=session.spawn(e,true,true);session.move(actor,{5,1,1});for(int i=0;i<6;++i) session.step();
    const auto before=encodeSnapshot(session.world().state());
    char folder[]="/tmp/mnm-ani-restore-XXXXXX";auto* made=mkdtemp(folder);check(made);
    const auto path=std::filesystem::path(made)/"saved";
    struct Cleanup {std::filesystem::path folder;~Cleanup(){std::error_code error;std::filesystem::remove_all(folder,error);}} cleanup{made};
    check(writeSnapshot(path,session.world().state()).durable);
    bool refused=false;
    try {session.restoreResources(path,[](const State& s){return mnm::sandbox::loadFrozenNavigation(s.map);});}catch(const std::exception&){refused=true;}
    check(refused && encodeSnapshot(session.world().state())==before);
    auto malformed=session.world().state();malformed.slots[actor.slot].entity->motion->fine->animation->pc=65535;
    const auto bad=std::filesystem::path(made)/"bad";check(writeSnapshot(bad,malformed).durable);
    NavigationResolver resolver=[](const State& s){return mnm::sandbox::loadFrozenNavigation(s.map,s.animation);};
    refused=false;try {session.restoreResources(bad,resolver);}catch(const std::exception&){refused=true;}
    check(refused && encodeSnapshot(session.world().state())==before);
    World copy(0);copy.restore(session.world().state());MovementSession expected(std::move(copy),navigation);
    session.step();expected.step();
    check(encodeSnapshot(session.world().state())==encodeSnapshot(expected.world().state()));
    session.restoreResources(path,resolver);expected.restoreResources(path,resolver);
    for(unsigned i=0;i<14;++i) {session.step();expected.step();}
    check(encodeSnapshot(session.world().state())==encodeSnapshot(expected.world().state()));
    std::cout<<"ANI in-place resource/state refusal, successful restoration and subsequent ticks passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
