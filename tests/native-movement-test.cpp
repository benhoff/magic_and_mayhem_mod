#include "simulation/movement.hpp"
#include "persistence/snapshot.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

using namespace mnm::game;
static void check(bool value) {if(!value) throw std::runtime_error("native movement assertion");}
template<class F> static void rejected(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
struct FixtureNavigation:Navigation {
    std::uint64_t fingerprint=1;
    bool blocked=false,fail=false,failAdvance=false;
    NavigationBinding binding() const override {return {{64,8,3},fingerprint};}
    std::uint32_t creatureType() const override {return 7;}
    RoutePlan plan(const Entity& e,Point target,std::uint32_t budget) const override {
        if(fail) throw std::runtime_error("fixture plan failure");
        if(blocked) return {PlanStatus::unreachable,{}};
        if(budget==2) return {PlanStatus::budgetExhausted,{}};
        RoutePlan result{PlanStatus::reachable,{}};
        auto x=e.x;
        while(x!=target.x && result.route.size()<16) {x+=x<target.x?1:-1;result.route.push_back({{x,target.y,target.z},x>e.x?2:6,0,0,720});}
        return result;
    }
    FineMotion prepareFineMotion(const Entity& e,const RoutePoint& p) const override {
        FineMotion f;f.rate=50;f.duration=100;f.heightOrigin=e.z*16;f.fine={e.x*32,e.y*32,e.z*16};
        if(e.motion->continuousMotion && e.motion->previous) {
            const auto& h=*e.motion->previous;f.accumulator=h.motion.accumulator;
            if(h.direction==p.direction) {f.progress=h.motion.progress-192;f.travelX=(p.direction==2?1:-1)*f.progress;f.frame=h.motion.frame;}
        }
        return f;
    }
    void validateSegmentHistory(const Entity&,const SegmentHistory& h) const override {if(h.motion.rate!=50 || h.motion.duration!=100) throw std::invalid_argument("fixture history profile mismatch");}
    void validateFineMotion(const Entity&,const RoutePoint&,const FineMotion& f) const override {if(f.rate!=50 || f.duration!=100) throw std::invalid_argument("fixture fine profile mismatch");}
    bool advanceFineMotion(const Entity& e,const RoutePoint& p,FineMotion& f) const override {
        if(failAdvance) throw std::runtime_error("fixture advance failure");
        f.accumulator+=f.rate;if(f.accumulator>=f.duration) {
            f.accumulator-=f.duration;f.progress+=30;f.travelX=(p.position.x>e.x?1:-1)*f.progress;
            f.fine.x=e.x*32+f.travelX/6;f.frame=(f.frame+1)%12;
        }
        return f.progress>=192;
    }
    bool accepts(const Entity& e,const RoutePoint& point) const override {return !blocked && std::abs(point.position.x-e.x)==1 && point.position.y==e.y && point.position.z==e.z;}
};
static MovementSession session(std::shared_ptr<const Navigation> navigation) {
    World world(4);auto state=world.state();state.map="fixture/map";state.navigation=navigation->binding();world.restore(state);
    return MovementSession(std::move(world),navigation);
}
static Handle creature(MovementSession& s) {Entity e;e.type=7;e.x=1;e.y=1;e.z=1;return s.spawn(e);}
static Bytes bytes(const MovementSession& s) {return encodeSnapshot(s.world().state());}
int main() try {
    auto navigation=std::make_shared<FixtureNavigation>();auto uninterrupted=session(navigation);auto actor=creature(uninterrupted);
    uninterrupted.move(actor,{5,1,1});auto queued=bytes(uninterrupted);
    check(uninterrupted.step({false,false,false}).advanced==false && bytes(uninterrupted)==queued);
    uninterrupted.step();check(uninterrupted.world().find(actor)->x==1 && uninterrupted.world().find(actor)->motion->action==Action::moving);
    uninterrupted.step();check(uninterrupted.world().find(actor)->x==2 && uninterrupted.world().find(actor)->motion->next==1);
    auto halfway=bytes(uninterrupted);
    char directory[]="/tmp/mnm-movement-XXXXXX";check(::mkdtemp(directory)!=nullptr);
    struct Cleanup {std::filesystem::path path;~Cleanup(){std::filesystem::remove_all(path);}} cleanup{directory};
    const auto checkpoint=cleanup.path/"halfway.mnw";check(writeSnapshot(checkpoint,uninterrupted.world().state()).durable);
    auto resumed=session(navigation);resumed.restore(checkpoint,[&](const std::string& name){check(name=="fixture/map");return navigation;});
    check(bytes(resumed)==halfway);
    for(unsigned i=0;i<6;++i) {uninterrupted.step();resumed.step();check(bytes(uninterrupted)==bytes(resumed));}
    check(uninterrupted.world().find(actor)->x==5 && uninterrupted.world().find(actor)->motion->action==Action::arrived);
    // Queue replacement at a tick boundary: no old waypoint executes after a new order.
    resumed.move(actor,{2,1,1});resumed.step();check(resumed.world().find(actor)->x==5);
    resumed.step();check(resumed.world().find(actor)->x==4);
    resumed.enqueue({Operation::cleanup,actor,{}});resumed.step();check(resumed.world().find(actor)->x==4 && resumed.world().find(actor)->motion->action==Action::cancelled);
    resumed.move(actor,{1,1,1});check(resumed.step().rejected==1);
    // Goal identity is a snapshot coordinate request with cancellation on release.
    auto guarded=session(navigation);auto moving=creature(guarded);Entity marker;marker.family=Family::mapLinked;marker.x=5;marker.y=1;marker.z=1;
    auto goal=guarded.spawn(marker);guarded.move(moving,{3,1,1},goal);guarded.step();
    check(guarded.world().find(moving)->motion->destination==Point{5,1,1});
    guarded.enqueue({Operation::release,goal,{}});guarded.step();
    check(guarded.world().find(moving)->x==1 && guarded.world().find(moving)->motion->action==Action::cancelled);
    auto replacement=guarded.spawn(marker);check(replacement.generation!=goal.generation);
    guarded.move(moving,{5,1,1},goal);check(guarded.step().rejected==1);
    guarded.move(moving,{5,1,1});guarded.step();guarded.enqueue({Operation::release,moving,{}});guarded.step();check(!guarded.world().find(moving));
    auto another=creature(guarded);check(another.generation!=moving.generation && guarded.world().find(another)->motion->action==Action::idle);
    // Alternate-mode/suppressed decisions, zero-distance, blocked and long-prefix transitions.
    auto mode=session(navigation);auto handle=creature(mode);mode.move(handle,{5,1,1});mode.step({true,true,false});
    check(mode.world().state().tick==0 && mode.world().find(handle)->motion->action==Action::planning);
    mode.step({true,false,true});check(mode.world().find(handle)->x==1 && mode.world().find(handle)->motion->action==Action::planning);
    mode.step();mode.step();check(mode.world().find(handle)->x==2);
    mode.move(handle,{2,1,1});mode.step();check(mode.world().find(handle)->motion->action==Action::arrived);
    mode.move(handle,{35,1,1});mode.step();for(unsigned i=0;i<33;++i) mode.step();
    check(mode.world().find(handle)->x==35 && mode.world().find(handle)->motion->action==Action::arrived);
    auto blockedNavigation=std::make_shared<FixtureNavigation>();blockedNavigation->blocked=true;
    auto blocked=session(blockedNavigation);auto subject=creature(blocked);blocked.move(subject,{5,1,1});blocked.step();
    check(blocked.world().find(subject)->x==1 && blocked.world().find(subject)->motion->action==Action::blocked);
    // Current edge legality is checked again, even after a route was admitted.
    auto changingNavigation=std::make_shared<FixtureNavigation>();auto changing=session(changingNavigation);auto change=creature(changing);
    changing.move(change,{5,1,1});changing.step();changingNavigation->blocked=true;changing.step();
    check(changing.world().find(change)->x==1 && changing.world().find(change)->motion->action==Action::blocked);
    auto failedNavigation=std::make_shared<FixtureNavigation>();auto failed=session(failedNavigation);auto failure=creature(failed);failed.move(failure,{5,1,1});
    const auto before=bytes(failed);failedNavigation->fail=true;rejected([&]{failed.step();});check(bytes(failed)==before);
    auto wrong=std::make_shared<FixtureNavigation>();wrong->fingerprint=2;
    rejected([&]{failed.restore(checkpoint,[&](const std::string&){return wrong;});});check(bytes(failed)==before);
    rejected([&]{failed.restore(checkpoint,[](const std::string&)->std::shared_ptr<const Navigation>{throw std::runtime_error("missing map");});});check(bytes(failed)==before);
    // Valid checksums are insufficient for malformed route progress and unsupported pending updates.
    auto invalid=decodeSnapshot(halfway);invalid.slots[actor.slot].entity->motion->next=99;
    rejected([&]{encodeSnapshot(invalid);});
    invalid=decodeSnapshot(halfway);invalid.slots[actor.slot].entity->motion->route[0].position={63,1,1};rejected([&]{encodeSnapshot(invalid);});
    rejected([&]{mode.move(handle,{-1,1,1});});
    auto limited=session(navigation);auto limitedActor=creature(limited);auto state=limited.world().state();state.slots[limitedActor.slot].entity->motion->budget=2;
    World w(0);w.restore(state);MovementSession low(std::move(w),navigation);low.move(limitedActor,{5,1,1});low.step();
    check(low.world().find(limitedActor)->motion->action==Action::searchLimited && low.world().find(limitedActor)->x==1);
    // Intra-cell state must participate in the same transaction/lifetime rules.
    auto fineNavigation=std::make_shared<FixtureNavigation>();auto fine=session(fineNavigation);
    Entity fineEntity;fineEntity.type=7;fineEntity.x=1;fineEntity.y=1;fineEntity.z=1;
    auto fineActor=fine.spawn(fineEntity,true);fine.move(fineActor,{5,1,1});fine.step();fine.step();fine.step();
    check(fine.world().find(fineActor)->x==1 && fine.world().find(fineActor)->motion->fine->fine.x==37);
    const auto fineBytes=bytes(fine);check(fineBytes[8]==3);
    auto fineState=decodeSnapshot(fineBytes);World restoredFine(0);restoredFine.restore(fineState);
    MovementSession fineContinued(std::move(restoredFine),fineNavigation);
    for(int i=0;i<4;++i) {fine.step();fineContinued.step();check(bytes(fine)==bytes(fineContinued));}
    const auto beforeFineFailure=bytes(fine);fineNavigation->failAdvance=true;
    rejected([&]{fine.step();});check(bytes(fine)==beforeFineFailure);fineNavigation->failAdvance=false;
    auto badFine=fine.world().state();badFine.slots[fineActor.slot].entity->motion->fine->rate=51;
    const auto badCheckpoint=cleanup.path/"bad-fine.mnw";writeSnapshot(badCheckpoint,badFine);
    rejected([&]{fine.restore(badCheckpoint,[&](const std::string&){return fineNavigation;});});check(bytes(fine)==beforeFineFailure);
    fine.move(fineActor,{2,1,1});fine.step();
    check(fine.world().find(fineActor)->motion->sampleMotion && !fine.world().find(fineActor)->motion->fine);
    fine.step();check(fine.world().find(fineActor)->motion->fine.has_value());
    fine.enqueue({Operation::cleanup,fineActor,{}});fine.step();
    check(fine.world().find(fineActor)->motion->sampleMotion && !fine.world().find(fineActor)->motion->fine);
    check(fine.world().find(fineActor)->motion->action==Action::cancelled);
    // Completed-segment continuation must remain transactional and owned.
    auto continuous=session(fineNavigation);auto continuousActor=continuous.spawn(fineEntity,true,true);
    continuous.move(continuousActor,{5,1,1});for(int i=0;i<15;++i) continuous.step();
    check(continuous.world().find(continuousActor)->x==2 && continuous.world().find(continuousActor)->motion->previous.has_value());
    continuous.step();check(continuous.world().find(continuousActor)->motion->fine.has_value());
    const auto continuousBytes=bytes(continuous);check(continuousBytes[8]==4);
    fineNavigation->failAdvance=true;rejected([&]{continuous.step();});check(bytes(continuous)==continuousBytes);fineNavigation->failAdvance=false;
    auto invalidHistory=continuous.world().state();invalidHistory.slots[continuousActor.slot].entity->motion->previous->motion.rate=51;
    const auto historyCheckpoint=cleanup.path/"invalid-history.mnw";writeSnapshot(historyCheckpoint,invalidHistory);
    rejected([&]{continuous.restore(historyCheckpoint,[&](const std::string&){return fineNavigation;});});check(bytes(continuous)==continuousBytes);
    continuous.move(continuousActor,{4,1,1});continuous.step();
    check(continuous.world().find(continuousActor)->motion->continuousMotion && !continuous.world().find(continuousActor)->motion->previous);
    continuous.enqueue({Operation::cleanup,continuousActor,{}});continuous.step();
    check(!continuous.world().find(continuousActor)->motion->previous && continuous.world().find(continuousActor)->motion->segmentTicks==0);
    std::cout<<"typed motion, staged orders, target/release, bounded routes and atomic map restoration passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
