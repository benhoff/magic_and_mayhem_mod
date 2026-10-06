#include "orders.hpp"
#include "movement_controls.hpp"
#include "frozen_navigation.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QPushButton>
#include <QComboBox>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool b,const char* why) {if(!b) throw std::runtime_error(why);}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);check(argc==4,"fixtures required");const auto ani=sandbox::loadMovementAnimation(argv[2],8);
    auto nav=sandbox::loadFrozenNavigation(argv[1],ani,true,true);
    const auto make=[&] {game::World w(40);auto s=w.state();s.map=argv[1];s.navigation=nav->binding();s.animation=ani;w.restore(s);return game::MovementSession(std::move(w),nav);};
    auto session=make();scene::Orders orders;scene::MovementControls controls;bool playing=false;unsigned calls=0;
    auto* button=controls.findChild<QPushButton*>("spawnCreature");auto* choices=controls.findChild<QComboBox*>("creatureSelection");check(button && choices,"spawn widgets missing");
    const auto bytes=[&] {return game::encodeSnapshot(session.world().state());};
    const auto refresh=[&] {controls.updateChoices(scene::creatureChoices(session.world().state()),orders.selected(),nav->binding().dimensions);controls.updateSpawning(orders.placement(),playing,session.canSpawnCreature());};
    controls.onSpawn=[&] {++calls;orders.spawn(session,!playing);refresh();};
    const auto reject=[&](auto f) {const auto before=bytes();const auto selected=orders.selected();bool caught=false;try {f();}catch(const std::exception&) {caught=true;}check(caught && before==bytes() && selected==orders.selected(),"spawn rejection changed session/selection");};
    refresh();check(!button->isEnabled(),"spawn needs terrain selection");reject([&] {orders.spawn(session,true);});
    orders.selectPlacement(session.world().state(),game::Point{1,1,1});refresh();check(button->isEnabled(),"paused selected cell must enable spawn");
    button->click();const auto a=*orders.selected();check(calls==1 && choices->currentIndex()==1 && choices->count()==2,"new creature must be selected");
    const auto* first=session.world().find(a);check(first && first->motion->pose && first->motion->action==game::Action::idle && first->motion->terrainMotion && session.world().state().tick==0 && session.world().state().pending.empty(),"spawn must own static zero-tick body");
    reject([&] {orders.spawn(session,true);}); // Selected occupied cell remains usable as a rejection case.
    orders.selectPlacement(session.world().state(),game::Point{1,3,1});playing=true;refresh();check(!button->isEnabled(),"playing must disable spawn");button->click();check(calls==1,"disabled spawn fired");reject([&] {orders.spawn(session,false);});
    playing=false;refresh();orders.selectPlacement(session.world().state(),game::Point{1,3,0});reject([&] {orders.spawn(session,true);});
    reject([&] {orders.selectPlacement(session.world().state(),game::Point{-1,1,1});});
    orders.move(session,{5,1,1});session.step();session.step();check(session.world().find(a)->motion->fine.has_value(),"reserve an unfinished edge");
    orders.selectPlacement(session.world().state(),game::Point{2,1,1});reject([&] {orders.spawn(session,true);});
    orders.selectPlacement(session.world().state(),game::Point{1,3,1});refresh();button->click();const auto b=*orders.selected();check(!(b==a) && choices->count()==3 && choices->currentIndex()==2,"second spawn identity/selection failed");
    orders.move(session,{5,3,1});check(session.world().state().pending.back().subject==b,"new actor did not receive its order");
    check(game::writeSnapshot(argv[3],session.world().state()).durable,"save failed");
    game::World w(0);w.restore(game::decodeSnapshot(bytes()));game::MovementSession restored(std::move(w),nav);scene::Orders transient;check(!transient.selected() && !transient.placement(),"transient selection survived reload");
    for(unsigned i=0;i<20;++i) {session.step();restored.step();check(bytes()==game::encodeSnapshot(restored.world().state()),"multi-actor continuation diverged");}
    // Actor cap must reject even with spare pool slots.
    auto full=make();scene::Orders cap;
    for(int y=1;y<=6;++y) for(int x=1;x<=6 && scene::creatureChoices(full.world().state()).size()<32;++x) {cap.selectPlacement(full.world().state(),game::Point{x,y,1});cap.spawn(full,true);}
    check(scene::creatureChoices(full.world().state()).size()==32 && !full.canSpawnCreature(),"32 actor admission cap failed");
    const auto before=game::encodeSnapshot(full.world().state());const auto selected=cap.selected();cap.selectPlacement(full.world().state(),game::Point{6,6,1});bool refused=false;try {cap.spawn(full,true);}catch(const std::exception&) {refused=true;}
    check(refused && game::encodeSnapshot(full.world().state())==before && cap.selected()==selected,"limit rejection mutated state");
    controls.updateSpawning(cap.placement(),false,full.canSpawnCreature());check(!button->isEnabled(),"limit must disable button");
    std::cout<<"Paused widgets/terrain spawn, occupied/unsupported/reserved refusal, 32 actor limit, selected orders and checkpoint continuation passed\n";return 0;
}catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
