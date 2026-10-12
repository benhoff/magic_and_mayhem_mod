#include "simulation/combat.hpp"
#include "simulation/occupancy.hpp"
#include "persistence/snapshot.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>

using namespace mnm::game;
namespace {
void check(bool v,const char* reason) {if(!v)throw std::runtime_error(reason);}
template<class F>void refused(F f) {bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught,"expected refusal");}
struct Grid:Navigation {
    bool fail=false,blocked=false;
    std::uint64_t fingerprint=1;
    NavigationBinding binding() const override {return {{12,12,3},fingerprint};}
    std::uint32_t creatureType() const override {return 0;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    void validateOccupants(const State& s) const override {(void)MovementReservations(s,binding().dimensions,0,{1,1,1});}
    bool accepts(const Entity& e,const RoutePoint& p) const override {
        return !blocked && contains(p.position,binding()) && p.position.z==e.z && p.position!=Point{e.x,e.y,e.z} &&
            std::abs(p.position.x-e.x)<=1 && std::abs(p.position.y-e.y)<=1;
    }
    RoutePlan plan(const Entity& e,Point target,std::uint32_t budget) const override {
        if(fail)throw std::runtime_error("planner failure");
        if(blocked)return {PlanStatus::unreachable,{},1};
        RoutePlan r{PlanStatus::reachable,{},0};Point p{e.x,e.y,e.z};
        while(p!=target && r.route.size()<16) {
            if(++r.expansions>budget)return {PlanStatus::budgetExhausted,{},budget};
            p.x+=(p.x<target.x)-(p.x>target.x);p.y+=(p.y<target.y)-(p.y>target.y);
            r.route.push_back({p,2,0,0,720});
        }return r;
    }
};
CombatSession empty(std::shared_ptr<Grid> nav,unsigned capacity=2) {
    World world(capacity);auto s=world.state();s.map="combat/grid";s.navigation=nav->binding();world.restore(s);return {std::move(world),nav};
}
Entity body(int x,unsigned owner=0,int y=1,int z=1) {Entity e;e.x=x;e.y=y;e.z=z;e.owner=owner;return e;}
CombatSession duel(std::shared_ptr<Grid> nav,int x=5,CombatStats a={20,6,2,3},CombatStats b={20,4,2,3}) {
    auto s=empty(nav);s.spawn(body(1),a);s.spawn(body(x,1),b);return s;
}
Bytes bytes(const CombatSession& s){return encodeSnapshot(s.world().state());}
CombatSession rebound(const CombatSession& s,std::shared_ptr<Grid> nav) {
    World w(0);w.restore(decodeSnapshot(bytes(s)));return {std::move(w),nav};
}
bool event(const CombatReport& r,CombatEventKind kind){for(const auto& e:r.events)if(e.kind==kind)return true;return false;}
void temporalContract() {
    auto nav=std::make_shared<Grid>();auto s=duel(nav,2);
    auto initial=bytes(s);check(!s.step({false,false,false}).movement.advanced && bytes(s)==initial,"nonadmitted tick changed combat");
    auto r=s.step();check(event(r,CombatEventKind::acquired) && event(r,CombatEventKind::windup) && !event(r,CombatEventKind::hit),"damage applied at acquisition");
    check(s.combat().fighters[0].remaining==2 && s.combat().fighters[1].health==20,"incorrect initial windup");
    const auto windup=s.combat();s.step({true,true,false});
    check(encodeCombat(s.combat())==encodeCombat(windup) && s.world().state().tick==1,"alternate tick advanced combat");
    r=s.step();check(!event(r,CombatEventKind::hit) && s.combat().fighters[0].remaining==1,"windup skipped tick");
    auto resumed=rebound(s,nav);r=s.step();auto rr=resumed.step();
    check(bytes(s)==bytes(resumed) && r.events.size()==rr.events.size(),"windup checkpoint changed outcome");
    check(s.combat().fighters[0].health==16 && s.combat().fighters[1].health==14,"incorrect first reciprocal hits");
    check(s.combat().fighters[0].action==CombatAction::recovering && s.combat().fighters[0].remaining==3,"missing recovery");
    for(int i=0;i<3;++i){r=s.step();check(!event(r,CombatEventKind::hit),"recovery allowed early hit");}
    check(s.combat().fighters[0].action==CombatAction::windingUp && s.combat().fighters[0].remaining==2,"recovery failed to restart windup");
    s.step();r=s.step();check(event(r,CombatEventKind::hit) && s.combat().fighters[1].health==8,"second hit timing");
}
void encounterAndContinuation() {
    auto nav=std::make_shared<Grid>();auto s=duel(nav);
    unsigned acquired=0,hits=0,deaths=0;bool approached=false;
    for(int i=0;i<48;++i) {
        auto resumed=rebound(s,nav);auto r=s.step(),rr=resumed.step();
        check(bytes(s)==bytes(resumed),"complete combat continuation differs");
        check(r.events.size()==rr.events.size(),"continued event count differs");
        for(std::size_t j=0;j<r.events.size();++j) {
            const auto& e=r.events[j];const auto& re=rr.events[j];
            check(e.kind==re.kind && e.actor==re.actor && e.target==re.target && e.amount==re.amount,"continued event differs");
            acquired+=e.kind==CombatEventKind::acquired;hits+=e.kind==CombatEventKind::hit;deaths+=e.kind==CombatEventKind::died;
            approached=approached || e.kind==CombatEventKind::approach;
        }
    }
    const auto c=s.combat();check(acquired==2 && approached && hits==7 && deaths==1,"encounter event totals");
    check(c.fighters[0].health==8 && c.fighters[1].health==0 && c.fighters[1].action==CombatAction::dead,"incorrect encounter winner");
    check(s.world().find(c.fighters[1].actor) && s.world().find(c.fighters[1].actor)->cleaned,"death released original native identity");
    check(!c.fighters[0].target && !s.world().find(c.fighters[1].actor)->motion->fine,"death left attack/motion state");
    Occupancy occupancy(s.world().state(),nav->binding().dimensions,0,{1,1,1});
    const auto& corpse=*s.world().find(c.fighters[1].actor);
    check(!occupancy.owner({corpse.x,corpse.y,corpse.z}),"dead body blocks navigation");
    auto simultaneous=duel(nav,2,{5,5,1,1},{5,5,1,1});simultaneous.step();const auto r=simultaneous.step();
    check(simultaneous.combat().fighters[0].health==5 && simultaneous.combat().fighters[1].health==0,"posthumous retaliation");
    check(r.events.size()==2 && r.events[0].kind==CombatEventKind::hit && r.events[1].kind==CombatEventKind::died,"lethal event order");
    // A valid saved windup can have its target outside melee reach. It must
    // cancel the pending hit and approach again rather than deal remote damage.
    auto interrupted=duel(nav,2);interrupted.step();auto state=interrupted.world().state();
    auto& displaced=*state.slots[0].entity;displaced.x=4;displaced.motion->origin=displaced.motion->destination={4,1,1};
    World w(0);w.restore(state);CombatSession remote(std::move(w),nav);const auto missed=remote.step();
    check(event(missed,CombatEventKind::missed) && !event(missed,CombatEventKind::hit),"out-of-reach windup hit");
    auto wrapped=duel(nav,2);wrapped.step();wrapped.step();state=wrapped.world().state();state.tick=0xffffffffU;
    World wrapping(0);wrapping.restore(state);CombatSession wrap(std::move(wrapping),nav);
    check(event(wrap.step(),CombatEventKind::hit) && wrap.world().state().tick==0,"tick rollover changed windup");
}
void eligibilityAndRollback() {
    auto nav=std::make_shared<Grid>();auto s=empty(nav,3);s.spawn(body(1));s.spawn(body(2));s.spawn(body(3,1));
    s.step();check(s.combat().fighters[0].target==std::optional<Handle>({2,1}),"friendly target acquired");
    auto same=empty(nav);same.spawn(body(1));same.spawn(body(2));same.step();check(!same.combat().fighters[0].target,"friendly-only world attacked");
    auto layers=empty(nav);layers.spawn(body(1));layers.spawn(body(2,1,1,2));layers.step();check(!layers.combat().fighters[0].target,"cross-layer attack acquired");
    auto range=duel(nav);auto state=range.world().state();auto c=range.combat();c.acquisitionRange=1;state.systems=encodeCombat(c);
    World w(0);w.restore(state);CombatSession far(std::move(w),nav);far.step();check(!far.combat().fighters[0].target,"outside acquisition range");
    auto ties=empty(nav,3);ties.spawn(body(3,0,3));ties.spawn(body(1,1,3));ties.spawn(body(5,1,3));ties.step();
    check(ties.combat().fighters[0].target==std::optional<Handle>({1,1}),"target tie not in slot order");
    auto failing=duel(nav);const auto before=bytes(failing);nav->fail=true;refused([&]{failing.step();});check(bytes(failing)==before,"planner failure published combat");nav->fail=false;
    auto suppressed=duel(nav);suppressed.step({true,false,true});check(suppressed.world().find({0,1})->x==1 && suppressed.world().find({0,1})->motion->action==Action::planning,"search suppression lost approach");
    auto blockedNav=std::make_shared<Grid>();blockedNav->blocked=true;auto blocked=duel(blockedNav);
    for(int i=0;i<12;++i)check(!event(blocked.step(),CombatEventKind::hit),"blocked route dealt damage");
    check(blocked.combat().fighters[0].health==20 && blocked.world().find({0,1})->x==1,"blocked world moved/damaged");
    auto cap=duel(nav);const auto full=bytes(cap);refused([&]{cap.spawn(body(8,1));});check(bytes(cap)==full,"failed spawn published combat");
    refused([&]{cap.spawn(body(8,1),{0,1,1,1});});check(bytes(cap)==full,"invalid stats changed state");
    World bare(0);bare.restore(cap.world().state());MovementSession movement(std::move(bare),nav);
    refused([&]{movement.step();});check(encodeSnapshot(movement.world().state())==full,"movement runner silently discarded combat");
}
void corruptState() {
    auto nav=std::make_shared<Grid>();auto s=duel(nav,2);s.step();const auto good=s.world().state();
    auto reject=[&](State state){World w(0);w.restore(std::move(state));refused([&]{CombatSession bad(std::move(w),nav);});};
    auto state=good;auto c=s.combat();c.fighters[0].target=Handle{1,2};state.systems=encodeCombat(c);reject(state);
    c=s.combat();c.fighters[0].health=21;state.systems=encodeCombat(c);reject(state);
    c=s.combat();c.fighters[0].remaining=0;state.systems=encodeCombat(c);reject(state);
    c=s.combat();c.fighters[0].action=static_cast<CombatAction>(99);state.systems=encodeCombat(c);reject(state);
    c=s.combat();c.fighters[0].health=0;state.systems=encodeCombat(c);reject(state);
    c=s.combat();c.fighters[1].actor=c.fighters[0].actor;state.systems=encodeCombat(c);reject(state);
    c=s.combat();c.fighters.pop_back();state.systems=encodeCombat(c);reject(state);
    auto raw=good.systems;raw.push_back(0);refused([&]{decodeCombat(raw);});
    raw=good.systems;raw[12]=255;refused([&]{decodeCombat(raw);});
    raw=good.systems;raw[0]=0;refused([&]{decodeCombat(raw);});
    raw=good.systems;raw.resize(20);refused([&]{decodeCombat(raw);});
    raw=good.systems;raw[16+36]=2;refused([&]{decodeCombat(raw);});
    raw=good.systems;raw[8]=0;c=decodeCombat(raw);refused([&]{validateCombat(good,c);});
}
void restoreTransactionAndLimits() {
    auto nav=std::make_shared<Grid>();auto s=duel(nav,2);s.step();const auto before=bytes(s);
    const auto pattern=(std::filesystem::path(std::getenv("TMPDIR")?std::getenv("TMPDIR"):"/tmp")/"mnm-combat-restore-XXXXXX").string();
    std::vector<char> directory(pattern.begin(),pattern.end());directory.push_back(0);
    check(::mkdtemp(directory.data())!=nullptr,"temporary directory");
    struct Cleanup {std::filesystem::path path;~Cleanup(){std::filesystem::remove_all(path);}} cleanup{directory.data()};
    const auto checkpoint=cleanup.path/"attack.mnms";check(writeSnapshot(checkpoint,s.world().state()).durable,"checkpoint publication");
    auto restored=empty(nav);restored.restore(checkpoint,[&](const State&){return nav;});
    check(bytes(restored)==before,"restore changed phase state");
    auto wrong=std::make_shared<Grid>();wrong->fingerprint=9;
    refused([&]{restored.restore(checkpoint,[&](const State&){return wrong;});});check(bytes(restored)==before,"wrong-resource restore changed world");
    refused([&]{restored.restore(checkpoint,{});});check(bytes(restored)==before,"missing resolver changed world");
    refused([&]{restored.restore(checkpoint,[](const State&)->std::shared_ptr<const Navigation>{throw std::runtime_error("resolver failed");});});
    check(bytes(restored)==before,"resolver failure changed world");
    auto bad=s.world().state();auto c=s.combat();c.fighters[0].remaining=999;bad.systems=encodeCombat(c);
    const auto corrupt=cleanup.path/"corrupt.mnms";check(writeSnapshot(corrupt,bad).durable,"malformed fixture publication");
    refused([&]{restored.restore(corrupt,[&](const State&){return nav;});});check(bytes(restored)==before,"bad semantic restore changed world");
    auto full=empty(nav,40);
    for(int i=0;i<32;++i)full.spawn(body(i%8+1,0,i/8+1));
    const auto capacity=bytes(full);refused([&]{full.spawn(body(9,0,9));});check(bytes(full)==capacity,"actor cap not atomic");
    // A second stop command exceeds this world's per-tick command limit. The
    // first staged cancellation and all attack state must be discarded too.
    auto pending=duel(nav,5);auto st=pending.world().state();
    for(unsigned i=0;i<2;++i) {
        auto& m=*st.slots[i].entity->motion;m.action=Action::planning;m.destination={i==0?4:2,1,1};
    }
    st.slots[1].entity->x=2;st.slots[1].entity->motion->origin={2,1,1};st.slots[1].entity->motion->destination={1,1,1};
    Limits limit;limit.commands=1;World bounded(0,limit);bounded.restore(st);CombatSession limited(std::move(bounded),nav);
    const auto untouched=bytes(limited);refused([&]{limited.step();});check(bytes(limited)==untouched,"command-limit rollback failed");
}
}
int main() try {
    temporalContract();encounterAndContinuation();eligibilityAndRollback();corruptState();restoreTransactionAndLimits();
    std::cout<<"Native melee timing, pursuit, hostility, damage/death ordering, cleanup, exact continuation, rollback and malformed state pass.\n";return 0;
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
