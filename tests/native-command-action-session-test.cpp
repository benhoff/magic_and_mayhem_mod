#pragma push_macro("main")
#undef main
#define main unused_behavior_session_main
#include "native-command-behavior-session-test.cpp"
#undef main
#pragma pop_macro("main")
namespace {
m::CommandActionProviders actionProviders(command_session::Continuation &c,
                                         std::shared_ptr<Faults> faults) {
  m::CommandActionProviders p;
  p.selection = [&c](unsigned) {
    m::ActionExecutionServices s;
    s.random = [&c] {
      unsigned n = 0; for (unsigned i=0;i<4;++i) n |= unsigned(c.backend[4+i]) << (8*i);
      n ^= n << 13; n ^= n >> 17; n ^= n << 5;
      for (unsigned i=0;i<4;++i) c.backend[4+i] = (n >> (8*i)) & 255;
      return n;
    };
    s.animation = [&c](auto &a,unsigned,unsigned) { ++c.backend[10]; c.backend[16+a.ai.slot]=0; };
    s.overlay = [&c](auto &,unsigned) { ++c.backend[11]; };
    s.overlayAnimation = [&c](auto &,unsigned,unsigned) { ++c.backend[12]; };
    s.attackAnimation = [&c](auto &,const auto *) { ++c.backend[13]; };
    s.flightAnimation = [](auto &) { return false; };
    s.repairOverlay = [&c](auto &) { ++c.backend[11]; };
    return s;
  };
  p.animation = [&c,faults](unsigned) {
    m::AnimationActionServices s;
    s.poll = [&c,faults](auto &a,bool overlay) {
      ++c.backend[8];
      if(faults->tick) throw std::runtime_error("injected ANI event failure");
      if (overlay) return 0;
      auto &cursor = c.backend[16+a.ai.slot];
      const auto phase = cursor++;
      return phase == 0 ? 2 : phase == 1 ? 1 : 0;
    };
    s.cast = [&c,faults](auto &) {
      ++c.backend[9];
      if(faults->event) throw std::runtime_error("injected cast failure");
    };
    s.restoreForm = [&c](auto &a,unsigned) { a.form=0;++c.backend[14]; };
    s.animation = [&c](auto &a,unsigned,unsigned) { ++c.backend[10]; c.backend[16+a.ai.slot]=0; };
    s.behavior = [](auto &) { throw std::runtime_error("supplied behavior used by owned action"); };
    return s;
  };
  // Deliberately supplied remaining action bodies: this milestone does not
  // claim their original per-frame/event semantics.
  p.remaining = [&c](auto &a,unsigned,auto &commands) { ++c.backend[15]; commands.dispatch(a); };
  return p;
}
command_session::HookFactory actionSessionProviders(std::shared_ptr<Faults> faults) {
  return [faults](auto &c) {
    need(c.backend.size()==19,"action backend profile differs");
    auto h = recoveredProviders(faults)(c);
    auto execution = h.execution;
    h.execution = [execution](unsigned slot) {
      auto s = execution(slot); s.movementRenewal = [](auto &) { return false; }; return s;
    };
    auto invoke = [&c,faults](auto &commands) {
      return m::OwnedCommandActions(c.world,c.execution,commands,
          {{1,64,2,32},c.backend[1],9,42},actionProviders(c,faults));
    };
    h.actionTransition = [invoke](auto &a,unsigned kind,auto &commands) { invoke(commands).transition(a,kind); };
    h.actionTick = [invoke](auto &a,auto &commands) { invoke(commands).tick(a); };
    h.advance = [&c](auto &,auto &commands,game::TickInput) {
      ++c.backend[1];
      for(auto &a:c.world.creatures) {
        if(c.bindings[a.ai.slot]==m::CommandBehaviorBinding::move && a.routePhase) {
          a.ai.position=a.destination; a.routePhase=0;
        }
        commands.tickAction(a);
      }
      return std::vector<game::CreatureRuleEvent>{{{0,c.generations[0]},"owned-command-action",{},c.backend[9]}};
    };
    return h;
  };
}
} // namespace
int main() try {
  auto seed=initial();seed.backend.resize(19);seed.backend[4]=17;
  seed.maintenance.resize(3);seed.movement.resize(3);
  for(unsigned i=0;i<3;++i) {
    auto &a=seed.world.creatures[i];a.ai.health=400;a.rangedDistance=3;a.animationKind=1;
    seed.execution[i].actionCallback=15; a.ai.action=15; a.cooldown=0;
  }
  auto faults=std::make_shared<Faults>();game::Bytes profile{64,1,28,4};
  auto runtime=std::make_shared<command_session::Runtime>(seed,profile,actionSessionProviders(faults));
  game::CreatureRuleSession s(runtime,profile,17,3);
  intent(s,game::CreatureOrderKind::move,{6,1,0});
  auto c=command_session::decode(s.world().state().systems);
  need(c.execution[0].actionCallback==15 && c.backend[10]>0,"command action bypassed recovered selector");
  auto before=snapshot(s);faults->tick=true;refuses([&]{s.step();});
  need(snapshot(s)==before,"failed ANI poll published staged backend/clock");faults->tick=false;
  s.step();c=command_session::decode(s.world().state().systems);
  need(c.world.creatures[0].ai.position==m::Cell{6,1,0} && c.bindings[0]==m::CommandBehaviorBinding::idle,"ready event did not resume recovered movement/reset");
  intent(s,game::CreatureOrderKind::attack,{},game::Handle{1,1});
  c=command_session::decode(s.world().state().systems);need(c.execution[0].actionCallback==8,"ranged provider bypassed recovered selector");
  before=snapshot(s);faults->event=true;refuses([&]{s.step();});
  need(snapshot(s)==before,"failed cast published staged consequence");faults->event=false;
  s.step();c=command_session::decode(s.world().state().systems);need(c.backend[9]==1,"ranged animation event did not cast");
  intent(s,game::CreatureOrderKind::coordinateAttack,{5,6,1});
  c=command_session::decode(s.world().state().systems);need(c.execution[0].actionCallback==9,"coordinate provider bypassed recovered selector");
  auto saved=snapshot(s);game::World world(0);world.restore(game::decodeSnapshot(saved));
  game::CreatureRuleSession restored(std::move(world),runtime);
  for(unsigned i=0;i<80;++i) {
    s.step({true,i%7==0,i%3==0});restored.step({true,i%7==0,i%3==0});
    need(snapshot(s)==snapshot(restored),"saved action/ANI/RNG continuation diverged");
  }
  c=command_session::decode(s.world().state().systems);
  need(c.backend[9]>1 && c.backend[8]>0 && c.backend[11]>0,"action session lacked ANI/cast/overlay consequences");
  auto hooks=actionSessionProviders(faults)(c);
  m::OwnedCreatureCommands commands(c.world,c.execution,c.bindings,hooks.execution,hooks.idle,hooks.follow,hooks.queue,hooks.body,hooks.supplied,hooks.release,hooks.peer);
  refuses([&]{commands.installActions({},hooks.actionTick);});
  refuses([&]{commands.installActions(hooks.actionTransition,{});});
  commands.installBodies(hooks.commandBodies);commands.installActions(hooks.actionTransition,hooks.actionTick);
  refuses([&]{commands.installActions(hooks.actionTransition,hooks.actionTick);});
  auto foreign=c.world.creatures[0];refuses([&]{commands.action(foreign,15);});refuses([&]{commands.tickAction(foreign);});
  auto p=actionProviders(c,faults);m::OwnedCommandActions actions(c.world,c.execution,commands,{{1,64,2,32},80,9,42},p);
  auto other=c.world;refuses([&]{m::OwnedCommandActions wrong(other,c.execution,commands,{},p);});
  refuses([&]{actions.tick(foreign);});
  auto &a=c.world.creatures[0];a.ai.motion=0;
  c.execution[0].actionCallback=6;refuses([&]{commands.tickAction(a);});
  c.execution[0].actionCallback=1;p.remaining={};
  m::OwnedCommandActions partial(c.world,c.execution,commands,{},p);refuses([&]{partial.tick(a);});
  commands.action(a,4);need(!c.execution[0].actionCallback,"null table entry not retained");
  commands.tickAction(a);
  // CMD1/v2 already stores installed callback, all transition/event state and
  // opaque staged backend. No wire extension is needed for this profile.
  need(command_session::encode(command_session::decode(command_session::encode(c)))==command_session::encode(c),"action roots lost on portable roundtrip");
  std::uint64_t digest=1469598103934665603ull;for(auto b:snapshot(restored))digest=(digest^b)*1099511628211ull;
  std::cout<<"{\"action_session_passed\":true,\"action_ticks\":80,\"action_digest\":"<<digest<<",\"action_commands\":3,\"action_rollbacks\":2,\"action_context_refusals\":9,\"action_casts\":"<<unsigned(command_session::decode(restored.world().state().systems).backend[9])<<",\"action_continuation\":true}\n";
  return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
