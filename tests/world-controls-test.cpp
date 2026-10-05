#include "movement_controls.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void check(bool b) {if(!b) throw std::runtime_error("Scene orders assertion failed");}
template<class F> static void refused(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
struct Navigation final:game::Navigation {
    game::NavigationBinding binding() const override {return {{12,12,3},13};}
    std::uint32_t creatureType() const override {return 7;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    bool accepts(const game::Entity& e,const game::RoutePoint& p) const override {return std::abs(e.x-p.position.x)==1 && e.y==p.position.y && e.z==p.position.z;}
    game::RoutePlan plan(const game::Entity& e,game::Point target,std::uint32_t) const override {
        if(e.y!=target.y || e.z!=target.z) return {};
        game::RoutePlan p{game::PlanStatus::reachable,{},1};
        auto x=e.x;while(x!=target.x) {const auto d=x<target.x?1:-1;x+=d;p.route.push_back({{x,e.y,e.z},d>0?2:6,0,0,720});}return p;
    }
};
static game::MovementSession session(game::State state) {
    game::World w(0);w.restore(std::move(state));return game::MovementSession(std::move(w),std::make_shared<Navigation>());
}
static game::Bytes bytes(const game::MovementSession& s) {return game::encodeSnapshot(s.world().state());}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);
    if(argc==5 && std::string(argv[1])=="resume") {
        auto s=session(game::readSnapshot(argv[2]));for(int i=0;i<std::stoi(argv[4]);++i) s.step();
        check(game::writeSnapshot(argv[3],s.world().state()).durable);return 0;
    }
    auto nav=std::make_shared<Navigation>();game::World w(8);auto state=w.state();state.map="synthetic-scene-orders";state.navigation=nav->binding();w.restore(state);
    game::MovementSession s(std::move(w),nav);game::Entity e;e.type=7;e.x=1;e.y=1;e.z=1;auto a=s.spawn(e);e.y=3;auto b=s.spawn(e);
    // Deliberately exercise identity outside signed 32-bit without game binaries.
    state=s.world().state();state.slots[a.slot].generation=0xfedcba98;a.generation=0xfedcba98;s=session(state);
    scene::Orders orders;scene::MovementControls controls;
    auto* combo=controls.findChild<QComboBox*>("creatureSelection");auto* move=controls.findChild<QPushButton*>("queueMove");
    auto* x=controls.findChild<QSpinBox*>("targetX");auto* y=controls.findChild<QSpinBox*>("targetY");auto* z=controls.findChild<QSpinBox*>("targetZ");
    check(combo && move && x && y && z);unsigned selections=0,commands=0;
    const auto refresh=[&] {orders.synchronize(s.world().state());controls.updateChoices(scene::creatureChoices(s.world().state()),orders.selected(),nav->binding().dimensions);};
    controls.onSelect=[&](std::optional<game::Handle> h) {++selections;orders.select(s.world().state(),h);refresh();};
    controls.onMove=[&](game::Point target) {++commands;orders.move(s,target);refresh();};
    refresh();check(combo->count()==3 && !move->isEnabled() && !x->isEnabled());
    auto before=bytes(s);refused([&] {orders.move(s,{3,1,1});});check(bytes(s)==before);
    combo->setCurrentIndex(1);check(orders.selected()==a && move->isEnabled() && x->value()==1 && y->value()==1 && z->value()==1);
    check(bytes(s)==before && selections==1);
    x->setValue(3);refresh();refresh();check(x->value()==3 && selections==1);
    move->click();check(commands==1 && s.world().state().tick==0 && s.world().state().pending.size()==1);
    check(s.world().find(a)->x==1 && s.world().find(b)->x==1);
    const auto& queued=s.world().state().pending.front();check(queued.subject==a && queued.destination==game::Point{3,1,1});
    if(argc==2) {std::filesystem::create_directories(argv[1]);check(game::writeSnapshot(std::filesystem::path(argv[1])/"pending.mnw",s.world().state()).durable);controls.show();app.processEvents();check(controls.grab().save(QString::fromStdString((std::filesystem::path(argv[1])/"controls.png").string())));}
    auto restored=session(game::decodeSnapshot(bytes(s)));scene::Orders newSelection;check(!newSelection.selected());
    for(unsigned i=0;i<3;++i) {s.step();restored.step();check(bytes(s)==bytes(restored));}
    check(s.world().find(a)->x==3 && s.world().find(b)->x==1 && s.world().find(a)->motion->action==game::Action::arrived);
    if(argc==2) check(game::writeSnapshot(std::filesystem::path(argv[1])/"whole.mnw",s.world().state()).durable);
    refresh();check(x->value()==3 && selections==1 && orders.selected()==a);
    before=bytes(s);refused([&] {orders.move(s,{-1,1,1});});refused([&] {orders.move(s,{12,1,1});});check(bytes(s)==before && orders.selected()==a);
    refused([&] {orders.select(s.world().state(),game::Handle{a.slot,1});});check(orders.selected()==a && bytes(s)==before);
    combo->setCurrentIndex(2);check(orders.selected()==b && x->value()==1 && y->value()==3);
    x->setValue(3);move->click();check(s.world().state().pending.back().subject==b);
    for(unsigned i=0;i<3;++i) s.step();
    check(s.world().find(b)->x==3 && s.world().find(a)->x==3);
    // A blocked target is a simulation result, not a speculative UI admission rule.
    y->setValue(4);move->click();s.step();refresh();check(s.world().find(b)->motion->action==game::Action::blocked);
    s.enqueue({game::Operation::cleanup,b,{}});s.step();before=bytes(s);refused([&] {orders.move(s,{4,3,1});});check(!orders.selected() && bytes(s)==before);refresh();check(combo->count()==2 && !move->isEnabled());
    s.enqueue({game::Operation::release,b,{}});s.step();e.x=1;e.y=3;auto replacement=s.spawn(e);check(replacement.slot==b.slot && replacement.generation!=b.generation);
    refused([&] {orders.select(s.world().state(),b);});refresh();check(!orders.selected() && !move->isEnabled());
    orders.select(s.world().state(),replacement);refresh();check(move->isEnabled());
    game::Entity marker;marker.family=game::Family::mapLinked;marker.x=1;marker.y=4;marker.z=1;auto m=s.spawn(marker);
    before=bytes(s);refused([&] {orders.select(s.world().state(),m);});check(bytes(s)==before && orders.selected()==replacement);
    auto choices=scene::creatureChoices(s.world().state());check(choices.size()==2);
    orders.select(s.world().state(),std::nullopt);refresh();check(!move->isEnabled());
    const auto count=commands;move->click();check(commands==count);
    for(auto* spin:{x,y,z}) check(!spin->isEnabled());
    // Capacity-sized refresh and invalid domain never emit semantic commands.
    const auto calls=selections;
    choices.clear();for(unsigned i=0;i<32;++i) choices.push_back({{i,0xfedcba98},{1,1,1},game::Action::idle});
    controls.updateChoices(choices,std::nullopt,{12,12,3});check(combo->count()==33 && selections==calls);
    refused([&] {controls.updateChoices(choices,std::nullopt,{0,12,3});});
    std::cout<<"{\"all_match\":true,\"widget_selection\":true,\"queued_until_step\":true,\"selected_actor_only\":true,\"unsigned_identity\":true,\"checkpoint_pending_order\":true,\"cleanup_reuse_refusal\":true,\"blocked_result\":true,\"capacity_choices\":32}"<<'\n';
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
