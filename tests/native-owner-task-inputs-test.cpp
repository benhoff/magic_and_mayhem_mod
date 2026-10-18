#include "../apps/command-session/native_owner_task_inputs.hpp"
#include "../apps/command-session/native_geometry.hpp"
#include "../apps/command-session/native_regional_danger.hpp"
#include "../apps/native-encounter/encounter.hpp"
#include <iostream>
namespace cs = mnm::command_session;
namespace en = mnm::encounter;
int main(int argc, char **argv) try {
  if (argc != 2 && argc != 3) throw std::runtime_error("INSTALL [--scheduler]");
  const bool scheduled = argc == 3 && std::string(argv[2]) == "--scheduler";
  if (argc == 3 && !scheduled) throw std::runtime_error("unknown mode");
  unsigned checks=0,restores=0;
  auto check=[&](bool v,const char *why){if(!v)throw std::runtime_error(why);++checks;};
  auto p=en::decodeInputs(en::loadInputs(argv[1],{0,0,32,32},0));
  en::enableRegionalNavigation(p);
  en::enableOwnerPlanner(p,std::filesystem::path(argv[1])/"AI/Brain.dat",17);
  // Default retains the v15 event/expired-retention regression. The second
  // invocation exercises fresh v16 scheduling with the same real producers.
  if (!scheduled) p.initial.native.ownerPlanner->grouped->scheduler.reset();
  auto initial=p.initial;
  check(initial.native.ownerPlanner->grouped->replanning->eventInputs,"fresh task events disabled");
  auto legacy=initial;
  legacy.native.ownerPlanner->grouped->scheduler.reset();
  legacy.native.ownerPlanner->grouped->replanning->eventInputs=false;
  const auto v14=cs::encode(legacy,14);
  check(!cs::decode(v14).native.ownerPlanner->grouped->replanning->eventInputs&&
        cs::encode(cs::decode(v14),14)==v14,"v14 changed policy");
  bool refused=false;
  try{(void)cs::encode(initial,14);}catch(const std::invalid_argument&){refused=true;}
  check(refused,"lossy v14 write accepted");
  auto malformed=cs::encode(initial);malformed[malformed.size()-4]=2;
  refused=false;try{(void)cs::decode(malformed);}catch(const std::invalid_argument&){refused=true;}
  check(refused,"malformed event flag accepted");
  auto withCommands=[&](auto &c,auto operation){
    auto h=cs::nativeProviders(p.native,en::consequences(p))(c);
    cs::model::OwnedCreatureCommands commands(c.world,c.execution,c.bindings,h.execution,h.idle,h.follow,h.queue,h.body,h.supplied,h.release,h.peer);
    commands.installBodies(h.commandBodies,h.originalBehaviorBindings);
    commands.installActions(h.actionTransition,h.actionTick);
    operation(commands);
  };
  auto exercise=[&](auto &c,auto operation){
    auto resume=cs::decode(cs::encode(c));
    operation(c);operation(resume);
    check(cs::encode(c)==cs::encode(resume),"event continuation differs");++restores;
  };
  auto &a=initial.world.creatures[0];
  initial.native.regionalDanger->inputs.observationPoints={a.ai.position};
  cs::updateNativeOwnerTaskInputs(initial);
  check(!initial.native.ownerPlanner->grouped->replanning->admission.allOffSpecialCells,"occupied special cell admitted");
  cs::NativeCreatureGeometry geo(initial,p.native);geo.reserve(a,a.ai.position,false);
  cs::updateNativeOwnerTaskInputs(initial);
  check(initial.native.ownerPlanner->grouped->replanning->admission.allOffSpecialCells,"released special cell stayed blocked");
  a.region=-1;cs::updateNativeOwnerTaskInputs(initial);
  check(!initial.native.ownerPlanner->grouped->replanning->admission.allAssigned,"missing region admitted");
  initial.native.ownerPlanner->percent=0;cs::updateNativeOwnerTaskInputs(initial);
  check(!initial.native.ownerPlanner->grouped->replanning->admission.enabled,"zero task percentage admitted");
  initial.native.regionalDanger->inputs.enabled[0]=false;
  cs::updateNativeOwnerTaskInputs(initial);
  check(!initial.native.ownerPlanner->grouped->replanning->admission.ownerAvailable,"disabled owner available");
  // A bounded authored model produces the same positive proposal every sweep.
  // Original grouped features/inference and travel-plus600/exponential duration draws run.
  auto c=p.initial;
  for(unsigned outcome=0;outcome<3;++outcome){
    auto &layer=c.native.ownerPlanner->model.layers[outcome];
    layer.observations=1;layer.outside=0;layer.nodes.assign(72,{});
    for(auto &node:layer.nodes)node[1]=10;
    layer.matrix.assign(72*72,0);layer.matrix[35*72+35]=1;
    layer.nodes[35][2]=outcome==0?0:outcome==2?4:10;
    layer.nodes[35][3]=outcome==2?0:outcome==0?4:10;
  }
  c.native.regions.assign(16,{});
  for(unsigned i=0;i<16;++i){
    auto &r=c.native.regions[i];r.position={int(i*2),2,2};r.parameter=1;
    if(i+1<16){auto &e=r.connections[0];e.destination=int(i+1);e.cost=4;e.clearance.fill(8);}
  }
  auto &d=*c.native.regionalDanger;
  d.state={};d.state.pendingDanger.resize(16);d.state.pendingDeduction.resize(16);
  d.state.pendingBase.resize(16);d.state.actorSupport.resize(d.inputs.actors.size());
  for(auto &actor:d.inputs.actors)actor.region=0;
  c.world.creatures[0].region=0;c.world.creatures[1].region=1;
  c.world.creatures[0].ai.engaged=c.world.creatures[0].ai.incoming=c.world.creatures[0].ai.readiness=0;
  c.world.creatures[0].ai.dirty932=1;
  cs::model::OwnerMemory memory;
  memory.memory.key=0;memory.memory.actor=d.inputs.actors[1];
  memory.memory.actor.region=1;memory.memory.actor.health=p.maximum;
  memory.memory.actor.position=c.native.regions[1].position;
  memory.memory.observedTick=1;memory.markedTick=1;
  memory.planning=cs::nativeOwnerPlannerActor(c,p.native,1);
  memory.planning->position=memory.memory.actor.position;
  memory.planning->combatValue.fill(0);memory.planning->combatValue[0]=1;
  c.native.ownerKnowledge->records[0]={memory};c.native.ownerKnowledge->nextKey[0]=1;
  d.inputs.memories[0]={memory.memory};
  auto slice=[&](auto &state){++state.native.tick;withCommands(state,[&](auto &commands){cs::updateNativeOwnerPlanner(state,p.native,commands);});};
  for(unsigned blocked=0;blocked<4;++blocked){
    auto inadmissible=c;
    if(blocked==0)inadmissible.native.ownerPlanner->percent=0;
    if(blocked==1)inadmissible.world.creatures[0].region=-1;
    if(blocked==2)inadmissible.native.regionalDanger->inputs.observationPoints={inadmissible.world.creatures[0].ai.position};
    if(blocked==3)inadmissible.native.regionalDanger->inputs.enabled[0]=false;
    for(unsigned i=0;i<8;++i)exercise(inadmissible,slice);
    check(!inadmissible.native.ownerPlanner->active,"event-ineligible source installed task");
  }
  for(unsigned i=0;i<80&&!c.native.ownerPlanner->active;++i)exercise(c,slice);
  auto &planner=*c.native.ownerPlanner;
  check(planner.active&&planner.grouped->replanning->previous->value>.99f,"positive task not installed");
  const auto task=*planner.grouped->replanning->previous;
  check(planner.features.at(68)>0&&task.expires-task.started>=600+planner.features.at(68),"installed lifetime omitted produced travel or600");
  const auto serial=planner.activeSerial;
  auto undamaged=c;
  // Actual ordinary health feedback calculates416d80 credit from installed
  // physical profile fields, then walks the native single-root task mapping.
  auto wound=[&](auto &state){withCommands(state,[&](auto &commands){
    auto effects=en::consequences(p)(state);effects.bindCommands(commands);
    effects.maintenance.health(state.world.creatures[0],-18,1,true);
  });};
  exercise(c,wound);
  const auto losses=planner.grouped->replanning->previous->attempts;
  check(losses>0&&planner.grouped->replanning->previous->successes==0,"damage did not cost its task");
  unsigned expired=0;
  for(unsigned i=0;i<6000&&planner.activeSerial==serial;++i){
    exercise(c,slice);slice(undamaged);
    expired+=int(c.native.tick)>=task.expires;
  }
  check(expired&&planner.activeSerial!=serial,(std::string("damage never triggered expired task replacement; start=")+std::to_string(task.started)+" expiry="+std::to_string(task.expires)+" now="+std::to_string(c.native.tick)+" travel="+std::to_string(planner.features.at(68))+" cost="+std::to_string(losses)+" value="+std::to_string(task.value)).c_str());
  if (scheduled) {
    check(undamaged.native.ownerPlanner->activeSerial != serial,
          "scheduled expiry retained control task");
    for(unsigned i=0;i<80&&(!planner.active||!undamaged.native.ownerPlanner->active);++i){
      exercise(c,slice);slice(undamaged);
    }
    check(planner.active&&undamaged.native.ownerPlanner->active,
          "scheduler did not regenerate after retirement");
    check(!planner.grouped->replanning->previous->completed &&
          !planner.grouped->scheduler->executionComplete,
          "new scheduled task inherited completion flags");
  } else {
    check(undamaged.native.ownerPlanner->activeSerial==serial,"zero-progress control replaced identical proposal");
  }
  check(!undamaged.native.ownerPlanner->grouped->replanning->previous->completed,"expiry incorrectly set replacement flag15");
  auto retainedV14=undamaged;
  retainedV14.native.ownerPlanner->grouped->scheduler.reset();
  retainedV14.native.ownerPlanner->grouped->replanning->eventInputs=false;
  const auto oldActive=cs::encode(retainedV14,14);
  check(cs::encode(cs::decode(oldActive),14)==oldActive,"v14 expired active resume differs");
  check(planner.grouped->replanning->previous->attempts==0,"new task inherited old costs");
  // Rewards come from a retained attacker identity, not owner attribution.
  c.native.random=cs::model::seedRandom(17);c.native.randomSeeded=true;
  c.world.creatures[0].ai.target=1;
  const auto beforeReward=planner.grouped->replanning->previous->successes;
  auto hit=[&](auto &state){withCommands(state,[&](auto &commands){
    auto effects=en::consequences(p)(state);effects.bindCommands(commands);
    effects.callbacks.hit(state.world.creatures[0]);
  });};
  exercise(c,hit);
  check(planner.grouped->replanning->previous->successes>beforeReward,"melee attacker task not rewarded");
  auto heal=[&](auto &state){withCommands(state,[&](auto &commands){
    auto effects=en::consequences(p)(state);effects.bindCommands(commands);
    effects.maintenance.health(state.world.creatures[0],9,-1,true);
  });};
  const auto beforeHeal=planner.grouped->replanning->previous->successes;
  exercise(c,heal);
  check(planner.grouped->replanning->previous->successes>beforeHeal,"healing did not reward own task");
  const auto beforeSilentHeal=planner.grouped->replanning->previous->successes;
  auto silentHeal=[&](auto &state){withCommands(state,[&](auto &commands){
    auto effects=en::consequences(p)(state);effects.bindCommands(commands);
    effects.maintenance.health(state.world.creatures[0],1,-1,false);
  });};
  exercise(c,silentHeal);
  check(planner.grouped->replanning->previous->successes==beforeSilentHeal,"silent healing credited task");
  // The existing original completion callback runs once for pending FC, with
  // its lower scoring service. Ordinary movement without FC produces no cost.
  auto completed=c;
  auto &route=completed.actions.segments[0];route.count=1;route.cursor=0;
  route.route[0].position=completed.world.creatures[0].ai.position;route.scorePendingFc=true;
  completed.world.creatures[0].ai.action=2;
  completed.world.creatures[0].ai.motion=1;
  completed.world.creatures[0].ai.pending=8;
  completed.execution[0].moveVariant=1;
  completed.execution[0].actionCallback=2;
  completed.actions.motion[0].samples.fill(1);
  completed.actions.motion[0].sampleCursor=0;
  completed.actions.motion[0].accumulator=0;
  auto finish=[&](auto &state){withCommands(state,[&](auto &commands){
    auto h=cs::nativeProviders(p.native,en::consequences(p))(state);
    // One final substep completes action2 through the native provider.
    state.actions.motion[0].duration=1;state.actions.motion[0].progress=191;
    h.actionTick(state.world.creatures[0],commands);
  });};
  // Direct credit checks also guard the no-task and unrelated-actor cases.
  auto empty=p.initial;cs::creditNativeOwnerMovement(empty,0,9);
  check(!empty.native.ownerPlanner->grouped->replanning->previous,"credit created absent task");
  const auto costBefore=planner.grouped->replanning->previous->attempts;
  cs::creditNativeOwnerMovement(c,1,9);
  check(planner.grouped->replanning->previous->attempts==costBefore,"unrelated actor charged");
  const auto beforeCompletion=completed.native.ownerPlanner->grouped->replanning->previous->attempts;
  exercise(completed,finish);
  check(!completed.actions.segments[0].scorePendingFc&&
        completed.native.ownerPlanner->grouped->replanning->previous->attempts==beforeCompletion+1000000,
        (std::string("flagged movement completion did not charge its task: ")+std::to_string(completed.native.ownerPlanner->grouped->replanning->previous->attempts)+" fc="+std::to_string(completed.actions.segments[0].scorePendingFc)+" cursor="+std::to_string(completed.actions.segments[0].cursor)+" action="+std::to_string(completed.world.creatures[0].ai.action)).c_str());
  exercise(completed,finish);
  check(completed.native.ownerPlanner->grouped->replanning->previous->attempts==beforeCompletion+1000000,
        "flagged movement completion charged twice");
  // Exercise original regional publication rather than inventing ratio fields.
  auto pressure=p.initial;
  for(auto &e:pressure.execution)e.combatValue[5]=10000;
  for(auto &actor:pressure.world.creatures)actor.region=0;
  bool published=false;
  for(unsigned i=0;i<128;++i){
    ++pressure.native.tick;cs::updateNativeRegionalDanger(pressure,p.native);
    cs::updateNativeOwnerTaskInputs(pressure);
    const auto &owned=*pressure.native.regionalDanger;
    const auto &policy=pressure.native.ownerPlanner->grouped->replanning->policy;
    check(policy.ownerNumerator==owned.state.dangerTotals[0]&&policy.ownerDenominator==owned.state.deductionTotals[0],"owner ratio bypassed publication");
    published|=policy.ownerNumerator>0||policy.ownerDenominator>0;
  }
  check(published,"regional ratio never published nonzero input");
  std::cout<<"{\"success\":true,\"checks\":"<<checks<<",\"restores\":"<<restores
           <<",\"damage_cost\":"<<losses<<",\"expired_slices\":"<<expired
           <<",\"replaced\":1,\"control_retained\":"<<unsigned(!scheduled)
           <<",\"scheduler\":"<<unsigned(scheduled)<<",\"published_ratio\":true}\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
