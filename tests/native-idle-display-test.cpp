#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
static void check(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
int main(int argc,char** argv) try {
    check(argc==4,"fixture arguments required");const auto animation=mnm::sandbox::loadMovementAnimation(argv[2],8);
    auto nav=mnm::sandbox::loadFrozenNavigation(argv[1],animation);
    World world(8);auto state=world.state();state.map=argv[1];state.navigation=nav->binding();state.animation=animation;world.restore(state);
    MovementSession session(std::move(world),nav);Entity e;e.type=nav->creatureType();e.x=e.y=e.z=1;
    auto actor=session.spawn(e,true,true);session.move(actor,{5,1,1});for(int i=0;i<6;++i) session.step();
    const auto shown=displayedPose(*session.world().find(actor)->motion);check(bool(shown),"fixture must display an ANI bitmap");
    session.stop(actor);session.step();const auto& stopped=*session.world().find(actor)->motion;
    check(stopped.pose && stopped.pose->sequence==shown->sequence && stopped.pose->displayed==shown->displayed,"Stop lost last display");
    check(stopped.action==Action::cancelled && !stopped.fine && !stopped.previous && !stopped.segmentTicks && stopped.route.empty(),"Stop retained movement continuation");
    const auto saved=encodeSnapshot(session.world().state());check(saved[8]==8,"retained pose requires v8");
    check(writeSnapshot(argv[3],session.world().state()).durable,"save failed");
    NavigationResolver resolver=[](const State& s){return mnm::sandbox::loadFrozenNavigation(s.map,s.animation);};
    World copy(0);copy.restore(decodeSnapshot(saved));MovementSession restored(std::move(copy),nav);
    for(int i=0;i<20;++i) {session.step();restored.step();check(encodeSnapshot(session.world().state())==encodeSnapshot(restored.world().state()),"idle restart drifted");}
    check(session.world().find(actor)->motion->pose->displayed==shown->displayed,"idle pose advanced");
    const auto before=encodeSnapshot(session.world().state());
    for(DisplayPose bad: {DisplayPose{7,0},DisplayPose{4095,0},DisplayPose{shown->sequence,65535},DisplayPose{shown->sequence,0}}) {
        auto corrupt=session.world().state();corrupt.slots[actor.slot].entity->motion->pose=bad;
        auto path=std::filesystem::path(argv[3]).string()+".bad";writeSnapshot(path,corrupt);
        bool refused=false;try {session.restoreResources(path,resolver);}catch(const std::invalid_argument&) {refused=true;}
        check(refused && encodeSnapshot(session.world().state())==before,"malformed pose restore must roll back");
        std::filesystem::remove(path);
    }
    auto noBinding=session.world().state();noBinding.animation.reset();bool refused=false;
    try {encodeSnapshot(noBinding);}catch(const std::invalid_argument&) {refused=true;}check(refused,"unbound pose accepted");
    session.stop(actor);session.step();check(session.world().find(actor)->motion->pose->displayed==shown->displayed,"repeated Stop lost pose");
    session.move(actor,{5,1,1});session.step({true,false,true});
    check(session.world().find(actor)->motion->action==Action::planning && session.world().find(actor)->motion->pose.has_value(),"reorder planning lost display");
    session.step();session.step();const auto& moving=*session.world().find(actor)->motion;
    check(moving.fine && moving.fine->animation && displayedPose(moving)->displayed==*moving.fine->animation->displayed,"active controller did not supersede static pose");
    session.stop(actor);session.move(actor,{5,1,1});session.step();check(session.world().find(actor)->motion->action==Action::moving,"FIFO stop/restart failed");
    session.enqueue({Operation::cleanup,actor,{}});session.step();check(session.world().find(actor)->cleaned && !session.world().find(actor)->motion->pose,"cleanup retained pose");
    session.enqueue({Operation::release,actor,{}});session.step();check(!session.world().find(actor),"release retained actor");
    auto replacement=session.spawn(e,true,true);check(replacement.generation!=actor.generation && !session.world().find(replacement)->motion->pose,"reused slot inherited pose");
    std::cout<<"Static Stop pose, v8 restart, bound ANI refusal/rollback, reorder, FIFO, cleanup and reuse passed\n";
    return 0;
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
