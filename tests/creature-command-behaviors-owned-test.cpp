#define main unused_command_fixture_main
#include "creature-transfer-commands-owned-test.cpp"
#undef main
#include "../reconstruction/creature-ai/command_behaviors_owned.hpp"
namespace {
struct BodyState {std::vector<CreatureMaintenance> maintenance;std::vector<MovementBehaviorState> movement;};
BodyState *bodyState;
void bodyTrace(unsigned code,unsigned slot,unsigned argument=0,Cell p={}) {
  event(code,(slot<<16)|argument);events.insert(events.end(),{unsigned(p.x),unsigned(p.y),unsigned(p.z)});
}
void bodySync() {
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  if(original)syncActor();
#endif
}
void bodyAction(AutonomousCreature &a,unsigned kind){bodyTrace(40,a.ai.slot,kind);a.ai.action=kind;bodySync();}
void bodyBegin(AutonomousCreature &a){bodyTrace(41,a.ai.slot);a.routePhase=1+current->number%2;bodySync();}
bool bodyReach(const AutonomousCreature &a,const AutonomousCreature &b,Cell p){bodyTrace(42,a.ai.slot,b.ai.slot,p);return current->number%4==0;}
bool bodyRange(AutonomousCreature &a,Cell p,unsigned r){bodyTrace(43,a.ai.slot,r,p);return current->number%3!=0;}
void bodyFace(AutonomousCreature &a,Cell p){bodyTrace(44,a.ai.slot,0,p);}
bool bodyAdjust(const AutonomousCreature &a,Cell &p){bodyTrace(45,a.ai.slot,0,p);if(current->number%5==0)return false;p.x=(p.x+1)%16;return true;}
ExecutionServices bodyExecution(unsigned slot){auto s=resetServices(slot);s.action=bodyAction;return s;}
BehaviorExecutionServices bodyIdle(unsigned slot){auto s=idleServices(slot);s.action=bodyAction;return s;}
FollowServices bodyFollow(unsigned){return {bodyAdjust,{}};}
CommandBehaviorProviders bodyProviders(){
 CommandBehaviorProviders p;
 p.behavior=[](unsigned){BehaviorExecutionServices s;s.action=bodyAction;s.beginMotion=bodyBegin;s.reach=bodyReach;
  s.face=[](auto &a,const auto &b){bodyTrace(46,a.ai.slot,b.ai.slot);};
  s.coordinateRange=bodyRange;s.coordinateFace=bodyFace;
  s.restoreForm=[](auto &a,unsigned mode){bodyTrace(47,a.ai.slot,mode);if(current->number%2)a.ai.behavior=2;bodySync();};
  s.reset=[](auto &){throw std::runtime_error("duplicate behavior reset escaped");};
  s.behavior=[](auto &,unsigned){throw std::runtime_error("duplicate transition escaped");};
  s.ranged=s.maintainTarget=[](auto &){throw std::runtime_error("duplicate target continuation escaped");};return s;};
 p.ranged=[](unsigned){RangedExecutionServices s;s.action=bodyAction;s.orient=bodyFace;
  s.blocked=[](auto &a){bodyTrace(48,a.ai.slot);return current->number%7==0;};
  s.inRange=[](auto &a,const auto &b,unsigned r){bodyTrace(49,a.ai.slot,b.ai.slot,{int(r),0,0});return current->number%3!=0;};
  s.closeResponse=[](auto &a,Cell from,Cell to){bodyTrace(50,a.ai.slot,0,from);bodyTrace(51,a.ai.slot,0,to);return current->number%4!=0;};
  s.prepareResponse=[](auto &a){bodyTrace(52,a.ai.slot);return current->number%5!=0;};
  s.maintainTarget=[](auto &){throw std::runtime_error("duplicate ranged continuation escaped");};return s;};
 p.movement=[](unsigned){MovementBehaviorServices s;s.action=bodyAction;s.beginMotion=bodyBegin;
  s.renew=[](auto &a,const auto &v){bodyTrace(53,a.ai.slot,v[0]);if(current->number%3)a.block723=1;bodySync();};
  s.fallback=[](auto &a){bodyTrace(54,a.ai.slot);return current->number%7==0;};
  s.landingHealth=[](auto &a,unsigned kind,int z){bodyTrace(55,a.ai.slot,kind,{z,0,0});return 77u;};
  s.setHealth=[](auto &a,unsigned n){bodyTrace(56,a.ai.slot,n);};
  s.adjust=bodyAdjust;s.releaseOccupancy=[](auto &a,Cell q){bodyTrace(57,a.ai.slot,0,q);};
  s.relocate=[](auto &a,Cell q){bodyTrace(58,a.ai.slot,a.ai.type,q);};
  s.clearOverlay=[](auto &a){bodyTrace(59,a.ai.slot);};
  s.invalidate=[](auto &a){bodyTrace(60,a.ai.slot);};
  s.settle=[](auto &a){bodyTrace(61,a.ai.slot);};s.refresh=[](auto &a){bodyTrace(62,a.ai.slot);};
  s.remove=[](auto &a){bodyTrace(63,a.ai.slot);};s.releaseEffect=[](unsigned n){bodyTrace(64,0,n);};
  s.move=[](auto &a,Cell p){bodyTrace(65,a.ai.slot,0,p);};
  s.renewMove=[](auto &,Cell){throw std::runtime_error("duplicate movement renewal order escaped");};
  s.reset=[](auto &){throw std::runtime_error("duplicate movement reset escaped");};return s;};
 return p;
}
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
void packBody(Bytes &actors,Bytes &defs,const Fixture &f,const BodyState &state){
 for(unsigned i=0;i<f.relations.creatures.size();++i){auto *p=actors.data()+i*0xe4b;auto *c=defs.data()+i*0x5c9;
  const auto &a=f.relations.creatures[i];const auto &e=f.executions[i];const auto &m=state.maintenance[i];const auto &v=state.movement[i];
  for(auto x:{std::pair<unsigned,unsigned>{0x6a4,a.targetMovingAttacker},{0x650,unsigned(a.chosenDestination.x)},{0x654,unsigned(a.chosenDestination.y)},{0x658,unsigned(a.chosenDestination.z)},{0x14,unsigned(a.finePosition.x)},{0x18,unsigned(a.finePosition.y)},{0x1c,unsigned(a.finePosition.z)},{0x160,e.rangedDeadline},{0x670,e.rangedLatch},{0x110,m.flight110},{0x809,m.flight809},{0x104,a.flag104},{0xa4,v.landingKind},{0x977,0},{0x97f,unsigned(v.nextRouteCell.x)},{0x983,unsigned(v.nextRouteCell.y)},{0x987,unsigned(v.nextRouteCell.z)}})put(p,x.first,x.second);
  p[0x71d]=m.block71d;put(p,0x805,m.flightEffect?reinterpret_cast<std::uintptr_t>(effects.data()+*m.flightEffect*4):0);
  for(unsigned j=0;j<8;++j){put(p,0x24+4*j,a.ai.visible[j]);put(p,0x724+4*j,v.renewedOrder[j]);}
  put(c,0x34,unsigned(a.rangedDistance));put(c,0x3c,a.formCapability);put(c,0x198,unsigned(a.aimXY));
 }
}
void importBody(){
 for(unsigned i=0;i<current->relations.creatures.size();++i){auto *p=actor.data()+i*0xe4b;auto &a=current->relations.creatures[i];auto &e=current->executions[i];auto &m=bodyState->maintenance[i];
  a.targetMovingAttacker=get(p,0x6a4);a.chosenDestination={int(get(p,0x650)),int(get(p,0x654)),int(get(p,0x658))};
  e.rangedLatch=get(p,0x670);e.rangedDeadline=get(p,0x160);a.flag104=get(p,0x104);m.flight110=get(p,0x110);m.flight809=get(p,0x809);m.block71d=p[0x71d];m.flightEffect=effectToken(get(p,0x805));
 }
}
AutonomousCreature &subject(void *p){importActor();return current->relations.creatures.at(actorSlot(p));}
extern "C" void __attribute__((thiscall)) bodyActionStub(void *p,unsigned kind){bodyAction(subject(p),kind);}
extern "C" void __attribute__((thiscall)) bodyBeginStub(void *p){bodyBegin(subject(p));}
extern "C" unsigned __attribute__((thiscall)) bodyReachStub(void *p,void *b,int x,int y,int z){auto &a=subject(p);return bodyReach(a,current->relations.creatures.at(actorSlot(b)),{x,y,z});}
extern "C" unsigned __attribute__((thiscall)) bodyRangeStub(void *p,int x,int y,int z,unsigned r){return bodyRange(subject(p),{x,y,z},r);}
extern "C" void __attribute__((thiscall)) bodyFaceStub(void *p,int x,int y){bodyFace(subject(p),{x,y,0});}
extern "C" void __attribute__((thiscall)) bodyTargetFaceStub(void *p,void *b){bodyTrace(46,subject(p).ai.slot,actorSlot(b));}
extern "C" void __attribute__((thiscall)) bodyRestoreStub(void *p,unsigned mode){bodyProviders().behavior(0).restoreForm(subject(p),mode);}
extern "C" unsigned __attribute__((thiscall)) bodyBlockedStub(void *p){return bodyProviders().ranged(0).blocked(subject(p));}
extern "C" unsigned __attribute__((thiscall)) bodyTargetRangeStub(void *p,void *b,unsigned r){return bodyProviders().ranged(0).inRange(subject(p),current->relations.creatures.at(actorSlot(b)),r);}
extern "C" unsigned __attribute__((thiscall)) bodyCloseStub(void *p,int x,int y,int z,int u,int v,int w){return bodyProviders().ranged(0).closeResponse(subject(p),{x,y,z},{u,v,w});}
extern "C" unsigned __attribute__((thiscall)) bodyResponseStub(void *p){return bodyProviders().ranged(0).prepareResponse(subject(p));}
extern "C" void __attribute__((thiscall)) bodyRenewStub(void *p,unsigned a,unsigned b,unsigned c,unsigned d,unsigned e,unsigned f,unsigned g,unsigned h){bodyProviders().movement(0).renew(subject(p),{a,b,c,d,e,f,g,h});}
extern "C" unsigned __attribute__((thiscall)) bodyFallbackStub(void *p){return bodyProviders().movement(0).fallback(subject(p));}
extern "C" unsigned __attribute__((thiscall)) bodyLandingStub(void *p,unsigned kind,int z){return bodyProviders().movement(0).landingHealth(subject(p),kind,z);}
extern "C" void __attribute__((thiscall)) bodyHealthStub(void *p,unsigned n,unsigned invalid,unsigned zero){if(invalid!=unsigned(-1)||zero)throw std::runtime_error("landing health ABI");bodyProviders().movement(0).setHealth(subject(p),n);}
extern "C" unsigned __attribute__((thiscall)) bodyAdjustStub(void *,unsigned id,const void *,int *x,int *y,int *z){importActor();Cell q{*x,*y,*z};bool yes=bodyAdjust(current->relations.creatures.at(id),q);*x=q.x;*y=q.y;*z=q.z;return yes;}
extern "C" void __attribute__((thiscall)) bodyOccupancyStub(void *p,int x,int y,int z){bodyProviders().movement(0).releaseOccupancy(subject(p),{x,y,z});}
extern "C" void __attribute__((thiscall)) bodyRelocateStub(void *p,int x,int y,int z,unsigned type){auto &a=subject(p);if(type!=a.ai.type)throw std::runtime_error("relocate type");bodyProviders().movement(0).relocate(a,{x,y,z});}
extern "C" void __attribute__((thiscall)) bodyOverlayStub(void *p){bodyTrace(59,actorSlot(static_cast<unsigned char *>(p)-0x7d5));}
extern "C" void __attribute__((thiscall)) bodyInvalidateStub(void *p){bodyTrace(60,subject(p).ai.slot);}
extern "C" void __attribute__((thiscall)) bodyInvalidateArgStub(void *p,unsigned zero){if(zero)throw std::runtime_error("invalidate ABI");bodyTrace(60,subject(p).ai.slot);}
extern "C" void __attribute__((thiscall)) bodyLandingMoveStub(void *p,int x,int y,int z){bodyProviders().movement(0).move(subject(p),{x,y,z});}
extern "C" void __attribute__((thiscall)) bodySettleStub(void *p){bodyTrace(61,subject(p).ai.slot);}
extern "C" void __attribute__((thiscall)) bodyRefreshStub(void *p){bodyTrace(62,subject(p).ai.slot);}
extern "C" void __attribute__((thiscall)) bodyRemoveStub(void *p){bodyTrace(63,subject(p).ai.slot);}
void installBodyProviders(){
 for(auto v:{std::pair<unsigned,void*>{0x50db60,reinterpret_cast<void*>(bodyActionStub)},{0x510e80,reinterpret_cast<void*>(bodyBeginStub)},{0x50bd60,reinterpret_cast<void*>(bodyReachStub)},{0x515350,reinterpret_cast<void*>(bodyRangeStub)},{0x508250,reinterpret_cast<void*>(bodyFaceStub)},{0x508810,reinterpret_cast<void*>(bodyTargetFaceStub)},{0x5142b0,reinterpret_cast<void*>(bodyRestoreStub)},{0x517750,reinterpret_cast<void*>(bodyBlockedStub)},{0x515620,reinterpret_cast<void*>(bodyTargetRangeStub)},{0x50efc0,reinterpret_cast<void*>(bodyCloseStub)},{0x50b650,reinterpret_cast<void*>(bodyResponseStub)},{0x51f270,reinterpret_cast<void*>(bodyRenewStub)},{0x50ed80,reinterpret_cast<void*>(bodyFallbackStub)},{0x513c20,reinterpret_cast<void*>(bodyLandingStub)},{0x514860,reinterpret_cast<void*>(bodyHealthStub)},{0x4f5230,reinterpret_cast<void*>(bodyAdjustStub)},{0x5093e0,reinterpret_cast<void*>(bodyOccupancyStub)},{0x5070e0,reinterpret_cast<void*>(bodyRelocateStub)},{0x464c80,reinterpret_cast<void*>(bodyOverlayStub)},{0x51aa30,reinterpret_cast<void*>(bodyInvalidateStub)},{0x51f5e0,reinterpret_cast<void*>(bodyInvalidateArgStub)},{0x509230,reinterpret_cast<void*>(bodyLandingMoveStub)},{0x51a8f0,reinterpret_cast<void*>(bodySettleStub)},{0x50f850,reinterpret_cast<void*>(bodyRefreshStub)},{0x506d30,reinterpret_cast<void*>(bodyRemoveStub)}})redirect(v.first,v.second);
}
#endif
}
int main(int argc,char **argv)try{
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
 if(argc!=2)throw std::runtime_error("pinned PE expected");
 install(argv[1],true);installBodyProviders();
#else
 (void)argc;(void)argv;
#endif
 std::uint64_t digest=1469598103934665603ull;auto mix=[&](unsigned v){digest=(digest^v)*1099511628211ull;};unsigned cases=0;
 for(unsigned n=0;n<1024;++n)for(unsigned op=0;op<6;++op){
  auto input=generateTransfer(n);BodyState state;state.maintenance.resize(input.executions.size());state.movement.resize(input.executions.size());
  for(unsigned i=0;i<input.executions.size();++i){auto &a=input.relations.creatures[i];auto &e=input.executions[i];auto &m=state.maintenance[i];auto &v=state.movement[i];
   a.ai.type=(n+i)%28;a.ai.health=400;a.ai.target=int((i+1)%input.executions.size());a.ai.action=15;a.ai.pending=0;a.ai.engaged=0;a.ai.marker=n%7==0;a.ai.motion=n%5==0;a.ai.incoming=n%3==0;
   a.ai.position={int((n+i)%14),2,int(n%3)};a.destination=n%4==0?a.ai.position:n%4==1?Cell{a.ai.position.x+1,2,a.ai.position.z}:Cell{15,7,3};
   a.chosenDestination=n%2?a.ai.position:Cell{6,7,2};a.form=n%13==0;a.formCapability=n%17==0;a.block784=a.mode=0;a.blockDE3=n%5;a.block71c=n%11==0;a.block723=n%4==0;a.routePhase=n%3==0;a.animationKind=n%4;a.rangedDistance=n%5?3:0;
   a.finePosition={a.ai.position.x*32,64,16};a.aimXY=n%16;a.ai.kind=n%28;e.routeChanged=n%9==0;e.rangedDeadline=n%5?1000:1001;e.rangedLatch=n%2;e.targetDistance=n%30;e.castAim={int(n%512),int(n%256),32};e.follow=-1;e.orders.clear();e.orderCursor.reset();e.queuedCount=e.queuedFlag=0;
   input.initialOrders[i].clear();input.idleBindings[i]=static_cast<CommandBehaviorBinding>(2+op%3);
   for(unsigned j=0;j<8;++j){a.ai.visible[j]=(n+i+j)%2;v.renewedOrder[j]=j+1;}
   m.flight110=n%2;m.block71d=n%3==0;m.flight809=2;v.landingKind=2;v.nextRouteCell={3,4,1};
   input.descriptorProfiles[i][2]=a.formCapability;
  }
  if(op%3==1&&n%19==0)input.relations.creatures[0].ai.target=-1;
  auto expected=input;auto expectedState=state;current=&expected;bodyState=&expectedState;original=false;events.clear();
  OwnedCreatureCommands commands(expected.relations,expected.executions,expected.idleBindings,bodyExecution,bodyIdle,bodyFollow,queueServices,commandBody,repairServices().behavior,services().releaseEffect,repairServices().peerEngagement);
  commands.installBodies([&](auto &a,unsigned kind,auto &c){OwnedCommandBehaviors b(expected.relations,expected.executions,expectedState.maintenance,expectedState.movement,c,{1000,42},bodyProviders());b.dispatch(a,kind);});
  if(op<3)commands.dispatch(expected.relations.creatures[0]);
  else commands.transition(expected.relations.creatures[0],2+op%3);
  auto trace=events;
#ifdef MNM_TRANSFER_COMMANDS_REFERENCE
  auto actualState=state;bodyState=&actualState;
  fixtureImportExtra=importBody;fixtureSyncExtra=[&]{packBody(actor,definition,*current,actualState);};
  compare(input,expected,trace,nullptr,[&](void *p){global(0x6bf3c4,42);global(0x6c4830,1000);
    if(op<3){using F=void(__attribute__((thiscall))*)(void*);reinterpret_cast<F>(std::array<unsigned,3>{0x510010,0x50afa0,0x50ccd0}[op])(p);}
    else {using F=void(__attribute__((thiscall))*)(void*,unsigned);reinterpret_cast<F>(0x50aa30)(p,2+op%3);}
  },true,[&](Bytes &a,Bytes &d,bool result){packBody(a,d,result?expected:input,result?expectedState:state);});
  fixtureImportExtra={};fixtureSyncExtra={};
#endif
  ++cases;for(const auto &a:expected.relations.creatures){mix(a.ai.behavior);mix(a.ai.action);mix(a.targetMovingAttacker);mix(a.cooldown);mix(a.routePhase);mix(unsigned(a.chosenDestination.x));}
  for(const auto &e:expected.executions){mix(e.routeChanged);mix(e.markerB73);mix(e.rangedLatch);mix(e.targetDistance);mix(e.block722);}
  for(const auto &m:expectedState.maintenance){mix(m.block71d);mix(m.flight809);}for(auto v:trace)mix(v);
 }
 std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"functions\":30,\"outcome_digest\":"<<digest<<",\"owned_behavior_bodies\":3,\"shared_command_context\":true}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
