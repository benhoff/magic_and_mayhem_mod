#include "session.hpp"
#include "movement_controls.hpp"
#include "playback_controls.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QComboBox>
#include <QEventLoop>
#include <QPushButton>
#include <QTimer>
#include <iostream>
#include <stdexcept>

using namespace mnm;
namespace {
void check(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
template<class F> void refuses(F action) {
    bool caught=false;try {action();} catch(const std::exception&) {caught=true;}
    check(caught,"Expected lifecycle refusal");
}
struct Navigation final:game::Navigation {
    std::function<void()> beforePlan;
    explicit Navigation(unsigned* disposed=nullptr):disposed_(disposed) {}
    ~Navigation() override {if(disposed_) ++*disposed_;}
    game::NavigationBinding binding() const override {return {{12,12,3},16};}
    std::uint32_t creatureType() const override {return 7;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    bool accepts(const game::Entity& e,const game::RoutePoint& p) const override {
        return std::abs(e.x-p.position.x)==1 && e.y==p.position.y && e.z==p.position.z;
    }
    game::RoutePlan plan(const game::Entity& e,game::Point goal,std::uint32_t) const override {
        if(beforePlan) beforePlan();
        if(e.y!=goal.y || e.z!=goal.z) return {};
        game::RoutePlan result{game::PlanStatus::reachable,{},1};auto x=e.x;
        while(x!=goal.x) {const int direction=x<goal.x?1:-1;x+=direction;
            result.route.push_back({{x,e.y,e.z},direction>0?2:6,0,0,720});}
        return result;
    }
private:
    unsigned* disposed_;
};
game::State initial(const std::shared_ptr<Navigation>& navigation) {
    game::World world(8);auto state=world.state();state.map="generated-scene-lifecycle";
    state.navigation=navigation->binding();world.restore(std::move(state));
    game::MovementSession simulation(std::move(world),navigation);
    game::Entity creature;creature.type=7;creature.x=1;creature.y=1;creature.z=1;
    simulation.spawn(creature);creature.y=3;simulation.spawn(creature);
    return simulation.world().state();
}
game::Bytes bytes(const scene::SceneSession& owner) {return game::encodeSnapshot(owner.simulation().world().state());}
void drainEvents() {QEventLoop loop;QTimer::singleShot(40,&loop,&QEventLoop::quit);loop.exec();}
}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);app.setQuitOnLastWindowClosed(false);
    if(argc==5 && std::string(argv[1])=="resume") {
        scene::SceneSession owner(game::readSnapshot(argv[2]),std::make_shared<Navigation>());
        for(int i=0;i<std::stoi(argv[4]);++i) owner.playback().step();
        check(game::writeSnapshot(argv[3],owner.simulation().world().state()).durable,"Fresh-process publication");
        owner.close();check(owner.playback().retired(),"Fresh-process retirement");return 0;
    }
    check(argc==2,"Supply new lifecycle artifact directory");
    const std::filesystem::path output=argv[1];check(std::filesystem::create_directories(output),"Fresh lifecycle directory");
    unsigned disposed=0;auto navigation=std::make_shared<Navigation>(&disposed);
    const std::weak_ptr<Navigation> weak=navigation;auto state=initial(navigation);
    const auto actor=game::Handle{0,state.slots[0].generation};std::uint64_t now=0;
    scene::SceneSession owner(state,navigation,[&] {return now;});navigation.reset();
    check(owner.active() && !owner.playback().playing() && !owner.orders().selected(),"Construction owns a paused unselected scene");
    const auto original=bytes(owner);unsigned refreshes=0,errors=0;
    owner.playback().onRefresh=[&] {++refreshes;};owner.playback().onError=[&](const std::string&) {++errors;};
    scene::MovementControls controls;scene::PlaybackControls playback;
    const auto refresh=[&] {controls.updateChoices(scene::creatureChoices(owner.simulation().world().state()),owner.orders().selected(),owner.navigation().binding().dimensions);};
    controls.onSelect=[&](std::optional<game::Handle> selected) {owner.orders().select(owner.simulation().world().state(),selected);refresh();};
    controls.onMove=[&](game::Point destination) {owner.orders().move(owner.simulation(),destination);};
    controls.onStop=[&] {owner.orders().stop(owner.simulation());};
    controls.onCancelQueuedMoves=[&] {owner.orders().cancelQueuedMoves(owner.simulation());};
    playback.onPlay=[&] {owner.playback().play();};playback.onPause=[&] {owner.playback().pause();};playback.onStep=[&] {owner.playback().step();};
    owner.playback().onPlaying=[&](bool playing) {playback.updatePlaying(playing);};
    refresh();controls.show();playback.show();app.processEvents();
    auto* selection=controls.findChild<QComboBox*>("creatureSelection");check(selection,"Selection widget exists");
    selection->setCurrentIndex(1);check(owner.orders().selected()==actor && bytes(owner)==original,"Qt selection does not tick");
    controls.setTarget({4,1,1});controls.findChild<QPushButton*>("queueMove")->click();
    check(owner.simulation().world().state().pending.size()==1,"Qt move reaches owned queue");
    playback.findChild<QPushButton*>("playSimulation")->click();now=100;owner.playback().poll();
    check(owner.simulation().world().state().tick==1 && owner.simulation().world().state().pending.empty(),"Owned playback admits queued order");
    playback.findChild<QPushButton*>("pauseSimulation")->click();
    owner.orders().stop(owner.simulation());const auto pending=bytes(owner);
    check(game::writeSnapshot(output/"pending.mnw",owner.simulation().world().state()).durable,"Publish pending checkpoint");
    drainEvents();check(bytes(owner)==pending,"Pause preserves pending continuation");
    for(unsigned i=0;i<3;++i) playback.findChild<QPushButton*>("stepSimulation")->click();
    check(game::writeSnapshot(output/"whole.mnw",owner.simulation().world().state()).durable,"Publish continued checkpoint");
    check(owner.simulation().world().find(actor)->motion->action==game::Action::cancelled,"Queued Stop admitted");
    const auto stable=bytes(owner);auto invalid=state;invalid.navigation->fingerprint^=1;
    refuses([&] {scene::SceneSession rejected(invalid,std::make_shared<Navigation>());});
    refuses([&] {scene::SceneSession rejected(state,{});});
    refuses([&] {scene::SceneSession::load(output/"missing.mnw");});
    check(owner.active() && owner.orders().selected()==actor && bytes(owner)==stable,"Rejected startup leaves other scene intact");
    owner.orders().selectPlacement(owner.simulation().world().state(),game::Point{5,1,1});
    playback.findChild<QPushButton*>("playSimulation")->click();const auto notifications=refreshes;
    owner.close();owner.close();check(!owner.active() && owner.playback().retired() && !owner.playback().playing(),"Idempotent terminal close");
    check(weak.expired() && disposed==1 && !owner.orders().selected() && !owner.orders().placement(),"Close releases provider/world/transient selection");
    refuses([&] {owner.simulation();});refuses([&] {static_cast<const scene::SceneSession&>(owner).simulation();});refuses([&] {owner.navigation();});
    now=100000;owner.playback().play();owner.playback().step();owner.playback().poll();owner.playback().pause();drainEvents();
    check(refreshes==notifications && !owner.playback().playing() && !errors,"Retired timer and direct actions are inert");
    {
        unsigned destroyed=0;auto n=std::make_shared<Navigation>(&destroyed);auto s=initial(n);std::weak_ptr<Navigation> lifetime=n;
        auto current=std::make_unique<scene::SceneSession>(s,n);n.reset();current->playback().play();current.reset();
        check(lifetime.expired() && destroyed==1,"Destruction releases a playing scene");drainEvents();
    }
    {
        unsigned destroyed=0;auto n=std::make_shared<Navigation>(&destroyed);auto s=initial(n);std::uint64_t clock=0;
        scene::SceneSession closing(s,n,[&] {return clock;});
        n->beforePlan=[&] {closing.close();check(destroyed==0,"In-flight storage cannot be freed in its navigation callback");};
        n.reset();closing.simulation().move(actor,{4,1,1});unsigned notified=0;
        closing.playback().onRefresh=[&] {++notified;};closing.playback().play();clock=400;closing.playback().poll();
        check(!closing.active() && closing.playback().retired() && destroyed==1 && !notified,"In-flight close retires batch and releases after step");
    }
    {
        auto n=std::make_shared<Navigation>();auto s=initial(n);scene::SceneSession failing(s,n);
        n->beforePlan=[] {throw std::runtime_error("injected plan failure");};failing.simulation().move(actor,{4,1,1});
        const auto before=bytes(failing);unsigned failures=0;
        failing.playback().onError=[&](const std::string& message) {check(message=="injected plan failure","Exact failure");++failures;};
        failing.playback().step();check(failures==1 && failing.active() && bytes(failing)==before,"Failed tick preserves committed state");
        drainEvents();check(failures==1,"No implicit fault retry");failing.close();
    }
    {
        auto n=std::make_shared<Navigation>();scene::SceneSession next(initial(n),n);
        check(!next.orders().selected() && !next.playback().playing() && next.simulation().world().state().tick==0,"New scene cannot inherit retired selection/clock");
        next.afterTick=[&] {next.close();};next.playback().step();check(!next.active(),"Close from committed-tick observer");
    }
    {
        auto n=std::make_shared<Navigation>();scene::SceneSession zero(initial(n),n,{},0);
        check(zero.geometryFingerprint()==0,"A zero geometry fingerprint is a value, not an absent override");
    }
    std::cout<<"{\"success\":true,\"qt_controls\":true,\"terminal_retirement\":true,\"inflight_close\":true,\"failed_startup_atomic\":true,\"failed_tick_atomic\":true,\"provider_release\":true,\"fresh_owner\":true}"<<'\n';
    return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
