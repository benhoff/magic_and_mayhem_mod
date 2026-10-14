#define main unused_native_command_session_main
#include "native-command-session-test.cpp"
#undef main
namespace {
command_session::HookFactory recoveredProviders(std::shared_ptr<Faults> faults){
 return [faults](auto &c){auto h=providers(faults)(c);h.body={};
  h.commandBodies=[&c,faults](auto &a,unsigned kind,auto &commands){
   m::CommandBehaviorProviders p;
   auto action=[&c,faults](auto &actor,unsigned n){actor.ai.action=n;++c.backend[3];if(faults->command)throw std::runtime_error("injected recovered body failure");};
   p.behavior=[action](unsigned){m::BehaviorExecutionServices s;s.action=action;
    s.beginMotion=[](auto &actor){actor.routePhase=1;};
    s.reach=[](const auto &actor,const auto &target,m::Cell){return actor.ai.position==target.ai.position;};
    s.coordinateRange=[](auto &,m::Cell,unsigned){return true;};
    s.coordinateFace=[](auto &,m::Cell){};s.face=[](auto &,const auto &){};
    s.restoreForm=[](auto &actor,unsigned){actor.form=0;};return s;};
   p.ranged=[action](unsigned){m::RangedExecutionServices s;s.action=action;
    s.orient=[](auto &,m::Cell){};s.blocked=[](auto &){return false;};
    s.inRange=[](auto &,const auto &,unsigned){return true;};
    s.closeResponse=[](auto &,m::Cell,m::Cell){return false;};s.prepareResponse=[](auto &){return false;};return s;};
   p.movement=[action,&c](unsigned){m::MovementBehaviorServices s;s.action=action;
    s.beginMotion=[](auto &actor){actor.routePhase=1;};s.fallback=[](auto &){return false;};
    s.renew=[](auto &actor,const auto &){actor.block723=1;};
    s.landingHealth=[](auto &,unsigned,int){return 100u;};s.setHealth=[](auto &actor,unsigned health){actor.ai.health=health;};
    s.adjust=[](const auto &,m::Cell &){return true;};s.releaseOccupancy=[](auto &,m::Cell){};
    s.relocate=[](auto &actor,m::Cell pos){actor.ai.position=pos;};
    s.move=[](auto &actor,m::Cell pos){actor.ai.position=pos;};
    s.clearOverlay=[](auto &){};s.invalidate=[](auto &){};s.settle=[](auto &){};s.refresh=[](auto &){};
    s.remove=[](auto &){throw std::runtime_error("dynamic removal outside native profile");};
    s.releaseEffect=[&c](unsigned){++c.backend[2];};return s;};
   m::OwnedCommandBehaviors bodies(c.world,c.execution,c.maintenance,c.movement,commands,{c.backend[1],42},p);bodies.dispatch(a,kind);
  };
  h.advance=[](auto &state,auto &commands,game::TickInput){
   ++state.backend[1];
   // Explicit synthetic route provider; recovered callbacks consume its result.
   for(auto &a:state.world.creatures){
    if(state.bindings[a.ai.slot]==m::CommandBehaviorBinding::move&&a.routePhase){
     a.ai.position=a.destination;a.routePhase=0;
    }
    commands.dispatch(a);
   }
   return std::vector<game::CreatureRuleEvent>{{{0,state.generations[0]},"owned-command-behavior",{},state.backend[1]}};
  };
  return h;
 };
}
}
int main()try{
 auto seed=initial();seed.maintenance.resize(3);seed.movement.resize(3);
 for(unsigned i=0;i<3;++i){auto &a=seed.world.creatures[i];a.rangedDistance=3;a.ai.health=400;
  seed.maintenance[i].visibilityAge[7]=801+i;seed.maintenance[i].lifespan=-7-int(i);
  seed.maintenance[i].flightEffect=991+i;seed.movement[i].renewedOrder[7]=177+i;seed.movement[i].nextRouteCell={5,6,1};}
 auto faults=std::make_shared<Faults>();game::Bytes profile{63,1,28,4};
 auto runtime=std::make_shared<command_session::Runtime>(seed,profile,recoveredProviders(faults));game::CreatureRuleSession s(runtime,profile,17,3);
 auto before=snapshot(s);faults->command=true;refuses([&]{intent(s,game::CreatureOrderKind::move,{6,1,0});});need(snapshot(s)==before,"owned callback failure published state");faults->command=false;
 intent(s,game::CreatureOrderKind::move,{6,1,0});auto c=command_session::decode(s.world().state().systems);
 need(c.world.creatures[0].routePhase==1&&c.bindings[0]==m::CommandBehaviorBinding::move,"move did not invoke recovered body");
 s.step();c=command_session::decode(s.world().state().systems);need(c.world.creatures[0].ai.position==m::Cell{6,1,0}&&c.bindings[0]==m::CommandBehaviorBinding::idle,"move completion did not reset shared context");
 intent(s,game::CreatureOrderKind::attack,{},game::Handle{1,1});c=command_session::decode(s.world().state().systems);need(c.world.creatures[0].ai.action==8,"target did not enter owned ranged continuation");
 before=snapshot(s);faults->command=true;refuses([&]{s.step();});need(snapshot(s)==before,"failed body tick published state");faults->command=false;
 intent(s,game::CreatureOrderKind::coordinateAttack,{5,6,1});c=command_session::decode(s.world().state().systems);need(c.world.creatures[0].ai.action==9,"coordinate did not invoke recovered callback");
 intent(s,game::CreatureOrderKind::clear);intent(s,game::CreatureOrderKind::move,{9,1,0},{},true);
 auto saved=snapshot(s);game::World world(0);world.restore(game::decodeSnapshot(saved));game::CreatureRuleSession restored(std::move(world),runtime);
 for(unsigned i=0;i<80;++i){s.step({true,i%7==0,i%3==0});restored.step({true,i%7==0,i%3==0});need(snapshot(s)==snapshot(restored),"owned behavior saved continuation diverged");}
 c=command_session::decode(restored.world().state().systems);need(c.maintenance[2].visibilityAge[7]==803&&c.maintenance[2].lifespan==-9&&c.maintenance[2].flightEffect==993&&c.movement[2].renewedOrder[7]==179&&c.movement[2].nextRouteCell==m::Cell{5,6,1},"full maintenance/movement continuation lost");
 // Provider installation/root identity admission is native policy.
 auto staged=c;auto hooks=recoveredProviders(faults)(staged);
 m::OwnedCreatureCommands commands(staged.world,staged.execution,staged.bindings,hooks.execution,hooks.idle,hooks.follow,hooks.queue,hooks.body,hooks.supplied,hooks.release,hooks.peer);
 refuses([&]{commands.installBodies({});});commands.installBodies(hooks.commandBodies);
 refuses([&]{commands.installBodies(hooks.commandBodies);});
 m::CommandBehaviorProviders probe;probe.movement=[](unsigned){return m::MovementBehaviorServices{};};probe.behavior=[](unsigned){return m::BehaviorExecutionServices{};};probe.ranged=[](unsigned){return m::RangedExecutionServices{};};
 m::OwnedCommandBehaviors bodies(staged.world,staged.execution,staged.maintenance,staged.movement,commands,{80,42},probe);
 auto foreign=staged.world.creatures[0];refuses([&]{bodies.dispatch(foreign,2);});
 refuses([&]{bodies.dispatch(staged.world.creatures[0],1);});
 auto other=staged.world;refuses([&]{m::OwnedCommandBehaviors wrong(other,staged.execution,staged.maintenance,staged.movement,commands,{80,42},probe);});
 auto arithmeticRefuses=[&](auto f){bool caught=false;try{f();}catch(const std::invalid_argument &e){caught=std::string(e.what())=="unsafe command behavior arithmetic";}need(caught,"arithmetic must refuse before lower providers");};
 auto &actor=staged.world.creatures[0];auto destination=actor.destination;
 actor.destination.x=std::numeric_limits<int>::min();arithmeticRefuses([&]{bodies.dispatch(actor,2);});actor.destination=destination;
 actor.ai.target=1;auto &target=staged.world.creatures[1];auto fine=target.finePosition;auto aim=target.aimXY;
 target.finePosition.x=std::numeric_limits<int>::max();target.aimXY=1;arithmeticRefuses([&]{bodies.dispatch(actor,3);});
 target.finePosition=fine;target.finePosition.y=std::numeric_limits<int>::min();target.aimXY=-1;arithmeticRefuses([&]{bodies.dispatch(actor,3);});target.finePosition=fine;target.aimXY=aim;
 auto simple=initial();auto bytes=command_session::encode(simple);bytes.resize(bytes.size()-8);bytes[4]=1;auto legacy=command_session::decode(bytes);need(legacy.maintenance.empty()&&legacy.movement.empty()&&command_session::encode(legacy)==command_session::encode(simple),"v1 continuation migration changed old state");
 auto mismatch=restored.world().state();auto missing=c;missing.maintenance.clear();missing.movement.clear();mismatch.systems=command_session::encode(missing);game::World rejected(0);rejected.restore(mismatch);refuses([&]{game::CreatureRuleSession wrong(std::move(rejected),runtime);});
 auto malformed=c;malformed.movement.pop_back();refuses([&]{command_session::encode(malformed);});
 std::uint64_t digest=1469598103934665603ull;for(auto byte:snapshot(restored))digest=(digest^byte)*1099511628211ull;
 std::cout<<"{\"behavior_session_passed\":true,\"behavior_ticks\":80,\"behavior_digest\":"<<digest<<",\"behavior_commands\":3,\"behavior_rollback\":true,\"v1_migration\":true,\"maintenance_continuation\":true,\"body_context_refusals\":8}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
