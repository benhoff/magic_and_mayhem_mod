#include "frozen_navigation.hpp"
#include "simulation/occupancy.hpp"
#include "persistence/snapshot.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

using namespace mnm::game;
static void checked(bool value,int line) {if(!value) throw std::runtime_error("multi-movement assertion at "+std::to_string(line));}
#define check(value) checked((value),__LINE__)
template<class F> static void refused(F f) {bool caught=false;try {f();} catch(const std::exception&) {caught=true;}check(caught);}
static Bytes bytes(const MovementSession& s) {return encodeSnapshot(s.world().state());}
static Entity entity(int x,int y=1,int z=1) {Entity e;e.x=x;e.y=y;e.z=z;return e;}
static MovementSession session(const std::string& map,std::shared_ptr<const Navigation> nav,unsigned capacity=64) {
    World w(capacity);auto state=w.state();state.map=map;state.navigation=nav->binding();state.animation=nav->animationBinding();w.restore(state);return MovementSession(std::move(w),nav);
}
static void moving(Entity& e,Point to,bool sample=false) {
    e.motion=CreatureMotion{};auto& m=*e.motion;m.origin={e.x,e.y,e.z};m.destination=to;m.action=Action::moving;
    const auto dx=to.x-e.x,dy=to.y-e.y;const int dirs[9]={7,0,1,6,0,2,5,4,3};
    m.route.push_back({to,dirs[dx+3*dy+4],0,0,720});m.sampleMotion=sample;m.continuousMotion=sample;
}
static MovementSession seeded(const std::string& map,std::shared_ptr<const Navigation> nav,std::vector<Entity> entities) {
    World w(64);auto state=w.state();state.map=map;state.navigation=nav->binding();
    for(unsigned i=0;i<entities.size();++i) state.slots[i].entity=std::move(entities[i]);
    w.restore(state);return MovementSession(std::move(w),nav);
}
static void reservationCases() {
    World w(4);auto a=w.spawn(entity(1,2));auto b=w.spawn(entity(2,1));
    MovementReservations claims(w.state(),{8,8,4},0,{1,1,1});
    check(claims.tryReserve(a,{1,2,1},{2,2,1}));
    std::vector<std::optional<Handle>> before;for(int z=0;z<4;++z) for(int y=0;y<8;++y) for(int x=0;x<8;++x) before.push_back(claims.owner({x,y,z}));
    check(!claims.tryReserve(b,{2,1,1},{2,2,1}));
    unsigned at=0;for(int z=0;z<4;++z) for(int y=0;y<8;++y) for(int x=0;x<8;++x) check(claims.owner({x,y,z})==before.at(at++));
    refused([&]{claims.tryReserve({a.slot,a.generation+1},{1,2,1},{2,2,1});});
    check(!claims.tryReserve(a,{1,2,1},{7,2,1}));
    // Opposing diagonal edges conservatively include both origins; neither wins.
    auto diagonal=w.state();diagonal.slots[0].entity=entity(1,1);diagonal.slots[1].entity=entity(2,1);
    MovementReservations crossing(diagonal,{8,8,4},0,{1,1,1});
    check(!crossing.tryReserve(a,{1,1,1},{2,2,1}) && !crossing.tryReserve(b,{2,1,1},{1,2,1}));
    // Height and width participate in swept volume, not just endpoint identity.
    auto boxes=w.state();boxes.slots[0].entity=entity(1,1,1);boxes.slots[1].entity=entity(4,1,1);
    MovementReservations wide(boxes,{8,8,8},0,{2,2,3});check(wide.tryReserve(a,{1,1,1},{2,1,1}));
    check(!wide.tryReserve(b,{4,1,1},{3,1,1}));check(wide.owner({3,2,3})==std::optional<Handle>(a));
    refused([&]{wide.tryReserve(a,{2,1,1},{3,1,1});});
    boxes.slots[1].entity=entity(1,1,5);
    MovementReservations tall(boxes,{8,8,8},0,{1,1,3});check(tall.tryReserve(a,{1,1,1},{1,1,2}));
    check(!tall.tryReserve(b,{1,1,5},{1,1,4}));
}
struct BudgetNavigation:Navigation {
    bool dishonest=false;
    NavigationBinding binding() const override {return {{12,12,3},777};}
    std::uint32_t creatureType() const override {return 0;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    void validateOccupants(const State& s) const override {(void)MovementReservations(s,binding().dimensions,0,{1,1,1});}
    bool accepts(const Entity& e,const RoutePoint& p) const override {return p.position.x==e.x+1 && p.position.y==e.y && p.position.z==e.z;}
    RoutePlan plan(const Entity&,Point target,std::uint32_t budget) const override {
        if(dishonest) return {PlanStatus::reachable,{},budget+1};
        if(budget<50) return {PlanStatus::budgetExhausted,{},budget};
        return {PlanStatus::reachable,{{target,2,0,0,720}},50};
    }
};
static void schedulingBudget() {
    auto nav=std::make_shared<BudgetNavigation>();auto s=session("budget-fixture",nav);
    std::vector<Handle> actors;for(int y=1;y<=3;++y){auto h=s.spawn(entity(1,y));actors.push_back(h);s.move(h,{2,y,1});}
    auto pending=bytes(s);check(!s.step({false,false,false}).advanced && bytes(s)==pending);
    auto first=s.step();check(first.searchExpansions==53);
    check(s.world().find(actors[0])->motion->action==Action::moving && s.world().find(actors[1])->motion->action==Action::planning);
    auto suppressed=bytes(s);s.step({true,false,true});check(s.world().find(actors[1])->motion->action==Action::planning);
    check(bytes(s)!=suppressed); // maintenance proceeds while planning is suppressed.
    for(int i=0;i<5;++i){auto r=s.step();check(r.searchExpansions<=53);}
    for(auto h:actors) check(s.world().find(h)->motion->action==Action::arrived);
    auto rollback=session("budget-fixture",nav);auto h=rollback.spawn(entity(1));rollback.move(h,{2,1,1});
    auto staged=rollback.world().state();auto transit=entity(1,4);moving(transit,{2,4,1});staged.slots[1].entity=transit;
    World restored(0);restored.restore(staged);rollback=MovementSession(std::move(restored),nav);
    nav->dishonest=true;auto prior=bytes(rollback);refused([&]{rollback.step();});check(bytes(rollback)==prior);
}
int main(int argc,char** argv) try {
    check(argc==5);reservationCases();schedulingBudget();const auto map=std::filesystem::absolute(argv[1]).string();const auto slowMap=std::filesystem::absolute(argv[2]).string();const auto animation=mnm::sandbox::loadMovementAnimation(argv[3],8);const std::filesystem::path root=argv[4];
    auto nav=mnm::sandbox::loadFrozenNavigation(map,{},true,true);
    auto stationary=mnm::sandbox::loadFrozenNavigation(map,{},true);check(!(nav->binding()==stationary->binding()));
    auto legacy=session(map,stationary);legacy.spawn(entity(1));refused([&]{legacy.spawn(entity(4));});
    auto s=session(map,nav);auto a=s.spawn(entity(1,1)),b=s.spawn(entity(1,4));s.move(a,{5,1,1});s.move(b,{5,4,1});
    for(int i=0;i<12;++i){auto r=s.step();check(r.searchExpansions<=53 && r.movementWaits==0);}
    check(s.world().find(a)->x==5 && s.world().find(b)->x==5);
    // Converging movers: slot order selects one; the other waits without losing its route.
    auto left=entity(1,2),up=entity(2,1);moving(left,{2,2,1});moving(up,{2,2,1});
    auto conflict=seeded(map,nav,{left,up});auto r=conflict.step();check(r.movementWaits==1);
    check(conflict.world().find({0,1})->x==2 && conflict.world().find({1,1})->y==1 && conflict.world().find({1,1})->motion->action==Action::moving);
    conflict.enqueue({Operation::release,{0,1},{}});conflict.step();check(conflict.world().find({1,1})->y==2);
    // Swaps and crossing diagonals cannot overlap; conservative waiting is explicit.
    left=entity(1,1);up=entity(2,1);moving(left,{2,1,1});moving(up,{1,1,1});
    auto swap=seeded(map,nav,{left,up});check(swap.step().movementWaits==2 && swap.world().find({0,1})->x==1 && swap.world().find({1,1})->x==2);
    moving(left,{2,2,1});moving(up,{1,2,1});auto crossing=seeded(map,nav,{left,up});check(crossing.step().movementWaits==2);
    check(writeSnapshot(root/"swap.mnms",swap.world().state()).durable);
    auto slow=mnm::sandbox::loadFrozenNavigation(slowMap,{},true,true);auto fine=session(slowMap,slow);
    auto mover=fine.spawn(entity(1),true,true);auto peer=fine.spawn(entity(1,4),true,true);fine.move(mover,{5,1,1});fine.move(peer,{5,4,1});fine.step();fine.step();
    check(fine.world().find(mover)->motion->fine && fine.world().find(peer)->motion->fine);
    const auto target=fine.world().find(mover)->motion->route.front().position;
    const auto before=bytes(fine);refused([&]{fine.spawnBlocker(entity(target.x,target.y,target.z));});check(bytes(fine)==before);
    refused([&]{fine.spawn(entity(target.x,target.y,target.z),true,true);});check(bytes(fine)==before);
    const auto checkpoint=root/"mid-fine.mnms";check(writeSnapshot(checkpoint,fine.world().state()).durable);
    auto restored=session(slowMap,slow);restored.restore(checkpoint,[&](const std::string&){return slow;});check(bytes(restored)==bytes(fine));
    const auto saved=bytes(restored);refused([&]{restored.restore(checkpoint,[&](const std::string&){return stationary;});});check(bytes(restored)==saved);
    for(int i=0;i<180;++i){fine.step();restored.step();check(bytes(fine)==bytes(restored));}
    check(fine.world().find(mover)->x==5 && fine.world().find(peer)->x==5);
    check(writeSnapshot(root/"fine-whole.mnms",fine.world().state()).durable);
    // Cancel/cleanup releases an ongoing edge, and recycled identities cannot reclaim it.
    auto c0=entity(1,2),c1=entity(2,1);moving(c0,{2,2,1},true);moving(c1,{2,2,1},true);
    auto cancellation=seeded(slowMap,slow,{c0,c1});check(cancellation.step().movementWaits==1);
    cancellation.move({0,1},{1,2,1});cancellation.step();
    check(cancellation.world().find({0,1})->motion->action==Action::arrived && cancellation.world().find({1,1})->motion->fine);
    cancellation.enqueue({Operation::cleanup,{1,1},{}});cancellation.step();
    check(cancellation.world().find({1,1})->motion->action==Action::cancelled && !cancellation.world().find({1,1})->motion->fine);
    cancellation.enqueue({Operation::release,{0,1},{}});cancellation.step();auto replacement=cancellation.spawn(entity(6,4),true,true);
    check(replacement.slot==0 && replacement.generation==2);
    cancellation.enqueue({Operation::release,{0,1},{}});check(cancellation.step().rejected==1 && cancellation.world().find(replacement));
    MovementReservations recycled(cancellation.world().state(),slow->binding().dimensions,0,{1,1,1});
    refused([&]{recycled.tryReserve({0,1},{6,4,1},{7,4,1});});
    // Corrupt reservation geometry is rejected transactionally before world/resource publication.
    auto malformed=readSnapshot(checkpoint);malformed.slots[1].entity=malformed.slots[0].entity;
    const auto bad=root/"overlap.mnms";check(writeSnapshot(bad,malformed).durable);const auto current=bytes(restored);
    refused([&]{restored.restore(bad,[&](const std::string&){return slow;});});check(bytes(restored)==current);
    // Independently valid fine states can have conflicting claims despite disjoint origins.
    auto lx=entity(1,2),uy=entity(2,1);moving(lx,{2,2,1},true);moving(uy,{2,2,1},true);
    auto one=seeded(slowMap,slow,{lx}),two=seeded(slowMap,slow,{uy});one.step();two.step();
    check(one.world().find({0,1})->motion->fine && two.world().find({0,1})->motion->fine);
    auto overlapping=one.world().state();overlapping.slots[1].entity=two.world().state().slots[0].entity;
    const auto badClaims=root/"overlapping-claims.mnms";check(writeSnapshot(badClaims,overlapping).durable);
    refused([&]{restored.restore(badClaims,[&](const std::string&){return slow;});});check(bytes(restored)==current);
    // Each ANI cursor evolves independently, including staggered starts and process restoration.
    auto aniNav=mnm::sandbox::loadFrozenNavigation(slowMap,animation,true,true);auto animated=session(slowMap,aniNav);
    auto aa=animated.spawn(entity(1),true,true),bb=animated.spawn(entity(1,4),true,true);
    animated.move(aa,{5,1,1});animated.step();for(int i=0;i<20;++i){animated.step();}
    animated.move(bb,{5,4,1});animated.step();animated.step();
    const auto aniCheckpoint=root/"ani-mid.mnms";check(writeSnapshot(aniCheckpoint,animated.world().state()).durable);
    auto stateA=animated.world().state(),stateB=stateA;stateA.slots[bb.slot].entity.reset();stateB.slots[aa.slot].entity.reset();
    World wa(0),wb(0);wa.restore(stateA);wb.restore(stateB);MovementSession aloneA(std::move(wa),aniNav),aloneB(std::move(wb),aniNav);
    for(int i=0;i<500;++i) {
        animated.step();aloneA.step();aloneB.step();auto compareA=animated.world().state(),compareB=compareA;
        compareA.slots[bb.slot].entity.reset();compareB.slots[aa.slot].entity.reset();
        check(encodeSnapshot(compareA)==bytes(aloneA) && encodeSnapshot(compareB)==bytes(aloneB));
    }
    check(animated.world().find(aa)->motion->action==Action::arrived && animated.world().find(bb)->motion->action==Action::arrived);
    check(writeSnapshot(root/"ani-whole.mnms",animated.world().state()).durable);
    auto capacity=session(map,nav);for(int i=0;i<32;++i)capacity.spawn(entity(i%10+1,i/10+1));
    const auto full=bytes(capacity);refused([&]{capacity.spawn(entity(5,5));});check(bytes(capacity)==full);
    refused([&]{mnm::sandbox::loadFrozenNavigation(map,{},false,true);});
    std::cout<<"Native multi-mover reservations, waiting, shared planning budget, fairness, cleanup, rollback, capacity and fine checkpoint continuation pass.\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
