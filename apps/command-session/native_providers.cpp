#include "native_geometry.hpp"
#include "../../reconstruction/creature-ai/attack_selection.hpp"
#include "../../reconstruction/pathfinding/route_world.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace mnm::command_session {
namespace {
namespace r=reconstruction;
void need(bool ok,const char *why){if(!ok)throw std::invalid_argument(why);}
std::array<unsigned,2> stamp(const NativeCreatureProfile &p){
  std::uint64_t h=1469598103934665603ull;
  auto word=[&](unsigned v){for(unsigned i=0;i<4;++i)h=(h^((v>>(8*i))&255))*1099511628211ull;};
  word(p.cameraTurn);word(p.defaultScalar);word(p.boundary);word(p.recovery);
  word(p.animation.size());word(p.terrain.size());word(p.terrainPropertyA0.size());for(bool v:p.terrainPropertyA0)word(v);
  for(const auto &a:p.animation){word(a.version);word(a.opaqueHeader);for(auto v:a.spriteName)word(v);word(a.starts.size());word(a.records.size());for(auto v:a.starts)word(v);for(const auto &v:a.records){word(v.opcode);word(v.argument);for(auto m:v.metadata)word(m);}}
  for(const auto &t:p.types){word(t.bodyAsset);word(t.regularOverlayAsset);word(t.meleeOverlayAsset);word(t.width);word(t.height);word(t.movementKind);word(t.scalar.type_3c);word(t.scalar.type_10);word(t.periodicResistance);word(t.routeCostFactor);word(t.routeFlagPenalty);for(auto v:t.scalar.type_d8)word(v);unsigned bits;std::memcpy(&bits,&t.scalar.slope_global,4);word(bits);}
  for(const auto &v:p.terrain){for(auto b:v.unknown_00)word(std::to_integer<unsigned>(b));word(v.classification_94);for(auto b:v.unknown_98)word(std::to_integer<unsigned>(b));word(v.flags_b0);for(auto b:v.unknown_b1)word(std::to_integer<unsigned>(b));}
  const auto &m=p.maintenance;
  for(auto v:{m.tick,m.upkeepInterval,m.upkeepCost,m.statusDamage,m.statusInterval,m.poisonInterval,m.powerInterval,m.powerMaximum,m.restoreMode,m.drainQuantity,unsigned(m.localOwner),unsigned(m.drainDivisor),unsigned(m.effectDamage),unsigned(m.diagnostic)})word(v);
  word(p.environment.waterline);word(p.environment.damage);
  for(auto v:p.valuation.configured)word(v);
  for(auto v:p.valuation.response)word(v);
  for(auto v:p.valuation.statusBonus)word(v);
  for(auto v:{p.valuation.hthBonus,p.valuation.responseBonus,p.valuation.factor5a9,p.valuation.factor5ad,p.valuation.factor5c5,p.valuation.factor589})word(v);
  word(p.periodic.terrainHasRegions);word(p.periodic.relocationStrength);word(p.periodic.regionCapacity);word(p.periodic.regions.size());for(auto v:p.periodic.regions){word(v.x);word(v.y);word(v.z);}
  return {unsigned(h),unsigned(h>>32)};
}
std::vector<assets::AnimationRecord> records(const NativeCreatureProfile &p,unsigned asset,unsigned sequence){
  const auto &a=p.animation.at(asset);need(sequence+1<a.starts.size(),"native ANI sequence unavailable");
  const auto first=a.starts[sequence],last=a.starts[sequence+1];need(first<last&&last<=a.records.size(),"native ANI sequence extent");
  return {a.records.begin()+first,a.records.begin()+last};
}
class NativeContext {
  Continuation &c_;
  std::shared_ptr<const NativeCreatureProfile> p_;
  NativeConsequences fx_;
  NativeCreatureGeometry geometry_;
  void bind(model::OwnedCreatureCommands &commands){if(fx_.bindCommands)fx_.bindCommands(commands);}
  NativeAnimationChannel &channel(model::AutonomousCreature &a,bool overlay){auto &v=c_.native.animation.at(a.ai.slot);return overlay?v.overlay:v.body;}
  void start(NativeAnimationChannel &v,unsigned asset,unsigned sequence,unsigned reverse){
    need(reverse<=1,"native ANI reverse mode");r::NoCdAnimationController player(records(*p_,asset,sequence));
    r::AnimationControllerState prior;prior.reverseCursor=v.state.reverseCursor;prior.reverseLength=v.state.reverseLength;player.restore(prior);
    player.start(reverse!=0);v={asset,sequence,true,player.state()};
  }
  int poll(NativeAnimationChannel &v,bool requestBreak=false){
    if(!v.bound)return 0;
    r::NoCdAnimationController player(records(*p_,v.asset,v.sequence));player.restore(v.state);if(requestBreak)player.requestBreak();
    const auto event=player.tick();v.state=player.state();return event;
  }
  int poll(model::AutonomousCreature &a,bool overlay){auto &v=channel(a,overlay);auto &state=c_.actions.callbacks.at(a.ai.slot);const auto event=poll(v,!overlay&&state.animationBreak);if(!overlay){state.animationBreak=v.state.forward.breakFlag!=0;state.sequence=v.sequence;}return event;}
  void play(model::AutonomousCreature &a,unsigned sequence,unsigned reverse){start(channel(a,false),p_->types.at(a.ai.type).bodyAsset,sequence,reverse);c_.actions.callbacks.at(a.ai.slot).sequence=sequence;}
  void rewind(model::AutonomousCreature &a){auto &v=channel(a,false);if(!v.bound)return;r::NoCdAnimationController player(records(*p_,v.asset,v.sequence));player.restore(v.state);player.rewind();v.state=player.state();}
  unsigned random(){return model::nextRandom(c_.native.random);}
  model::AiGeometry aiGeometry(){auto g=fx_.autonomy.geometry;
    g.cell=[this](model::Cell p){p=normalized(p);auto d=c_.world.dimensions;const auto &cell=c_.native.cells.at((std::size_t(p.z)*d.y+p.y)*d.x+p.x);return model::AiTerrainCell{cell.occupant==0xffff?std::uint16_t(0xffff):std::uint16_t(cell.occupant-1),cell.flags,p_->terrainPropertyA0.at(cell.kind)};};
    g.legal=[this](const auto &a,model::Cell from,model::Cell to,unsigned){unsigned category=5;return geometry_.move(a,normalized(from),normalized(to),category);};
    g.reach=[this](const auto &a,const auto &b,model::Cell p){return geometry_.reach(a,b,normalized(p));};return g;
  }
  r::CreatureScalarState scalar(const model::AutonomousCreature &a){auto s=p_->types.at(a.ai.type).scalar;s.default_scalar=p_->defaultScalar;s.object_778=int(c_.maintenance.at(a.ai.slot).status778);s.object_77c=int(c_.maintenance.at(a.ai.slot).status77c);return s;}
  model::Cell normalized(model::Cell point){auto d=c_.world.dimensions;point.x=r::normalize_coordinate(point.x,d.x);point.y=r::normalize_coordinate(point.y,d.y);need(point.z>=0&&point.z<d.z,"native route layer unavailable");return point;}
  void facing(model::AutonomousCreature &a,model::Cell target){
    auto d=c_.world.dimensions;target=normalized(target);
    const int x=r::wrapped_difference(target.x,a.ai.position.x,d.x),y=r::wrapped_difference(target.y,a.ai.position.y,d.y);
    if(!x&&!y)return;
    const auto sign=[](int n){return n<0?-1:n>0?1:0;};int dx=sign(x),dy=sign(y);
    if(dx&&dy){const auto ax=std::abs(x),ay=std::abs(y);if(2*std::max(ax,ay)>3*std::min(ax,ay)){if(ax>ay)dy=0;else dx=0;}}
    constexpr unsigned table[]={7,0,1,6,0,2,5,4,3};auto &m=c_.actions.motion.at(a.ai.slot);m.direction=table[dx+3*dy+4];
    auto &e=c_.execution.at(a.ai.slot);e.animationFacing=(m.direction-2*p_->cameraTurn-(e.block722?4:0))&7;
  }
  void complete(model::AutonomousCreature &a,model::OwnedCreatureCommands &commands){
    auto slot=a.ai.slot;auto s=fx_.completion;
    s.stopOverlay=[this](auto &actor){channel(actor,true)={};};s.releaseOverlay=fx_.callbacks.releaseEffect;
    s.fallDamage=fx_.flight.fallDamage;s.defended=fx_.flight.defended;
    s.snap=[this](auto &actor,model::Cell p,unsigned){geometry_.snap(actor,p);};
    s.reset=[&commands](auto &actor){commands.reset(actor);};

    s.legal=[this](const auto &actor,model::Cell p){return geometry_.legal(actor,normalized(p));};
    model::completeMotionSegment(a,c_.execution.at(slot),c_.actions.motion.at(slot),c_.movement.at(slot),c_.actions.segments.at(slot),{},s);
  }
  void begin(model::AutonomousCreature &a,model::OwnedCreatureCommands &commands){
    const auto slot=a.ai.slot;auto &initial=c_.native.initialization.at(slot);auto &motion=c_.actions.motion.at(slot);
    initial.slow778=int(c_.maintenance.at(slot).status778);initial.fast77c=int(c_.maintenance.at(slot).status77c);
    auto s=fx_.segment;
    s.base=[this](const auto &actor,unsigned mode,model::Cell delta){return r::base_movement_scalar(int(mode),{delta.x,delta.y,delta.z},scalar(actor));};
    s.scalar=[this](const auto &actor,unsigned argument,unsigned mode,model::Cell delta,int,unsigned &rate,int &length){auto value=r::creature_movement_scalar(int(argument),int(mode),{delta.x,delta.y,delta.z},rate,scalar(actor));rate=value.scalar;length=12;};
    s.accept=[this](auto &actor,model::Cell from,model::Cell to,unsigned &mode){return geometry_.move(actor,normalized(from),normalized(to),mode);};
    s.release=[this](auto &actor,model::Cell p){geometry_.reserve(actor,normalized(p),false);};s.reserve=[this](auto &actor,model::Cell p){geometry_.reserve(actor,normalized(p),true);};
    s.turn=[](unsigned old,unsigned now){auto d=int(now)-int(old);if(d>4)d-=8;if(d<-4)d+=8;return d;};
    s.scalarArgument=[](model::Cell,model::Cell){return 4u;};
    s.distance=[](model::Cell p){return unsigned(r::distance_metric(p.x*2,p.y*2,p.z));};
    s.tileKind=[this](model::Cell p){const auto d=c_.world.dimensions;return c_.native.cells.at((std::size_t(p.z)*d.y+p.y)*d.x+p.x).kind;};
    s.tileHeight=[this](unsigned kind){return p_->terrain.at(kind).classification_94;};
    s.animation=[this](auto &actor,unsigned sequence,unsigned reverse){play(actor,sequence,reverse);};
    s.sample=[this,&initial](const auto &actor,unsigned bank,unsigned index){if(bank==0x71e)return initial.inlineSample;need(bank>=0xd8&&bank<0xd8+48*4,"native motion profile bank unavailable");return int(scalar(actor).type_d8.at((bank-0xd8)/4+index));};
    s.attack=[this](auto &actor,const auto &target){facing(actor,target.ai.position);play(actor,24+c_.execution.at(actor.ai.slot).animationFacing,0);};
    s.action=[&commands](auto &actor,unsigned kind){commands.action(actor,kind);};
    model::initializeMotionSegment(a,c_.execution.at(slot),motion,initial,c_.actions.segments.at(slot),c_.world,{p_->cameraTurn,p_->defaultScalar},s);
    if(initial.sampleBank==0x71e)motion.samples.fill(initial.inlineSample);
    else {need(initial.sampleBank>=0xd8&&initial.sampleBank<0xd8+48*4,"native sample bank unavailable");const auto base=(initial.sampleBank-0xd8)/4;auto values=scalar(a).type_d8;motion.samples.fill(0);for(unsigned i=0;i<48-base;++i)motion.samples[i]=int(values[base+i]);}
  }
  unsigned request(model::AutonomousCreature &a,model::Cell target){
    target=normalized(target);auto &saved=c_.native.search.at(a.ai.slot);r::SearchState search;r::RouteContextPrefix context{};
    if(saved.initialized&&saved.destination==target){for(auto v:saved.open)search.open.emplace(v.priority,v.node);for(auto v:saved.records)search.records.emplace(v.node,v.record);search.best_node=saved.bestNode;search.best_heuristic=saved.bestHeuristic;search.initialized=true;std::memcpy(&context,saved.context.data(),sizeof(context));}
    else {saved={};saved.destination=target;context.unknown_route_flag=1;}
    auto snapshot=geometry_.snapshot(a,target);auto neighbors=r::snapshot_neighbors(snapshot);
    r::NeighborDescriptor descriptor;descriptor.object=a.ai.slot+1;descriptor.start={a.ai.position.x,a.ai.position.y,a.ai.position.z};descriptor.target={target.x,target.y,target.z};
    r::SearchWorld world;world.dimensions=neighbors.dimensions;world.node_at=neighbors.node_at;world.coordinates=neighbors.coordinates;
    world.expand=[neighbors,descriptor](auto node,const auto &payload,const auto &,unsigned){return r::generate_neighbors(node,payload,descriptor,neighbors);};
    const auto result=r::route_search(context,snapshot->object,0,{target.x,target.y,target.z},c_.scheduler.budget,search,world);
    saved.open.clear();saved.records.clear();for(auto v:search.open)saved.open.push_back({v.first,v.second});for(auto v:search.records)saved.records.push_back({v.first,v.second});saved.bestNode=search.best_node;saved.bestHeuristic=search.best_heuristic;saved.initialized=search.initialized;std::memcpy(saved.context.data(),&context,sizeof(context));
    if(result.stop==r::SearchStop::budget_exhausted)return 2;
    auto &route=c_.actions.segments.at(a.ai.slot);route.cursor=0;route.count=0;
    need(context.route.waypoint_count<=16,"native route waypoint span");
    for(unsigned i=0;i<context.route.waypoint_count;++i){r::Waypoint point{};std::memcpy(&point,context.route.unknown_14.data()+i*sizeof(point),sizeof(point));route.route[route.count++]={{point.x,point.y,point.z},{point.direction,point.vertical_delta,int(point.category_is_1_to_3),point.category}};}
    a.ai.marker=route.count;
    saved.initialized=false;return route.count?1:0;
  }
  model::AutonomousServices autonomy(model::OwnedCreatureCommands &commands){
    auto s=fx_.autonomy;s.geometry=aiGeometry();s.random=[this]{return random();};
    s.action=[&commands](auto &a,unsigned n){commands.action(a,n);};s.behavior=[&commands](auto &a,unsigned n){commands.transition(a,n);};s.reset=[&commands](auto &a){commands.reset(a);};
    s.beginMotion=[this,&commands](auto &a){begin(a,commands);};s.faceTarget=[this](auto &a,const auto &b){facing(a,b.ai.position);};s.move=[&commands](auto &a,model::Cell p){commands.move(a,p);};
    s.routeMove=[this](auto &a,model::Cell p){return request(a,p)==1;};return s;
  }
  model::BehaviorExecutionServices behavior(model::OwnedCreatureCommands &commands){
    auto s=fx_.behavior;
    s.immediate=[this](auto &a){return model::immediateOpponent(a,c_.world,aiGeometry(),[this]{return random();});};s.interaction=[this](auto &a){return model::chooseInteraction(a,c_.world,fx_.autonomy.objects,aiGeometry());};
    s.action=[&commands](auto &a,unsigned n){commands.action(a,n);};s.behavior=[&commands](auto &a,unsigned n){commands.transition(a,n);};s.reset=[&commands](auto &a){commands.reset(a);};
    s.beginMotion=[this,&commands](auto &a){begin(a,commands);};s.reach=[this](const auto &a,const auto &b,model::Cell p){return geometry_.reach(a,b,p);};
    s.face=[this](auto &a,const auto &b){facing(a,b.ai.position);};s.coordinateFace=[this](auto &a,model::Cell p){facing(a,p);};

    return s;
  }
public:
  NativeContext(Continuation &c,std::shared_ptr<const NativeCreatureProfile> p,NativeConsequences fx):c_(c),p_(std::move(p)),fx_(std::move(fx)),geometry_(c_,*p_){
    need(c_.native.present&&c_.native.profileStamp==stamp(*p_),"native immutable profile differs");
    if(!c_.native.randomSeeded){c_.native.random=model::seedRandom(c_.seed);c_.native.randomSeeded=true;}
  }
  void release(unsigned value){need(bool(fx_.callbacks.releaseEffect),"native release service unavailable");fx_.callbacks.releaseEffect(value);}
  model::ExecutionServices execution(){auto s=fx_.execution;s.orient=[this](auto &a,model::Cell p){facing(a,{int(unsigned(p.x)>>5),int(unsigned(p.y)>>5),a.ai.position.z});};s.terrainHeight=[this](model::Cell p){return geometry_.height(normalized(p));};return s;}
  model::BehaviorExecutionServices idle(){auto s=fx_.behavior;s.immediate=[this](auto &a){return model::immediateOpponent(a,c_.world,aiGeometry(),[this]{return random();});};s.interaction=[this](auto &a){return model::chooseInteraction(a,c_.world,fx_.autonomy.objects,aiGeometry());};return s;}
  model::FollowServices follow(){return {[this](const auto &a,model::Cell &p){p=normalized(p);return geometry_.legal(a,p);},{}};}
  model::CommandActionProviders selection(){model::CommandActionProviders p;
    p.selection=[this](unsigned){model::ActionExecutionServices s;s.random=[this]{return random();};s.animation=[this](auto &a,unsigned seq,unsigned rev){play(a,seq,rev);};
      s.overlay=[this](auto &a,unsigned asset){channel(a,true).asset=asset;};s.overlayAnimation=[this](auto &a,unsigned seq,unsigned rev){auto &v=channel(a,true);start(v,v.asset,seq,rev);};
      s.attackAnimation=[this](auto &a,const auto *target){auto &e=c_.execution.at(a.ai.slot);auto &motion=c_.actions.motion.at(a.ai.slot);
        model::AttackSelectionState state{a.ai.position,a.width,motion.direction,e.animationFacing,a.animationKind,e.animationOffset,e.block722!=0,false,e.block855!=0};
        std::optional<model::AttackSelectionTarget> other;if(target)other=model::AttackSelectionTarget{target->ai.position,target->width};
        const auto result=model::selectAttackAnimation(state,c_.world.dimensions,p_->cameraTurn,other);
        motion.direction=result.state.direction;e.animationFacing=result.state.facing;e.animationOffset=result.state.offset;
        for(auto effect:result.events){need(effect.kind==model::AttackSelectionEffect::play,"native special attack attachment provider unavailable");play(a,effect.argument,0);}};
      s.flightAnimation=[](auto &a){return model::corpseFlightAnimation(a.ai.health,a.ai.corpseA,a.ai.corpseB);};
      s.repairOverlay=[](auto &a){need(a.animationKind!=1,"native configured repair-overlay profile unavailable");};return s;};
    p.animation=[this](unsigned){model::AnimationActionServices s;s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.animation=[this](auto &a,unsigned seq,unsigned rev){play(a,seq,rev);};s.cast=fx_.callbacks.interactionHit;s.restoreForm=fx_.behavior.restoreForm;return s;};return p;
  }
  void action(model::AutonomousCreature &a,unsigned kind,model::OwnedCreatureCommands &commands){bind(commands);auto p=selection();const auto &t=p_->types.at(a.ai.type);model::OwnedCommandActions actions(c_.world,c_.execution,commands,{{t.regularOverlayAsset,64,t.meleeOverlayAsset,32},c_.native.tick,p_->recovery,p_->maintenance.restoreMode},p);actions.transition(a,kind);}
  void tickAction(model::AutonomousCreature &a,model::OwnedCreatureCommands &commands){bind(commands);auto p=selection();p.remaining=[this](auto &actor,unsigned handler,auto &context){auto body=bodyProviders(context);model::OwnedCommandActionBodies bodies(c_.world,c_.execution,c_.maintenance,c_.movement,c_.actions,context,{{0,false},{c_.world.dimensions,unsigned(p_->environment.waterline),false}},body);bodies.tick(actor,handler);};const auto &t=p_->types.at(a.ai.type);model::OwnedCommandActions actions(c_.world,c_.execution,commands,{{t.regularOverlayAsset,64,t.meleeOverlayAsset,32},c_.native.tick,p_->recovery,p_->maintenance.restoreMode},p);actions.tick(a);}
  model::CommandActionBodyProviders bodyProviders(model::OwnedCreatureCommands &commands){model::CommandActionBodyProviders p;
    p.callbacks=[this](unsigned){auto s=fx_.callbacks;s.immediate=[this](auto &a){return model::immediateOpponent(a,c_.world,aiGeometry(),[this]{return random();});};s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.random=[this]{return random();};s.rewind=[this](auto &a){rewind(a);};return s;};
    p.motion=[this,&commands](unsigned){auto s=fx_.motion;s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.rewind=[this](auto &a){rewind(a);};s.completeSegment=[this,&commands](auto &a){complete(a,commands);};s.updateOrders=fx_.callbacks.updateOrders;s.legal=[this](const auto &a,model::Cell p){return geometry_.legal(a,normalized(p));};s.stun=fx_.callbacks.stun;s.effect=fx_.callbacks.effect;s.releaseEffect=fx_.callbacks.releaseEffect;s.cameraTurn=p_->cameraTurn;return s;};
    p.flight=[this,&commands](unsigned){auto s=fx_.flight;s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.rewind=[this](auto &a){rewind(a);};s.completeSegment=[this,&commands](auto &a){complete(a,commands);};s.animation=[this](auto &a,unsigned seq,unsigned rev){play(a,seq,rev);};s.occupied=[this](const auto &a,model::Cell p){return !geometry_.cell(a,normalized(p));};s.passable=[this](const auto &a,model::Cell p){return geometry_.legal(a,normalized(p));};s.ground=[this](const auto &a,model::Cell p){return geometry_.ground(a,normalized(p));};s.random=[this]{return random();};s.release=[this](auto &a,model::Cell p){geometry_.reserve(a,normalized(p),false);};s.reserve=[this](auto &a,model::Cell p){geometry_.reserve(a,normalized(p),true);};s.snap=[this](auto &a,model::Cell p,unsigned){geometry_.snap(a,normalized(p));};s.relocateDead=[this](auto &a,model::Cell p){geometry_.snap(a,normalized(p));};return s;};
    p.objects=[this](unsigned slot){model::ObjectActionServices s;s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.cue=[this,slot](unsigned cue,model::Cell p){auto &a=c_.world.creatures.at(slot);fx_.callbacks.effect(a,cue,p);};return s;};return p;
  }
  void body(model::AutonomousCreature &a,unsigned kind,model::OwnedCreatureCommands &commands){bind(commands);
    auto s=behavior(commands);model::CommandBehaviorProviders p;
    p.behavior=[s](unsigned){return s;};
    p.ranged=[this,s,&commands](unsigned){auto out=fx_.ranged;out.orient=[this](auto &actor,model::Cell point){facing(actor,{int(unsigned(point.x)>>5),int(unsigned(point.y)>>5),actor.ai.position.z});};out.action=[&commands](auto &actor,unsigned n){commands.action(actor,n);};out.maintainTarget=[this,s](auto &actor){model::maintainTargetBehavior(actor,c_.execution.at(actor.ai.slot),c_.world,p_->maintenance.restoreMode,s);};return out;};
    p.movement=[this,&commands](unsigned){auto out=fx_.movement;out.beginMotion=[this,&commands](auto &actor){begin(actor,commands);};out.move=[&commands](auto &actor,model::Cell point){commands.move(actor,point);};out.relocate=[this](auto &actor,model::Cell point){geometry_.snap(actor,normalized(point));};out.releaseOccupancy=[this](auto &actor,model::Cell point){geometry_.reserve(actor,normalized(point),false);};out.adjust=[this](const auto &actor,model::Cell &point){point=normalized(point);return geometry_.legal(actor,point);};return out;};
    if(kind>=2&&kind<=4){model::OwnedCommandBehaviors bodies(c_.world,c_.execution,c_.maintenance,c_.movement,commands,{c_.native.tick,p_->maintenance.restoreMode},p);bodies.dispatch(a,kind);}
    else if(kind==5){auto d=fx_.death;d.action=[&commands](auto &actor,unsigned n){commands.action(actor,n);};model::deathBehavior(a,c_.maintenance.at(a.ai.slot),c_.movement.at(a.ai.slot),{},d);}
    else if(kind==9)model::immediateAttackBehavior(a,c_.world,s);
    else if(kind==10)model::standingRangedBehavior(a,c_.execution.at(a.ai.slot),c_.world,c_.native.tick,s);
    else if(kind==13)model::displacementBehavior(a,c_.execution.at(a.ai.slot),s);
    else if(kind==11){auto services=autonomy(commands);services.orient=[this,&a](model::Cell point){facing(a,{int(unsigned(point.x)>>5),int(unsigned(point.y)>>5),a.ai.position.z});};model::autonomousBehavior(a,c_.world,services);}
    else model::specialBehavior(a,c_.execution.at(a.ai.slot),kind,s);
  }
  std::vector<game::CreatureRuleEvent> frame(model::OwnedCreatureCommands &commands){
    bind(commands);
    ++c_.native.sequence;if(c_.native.alternate)return {};++c_.native.tick;
    for(auto &a:c_.world.creatures)if(a.ai.active){if(!channel(a,false).bound&&c_.execution.at(a.ai.slot).actionCallback)commands.action(a,a.ai.action);
      auto s=fx_.maintenance;s.random=[this]{return random();};s.action=[&commands](auto &actor){commands.tickAction(actor);};s.poll=[this](auto &actor,model::MaintenanceAnimation which){return poll(c_.native.animation.at(actor.ai.slot).auxiliary.at(unsigned(which)));};
      auto env=p_->maintenance;env.tick=c_.native.tick;model::maintainCreature(a,c_.execution.at(a.ai.slot),c_.maintenance.at(a.ai.slot),c_.world,env,s);
    }
    for(auto &v:c_.native.worldAnimation)poll(v);
    model::SchedulerServices schedule;
    schedule.route=[this,&commands](unsigned slot,auto &scheduler){auto &a=c_.world.creatures.at(slot);model::RouteExecutionServices s;s.request=[this](auto &actor,model::Cell point,unsigned,unsigned){return request(actor,point);};s.resumeDisplacement=[](auto &){return false;};s.queueFallback=[&commands](auto &actor){return commands.queue(actor);};s.reach=[this](const auto &actor,const auto &target,model::Cell point){return geometry_.reach(actor,target,point);};s.adjust=[this](const auto &actor,const auto &,model::Cell &point){point=normalized(point);return geometry_.legal(actor,point);};s.legal=[this](const auto &actor,model::Cell point){return geometry_.legal(actor,normalized(point));};s.localReach=s.legal;s.beginMotion=[this,&commands](auto &actor){begin(actor,commands);};s.reset=[&commands](auto &actor){commands.reset(actor);};s.action=[&commands](auto &actor,unsigned n){commands.action(actor,n);};const auto yielded=model::continueCreatureRoute(a,c_.execution.at(slot),c_.world,s);scheduler.continuation=a.routePhase&&c_.native.search.at(slot).initialized;return yielded;};
    schedule.preDecision=[this,&commands](unsigned slot){auto &a=c_.world.creatures.at(slot);model::periodicTerrain(a,c_.execution.at(slot),p_->types.at(a.ai.type).periodicResistance,p_->periodic,{[this,slot](model::Cell p){return geometry_.ground(c_.world.creatures.at(slot),normalized(p));},[this]{return random();},[&commands](auto &actor){commands.reset(actor);},[&commands](auto &actor,model::Cell p){commands.move(actor,p);}});if(fx_.preDecision)fx_.preDecision(a);};
    schedule.decision=[this,&commands](unsigned slot){auto &a=c_.world.creatures.at(slot);auto services=autonomy(commands);model::autonomousDecision(a,c_.world,services);};
    schedule.valuation=[this](unsigned slot){auto &a=c_.world.creatures.at(slot);model::updateCreatureValue(a,c_.execution.at(slot),p_->valuation);};schedule.region=[this](unsigned slot){need(bool(fx_.region),"native region service unavailable");return fx_.region(c_.world.creatures.at(slot));};schedule.ownerWork=[this](unsigned slot){need(bool(fx_.ownerWork),"native owner service unavailable");fx_.ownerWork(c_.world.creatures.at(slot));};
    model::scheduleCreatures(c_.world,c_.scheduler,c_.environment,schedule);
    model::creatureEnvironmentPass(c_.world,c_.native.environment,p_->environment,{[this](const auto &,model::Cell p){auto point=normalized(p);const auto d=c_.world.dimensions;auto kind=c_.native.cells.at((std::size_t(point.z)*d.y+point.y)*d.x+point.x).kind;return kind&&(p_->terrain.at(kind).flags_b0&2);},fx_.maintenance.health});
    return {{{0,c_.generations[0]},"native-creature-frame",{},std::int32_t(c_.native.tick)}};
  }
};
}
void prepareNativeContinuation(Continuation &c,const NativeCreatureProfile &p){
  need(!p.animation.empty()&&!p.terrain.empty()&&p.terrainPropertyA0.size()==p.terrain.size(),"native profile assets are missing");
  const auto n=c.world.creatures.size();c.originalBehaviorBindings=true;c.maintenance.resize(n);c.movement.resize(n);c.actions.callbacks.resize(n);c.actions.motion.resize(n);c.actions.segments.resize(n);
  c.actions.objects.dimensions=c.world.dimensions;if(c.actions.objects.heads.empty())c.actions.objects.heads.assign(std::size_t(c.world.dimensions.x)*c.world.dimensions.y*c.world.dimensions.z,0xffff);
  c.native.present=true;c.native.profileStamp=stamp(p);c.native.animation.resize(n);c.native.search.resize(n);c.native.initialization.resize(n);
  need(c.native.cells.size()==c.actions.objects.heads.size(),"native terrain inputs are missing");
  NativeCreatureGeometry geometry(c,p);for(auto &a:c.world.creatures){geometry.snap(a,a.ai.position);geometry.reserve(a,a.ai.position,true);}
}
HookFactory nativeProviders(NativeCreatureProfile profile,NativeConsequenceFactory lower){
  need(bool(lower),"native consequence factory missing");auto p=std::make_shared<const NativeCreatureProfile>(std::move(profile));
  return [p,lower](Continuation &c){auto context=std::make_shared<NativeContext>(c,p,lower(c));Hooks h;h.originalBehaviorBindings=true;
    h.execution=[context](unsigned){return context->execution();};h.queue=[](unsigned){return model::ExecutionServices{};};h.idle=[context](unsigned){return context->idle();};h.follow=[context](unsigned){return context->follow();};
    h.commandBodies=[context](auto &a,unsigned kind,auto &commands){context->body(a,kind,commands);};h.actionTransition=[context](auto &a,unsigned kind,auto &commands){context->action(a,kind,commands);};h.actionTick=[context](auto &a,auto &commands){context->tickAction(a,commands);};
    h.supplied=[](auto &){throw std::invalid_argument("native supplied behavior descriptor unavailable");};h.release=[context](unsigned value){context->release(value);};h.peer=[](auto &a,bool){if(a.ai.incoming)--a.ai.incoming;};h.advance=[context](auto &,auto &commands,game::TickInput){return context->frame(commands);};return h;};
}
} // namespace mnm::command_session
