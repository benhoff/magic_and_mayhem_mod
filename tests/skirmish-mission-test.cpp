#include "simulation/skirmish.hpp"
#include "simulation/occupancy.hpp"
#include "persistence/snapshot.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace {
void need(bool v,const char* why) {if(!v) throw std::runtime_error(why);}
template<class F> void refuses(F action) {bool failed=false;try {action();} catch(const std::exception&) {failed=true;}need(failed,"expected refusal");}
struct Grid final:Navigation {
    SkirmishMission mission;
    explicit Grid(SkirmishMission m):mission(std::move(m)) {}
    std::uint64_t fingerprint=1;
    NavigationBinding binding() const override {return {mission.dimensions,fingerprint};}
    std::uint32_t creatureType() const override {return 0;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    void validateSpawnPosition(Point p) const override {need(missionCell(mission,p),"synthetic arena placement refused");}
    void validateOccupants(const State& s) const override {(void)MovementReservations(s,binding().dimensions,0,{1,1,1});}
    bool accepts(const Entity& e,const RoutePoint& p) const override {
        return contains(p.position,binding()) && p.position.z==2 && p.position!=Point{e.x,e.y,e.z} &&
            std::abs(p.position.x-e.x)<=1 && std::abs(p.position.y-e.y)<=1;
    }
    RoutePlan plan(const Entity& e,Point target,std::uint32_t budget) const override {
        RoutePlan r{PlanStatus::reachable,{},0};Point p{e.x,e.y,e.z};
        while(p!=target && r.route.size()<16) {
            if(++r.expansions>budget) return {PlanStatus::budgetExhausted,{},budget};
            p.x+=(p.x<target.x)-(p.x>target.x);p.y+=(p.y<target.y)-(p.y>target.y);
            r.route.push_back({p,2,0,0,720});
        }
        return r;
    }
};

void contract() {
    SkirmishMission m{"custom-mission",{10,7,4},{{{1,5,2},{8,1,2}}},{{7,2,2},{7,1,2}}, {}};
    for(int x=1;x<9;++x) for(int y=1;y<6;++y) m.standing.push_back({x,y,2});
    validateMission(m);auto nav=std::make_shared<Grid>(m);SkirmishSession session(m,nav);
    need(session.world().state().map==m.map && session.world().find({0,1})->z==2,"mission spawn/map ignored");
    auto before=encodeSnapshot(session.world().state());
    refuses([&]{session.enqueue({SkirmishOrderKind::summon,{0,1},{},Point{2,4,1}});});
    need(encodeSnapshot(session.world().state())==before,"rejected destination changed state");
    session.enqueue({SkirmishOrderKind::summon,{0,1},{},Point{2,4,2}});
    for(int i=0;i<3;++i) session.step();
    need(session.state().units.size()==4,"custom player/AI summons failed");
    session.enqueue({SkirmishOrderKind::move,{2,1},{},Point{3,4,2}});
    session.enqueue({SkirmishOrderKind::bolt,{0,1},Handle{1,1},{}});
    session.step();auto saved=session.world().state();World world(0);world.restore(saved);
    SkirmishSession restored(std::move(world),nav);
    need(sameMission(restored.mission(),m),"mission continuation lost");
    for(int i=0;i<6;++i) {session.step();restored.step();}
    need(encodeSnapshot(session.world().state())==encodeSnapshot(restored.world().state()),"split custom continuation differs");
    auto encoded=encodeSkirmish(session.state());need(encoded[7]=='2',"custom version missing");
    need(encodeSkirmish(decodeSkirmish(encoded))==encoded,"custom codec not canonical");
    auto bad=encoded;bad.pop_back();refuses([&]{decodeSkirmish(bad);});
    bad=encoded;bad.push_back(0);refuses([&]{decodeSkirmish(bad);});
    bad=encoded;bad[16]=255;refuses([&]{decodeSkirmish(bad);});
    auto reject=[&](auto edit) {auto invalid=m;edit(invalid);refuses([&]{SkirmishSession rejected(invalid,nav);});};
    reject([](auto& v){v.dimensions.x=17;});
    reject([](auto& v){v.standing.push_back(v.standing.front());});
    reject([](auto& v){v.wizards[0]=v.wizards[1];});
    reject([](auto& v){v.aiSummons.push_back(v.aiSummons.front());});
    reject([](auto& v){v.aiSummons={{1,1,2}};});
    reject([](auto& v){v.map.clear();});
    auto wrong=std::make_shared<Grid>(m);wrong->fingerprint=2;
    World mismatched(0);mismatched.restore(saved);refuses([&]{SkirmishSession rejected(std::move(mismatched),wrong);});
    // Explicit arena definitions use the exact old record and decode as implicit.
    auto legacy=session.state();legacy.mission.reset();legacy.pending.clear();
    auto explicitArena=legacy;explicitArena.mission=arenaMission();
    need(encodeSkirmish(legacy)==encodeSkirmish(explicitArena) && encodeSkirmish(legacy)[7]=='1',"arena wire changed");
    need(!decodeSkirmish(encodeSkirmish(explicitArena)).mission,"arena wire is not canonical");
}
}
int main() {try {contract();std::cout<<"mission policy and continuation passed\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
