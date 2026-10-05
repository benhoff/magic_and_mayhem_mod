#include "movement_controls.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QPushButton>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool b) {if(!b) throw std::runtime_error("Stop orders assertion failed");}
template<class F> static void refused(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
struct Navigation final:game::Navigation {
    game::NavigationBinding binding() const override {return {{12,12,3},15};}
    std::uint32_t creatureType() const override {return 7;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    bool accepts(const game::Entity& e,const game::RoutePoint& p) const override {return std::abs(e.x-p.position.x)==1 && e.y==p.position.y && e.z==p.position.z;}
    game::RoutePlan plan(const game::Entity& e,game::Point target,std::uint32_t) const override {
        if(e.y!=target.y || e.z!=target.z) return {};
        game::RoutePlan p{game::PlanStatus::reachable,{},1};auto x=e.x;
        while(x!=target.x) {const auto d=x<target.x?1:-1;x+=d;p.route.push_back({{x,e.y,e.z},d>0?2:6,0,0,720});}return p;
    }
    game::FineMotion prepareFineMotion(const game::Entity& e,const game::RoutePoint&) const override {
        game::FineMotion f;f.rate=50;f.duration=100;f.heightOrigin=e.z*16;f.fine={e.x*32,e.y*32,e.z*16};return f;
    }
    bool advanceFineMotion(const game::Entity& e,const game::RoutePoint& p,game::FineMotion& f) const override {
        f.accumulator+=f.rate;if(f.accumulator>=f.duration) {f.accumulator-=f.duration;f.progress+=30;f.travelX=(p.position.x>e.x?1:-1)*f.progress;f.fine.x=e.x*32+f.travelX/6;f.frame=(f.frame+1)%12;}return f.progress>=192;
    }
    void validateFineMotion(const game::Entity&,const game::RoutePoint&,const game::FineMotion& f) const override {check(f.rate==50 && f.duration==100);}
};
static game::MovementSession session(game::State state) {game::World w(0);w.restore(std::move(state));return {std::move(w),std::make_shared<Navigation>()};}
static game::Bytes bytes(const game::MovementSession& s) {return game::encodeSnapshot(s.world().state());}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);
    if(argc==5 && std::string(argv[1])=="resume") {auto s=session(game::readSnapshot(argv[2]));for(int i=0;i<std::stoi(argv[4]);++i) s.step();check(game::writeSnapshot(argv[3],s.world().state()).durable);return 0;}
    auto nav=std::make_shared<Navigation>();game::World w(8);auto st=w.state();st.map="synthetic-stop-orders";st.navigation=nav->binding();w.restore(st);game::MovementSession s(std::move(w),nav);
    game::Entity e;e.type=7;e.x=1;e.y=1;e.z=1;auto a=s.spawn(e,true);e.y=3;auto b=s.spawn(e);
    st=s.world().state();st.slots[a.slot].generation=0xfedcba98;a.generation=0xfedcba98;s=session(st);
    scene::Orders orders;scene::MovementControls controls;auto* stop=controls.findChild<QPushButton*>("queueStop");auto* cancel=controls.findChild<QPushButton*>("cancelQueuedMoves");check(stop && cancel);
    const auto refresh=[&] {orders.synchronize(s.world().state());controls.updateChoices(scene::creatureChoices(s.world().state()),orders.selected(),nav->binding().dimensions);};
    unsigned calls=0,removed=0;controls.onStop=[&] {++calls;orders.stop(s);refresh();};controls.onCancelQueuedMoves=[&] {++calls;removed=orders.cancelQueuedMoves(s);refresh();};
    refresh();check(!stop->isEnabled() && !cancel->isEnabled());auto before=bytes(s);stop->click();cancel->click();check(calls==0);
    refused([&] {orders.stop(s);});refused([&] {orders.cancelQueuedMoves(s);});check(bytes(s)==before);
    orders.select(s.world().state(),a);refresh();check(stop->isEnabled() && cancel->isEnabled());
    s.move(a,{4,1,1});s.move(b,{4,3,1});s.enqueue({game::Operation::target,a,b});s.stop(a);s.move(a,{5,1,1});
    const auto retained=s.world().state().pending;cancel->click();check(removed==2 && s.world().state().tick==0 && s.world().state().pending.size()==3);
    for(unsigned i=0;i<3;++i) {const auto& c=s.world().state().pending[i];check(c.operation==retained[i+1].operation && c.subject==retained[i+1].subject && c.destination==retained[i+1].destination && c.target==retained[i+1].target);}
    before=bytes(s);cancel->click();check(removed==0 && bytes(s)==before);
    s.step();check(s.world().find(a)->motion->action==game::Action::cancelled && s.world().find(a)->target==b && s.world().find(b)->motion->action==game::Action::moving);
    // Active cancellation removes only pending moves; the route still advances.
    s.move(a,{5,1,1});s.step();s.step();s.step();check(s.world().find(a)->motion->fine && s.finePosition(*s.world().find(a)).x>32);
    s.move(a,{6,1,1});before=bytes(s);const auto progress=s.world().find(a)->motion->fine->progress;cancel->click();check(removed==1 && s.world().find(a)->motion->fine->progress==progress);s.step();
    // Pending stop does not mutate fine position until an admitted Step.
    const auto fine=s.finePosition(*s.world().find(a));const auto tick=s.world().state().tick;stop->click();check(s.world().state().tick==tick && s.finePosition(*s.world().find(a))==fine);
    before=bytes(s);check(before[8]==7 && bytes(session(game::decodeSnapshot(before)))==before);
    check(!s.step({false,false,false}).advanced && bytes(s)==before);
    auto oldVersion=before;oldVersion[8]=6;refused([&] {game::decodeSnapshot(oldVersion);});
    if(argc==2) {std::filesystem::create_directories(argv[1]);check(game::writeSnapshot(std::filesystem::path(argv[1])/"pending.mnw",s.world().state()).durable);controls.show();app.processEvents();check(controls.grab().save(QString::fromStdString((std::filesystem::path(argv[1])/"controls.png").string())));}
    auto restored=session(game::decodeSnapshot(before));const auto bx=s.world().find(b)->x;
    for(unsigned i=0;i<3;++i) {s.step();restored.step();check(bytes(s)==bytes(restored));}
    const auto& stopped=*s.world().find(a);check(stopped.x==1 && stopped.motion->action==game::Action::cancelled && stopped.motion->route.empty() && !stopped.motion->fine && !stopped.motion->previous && !stopped.motion->goal && stopped.motion->next==0 && stopped.motion->segmentTicks==0 && stopped.motion->sampleMotion && !stopped.cleaned && stopped.target==b);
    check(s.finePosition(stopped)==game::Point{32,32,16} && s.world().find(b)->x>=bx && bytes(s)[8]==3);
    if(argc==2) check(game::writeSnapshot(std::filesystem::path(argv[1])/"whole.mnw",s.world().state()).durable);
    // FIFO: stop after move wins; move after stop restarts. Repeated stop is valid.
    s.move(a,{4,1,1});s.stop(a);s.stop(a);s.step();check(s.world().find(a)->motion->action==game::Action::cancelled);
    s.stop(a);s.move(a,{4,1,1});s.step();check(s.world().find(a)->motion->action==game::Action::moving);
    // Stale queued stops are counted, never applied to replacement generations.
    s.enqueue({game::Operation::release,a,{}});s.stop(a);check(s.step().rejected==1);e.y=1;auto replacement=s.spawn(e);check(replacement.slot==a.slot && replacement.generation!=a.generation);
    before=bytes(s);refused([&] {orders.stop(s);});refused([&] {orders.cancelQueuedMoves(s);});check(bytes(s)==before && !orders.selected());refresh();check(!stop->isEnabled() && !cancel->isEnabled());
    s.stop(a);check(s.step().rejected==1 && s.world().find(replacement)->motion->action==game::Action::idle);
    orders.select(s.world().state(),replacement);s.enqueue({game::Operation::cleanup,replacement,{}});s.stop(replacement);check(s.step().rejected==1);before=bytes(s);refused([&] {s.cancelQueuedMoves(replacement);});check(bytes(s)==before);
    // Malformed stop payloads and absent navigation are refused atomically.
    game::Command bad{game::Operation::stop,b,a};before=bytes(s);refused([&] {s.enqueue(bad);});bad.target.reset();bad.destination=game::Point{1,1,1};refused([&] {s.enqueue(bad);});check(bytes(s)==before);
    game::World empty(1);refused([&] {empty.enqueue({game::Operation::stop,{0,1},{}});});
    // A failed tick rolls pending stop and movement back together.
    auto rollback=s.world().state();rollback.pending.clear();rollback.pending.push_back({game::Operation::stop,b,{}});game::World raw(0);raw.restore(rollback);auto rawBytes=game::encodeSnapshot(raw.state());
    refused([&] {raw.step({},[](game::Phase,const game::State&,std::vector<game::Command>&) {throw std::runtime_error("fixture failure");});});check(game::encodeSnapshot(raw.state())==rawBytes);
    refused([&] {raw.step({},[&](game::Phase,const game::State&,std::vector<game::Command>&) {raw.cancelQueuedMoves(b);});});check(game::encodeSnapshot(raw.state())==rawBytes);
    // Queue budget cannot be bypassed by stop admission.
    game::Limits limits;limits.commands=0;game::World limited(0,limits);auto bounded=rollback;bounded.pending.clear();limited.restore(bounded);before=game::encodeSnapshot(limited.state());refused([&] {limited.enqueue({game::Operation::stop,b,{}});});check(game::encodeSnapshot(limited.state())==before);
    std::cout<<"{\"all_match\":true,\"selected_pending_only\":true,\"active_route_preserved_by_cancel\":true,\"tick_boundary_stop\":true,\"fine_snap_to_logical_cell\":true,\"fifo_restart\":true,\"v7_roundtrip\":true,\"legacy_stop_refusal\":true,\"stale_cleanup_refusal\":true,\"transaction_rollback\":true}"<<'\n';return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
