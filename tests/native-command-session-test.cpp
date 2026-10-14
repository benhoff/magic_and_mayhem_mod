#include "../apps/command-session/runtime.hpp"
#include "../game/persistence/snapshot.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm;
namespace {
namespace m = reconstruction::combat;
void need(bool ok, const char *why) { if (!ok) throw std::runtime_error(why); }
template<class F> void refuses(F f) {
  bool caught = false;
  try { f(); } catch (const std::exception &) { caught = true; }
  need(caught, "operation must refuse");
}
struct Faults { bool command=false, tick=false, event=false, topology=false, reentry=false, height=false; };
command_session::Continuation initial() {
  command_session::Continuation s;
  s.world.dimensions={16,16,4};s.world.creatures.resize(3);
  s.execution.resize(3);s.bindings.resize(3,m::CommandBehaviorBinding::idle);
  s.generations={1,1,1};s.backend={0,0,0,0};
  for(unsigned i=0;i<3;++i) {
    auto &a=s.world.creatures[i];a.ai.slot=i;a.ai.owner=int(i%2);
    a.ai.position={int(i*4),1,0};a.ai.home=a.ai.position;
    a.ai.hth[27]=713+i;a.inventory[41].count=81+i;
    s.execution[i].previousMoveVariant=317+i;
  }
  return s;
}
command_session::HookFactory providers(std::shared_ptr<Faults> faults) {
  return [faults](auto &s) {
    command_session::Hooks h;
    h.execution=[&s,faults](unsigned) {
      m::ExecutionServices e;
      e.action=[](auto &a,unsigned kind){a.ai.action=kind;};
      e.restoreColor=[&s](unsigned){++s.backend[3];};
      e.movementRenewal=[&s,faults](auto &){++s.backend[0];if(faults->command)throw std::runtime_error("injected command failure");return true;};
      e.orient=[](auto &,m::Cell){};
      e.deliverQueue=[&s](auto &){++s.backend[3];};
      e.updateQueue=[&s](auto &){++s.backend[3];};
      e.terrainHeight=[faults](m::Cell){return faults->height?std::numeric_limits<int>::max():0;};
      e.adjustDuplicate=[](m::Cell &){return false;};
      e.duplicateMove=[](auto &,m::Cell p){return p;};
      return e;
    };
    h.queue=[](unsigned){return m::ExecutionServices{};};
    h.idle=[](unsigned){
      m::BehaviorExecutionServices b;
      b.immediate=[](auto &){return false;};b.interaction=[](auto &){return false;};
      b.action=[](auto &a,unsigned n){a.ai.action=n;};return b;
    };
    h.follow=[](unsigned){return m::FollowServices{[](const auto &,m::Cell &){return true;},{}};};
    h.body=[&s,faults](auto &a,unsigned kind){
      ++a.cooldown;
      a.ai.motion=kind==2;
      if(faults->topology)s.execution[a.ai.slot].orders.clear();
    };
    h.supplied=[](auto &){};
    h.release=[&s](unsigned){++s.backend[2];};
    h.peer=[](auto &a,bool){if(a.ai.incoming)--a.ai.incoming;};
    h.advance=[faults](auto &c,auto &commands,game::TickInput) {
      ++c.backend[1];
      if(faults->tick){c.world.creatures[0].ai.position.x=7;throw std::runtime_error("injected tick failure");}
      if(faults->topology||faults->reentry) {
        if(faults->reentry) {
          // Type27 target admission resets and recursively enters idle queue.
          c.world.creatures[0].ai.type=27;
        }
        commands.queue(c.world.creatures[0]);
      } else {
        for(auto &a:c.world.creatures) {
          if(a.ai.motion) {a.ai.position=a.destination;a.ai.motion=false;commands.reset(a);}
          else commands.dispatch(a);
        }
      }
      auto actor=game::Handle{0,c.generations[0]};
      if(faults->event)++actor.generation;
      return std::vector<game::CreatureRuleEvent>{{actor,"command-frame",{},int(c.backend[1])}};
    };
    return h;
  };
}
game::Bytes snapshot(const game::CreatureRuleSession &s) {return game::encodeSnapshot(s.world().state());}
void intent(game::CreatureRuleSession &s,game::CreatureOrderKind kind,game::Point p={},std::optional<game::Handle> target={},bool queued=false,bool repeat=false) {
  s.command({{0,1},kind,p,target,queued,repeat});
}
} // namespace
int main() try {
  auto faults=std::make_shared<Faults>();
  const game::Bytes profile{1,7,23,91};
  auto rules=std::make_shared<command_session::Runtime>(initial(),profile,providers(faults));
  game::CreatureRuleSession s(rules,profile,9,3);
  auto start=snapshot(s);
  need(s.step({false,false,false}).empty()&&snapshot(s)==start,"paused tick changed state");
  faults->command=true;refuses([&]{intent(s,game::CreatureOrderKind::move,{3,4,0});});
  need(snapshot(s)==start,"failed command published partial actor/backend state");faults->command=false;
  faults->height=true;refuses([&]{intent(s,game::CreatureOrderKind::coordinateAttack,{3,4,1});});
  need(snapshot(s)==start,"overflowing terrain aim published state");faults->height=false;
  faults->tick=true;refuses([&]{s.step();});
  need(snapshot(s)==start,"failed tick published clock/backend changes");faults->tick=false;
  faults->event=true;refuses([&]{s.step();});
  need(snapshot(s)==start,"stale event published a tick");faults->event=false;
  refuses([&]{s.command({{0,2},game::CreatureOrderKind::move,{2,3,0},{},false,false});});
  refuses([&]{intent(s,game::CreatureOrderKind::attack,{},game::Handle{1,2});});
  refuses([&]{intent(s,game::CreatureOrderKind::follow,{},{});});
  refuses([&]{intent(s,game::CreatureOrderKind::clear,{},{},true);});
  refuses([&]{intent(s,game::CreatureOrderKind::move,{2,3,0},{},false,true);});
  refuses([&]{intent(s,game::CreatureOrderKind::move,{-1,3,0});});
  refuses([&]{intent(s,game::CreatureOrderKind::coordinateAttack,{0,0,4});});
  need(snapshot(s)==start,"refused command changed state");
  intent(s,game::CreatureOrderKind::move,{3,4,0});
  auto c=command_session::decode(s.world().state().systems);
  need(c.bindings[0]==m::CommandBehaviorBinding::move&&c.world.creatures[0].destination==m::Cell{3,4,0},"move not wired to owned context");
  s.step();need(s.world().state().slots[0].entity->x==3,"native frame projection not published");
  intent(s,game::CreatureOrderKind::attack,{},game::Handle{1,1});
  need(s.world().state().slots[0].entity->target==game::Handle{1,1},"target not projected");
  intent(s,game::CreatureOrderKind::coordinateAttack,{5,6,1});
  c=command_session::decode(s.world().state().systems);
  need(c.execution[0].castAim==m::Cell{176,208,16}&&c.world.creatures[0].castTarget==-1,"coordinate cast aim not wired");
  intent(s,game::CreatureOrderKind::follow,{},game::Handle{1,1});
  need(command_session::decode(s.world().state().systems).execution[0].follow==1,"follow not retained");
  intent(s,game::CreatureOrderKind::clear);
  // Restore a clean initial session for queued delivery, exact restart and faults.
  game::CreatureRuleSession queued(rules,profile,11,3);
  intent(queued,game::CreatureOrderKind::move,{6,1,0},{},true);
  intent(queued,game::CreatureOrderKind::coordinateAttack,{7,2,1},{},true);
  need(command_session::decode(queued.world().state().systems).execution[0].orders.size()==2,"queued ingress not retained");
  auto saved=snapshot(queued);game::World restored(0);restored.restore(game::decodeSnapshot(saved));
  game::CreatureRuleSession resumed(std::move(restored),rules);
  for(unsigned i=0;i<80;++i){
    game::TickInput input{true,i%7==0,i%5==0};queued.step(input);resumed.step(input);
    need(snapshot(queued)==snapshot(resumed),"restored native command continuation diverged");
  }
  c=command_session::decode(resumed.world().state().systems);
  need(c.world.creatures[2].ai.hth[27]==715&&c.world.creatures[2].inventory[41].count==83&&c.execution[2].previousMoveVariant==319,"deep profile/continuation fields lost");
  auto bytes=command_session::encode(c);
  auto excessive=bytes;for(unsigned i=20;i<24;++i)excessive[i]=255;
  refuses([&]{command_session::decode(excessive);});
  auto staleWorld=resumed.world().state();++staleWorld.slots[0].generation;
  game::World stale(0);stale.restore(staleWorld);refuses([&]{game::CreatureRuleSession rejected(std::move(stale),rules);});
  auto wrongProfile=resumed.world().state();wrongProfile.campaign[0]^=1;
  game::World wrong(0);wrong.restore(wrongProfile);refuses([&]{game::CreatureRuleSession rejected(std::move(wrong),rules);});
  auto trailing=bytes;trailing.push_back(0);refuses([&]{command_session::decode(trailing);});
  auto version=bytes;version[4]=255;refuses([&]{command_session::decode(version);});
  for(unsigned n:{0u,4u,8u,unsigned(bytes.size()-1)}){auto cut=bytes;cut.resize(n);refuses([&]{command_session::decode(cut);});}
  // Stage unsafe native continuations; the public session must preserve every byte.
  for(unsigned mode=0;mode<3;++mode){
    auto seed=initial();auto &e=seed.execution[0];e.orders.resize(3);e.queuedCount=3;e.queuedFlag=mode==0?0:1;e.orderCursor=mode==0?2:1;e.nextOrderSerial=4;
    for(unsigned j=0;j<3;++j){e.orders[j].serial=j+1;e.orders[j].kind=2;e.orders[j].destination={3,2,0};}
    if(mode==0){e.orders[2].kind=3;e.orders[2].target=1;seed.world.creatures[1].ai.health=0;}
    if(mode==2){e.orders[1].kind=3;e.orders[1].target=1;}
    auto rt=std::make_shared<command_session::Runtime>(seed,profile,providers(faults));
    game::CreatureRuleSession bad(rt,profile,13,3);auto prior=snapshot(bad);
    faults->topology=mode==1;faults->reentry=mode==2;
    if(mode==0){auto state=bad.world().state();auto k=command_session::decode(state.systems);k.bindings[0]=m::CommandBehaviorBinding::idle;state.systems=command_session::encode(k);game::World w(0);w.restore(state);bad=game::CreatureRuleSession(std::move(w),rt);prior=snapshot(bad);}
    refuses([&]{bad.step();});need(snapshot(bad)==prior,"unsafe queue step published mutations");
    faults->topology=faults->reentry=false;
    intent(bad,game::CreatureOrderKind::clear);need(command_session::decode(bad.world().state().systems).execution[0].orders.empty(),"clear did not recover refused queue");
  }
  std::uint64_t digest=1469598103934665603ull;
  for(auto byte:snapshot(resumed))digest=(digest^byte)*1099511628211ull;
  std::cout<<"{\"success\":true,\"snapshot_digest\":"<<digest<<",\"restored_ticks\":80,\"semantic_commands\":5,\"unsafe_queue_rollbacks\":3,\"generation_guards\":true,\"portable_continuation\":true,\"no_wine\":true}\n";
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
