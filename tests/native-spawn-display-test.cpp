#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
static void check(bool ok,const char* reason) {if(!ok) throw std::runtime_error(reason);}
int main(int argc,char** argv) try {
    check(argc==4,"fixture arguments required");auto animation=mnm::sandbox::loadMovementAnimation(argv[2],8);
    auto nav=mnm::sandbox::loadFrozenNavigation(argv[1],animation);
    const auto make=[&](std::shared_ptr<const Navigation> resource) {
        World world(8);auto state=world.state();state.map=argv[1];state.navigation=resource->binding();state.animation=resource->animationBinding();world.restore(state);
        return MovementSession(std::move(world),resource);
    };
    auto session=make(nav);Entity e;e.type=nav->creatureType();e.x=e.y=e.z=1;
    auto actor=session.spawn(e,true,true,false,true);const auto& m=*session.world().find(actor)->motion;
    check(m.action==Action::idle && m.pose && m.pose->sequence==8 && m.pose->displayed==1,"spawn must choose first bitmap of direction zero");
    check(!m.fine && !m.previous && m.route.empty() && !m.segmentTicks && session.world().state().tick==0 && session.world().state().pending.empty(),"spawn executed movement or clock");
    const auto saved=encodeSnapshot(session.world().state());check(saved[8]==8,"initial pose must persist in v8");
    check(writeSnapshot(argv[3],session.world().state()).durable,"initial checkpoint publication failed");
    World world(0);world.restore(decodeSnapshot(saved));MovementSession copy(std::move(world),nav);
    for(unsigned i=0;i<20;++i) {session.step();copy.step();check(encodeSnapshot(session.world().state())==encodeSnapshot(copy.world().state()),"idle restore drifted");}
    check(session.world().find(actor)->motion->pose->sequence==8 && session.world().find(actor)->motion->pose->displayed==1,"idle pose advanced");
    auto invalidSession=make(nav);const auto before=encodeSnapshot(invalidSession.world().state());
    for(const auto family: {Family::missileEffect,Family::mapLinked}) {
        Entity invalid;invalid.family=family;
        bool refused=false;try {invalidSession.spawn(invalid,true,true,false,true);}catch(const std::invalid_argument&) {refused=true;}
        check(refused && encodeSnapshot(invalidSession.world().state())==before,"invalid-family spawn mutated state");
    }
    bool refused=false;try {invalidSession.spawn(e,false,false,false,true);}catch(const std::invalid_argument&) {refused=true;}
    check(refused && encodeSnapshot(invalidSession.world().state())==before,"invalid policy mutated state");
    auto clean=e;clean.cleaned=true;refused=false;try {invalidSession.spawn(clean,true,true,false,true);}catch(const std::invalid_argument&) {refused=true;}
    check(refused && encodeSnapshot(invalidSession.world().state())==before,"cleaned initial pose accepted");
    session.move(actor,{5,1,1});session.step({true,false,true});check(session.world().find(actor)->motion->pose->sequence==8,"planning lost initial display");
    session.step();session.step();const auto active=displayedPose(*session.world().find(actor)->motion);
    check(active && active->sequence==10,"movement must supersede direction-zero pose");
    session.stop(actor);session.step();check(session.world().find(actor)->motion->pose->sequence==10,"Stop restored obsolete spawn pose");
    session.enqueue({Operation::release,actor,{}});session.step();auto replacement=session.spawn(e,true,true,false,true);
    check(replacement.generation!=actor.generation && session.world().find(replacement)->motion->pose->sequence==8,"reuse must initialize a fresh pose");
    // Selected sequence contains only controls: explicit initialization must refuse transactionally.
    animation.data[44+17*4+9*44]=6;
    animation.data[44+17*4+10*44]=6;
    auto missing=make(mnm::sandbox::loadFrozenNavigation(argv[1],animation));const auto untouched=encodeSnapshot(missing.world().state());
    refused=false;try {missing.spawn(e,true,true,false,true);}catch(const std::invalid_argument&) {refused=true;}
    check(refused && encodeSnapshot(missing.world().state())==untouched,"missing bitmap changed world");
    auto unbound=make(mnm::sandbox::loadFrozenNavigation(argv[1]));const auto unboundBefore=encodeSnapshot(unbound.world().state());
    refused=false;try {unbound.spawn(e,true,true,false,true);}catch(const std::invalid_argument&) {refused=true;}
    check(refused && encodeSnapshot(unbound.world().state())==unboundBefore,"unbound initial display changed world");
    std::cout<<"Initial static pose, no ticks/events, checkpoint restart, policy refusal/rollback, movement priority and fresh generation reuse passed\n";return 0;
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
