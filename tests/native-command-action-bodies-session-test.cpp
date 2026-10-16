#pragma push_macro("main")
#undef main
#define main unused_action_session_main
#include "native-command-action-session-test.cpp"
#undef main
#pragma pop_macro("main")
namespace {
command_session::Continuation fullInitial(unsigned handler) {
  auto c=initial();c.backend.resize(19);c.backend[4]=17;
  c.maintenance.resize(3);c.movement.resize(3);c.originalBehaviorBindings=true;
  c.actions.callbacks.resize(3);c.actions.motion.resize(3);c.actions.segments.resize(3);
  c.actions.objects.dimensions=c.world.dimensions;
  c.actions.objects.heads.assign(16*16*4,0xffff);
  for(unsigned i=0;i<3;++i) {
    auto &a=c.world.creatures[i];a.ai.health=400;a.animationKind=1;a.ai.behavior=4;
    a.ai.action=handler;a.ai.target=(i+1)%3;a.alternateTarget=(i+2)%3;
    a.rangedDistance=3;a.cooldown=0;c.bindings[i]=m::CommandBehaviorBinding::coordinate;
    c.execution[i].actionCallback=handler;c.execution[i].moveVariant=192;
    auto &motion=c.actions.motion[i];motion.samples.fill(32);motion.duration=192;
    motion.heightOrigin=a.finePosition.z;motion.direction=2;
    auto &segment=c.actions.segments[i];segment.count=1;segment.route[0].position=a.ai.position;
    c.actions.callbacks[i].contributors={0,1};
  }
  return c;
}
m::CommandActionBodyProviders bodyProviders(command_session::Continuation &c,
                                            std::shared_ptr<Faults> faults) {
  m::CommandActionBodyProviders p;
  p.callbacks=[&c,faults](unsigned slot) {
    m::ActionCallbackServices s;auto ani=actionProviders(c,faults).animation(slot);
    s.poll=ani.poll;s.random=actionProviders(c,faults).selection(slot).random;
    s.immediate=[](auto &){return false;};s.rangedOpponent=[](auto &){return false;};
    s.rewind=[&c](auto &a){c.backend[16+a.ai.slot]=0;};
    s.hit=[&c,faults](auto &){++c.backend[3];if(faults->event)throw std::runtime_error("injected melee hit failure");};
    s.interactionHit=s.hit;s.notifyHit=[](auto &){};
    s.effect=[&c](auto &,unsigned,m::Cell){++c.backend[3];};
    s.cue=[&c](auto &,m::ActionCue,unsigned){++c.backend[2];};
    s.defendedDamage=[](auto &a,unsigned n,int,const auto &){a.ai.health=a.ai.health>n?a.ai.health-n:0;};
    s.releaseEngagement=[](auto &a){if(a.ai.incoming)--a.ai.incoming;};
    s.diagnostic=[](unsigned){};s.stun=[](auto &,unsigned,unsigned){};
    s.drainForm=[](auto &,unsigned){};s.clearColor=[](auto &,unsigned){};s.pollColor=[](auto &,unsigned){};
    s.releaseEffect=[&c](unsigned){++c.backend[2];};s.updateOrders=[](auto &){};
    return s;
  };
  p.motion=[&c,faults](unsigned slot) {
    m::MotionActionServices s;s.poll=actionProviders(c,faults).animation(slot).poll;
    s.rewind=[&c](auto &a){c.backend[16+a.ai.slot]=0;};
    s.completeSegment=[&c](auto &a){a.ai.motion=0;++c.backend[3];};
    s.updateOrders=[](auto &){};s.visualFacing=[](auto &){};
    s.response=[](auto &){return false;};s.legal=[](const auto &,m::Cell){return true;};
    s.stun=[](auto &,unsigned,unsigned){};s.effect=[&c](auto &,unsigned,m::Cell){++c.backend[3];};
    s.footstep=[&c](auto &){++c.backend[2];};s.releaseEffect=[&c](unsigned){++c.backend[2];};return s;
  };
  p.flight=[&c,faults](unsigned slot) {
    m::SpecialFlightServices s;s.poll=actionProviders(c,faults).animation(slot).poll;
    s.rewind=[&c](auto &a){c.backend[16+a.ai.slot]=0;};s.facing=[](auto &){};
    s.completeSegment=[](auto &a){a.ai.motion=0;};s.environment=[](auto &){};
    s.animationUpdate=[](auto &){};s.terrain=[](auto &){};
    s.animation=actionProviders(c,faults).selection(slot).animation;
    s.stun=[](auto &,unsigned,unsigned,unsigned){};
    s.occupied=[](const auto &,m::Cell){return false;};s.passable=[](const auto &,m::Cell){return true;};
    s.ground=[](const auto &,m::Cell p){return p.z==0;};s.regionAt=[](const auto &,m::Cell){return -1;};
    s.regionRoute=[](const auto &,int,int){return true;};s.random=actionProviders(c,faults).selection(slot).random;
    s.diagnostic=[](unsigned){};s.release=[](auto &,m::Cell){};s.reserve=[](auto &,m::Cell){};
    s.relocateDead=[](auto &a,m::Cell pos){a.ai.position=pos;};
    s.snap=[](auto &a,m::Cell pos,unsigned){a.ai.position=pos;};
    s.adjustedLanding=[](const auto &,m::Cell &,unsigned,bool){return false;};
    s.response=[](auto &){return false;};s.fallDamage=[](auto &,unsigned,int){return 0u;};
    s.defended=[](auto &a,unsigned n,int,bool){a.ai.health=a.ai.health>n?a.ai.health-n:0;};return s;
  };
  p.objects=[&c,faults](unsigned slot) {
    m::ObjectActionServices s;s.poll=actionProviders(c,faults).animation(slot).poll;
    s.cue=[&c](unsigned,m::Cell){++c.backend[2];};return s;
  };
  return p;
}
command_session::HookFactory fullProviders(std::shared_ptr<Faults> faults) {
  return [faults](auto &c) {
    auto h=actionSessionProviders(faults)(c);h.originalBehaviorBindings=true;
    auto commandBodies=h.commandBodies;
    h.commandBodies=[commandBodies,&c](auto &a,unsigned kind,auto &commands) {
      if(kind<=4)commandBodies(a,kind,commands);
      else {++c.backend[3];commands.action(a,kind==5?3:kind==6?13:kind==8?14:kind==15?23:kind==16?24:15);}
    };
    h.actionTick=[&c,faults](auto &a,auto &commands) {
      auto p=actionProviders(c,faults);
      p.remaining=[&c,faults](auto &actor,unsigned handler,auto &context) {
        m::OwnedCommandActionBodies bodies(c.world,c.execution,c.maintenance,c.movement,c.actions,context,
              {{23,false},{c.world.dimensions,0,false}},bodyProviders(c,faults));
        bodies.tick(actor,handler);
      };
      m::OwnedCommandActions actions(c.world,c.execution,commands,{{1,64,2,32},c.backend[1],9,42},p);
      actions.tick(a);
    };
    return h;
  };
}
} // namespace
int main()try {
  constexpr unsigned handlers[]={1,2,3,8,9,10,13,14,15,18,20,21,23,24};
  auto faults=std::make_shared<Faults>();unsigned ticks=0;
  std::uint64_t digest=1469598103934665603ull;
  for(auto handler:handlers) {
    auto seed=fullInitial(handler);game::Bytes profile{65,std::uint8_t(handler)};
    auto runtime=std::make_shared<command_session::Runtime>(seed,profile,fullProviders(faults));
    game::CreatureRuleSession s(runtime,profile,17,3);
    auto saved=snapshot(s);game::World world(0);world.restore(game::decodeSnapshot(saved));
    game::CreatureRuleSession resumed(std::move(world),runtime);
    for(unsigned i=0;i<24;++i) {s.step();resumed.step();need(snapshot(s)==snapshot(resumed),"full action continuation diverged");++ticks;}
    for(auto byte:snapshot(s))digest=(digest^byte)*1099511628211ull;
  }
  auto seed=fullInitial(10);game::Bytes profile{65,10};
  auto runtime=std::make_shared<command_session::Runtime>(seed,profile,fullProviders(faults));
  game::CreatureRuleSession s(runtime,profile,17,3);auto saved=snapshot(s);
  faults->event=true;refuses([&]{s.step();});need(snapshot(s)==saved,"failed melee body published state");faults->event=false;
  auto staged=seed;auto h=fullProviders(faults)(staged);
  m::OwnedCreatureCommands commands(staged.world,staged.execution,staged.bindings,h.execution,h.idle,h.follow,h.queue,h.body,h.supplied,h.release,h.peer);
  commands.installBodies(h.commandBodies,true);commands.installActions(h.actionTransition,h.actionTick);
  auto &a=staged.world.creatures[0];commands.transition(a,12);
  need(staged.bindings[0]==m::CommandBehaviorBinding::none,"null original behavior retained old descriptor");
  commands.dispatch(a);a.ai.motion=1;commands.transition(a,6);
  need(a.ai.pending==6&&staged.bindings[0]==m::CommandBehaviorBinding::none,"deferred behavior changed callback");
  a.ai.motion=0;commands.transition(a,6);need(staged.bindings[0]==m::CommandBehaviorBinding::state6,"special binding not installed");
  auto bytes=command_session::encode(seed);need(bytes[4]==12,"full action wire version missing");
  auto simple=initial();auto v2=command_session::encode(simple,2);
  auto old=command_session::decode(v2);need(old.actions.motion.empty()&&!old.originalBehaviorBindings,"v2 migration invented action state");
  auto missing=seed;missing.actions={};auto wrong=s.world().state();wrong.systems=command_session::encode(missing);
  game::World rejected(0);rejected.restore(wrong);refuses([&]{game::CreatureRuleSession bad(std::move(rejected),runtime);});
  std::cout<<"{\"action_bodies_passed\":true,\"action_body_handlers\":14,\"action_body_ticks\":"<<ticks<<",\"action_body_digest\":"<<digest<<",\"action_body_rollback\":true,\"v2_action_migration\":true}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
