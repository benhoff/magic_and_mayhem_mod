#include "playback.hpp"
#include "playback_controls.hpp"
#include "orders.hpp"
#include "persistence/snapshot.hpp"
#include <QApplication>
#include <QEventLoop>
#include <QPushButton>
#include <QTimer>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm;
static void check(bool b) {if(!b) throw std::runtime_error("Playback assertion failed");}
template<class F> static void refused(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
struct Navigation final:game::Navigation {
    game::NavigationBinding binding() const override {return {{12,12,3},16};}
    std::uint32_t creatureType() const override {return 7;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    bool accepts(const game::Entity& e,const game::RoutePoint& p) const override {return std::abs(e.x-p.position.x)==1 && e.y==p.position.y && e.z==p.position.z;}
    game::RoutePlan plan(const game::Entity& e,game::Point target,std::uint32_t) const override {
        if(e.y!=target.y || e.z!=target.z) return {};
        game::RoutePlan p{game::PlanStatus::reachable,{},1};auto x=e.x;
        while(x!=target.x) {const auto d=x<target.x?1:-1;x+=d;p.route.push_back({{x,e.y,e.z},d>0?2:6,0,0,720});}return p;
    }
};
static game::MovementSession session(game::State state) {game::World w(0);w.restore(std::move(state));return {std::move(w),std::make_shared<Navigation>()};}
static game::Bytes bytes(const game::MovementSession& s) {return game::encodeSnapshot(s.world().state());}
static void wait(unsigned ms) {QEventLoop loop;QTimer::singleShot(ms,&loop,&QEventLoop::quit);loop.exec();}
int main(int argc,char** argv) try {
    QApplication app(argc,argv);
    if(argc==5 && std::string(argv[1])=="resume") {auto s=session(game::readSnapshot(argv[2]));for(int i=0;i<std::stoi(argv[4]);++i) s.step();check(game::writeSnapshot(argv[3],s.world().state()).durable);return 0;}
    // Independent virtual-time reference uses individual millisecond admission.
    std::uint64_t comparisons=0;
    for(unsigned phase=0;phase<100;++phase) {
        game::TickClock c;std::uint64_t now=phase,accumulated=0;c.play(now);
        for(unsigned wake=0;wake<100;++wake) {
            const unsigned delay=(phase*37+wake*113)%1200;std::uint64_t due=0;
            for(unsigned ms=0;ms<delay;++ms) {if(++accumulated==100) {++due;accumulated=0;}}
            now+=delay;const auto batch=c.admit(now);check(batch.ticks==std::min<std::uint64_t>(due,4) && batch.dropped==due-batch.ticks);++comparisons;
            if(wake%11==0) {c.pause();now+=30000;check(c.admit(now).ticks==0);accumulated=0;c.play(now);}
        }
    }
    game::TickClock c;c.play(100);check(c.admit(199).ticks==0);c.play(199);check(c.admit(200).ticks==1);
    refused([&] {c.admit(199);});check(c.admit(300).ticks==1);c.pause();c.play(0);
    auto huge=c.admit(std::numeric_limits<std::uint64_t>::max());check(huge.ticks==4 && huge.dropped==std::numeric_limits<std::uint64_t>::max()/100-4);check(c.admit(std::numeric_limits<std::uint64_t>::max()).ticks==0);
    auto nav=std::make_shared<Navigation>();game::World w(8);auto state=w.state();state.map="synthetic-playback";state.navigation=nav->binding();w.restore(state);game::MovementSession s(std::move(w),nav);
    game::Entity e;e.type=7;e.x=1;e.y=1;e.z=1;const auto a=s.spawn(e);e.y=3;const auto b=s.spawn(e);
    std::uint64_t now=1000;scene::Playback driver([&] {s.step();},[&] {return now;});scene::PlaybackControls controls;
    auto* play=controls.findChild<QPushButton*>("playSimulation");auto* pause=controls.findChild<QPushButton*>("pauseSimulation");auto* step=controls.findChild<QPushButton*>("stepSimulation");check(play && pause && step);
    unsigned refreshes=0,errors=0;driver.onRefresh=[&] {++refreshes;};driver.onPlaying=[&](bool value) {controls.updatePlaying(value);};driver.onError=[&](const std::string&) {++errors;};
    controls.onPlay=[&] {driver.play();};controls.onPause=[&] {driver.pause();};controls.onStep=[&] {driver.step();};
    scene::Orders orders;orders.select(s.world().state(),a);orders.move(s,{5,1,1});s.move(b,{5,3,1});
    auto before=bytes(s);check(!driver.playing() && play->isEnabled() && !pause->isEnabled() && step->isEnabled());driver.poll();check(bytes(s)==before);
    play->click();check(driver.playing() && !play->isEnabled() && pause->isEnabled() && !step->isEnabled() && bytes(s)==before);
    step->click();driver.step();check(bytes(s)==before);now=1099;driver.poll();check(bytes(s)==before && refreshes==0);now=1100;driver.poll();check(s.world().state().tick==1 && refreshes==1 && s.world().state().pending.empty());
    now=1150;driver.poll();pause->click();before=bytes(s);now=90000;driver.poll();check(bytes(s)==before && orders.selected()==a);
    orders.stop(s);const auto pending=bytes(s);now+=50000;driver.poll();check(bytes(s)==pending && s.world().state().pending.size()==1);
    if(argc==2) {std::filesystem::create_directories(argv[1]);check(game::writeSnapshot(std::filesystem::path(argv[1])/"pending.mnw",s.world().state()).durable);controls.show();app.processEvents();check(controls.grab().save(QString::fromStdString((std::filesystem::path(argv[1])/"controls.png").string())));}
    auto restored=session(game::decodeSnapshot(pending));
    // Resume discards pre-pause fractional debt; equal admitted ticks produce equal bytes.
    play->click();now+=99;driver.poll();check(bytes(s)==pending);now+=1;driver.poll();restored.step();check(bytes(s)==bytes(restored) && s.world().find(a)->motion->action==game::Action::cancelled);
    now+=250;driver.poll();restored.step();restored.step();check(bytes(s)==bytes(restored));pause->click();
    if(argc==2) check(game::writeSnapshot(std::filesystem::path(argv[1])/"whole.mnw",s.world().state()).durable);
    // Step advances exactly once while paused and leaves the timing baseline fresh.
    before=bytes(s);step->click();check(s.world().state().tick==game::decodeSnapshot(before).tick+1);before=bytes(s);driver.poll();check(bytes(s)==before);
    play->click();now+=100000;const auto tick=s.world().state().tick,frames=refreshes;driver.poll();check(s.world().state().tick==tick+4 && refreshes==frames+1);driver.poll();check(s.world().state().tick==tick+4);pause->click();
    // A failure pauses at the last committed tick; automatic retry is forbidden.
    unsigned attempted=0;std::uint64_t faultNow=0;auto faultState=s.world().state();auto faultSession=session(faultState);
    scene::Playback fault([&] {if(++attempted==2) throw std::runtime_error("fixture tick failure");faultSession.step();},[&] {return faultNow;});unsigned faults=0;fault.onError=[&](const std::string& text) {check(text=="fixture tick failure");++faults;};
    fault.play();faultNow=400;fault.poll();check(!fault.playing() && faults==1 && attempted==2 && faultSession.world().state().tick==faultState.tick+1);fault.poll();check(attempted==2);
    fault.play();faultNow=399;fault.poll();check(!fault.playing() && attempted==2); // Backwards time pauses before ticking.
    // Reentrant polling/stepping cannot add ticks; pause inside a callback stops the batch.
    unsigned nested=0;std::uint64_t nestedNow=0;scene::Playback* pointer=nullptr;
    scene::Playback recursive([&] {++nested;pointer->poll();pointer->step();pointer->play();pointer->pause();},[&] {return nestedNow;});pointer=&recursive;recursive.play();nestedNow=400;recursive.poll();check(nested==1 && !recursive.playing());
    // Notification faults cannot escape Qt-style dispatch, and leave timing paused.
    scene::Playback notify([] {},[] {return 0;});notify.onPlaying=[](bool) {throw std::runtime_error("refresh failure");};notify.onError=[](const std::string&) {throw std::runtime_error("error display failure");};notify.play();check(!notify.playing());
    // Actual QTimer dispatch with a fake monotonic source; no sleeps or timing thresholds determine tick count.
    unsigned timerTicks=0;std::uint64_t timerNow=0;scene::Playback timer([&] {++timerTicks;},[&] {return timerNow;});timer.play();timerNow=100;
    QEventLoop loop;timer.onRefresh=[&] {timer.pause();loop.quit();};QTimer::singleShot(2000,&loop,&QEventLoop::quit);loop.exec();check(timerTicks==1 && !timer.playing());
    // Exercise the default real elapsed source as well; wait for the first admitted tick, then pause.
    unsigned realTicks=0;scene::Playback real([&] {++realTicks;});QEventLoop realLoop;real.onRefresh=[&] {real.pause();realLoop.quit();};real.play();QTimer::singleShot(3000,&realLoop,&QEventLoop::quit);realLoop.exec();check(realTicks>=1 && realTicks<=4 && !real.playing());auto count=realTicks;wait(150);check(realTicks==count);
    std::cout<<"{\"all_match\":true,\"clock_oracle_comparisons\":"<<comparisons<<",\"pause_preserves_pending\":true,\"paused_step_only\":true,\"bounded_catch_up\":true,\"fresh_resume_interval\":true,\"same_tick_checkpoint_bytes\":true,\"failure_pauses\":true,\"reentrancy_guard\":true,\"qt_timer_dispatch\":true,\"real_elapsed_source\":true}"<<'\n';return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
