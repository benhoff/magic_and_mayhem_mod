#define main unused_all_action_bodies_main
#include "native-command-action-bodies-session-test.cpp"
#undef main
#include "../apps/command-session/native_providers.hpp"
#include "../apps/command-session/native_geometry.hpp"
#include <algorithm>
#include <fstream>
#include <iterator>
namespace {
command_session::NativeCreatureProfile profile(const char *path){
  command_session::NativeCreatureProfile p;assets::Animation ani;
  if(path&&*path){std::ifstream file(path,std::ios::binary);need(bool(file),"installed ANI unavailable");std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(file),{});auto decoded=assets::decodeAnimation(bytes);need(std::holds_alternative<assets::Animation>(decoded),"installed ANI decode failed");ani=std::get<assets::Animation>(std::move(decoded));}
  else {ani.starts.push_back(0);for(unsigned sequence=0;sequence<328;++sequence){if(sequence<8){for(unsigned i=0;i<12;++i){assets::AnimationRecord r{0,int(i),{}};r.metadata[0]=32-i*2;ani.records.push_back(r);}ani.records.push_back({5,2,{}});assets::AnimationRecord repeat{0,0,{}};repeat.metadata[0]=0;ani.records.push_back(repeat);}else{ani.records.push_back({0,0,{}});ani.records.push_back({5,2,{}});ani.records.push_back({5,1,{}});}ani.records.push_back({6,0,{}});ani.starts.push_back(ani.records.size());}}
  p.animation.push_back(ani);p.terrain.resize(2);p.terrainPropertyA0.resize(2,false);p.terrain[1].flags_b0=8;p.terrain[1].classification_94=0;
  for(auto &type:p.types){type.scalar.type_d8=reconstruction::groundMovementSamples(ani);type.scalar.type_3c=reconstruction::groundMovementMaximum(type.scalar.type_d8);type.scalar.type_10=720;type.scalar.default_scalar=720;}
  p.maintenance.localOwner=-1;p.maintenance.drainDivisor=1;p.environment.waterline=0;
  return p;
}
command_session::NativeConsequenceFactory lower(std::shared_ptr<Faults> faults){return [faults](auto &c){
  command_session::NativeConsequences f;auto old=providers(faults)(c);auto b=bodyProviders(c,faults);
  f.execution=old.execution(0);f.execution.movementRenewal=[](auto &){return false;};f.execution.adjustDuplicate=[](m::Cell &){return false;};f.execution.duplicateMove=[](auto &,m::Cell p){return p;};
  f.callbacks=b.callbacks(0);f.callbacks.rangedOpponent=[](auto &){return false;};
  f.callbacks.hit=[&c,faults](auto &a){auto &target=c.world.creatures.at(unsigned(a.ai.target));target.ai.health-=std::min(target.ai.health,1u);++c.backend[3];if(faults->event)throw std::runtime_error("injected native effect failure");};
  f.callbacks.interactionHit=[&c,faults](auto &){++c.backend[3];if(faults->event)throw std::runtime_error("injected native cast failure");};
  f.behavior.immediate=[](auto &){return false;};f.behavior.interaction=[](auto &){return false;};f.behavior.restoreForm=[](auto &a,unsigned){a.form=0;};f.behavior.rangedOpponent=[](auto &){return false;};f.behavior.blocked=[](auto &){return false;};
  f.motion=b.motion(0);f.motion.response=[](auto &){return false;};f.motion.footstep=[&c](auto &){++c.backend[2];};
  f.flight=b.flight(0);f.flight.adjustedLanding=[](const auto &,m::Cell &,unsigned,bool){return false;};
  f.movement.fallback=[](auto &){return false;};f.movement.renew=[](auto &,const auto &){};f.movement.clearOverlay=[](auto &){};f.movement.invalidate=[](auto &){};f.movement.settle=[](auto &){};f.movement.refresh=[](auto &){};f.movement.remove=[](auto &a){a.ai.active=false;};
  f.segment.precheck=[](auto &){return false;};f.segment.immediate=[](auto &){return false;};f.segment.fallback=[](auto &){return false;};f.segment.visualFacing=[](auto &){};f.segment.hazard=[](const auto &,m::Cell){return false;};
  f.completion.environment=[](auto &){};f.completion.animation=[](auto &){};f.completion.terrain=[](auto &){};
  f.maintenance.repairAnimation=[faults](auto &){if(faults->tick)throw std::runtime_error("injected maintenance failure");};f.maintenance.refresh=[](auto &){};
  f.maintenance.color=[](auto &,m::MaintenanceColor,unsigned){};f.maintenance.health=[](auto &a,int amount,int,bool){a.ai.health=unsigned(std::clamp(std::int64_t(a.ai.health)+amount,std::int64_t(0),std::int64_t(a.ai.maximumHealth)));};
  f.maintenance.upkeep=[](auto &){};f.maintenance.remove=[](auto &a){a.ai.active=false;};f.maintenance.removeFromWorld=[](auto &){};
  f.ranged.blocked=[](auto &){return false;};f.ranged.inRange=[](auto &,const auto &,unsigned){return true;};f.ranged.closeResponse=[](auto &,m::Cell,m::Cell){return false;};f.ranged.prepareResponse=[](auto &){return false;};
  f.autonomy.geometry.hazardous=[](const auto &,m::Cell){return false;};f.autonomy.geometry.objectReachable=[](const auto &,m::Cell,m::Cell){return true;};f.autonomy.geometry.lineOfSight=[](const auto &,m::Cell){return true;};f.autonomy.geometry.threatReaches=[](const auto &,const auto &){return false;};
  f.autonomy.prepareInteraction=[](auto &){};f.autonomy.useInventory=[](auto &,auto &){};f.autonomy.refreshInventory=[](unsigned){};f.autonomy.orient=[](m::Cell){};
  f.death.remove=[](auto &a){a.ai.active=false;};f.death.registerCorpse=[](auto &){};f.death.releaseReference=[](auto &){};f.death.releaseUpkeep=[](auto &){};f.death.releaseOccupancy=[](auto &,m::Cell){};
  f.region=[](auto &){return 0;};f.ownerWork=[](auto &){};
  return f;
};}
} // namespace
int main(int argc,char **argv)try {
  const char *asset=argc>1?argv[1]:nullptr;auto p=profile(asset);auto c=initial();c.backend.resize(19);
  c.native.cells.resize(16*16*4);for(unsigned i=0;i<16*16;++i)c.native.cells[i].kind=1;
  for(auto &a:c.world.creatures){a.ai.health=a.ai.maximumHealth=360;a.animationKind=0;a.rangedDistance=0;a.ai.type=17;a.ai.kind=1;a.ai.action=15;c.execution[a.ai.slot].actionCallback=15;}
  // Compatible owners keep this movement fixture focused. Actual autonomous
  // orchestration still runs its recovered gates and20/90 phases.
  for(auto &row:c.world.relation)row.fill(1);
  for(auto &a:c.world.creatures)c.world.human[a.ai.owner]=true;
  command_session::prepareNativeContinuation(c,p);auto faults=std::make_shared<Faults>();game::Bytes identity{66,17};
  auto v4=command_session::encode(c,4);auto migrated=command_session::decode(v4);
  need(command_session::encode(migrated)==command_session::encode(c),"idle v4 migration differs");
  auto unsupported=c;unsupported.native.route.mode=1;refuses([&]{command_session::encode(unsupported,4);});
  unsupported=c;unsupported.native.route.scratch.count=17;refuses([&]{command_session::encode(unsupported);});
  unsupported=c;unsupported.native.route.active=2;refuses([&]{command_session::encode(unsupported);});
  unsupported=c;unsupported.native.route.active=1;refuses([&]{command_session::encode(unsupported);});
  auto runtime=std::make_shared<command_session::Runtime>(c,identity,command_session::nativeProviders(p,lower(faults)));
  game::CreatureRuleSession s(runtime,identity,17,3);intent(s,game::CreatureOrderKind::move,{6,3,0});
  auto prior=snapshot(s);faults->tick=true;refuses([&]{s.step();});need(snapshot(s)==prior,"failed native frame published state");faults->tick=false;
  auto saved=snapshot(s);game::World world(0);world.restore(game::decodeSnapshot(saved));game::CreatureRuleSession resumed(std::move(world),runtime);
  bool sawMotion=false,sawFine=false,sawArrival=false;unsigned sprites=0;
  for(unsigned i=0;i<120;++i){s.step();resumed.step();need(snapshot(s)==snapshot(resumed),"owned providers resumed frame diverged");auto state=command_session::decode(s.world().state().systems);auto &a=state.world.creatures[0];sawMotion|=a.ai.motion;sawFine|=a.finePosition.x!=a.ai.position.x*32||a.finePosition.y!=a.ai.position.y*32;sawArrival|=a.ai.position==m::Cell{6,3,0};sprites+=state.native.animation[0].body.state.forward.displayedRecord.has_value();}
  if(!(sawMotion&&sawFine&&sawArrival&&sprites>0)){auto q=command_session::decode(s.world().state().systems);auto &a=q.world.creatures[0];auto &m=q.actions.motion[0];std::cerr<<"motion "<<sawMotion<<" fine "<<sawFine<<" arrival "<<sawArrival<<" sprites "<<sprites<<" pos "<<a.ai.position.x<<","<<a.ai.position.y<<" action "<<a.ai.action<<" behavior "<<a.ai.behavior<<" duration "<<m.duration<<" rate "<<q.execution[0].moveVariant<<" cursor "<<q.actions.segments[0].cursor<<" count "<<q.actions.segments[0].count<<" marker "<<a.ai.marker<<" route "<<a.routePhase<<"\n";}
  need(sawMotion&&sawFine&&sawArrival&&sprites>0,"owned route/ANI motion did not complete");
  auto state=command_session::decode(s.world().state().systems);need(state.native.randomSeeded&&state.native.tick==120&&state.scheduler.phase20==0&&state.scheduler.phase90==30,"original frame phases/counters diverged");
  auto geometryState=c;command_session::NativeCreatureGeometry geometry(geometryState,p);unsigned category=5;
  need(geometry.move(geometryState.world.creatures[0],{0,1,0},{1,1,0},category)&&category==0,"owned ground admission failed");
  need(geometry.move(geometryState.world.creatures[0],{0,1,0},{15,1,0},category),"owned wrap admission failed");
  need(!geometry.move(geometryState.world.creatures[0],{3,1,0},{4,1,0},category),"occupied destination admitted");
  auto alternate=c;alternate.native.alternate=true;auto alternateRuntime=std::make_shared<command_session::Runtime>(alternate,identity,command_session::nativeProviders(p,lower(faults)));game::CreatureRuleSession alternateSession(alternateRuntime,identity,17,3);alternateSession.step();auto alternateState=command_session::decode(alternateSession.world().state().systems);
  need(alternateState.native.sequence==1&&alternateState.native.tick==0&&alternateState.scheduler.phase20==0&&alternateState.scheduler.phase90==0,"alternate frame ran normal creature passes");
  auto budget=initial();budget.backend.resize(19);budget.world.dimensions={128,128,1};budget.native.cells.resize(128*128);for(auto &cell:budget.native.cells)cell.kind=1;
  for(auto &a:budget.world.creatures){a.ai.health=a.ai.maximumHealth=360;a.ai.type=17;a.ai.kind=1;a.ai.action=15;budget.execution[a.ai.slot].actionCallback=15;}
  for(auto &row:budget.world.relation)row.fill(1);
  command_session::prepareNativeContinuation(budget,p);auto budgetRuntime=std::make_shared<command_session::Runtime>(budget,identity,command_session::nativeProviders(p,lower(faults)));game::CreatureRuleSession pending(budgetRuntime,identity,17,3);intent(pending,game::CreatureOrderKind::move,{64,64,0});pending.step();auto pendingState=command_session::decode(pending.world().state().systems);
  need(pendingState.native.search[0].initialized&&!pendingState.native.search[0].open.empty()&&pendingState.scheduler.continuation,"route did not preserve exhausted search");
  need(pendingState.native.route.active&&pendingState.native.route.requestBudget>0&&!pendingState.native.routeMetadata.empty(),"shared request did not retain budget/metadata");
  need(command_session::encode(command_session::decode(command_session::encode(pendingState)))==command_session::encode(pendingState),"v5 shared search roundtrip differs");
  auto pendingBefore=snapshot(pending);faults->tick=true;refuses([&]{pending.step();});faults->tick=false;need(snapshot(pending)==pendingBefore,"failed shared-search frame published state");
  refuses([&]{command_session::encode(pendingState,4);});
  // Forge a structurally valid old per-actor search payload. Its missing shared
  // request identity must be rejected rather than silently rebound on read.
  auto oldSearch=c;oldSearch.native.search[0].initialized=true;
  auto oldBytes=command_session::encode(oldSearch);
  const auto appended=command_session::encode(c).size()-v4.size();
  need(oldBytes.size()>=8+appended,"legacy fixture extent");
  oldBytes.resize(oldBytes.size()-appended);oldBytes[4]=4;
  refuses([&]{command_session::decode(oldBytes);});
  unsupported=c;unsupported.world.creatures[0].ai.motion=true;
  refuses([&]{command_session::encode(unsupported,4);});
  auto oldMotion=command_session::encode(unsupported);oldMotion.resize(oldMotion.size()-appended);oldMotion[4]=4;
  refuses([&]{command_session::decode(oldMotion);});
  game::World resumedBudgetWorld(0);resumedBudgetWorld.restore(game::decodeSnapshot(snapshot(pending)));game::CreatureRuleSession resumedBudget(std::move(resumedBudgetWorld),budgetRuntime);
  for(unsigned i=0;i<30;++i){pending.step();resumedBudget.step();need(snapshot(pending)==snapshot(resumedBudget),"budgeted search resume diverged");}
  auto largeProfile=p;for(auto &type:largeProfile.types){type.width=2;type.height=2;}
  auto largeState=initial();largeState.backend.resize(19);largeState.native.cells.resize(16*16*4);for(unsigned i=0;i<16*16;++i)largeState.native.cells[i].kind=1;
  largeState.world.creatures[1].ai.position={2,1,0};for(auto &actor:largeState.world.creatures){actor.width=actor.height=2;actor.ai.action=15;largeState.execution[actor.ai.slot].actionCallback=15;}
  command_session::prepareNativeContinuation(largeState,largeProfile);auto largeRuntime=std::make_shared<command_session::Runtime>(largeState,identity,command_session::nativeProviders(largeProfile,lower(faults)));game::CreatureRuleSession largeSession(largeRuntime,identity,17,3);intent(largeSession,game::CreatureOrderKind::attack,{},game::Handle{1,1});auto largeSelected=command_session::decode(largeSession.world().state().systems);
  need(largeSelected.world.creatures[0].ai.action==10&&largeSelected.native.animation[0].body.sequence>=24&&largeSelected.native.animation[0].body.sequence<48,"large attack selector did not bind installed ANI");
  game::World largeResumedWorld(0);largeResumedWorld.restore(game::decodeSnapshot(snapshot(largeSession)));game::CreatureRuleSession largeResumed(std::move(largeResumedWorld),largeRuntime);
  for(unsigned i=0;i<24;++i){largeSession.step();largeResumed.step();need(snapshot(largeSession)==snapshot(largeResumed),"large provider continuation diverged");}
  auto altered=p;altered.animation[0].records[0].argument+=1;auto incompatible=std::make_shared<command_session::Runtime>(c,identity,command_session::nativeProviders(altered,lower(faults)));game::World bad(0);bad.restore(game::decodeSnapshot(snapshot(s)));game::CreatureRuleSession wrong(std::move(bad),incompatible);auto before=snapshot(wrong);refuses([&]{wrong.step();});need(snapshot(wrong)==before,"changed immutable ANI profile published state");
  auto legacy=command_session::encode(initial(),3);need(!command_session::decode(legacy).native.present,"v3 migration invented native state");refuses([&]{command_session::encode(state,3);});
  auto paused=snapshot(s);s.step({false,false,false});need(snapshot(s)==paused,"paused native frame changed state");
  std::uint64_t digest=1469598103934665603ull;for(auto b:snapshot(resumed))digest=(digest^b)*1099511628211ull;
  std::cout<<"{\"native_providers_passed\":true,\"provider_ticks\":120,\"provider_digest\":"<<digest<<",\"provider_motion\":true,\"provider_rollback\":true,\"installed_ani\":"<<(asset&&*asset?"true":"false")<<",\"provider_budget_resume\":true,\"provider_large_attack\":true,\"provider_geometry\":true,\"provider_alternate\":true,\"provider_v3_migration\":true}\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
