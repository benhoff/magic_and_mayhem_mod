#include "native_owner_visibility.hpp"
#include "native_regional_danger.hpp"
#include "native_geometry.hpp"
#include "native_route.hpp"
#include "native_regional.hpp"
#include "native_regional_graph.hpp"
#include "../../reconstruction/creature-ai/friendly_displacement.hpp"
#include "../../reconstruction/creature-ai/cached_attack.hpp"
#include "../../reconstruction/creature-ai/actor_route_request.hpp"
#include "../../reconstruction/creature-ai/idle_gesture.hpp"
#include "../../reconstruction/creature-ai/motion_response.hpp"
#include "../../reconstruction/creature-ai/coarse_route.hpp"
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
  for(const auto &t:p.types){if(t.regionalFlightClearance){word(0x5245474eu);word(t.regionalFlightClearance);}word(t.bodyAsset);word(t.regularOverlayAsset);word(t.meleeOverlayAsset);word(t.width);word(t.height);word(t.movementKind);word(t.scalar.type_3c);word(t.scalar.type_10);word(t.periodicResistance);word(t.routeCostFactor);word(t.routeFlagPenalty);for(auto v:t.gestureScores)word(v);for(auto v:t.scalar.type_d8)word(v);unsigned bits;std::memcpy(&bits,&t.scalar.slope_global,4);word(bits);}
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
  void play(model::AutonomousCreature &a,unsigned sequence,unsigned reverse){start(channel(a,false),p_->types.at(a.ai.type).bodyAsset,sequence,reverse);c_.actions.callbacks.at(a.ai.slot).sequence=sequence;c_.actions.callbacks.at(a.ai.slot).animationBreak=false;}
  void rewind(model::AutonomousCreature &a){auto &v=channel(a,false);if(!v.bound)return;r::NoCdAnimationController player(records(*p_,v.asset,v.sequence));player.restore(v.state);player.rewind();v.state=player.state();}
  unsigned random(){return model::nextRandom(c_.native.random);}
  model::AiGeometry aiGeometry(){auto g=fx_.autonomy.geometry;
    g.cell=[this](model::Cell p){p=normalized(p);auto d=c_.world.dimensions;const auto &cell=c_.native.cells.at((std::size_t(p.z)*d.y+p.y)*d.x+p.x);return model::AiTerrainCell{cell.occupant==0xffff?std::uint16_t(0xffff):std::uint16_t(cell.occupant-1),cell.flags,p_->terrainPropertyA0.at(cell.kind)};};
    g.legal=[this](const auto &a,model::Cell from,model::Cell to,unsigned ignore){int category=5;from=normalized(from);to=normalized(to);return r::accept_creature_move(a.ai.slot+1,{from.x,from.y,from.z},{to.x,to.y,to.z},ignore!=0,category,geometry_.acceptance());};
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
  void fineFacing(model::AutonomousCreature &a,model::Cell point){
    if(!c_.native.ownerVisibility){facing(a,{int(unsigned(point.x)>>5),int(unsigned(point.y)>>5),a.ai.position.z});return;}
    const auto direction=model::fineCoordinateFacing(a.finePosition,a.aimXY,point,c_.world.dimensions);
    if(!direction)return;
    need(a.animationKind!=2&&a.animationKind!=3&&!c_.execution.at(a.ai.slot).block855,"native fine-facing visual attachment unavailable");
    c_.actions.motion.at(a.ai.slot).direction=*direction;
    auto &e=c_.execution.at(a.ai.slot);e.animationFacing=(*direction-2*p_->cameraTurn-(e.block722?4:0))&7;
  }
  void facingOpponent(model::AutonomousCreature &a,const model::AutonomousCreature &b){
    auto direction=model::opponentFacing(a,b,c_.world.dimensions);if(!direction)return;
    auto &e=c_.execution.at(a.ai.slot);need(a.animationKind!=2&&a.animationKind!=3&&!e.block855,"native opponent visual facing unavailable");
    c_.actions.motion.at(a.ai.slot).direction=*direction;e.animationFacing=(*direction-2*p_->cameraTurn-(e.block722?4:0))&7;
  }
  void complete(model::AutonomousCreature &a,model::OwnedCreatureCommands &commands){
    auto slot=a.ai.slot;auto s=fx_.completion;
    s.stopOverlay=[this](auto &actor){channel(actor,true)={};};s.releaseOverlay=fx_.callbacks.releaseEffect;
    s.fallDamage=fx_.flight.fallDamage;s.defended=fx_.flight.defended;
    s.snap=[this](auto &actor,model::Cell p,unsigned){geometry_.snap(actor,p);};
    s.reset=[&commands](auto &actor){commands.reset(actor);};

    s.legal=[this](const auto &actor,model::Cell p){return geometry_.legal(actor,normalized(p));};
    s.regionCandidate=[this](auto &actor,auto &candidate,unsigned parameter,auto desired){nativeRegionalCandidate(c_,*p_,geometry_,actor,candidate,parameter,desired);};
    model::completeMotionSegment(a,c_.execution.at(slot),c_.actions.motion.at(slot),c_.movement.at(slot),c_.actions.segments.at(slot),nativeRegionalEnvironment(c_),s);
  }
  void begin(model::AutonomousCreature &a,model::OwnedCreatureCommands &commands){
    const auto slot=a.ai.slot;auto &initial=c_.native.initialization.at(slot);auto &motion=c_.actions.motion.at(slot);
    bool selectedSamples=false;
    initial.slow778=int(c_.maintenance.at(slot).status778);initial.fast77c=int(c_.maintenance.at(slot).status77c);
    auto s=fx_.segment;s.precheck=[this](auto &a){return model::cachedMotionOpponent(a,c_.world,aiGeometry());};
    s.immediate=[this,&commands](auto &actor){return model::attackCachedOpponent(actor,c_.maintenance.at(actor.ai.slot),c_.world,{[this](const auto &a,const auto &b,auto p){return geometry_.reach(a,b,p);},[this](const auto &a,auto p){return model::footprintObstacle(a,p,c_.world.dimensions,[this](auto q){const auto d=c_.world.dimensions;const auto &v=c_.native.cells.at((q.z*d.y+q.y)*d.x+q.x);return model::ObstacleCell{v.flags,v.occupant==0xffff?0u:unsigned(v.occupant)};});},[&commands](auto &a,unsigned kind){commands.action(a,kind);},[this](auto &a,const auto &b){facingOpponent(a,b);}});};
    s.fallback=[this,&commands](auto &a){const auto &route=c_.actions.segments.at(a.ai.slot);return model::displacementWait(a,c_.execution.at(a.ai.slot),route.route.at(route.cursor).position,c_.world,c_.execution,[this](auto p){auto d=c_.world.dimensions;auto v=c_.native.cells.at((p.z*d.y+p.y)*d.x+p.x);return model::ObstacleCell{v.flags,v.occupant==0xffff?0u:unsigned(v.occupant)};},friendly(commands));};
    s.base=[this](const auto &actor,unsigned mode,model::Cell delta){return r::base_movement_scalar(int(mode),{delta.x,delta.y,delta.z},scalar(actor));};
    s.scalar=[this](const auto &actor,unsigned argument,unsigned mode,model::Cell delta,int,unsigned &rate,int &length){auto value=r::creature_movement_scalar(int(argument),int(mode),{delta.x,delta.y,delta.z},rate,scalar(actor));rate=value.scalar;length=12;};
    s.accept=[this](auto &actor,model::Cell from,model::Cell to,unsigned &mode){return geometry_.move(actor,normalized(from),normalized(to),mode);};
    s.release=[this](auto &actor,model::Cell p){geometry_.reserve(actor,normalized(p),false);};s.reserve=[this](auto &actor,model::Cell p){geometry_.reserve(actor,normalized(p),true);};
    s.turn=[](unsigned old,unsigned now){auto d=int(now)-int(old);if(d>4)d-=8;if(d<-4)d+=8;return d;};
    s.scalarArgument=[](model::Cell,model::Cell){return 4u;};
    s.distance=[](model::Cell p){return unsigned(r::distance_metric(p.x*2,p.y*2,p.z));};
    s.tileKind=[this](model::Cell p){const auto d=c_.world.dimensions;return c_.native.cells.at((std::size_t(p.z)*d.y+p.y)*d.x+p.x).kind;};
    s.tileHeight=[this](unsigned kind){return p_->terrain.at(kind).classification_94;};
    s.animation=[this,&selectedSamples](auto &actor,unsigned sequence,unsigned reverse){selectedSamples=true;play(actor,sequence,reverse);};
    s.sample=[this,&initial](const auto &actor,unsigned bank,unsigned index){if(bank==0x71e)return initial.inlineSample;need(bank>=0xd8&&bank<0xd8+48*4,"native motion profile bank unavailable");return int(scalar(actor).type_d8.at((bank-0xd8)/4+index));};
    s.attack=[this](auto &actor,const auto &target){facing(actor,target.ai.position);play(actor,24+c_.execution.at(actor.ai.slot).animationFacing,0);};
    s.action=[&commands](auto &actor,unsigned kind){commands.action(actor,kind);};
    model::initializeMotionSegment(a,c_.execution.at(slot),motion,initial,c_.actions.segments.at(slot),c_.world,{p_->cameraTurn,p_->defaultScalar},s);
    if(!selectedSamples)return;
    if(initial.sampleBank==0x71e)motion.samples.fill(initial.inlineSample);
    else {need(initial.sampleBank>=0xd8&&initial.sampleBank<0xd8+48*4,"native sample bank unavailable");const auto base=(initial.sampleBank-0xd8)/4;auto values=scalar(a).type_d8;for(unsigned i=0;i<48;++i)motion.samples[i]=int(values[i]);motion.sampleCursor+=base;motion.initialCursor+=base;}
  }
  model::ActorRouteSnapshot actorSnapshot(const model::AutonomousCreature &a){
    const auto &m=c_.actions.motion.at(a.ai.slot);const auto &r=c_.actions.segments.at(a.ai.slot);const auto &e=c_.execution.at(a.ai.slot);
    return {a.destination,r.cursor,r.count,r.route,{unsigned(m.progress),unsigned(m.step),unsigned(m.travelX),unsigned(m.travelY),unsigned(m.deltaX),unsigned(m.deltaY),unsigned(m.heightDelta),unsigned(m.heightStep),unsigned(m.heightOrigin),e.moveVariant,unsigned(m.duration),unsigned(m.accumulator),e.routeChanged,e.markerB73}};
  }
  static std::array<unsigned,131> routeWords(const model::ActorRouteSnapshot &s){
    std::array<unsigned,131> v{};v[0]=unsigned(s.destination.x);v[1]=unsigned(s.destination.y);v[2]=unsigned(s.destination.z);v[3]=s.cursor;v[4]=s.count;
    for(unsigned i=0;i<16;++i){auto &q=s.points[i];auto at=5+7*i;v[at]=unsigned(q.position.x);v[at+1]=unsigned(q.position.y);v[at+2]=unsigned(q.position.z);for(unsigned j=0;j<4;++j)v[at+3+j]=unsigned(q.remainder[j]);}
    std::copy(s.motionWords.begin(),s.motionWords.end(),v.begin()+117);return v;
  }
  static model::ActorRouteSnapshot routeSnapshot(const r::RouteSnapshot &raw){
    std::array<unsigned,131> v{};std::memcpy(v.data(),&raw,sizeof(raw));model::ActorRouteSnapshot s;
    auto bits=[](unsigned n){return n<=0x7fffffffu?int(n):-1-int(~n);};
    s.destination={bits(v[0]),bits(v[1]),bits(v[2])};s.cursor=v[3];s.count=v[4];need(s.count<=16,"native shared route extent");
    for(unsigned i=0;i<16;++i){auto at=5+7*i;s.points[i].position={bits(v[at]),bits(v[at+1]),bits(v[at+2])};for(unsigned j=0;j<4;++j)s.points[i].remainder[j]=bits(v[at+3+j]);}
    std::copy(v.begin()+117,v.end(),s.motionWords.begin());return s;
  }
  unsigned request(model::AutonomousCreature &a,model::Cell target,unsigned mode,unsigned regions){
    auto &shared=c_.native.route;shared.budget=c_.scheduler.budget;
    if(c_.native.routeMetadata.empty())c_.native.routeMetadata.resize(c_.world.creatures.size());
    model::ActorRouteRequestServices services;
    services.regionAt=[this,&a](auto point,const auto &query,int dx,int dy){auto lower=nativeRegionalServices(c_,*p_,geometry_,a);return model::lookupNavigationRegion(c_.native.regions,point,nativeRegionalQuery(geometry_,a,query),c_.world.dimensions,p_->boundary,dx,dy,lower);};
    services.regionRoute=[this,&a](int from,int to,const auto &query,auto &path,unsigned limit,bool reset){int budget=int(limit);return model::searchNavigationRegions(c_.native.regions,from,to,nativeRegionalQuery(geometry_,a,query),c_.world.dimensions,path,budget,reset,c_.native.regionalSearch,nativeRegionalServices(c_,*p_,geometry_,a));};
    services.regionDistance=[this](int from,int to){return model::regionEdgeDistance(c_.native.regions,from,to);};
    services.regionCandidate=[this](auto &actor,auto &candidate,unsigned parameter,auto desired){nativeRegionalCandidate(c_,*p_,geometry_,actor,candidate,parameter,desired);};
    services.legal=[this](const auto &actor,auto point){return nativeRegionalLegal(geometry_,actor,point,p_->boundary);};
    services.adjustLegal=[this](const auto &actor,auto &point){nativeRegionalAdjust(c_,geometry_,actor,point,p_->boundary);};
    services.coarse=[this](auto &actor,auto to,auto &out){out=actorSnapshot(actor);return model::coarseCreatureRoute(actor,c_.actions.motion.at(actor.ai.slot).direction,c_.world.dimensions,to,out,[this,&actor](auto from,auto end,unsigned &category){return geometry_.terrainMove(actor,from,end,category);},[this,&actor](auto q){return fx_.autonomy.geometry.hazardous(actor,q);});};
    services.search=[this](auto &actor,unsigned mode,auto target,auto &request,int &budget){
      auto &saved=c_.native.search.at(0);r::SearchState search;r::RouteContextPrefix context{};
      if(!request.scratchDirty){need(saved.initialized,"native shared search absent");for(auto v:saved.open)search.open.emplace(v.priority,v.node);for(auto v:saved.records)search.records.emplace(v.node,v.record);search.best_node=saved.bestNode;search.best_heuristic=saved.bestHeuristic;search.initialized=true;std::memcpy(&context,saved.context.data(),sizeof(context));}
      else {saved={};context.unknown_route_flag=1;}
      auto snapshot=geometry_.snapshot(actor,target);auto words=routeWords(actorSnapshot(actor));std::memcpy(&snapshot->object.route,words.data(),sizeof(snapshot->object.route));
      auto neighbors=r::snapshot_neighbors(snapshot);auto world=r::neighbor_search_world(neighbors,{target.x,target.y,target.z},actor.ai.slot+1);
      r::route_search(context,snapshot->object,mode,{target.x,target.y,target.z},budget,search,world);
      saved.open.clear();saved.records.clear();for(auto v:search.open)saved.open.push_back({v.first,v.second});for(auto v:search.records)saved.records.push_back({v.first,v.second});saved.bestNode=search.best_node;saved.bestHeuristic=search.best_heuristic;saved.initialized=search.initialized;saved.destination=target;std::memcpy(saved.context.data(),&context,sizeof(context));
      request.scratchDirty=context.unknown_route_flag!=0;request.scratch=routeSnapshot(context.route);
    };
    auto result=model::requestCreatureRoute(a,c_.execution.at(a.ai.slot),c_.actions.motion.at(a.ai.slot),c_.actions.segments.at(a.ai.slot),c_.native.routeMetadata.at(a.ai.slot),shared,nativeRegionalEnvironment(c_),c_.world.dimensions,target,mode,regions!=0,services);
    c_.scheduler.budget=shared.budget;c_.scheduler.continuation=shared.active!=0;return result;
  }
  unsigned obstacle(const model::AutonomousCreature &a,model::Cell point){return model::footprintObstacle(a,point,c_.world.dimensions,[this](auto q){const auto d=c_.world.dimensions;const auto &v=c_.native.cells.at((q.z*d.y+q.y)*d.x+q.x);return model::ObstacleCell{v.flags,v.occupant==0xffff?0u:unsigned(v.occupant)};});}
  model::FriendlyDisplacementServices friendly(model::OwnedCreatureCommands &commands){
    model::FriendlyDisplacementServices s;
    s.move=[this](const auto &a,auto from,auto to,bool ignore){int category=5;return r::accept_creature_move(a.ai.slot+1,{from.x,from.y,from.z},{to.x,to.y,to.z},ignore,category,geometry_.acceptance());};
    s.flags=[this](auto p){auto d=c_.world.dimensions;return unsigned(c_.native.cells.at((p.z*d.y+p.y)*d.x+p.x).flags);};
    s.obstacle=[this](const auto &a,auto p){return obstacle(a,p);};
    s.hazard=[this](const auto &a,auto p){auto d=c_.world.dimensions;for(unsigned y=0;y<a.width;++y)for(unsigned x=0;x<a.width;++x)for(int z=p.z;z>=p.z-1;--z)if(z>=0&&z<d.z){auto q=normalized({p.x+int(x),p.y+int(y),z});if(c_.native.cells.at((q.z*d.y+q.y)*d.x+q.x).flags&0x10)return true;}return false;};
    s.action=[&commands](auto &a,unsigned n){commands.action(a,n);};
    s.order=[this,&commands](auto &a,auto p){
      auto &e=c_.execution.at(a.ai.slot);
      if(a.ai.type==27){commands.reset(a);return;}
      if(a.ai.type==22||a.ai.pending==8||a.ai.behavior==15||a.ai.behavior==6||a.ai.behavior==8||a.ai.behavior==16||a.mode||(a.form&&!p_->types.at(a.ai.type).scalar.type_3c&&a.animationKind!=3))return;
      a.ai.marker=a.ai.readiness=0;
      if(a.ai.engaged&&(a.ai.action==10||a.ai.action==20)){auto &peer=c_.world.creatures.at(unsigned(a.ai.action==10?a.ai.target:a.alternateTarget));a.ai.engaged=0;peer.ai.incoming=peer.ai.incoming?peer.ai.incoming-1:0;}
      if(a.ai.motion)return;
      a.destination=p;commands.action(a,15);a.routePhase=1;e.displacementActive=0;commands.transition(a,13);
      if(e.routeEffect){release(*e.routeEffect);e.routeEffect.reset();}for(auto &o:e.orders)if(o.kind==2&&o.effect){release(*o.effect);o.effect.reset();}
      if(e.marker68)fx_.callbacks.updateOrders(a);
    };
    return s;
  }
  bool displace(model::AutonomousCreature &a,model::OwnedCreatureCommands &commands){
    auto lower=friendly(commands);auto &shared=c_.native.route;

    model::ActorDisplacementServices s;
    s.meleeReached=[this](const auto &a,const auto *b,auto p){return b&&geometry_.reach(a,*b,p);};
    s.rangedReached=[this](const auto &a,const auto *b){return b&&geometry_.reach(a,*b,a.ai.position);};
    s.blocker=[this,&lower](auto &a){const auto &r=c_.actions.segments.at(a.ai.slot);need(r.cursor<r.count,"native displacement route endpoint absent");return model::displacementBlocker(a,r.route.at(r.cursor).position,lower);};
    s.canDisplace=[this](auto &a,unsigned slot){return model::canDisplaceCreature(a,c_.world.creatures.at(slot),c_.execution.at(slot),c_.world);};
    s.issue=[this,&lower](auto &a,unsigned slot){return model::issueFriendlyDisplacement(a,c_.world.creatures.at(slot),c_.world,c_.execution,c_.actions.segments,c_.actions.motion,c_.native.displacementOwners,lower);};
    s.obstacle=lower.obstacle;
    s.update=[this](auto &a,auto &shared){
      const auto slot=a.ai.slot;auto &route=c_.actions.segments.at(slot);
      const auto next=route.route.at(route.cursor).position;
      if(next==a.destination&&(a.ai.position.x==a.destination.x||a.ai.position.y==a.destination.y||obstacle(a,a.destination))){shared.active=0;return 0u;}
      // 51b300 probes with a temporary route, restores all131 actor words,
      // region bookkeeping and marker; shared search/budget mutations persist.
      const auto actor=a;const auto execution=c_.execution.at(slot);const auto motion=c_.actions.motion.at(slot);const auto saved=route;
      const auto contains=[](const auto &route,auto point){for(unsigned i=0;i<route.count;++i)if(route.route.at(i).position==point)return true;return false;};
      bool priorGoal=contains(saved,a.destination);auto target=a.destination;
      if(route.regionPending){
        priorGoal=true;const auto candidate=[&](unsigned cursor){need(cursor<=50,"native displacement region cursor");
          //51b300 reads one DWORD beyond the50-entry path: actor+dcf is
          //the saved regional cursor itself. Model that alias explicitly.
          int id=cursor==50?int(route.regionCursor):route.regionPath.at(cursor);need(id>=0,"native displacement region reference");const auto &v=c_.native.regions.at(unsigned(id));auto point=v.position;nativeRegionalCandidate(c_,*p_,geometry_,a,point,v.parameter,a.destination);return point;};
        target=candidate(route.regionCursor);
        if(next==target||route.count<=route.cursor+2){
          if(route.regionCursor+1<50&&route.regionPath.at(route.regionCursor+1)==-1){target=a.destination;++route.regionCursor;}
          else {target=candidate(route.regionCursor+1);route.regionPending=false;}
        }
      }
      a.routeMode=0;const auto result=request(a,target,0,0);
      const bool reached=result==1&&contains(route,target);
      const bool pending=route.regionPending;const unsigned cursor=route.regionCursor;
      const auto mode=a.routeMode;a.destination=actor.destination;a.ai.marker=actor.ai.marker;a.routeMode=mode;
      c_.execution.at(slot)=execution;c_.actions.motion.at(slot)=motion;route=saved;
      if(saved.regionPending&&(reached||!priorGoal)){route.regionPending=pending;route.regionCursor=cursor;}
      return reached||!priorGoal?1u:0u;
    };
    s.random=[this]{return random();};s.diagnostic=[](unsigned){throw std::invalid_argument("native displacement diagnostic unavailable");};
    s.action=lower.action;s.reset=[&commands](auto &a){commands.reset(a);};
    return model::displaceCreatureRoute(a,c_.world,c_.native.displacementOwners,c_.execution,shared,s);
  }
  model::AutonomousServices autonomy(model::OwnedCreatureCommands &commands){
    auto s=fx_.autonomy;s.geometry=aiGeometry();s.random=[this]{return random();};
    s.action=[&commands](auto &a,unsigned n){commands.action(a,n);};s.behavior=[&commands](auto &a,unsigned n){commands.transition(a,n);};s.reset=[&commands](auto &a){commands.reset(a);};
    s.beginMotion=[this,&commands](auto &a){begin(a,commands);};s.faceTarget=[this](auto &a,const auto &b){facingOpponent(a,b);};s.move=[&commands](auto &a,model::Cell p){commands.move(a,p);};
    s.routeMove=[this](auto &a,model::Cell p){return immediateNativeRoute(c_,*p_,a,normalized(p),a.routeMode);};return s;
  }
  model::BehaviorExecutionServices behavior(model::OwnedCreatureCommands &commands){
    auto s=fx_.behavior;
    s.immediate=[this](auto &a){return model::immediateOpponent(a,c_.world,aiGeometry(),[this]{return random();});};s.interaction=[this](auto &a){return model::idleGesture(a,c_.world,aiGeometry(),p_->types.at(a.ai.type).gestureScores,[this]{return random();},[this](auto &actor,model::Cell point){fineFacing(actor,point);});};
    s.action=[&commands](auto &a,unsigned n){commands.action(a,n);};s.behavior=[&commands](auto &a,unsigned n){commands.transition(a,n);};s.reset=[&commands](auto &a){commands.reset(a);};
    s.beginMotion=[this,&commands](auto &a){begin(a,commands);};s.reach=[this](const auto &a,const auto &b,model::Cell p){return geometry_.reach(a,b,p);};
    s.face=[this](auto &a,const auto &b){facingOpponent(a,b);};s.coordinateFace=[this](auto &a,model::Cell p){facing(a,p);};

    return s;
  }
public:
  NativeContext(Continuation &c,std::shared_ptr<const NativeCreatureProfile> p,NativeConsequences fx):c_(c),p_(std::move(p)),fx_(std::move(fx)),geometry_(c_,*p_){
    need(c_.native.present&&c_.native.profileStamp==stamp(*p_),"native immutable profile differs");
    if(!c_.native.randomSeeded){c_.native.random=model::seedRandom(c_.seed);c_.native.randomSeeded=true;}
  }
  void release(unsigned value){need(bool(fx_.callbacks.releaseEffect),"native release service unavailable");fx_.callbacks.releaseEffect(value);}
  model::ExecutionServices execution(){auto s=fx_.execution;s.orient=[this](auto &a,model::Cell p){fineFacing(a,p);};s.terrainHeight=[this](model::Cell p){return geometry_.height(normalized(p));};return s;}
  model::BehaviorExecutionServices idle(){auto s=fx_.behavior;s.immediate=[this](auto &a){return model::immediateOpponent(a,c_.world,aiGeometry(),[this]{return random();});};s.interaction=[this](auto &a){return model::idleGesture(a,c_.world,aiGeometry(),p_->types.at(a.ai.type).gestureScores,[this]{return random();},[this](auto &actor,model::Cell point){fineFacing(actor,point);});};return s;}
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
    p.motion=[this,&commands](unsigned){auto s=fx_.motion;s.response=[this](auto &a){return model::motionOpponentResponse(a,c_.world,aiGeometry());};s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.rewind=[this](auto &a){rewind(a);};s.completeSegment=[this,&commands](auto &a){complete(a,commands);};s.updateOrders=fx_.callbacks.updateOrders;s.legal=[this](const auto &a,model::Cell p){return geometry_.legal(a,normalized(p));};s.stun=fx_.callbacks.stun;s.effect=fx_.callbacks.effect;s.releaseEffect=fx_.callbacks.releaseEffect;s.cameraTurn=p_->cameraTurn;return s;};
    p.flight=[this,&commands](unsigned){auto s=fx_.flight;s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.rewind=[this](auto &a){rewind(a);};s.completeSegment=[this,&commands](auto &a){complete(a,commands);};s.animation=[this](auto &a,unsigned seq,unsigned rev){play(a,seq,rev);};s.occupied=[this](const auto &a,model::Cell p){return !geometry_.cell(a,normalized(p));};s.passable=[this](const auto &a,model::Cell p){return geometry_.legal(a,normalized(p));};s.ground=[this](const auto &a,model::Cell p){return geometry_.ground(a,normalized(p));};s.random=[this]{return random();};s.release=[this](auto &a,model::Cell p){geometry_.reserve(a,normalized(p),false);};s.reserve=[this](auto &a,model::Cell p){geometry_.reserve(a,normalized(p),true);};s.snap=[this](auto &a,model::Cell p,unsigned){geometry_.snap(a,normalized(p));};s.relocateDead=[this](auto &a,model::Cell p){geometry_.snap(a,normalized(p));};return s;};
    p.objects=[this](unsigned slot){model::ObjectActionServices s;s.poll=[this](auto &a,bool overlay){return poll(a,overlay);};s.cue=[this,slot](unsigned cue,model::Cell p){auto &a=c_.world.creatures.at(slot);fx_.callbacks.effect(a,cue,p);};return s;};return p;
  }
  void body(model::AutonomousCreature &a,unsigned kind,model::OwnedCreatureCommands &commands){bind(commands);
    auto s=behavior(commands);model::CommandBehaviorProviders p;
    p.behavior=[s](unsigned){return s;};
    p.ranged=[this,s,&commands](unsigned){auto out=fx_.ranged;out.orient=[this](auto &actor,model::Cell point){fineFacing(actor,point);};out.action=[&commands](auto &actor,unsigned n){commands.action(actor,n);};out.maintainTarget=[this,s](auto &actor){model::maintainTargetBehavior(actor,c_.execution.at(actor.ai.slot),c_.world,p_->maintenance.restoreMode,s);};return out;};
    p.movement=[this,&commands](unsigned){auto out=fx_.movement;out.beginMotion=[this,&commands](auto &actor){begin(actor,commands);};out.move=[&commands](auto &actor,model::Cell point){commands.move(actor,point);};out.relocate=[this](auto &actor,model::Cell point){geometry_.snap(actor,normalized(point));};out.releaseOccupancy=[this](auto &actor,model::Cell point){geometry_.reserve(actor,normalized(point),false);};out.adjust=[this](const auto &actor,model::Cell &point){point=normalized(point);return geometry_.legal(actor,point);};return out;};
    if(kind>=2&&kind<=4){model::OwnedCommandBehaviors bodies(c_.world,c_.execution,c_.maintenance,c_.movement,commands,{c_.native.tick,p_->maintenance.restoreMode},p);bodies.dispatch(a,kind);}
    else if(kind==5){auto d=fx_.death;d.action=[&commands](auto &actor,unsigned n){commands.action(actor,n);};model::deathBehavior(a,c_.maintenance.at(a.ai.slot),c_.movement.at(a.ai.slot),{},d);}
    else if(kind==9)model::immediateAttackBehavior(a,c_.world,s);
    else if(kind==10)model::standingRangedBehavior(a,c_.execution.at(a.ai.slot),c_.world,c_.native.tick,s);
    else if(kind==13)model::displacementBehavior(a,c_.execution.at(a.ai.slot),s);
    else if(kind==11){auto services=autonomy(commands);services.orient=[this,&a](model::Cell point){fineFacing(a,point);};model::autonomousBehavior(a,c_.world,services);}
    else model::specialBehavior(a,c_.execution.at(a.ai.slot),kind,s);
  }
  std::vector<game::CreatureRuleEvent> frame(model::OwnedCreatureCommands &commands){
    bind(commands);
    ++c_.native.sequence;if(c_.native.alternate)return {};++c_.native.tick;
    updateNativeOwnerVisibility(c_, *p_);
    updateNativeRegionalDanger(c_, *p_);
    for(auto &a:c_.world.creatures)if(a.ai.active){if(!channel(a,false).bound&&c_.execution.at(a.ai.slot).actionCallback)commands.action(a,a.ai.action);
      auto s=fx_.maintenance;s.random=[this]{return random();};s.action=[&commands](auto &actor){commands.tickAction(actor);};s.poll=[this](auto &actor,model::MaintenanceAnimation which){return poll(c_.native.animation.at(actor.ai.slot).auxiliary.at(unsigned(which)));};
      auto env=p_->maintenance;env.tick=c_.native.tick;model::maintainCreature(a,c_.execution.at(a.ai.slot),c_.maintenance.at(a.ai.slot),c_.world,env,s);
    }
    for(auto &v:c_.native.worldAnimation)poll(v);
    model::SchedulerServices schedule;
    schedule.route=[this,&commands](unsigned slot,auto &scheduler){c_.native.route.active=scheduler.continuation?1u:0u;c_.native.route.budget=scheduler.budget;auto &a=c_.world.creatures.at(slot);model::RouteExecutionServices s;s.request=[this](auto &actor,model::Cell point,unsigned mode,unsigned regions){return request(actor,point,mode,regions);};s.resumeDisplacement=[this,&commands,&s](auto &a){const auto result=displace(a,commands);s.displacementPending=c_.native.route.displacementContinuation!=0;return result;};s.queueFallback=[&commands](auto &actor){return commands.queue(actor);};s.reach=[this](const auto &actor,const auto &target,model::Cell point){return geometry_.reach(actor,target,point);};s.adjust=[this](const auto &actor,const auto &,model::Cell &point){point=normalized(point);return geometry_.legal(actor,point);};s.legal=[this](const auto &actor,model::Cell point){return geometry_.legal(actor,normalized(point));};s.localReach=s.legal;s.beginMotion=[this,&commands](auto &actor){begin(actor,commands);};s.reset=[&commands](auto &actor){commands.reset(actor);};s.action=[&commands](auto &actor,unsigned n){commands.action(actor,n);};const auto yielded=model::continueCreatureRoute(a,c_.execution.at(slot),c_.world,s);scheduler.continuation=c_.native.route.active!=0;return yielded;};
    schedule.preDecision=[this,&commands](unsigned slot){auto &a=c_.world.creatures.at(slot);model::periodicTerrain(a,c_.execution.at(slot),p_->types.at(a.ai.type).periodicResistance,p_->periodic,{[this,slot](model::Cell p){return geometry_.ground(c_.world.creatures.at(slot),normalized(p));},[this]{return random();},[&commands](auto &actor){commands.reset(actor);},[&commands](auto &actor,model::Cell p){commands.move(actor,p);}});if(fx_.preDecision)fx_.preDecision(a);};
    schedule.decision=[this,&commands](unsigned slot){auto &a=c_.world.creatures.at(slot);auto services=autonomy(commands);if(c_.native.ownerVisibility)services.orient=[this,&a](model::Cell point){fineFacing(a,point);};model::autonomousDecision(a,c_.world,services);};
    schedule.valuation=[this](unsigned slot){auto &a=c_.world.creatures.at(slot);auto valuation=p_->valuation;
      // The regional-danger composition reads the same CFG words as route aging.
      // Keep the older explicit valuation environment for danger-disabled sessions.
      if(c_.native.regionalDanger){const auto &type=p_->types.at(a.ai.type);valuation.factor589=type.routeCostFactor;valuation.factor5ad=type.routeFlagPenalty;}
      model::updateCreatureValue(a,c_.execution.at(slot),valuation);};schedule.region=[this](unsigned slot){auto &a=c_.world.creatures.at(slot);if(c_.native.route.useRegions)return model::lookupNavigationRegion(c_.native.regions,a.ai.position,nativeRegionalQuery(geometry_,a,{slot,a.ai.owner,0,0}),c_.world.dimensions,p_->boundary,0,0,nativeRegionalServices(c_,*p_,geometry_,a));need(bool(fx_.region),"native region service unavailable");return fx_.region(a);};schedule.ownerWork=[this](unsigned slot){need(bool(fx_.ownerWork),"native owner service unavailable");fx_.ownerWork(c_.world.creatures.at(slot));};
    model::scheduleCreatures(c_.world,c_.scheduler,c_.environment,schedule);
    c_.native.route.budget=c_.scheduler.budget;
    // Manager+0x22c and shared request active both name original 0x6df184.
    // Scheduling may clear it after a non-yielding or removed actor.
    c_.native.route.active=c_.scheduler.continuation?1u:0u;
    model::creatureEnvironmentPass(c_.world,c_.native.environment,p_->environment,{[this](const auto &,model::Cell p){auto point=normalized(p);const auto d=c_.world.dimensions;auto kind=c_.native.cells.at((std::size_t(point.z)*d.y+point.y)*d.x+point.x).kind;return kind&&(p_->terrain.at(kind).flags_b0&2);},fx_.maintenance.health});
    maintainNativeRegionalGraph(c_, *p_);
    return {{{0,c_.generations[0]},"native-creature-frame",{},std::int32_t(c_.native.tick)}};
  }
};
}
void prepareNativeContinuation(Continuation &c,const NativeCreatureProfile &p){
  need(!p.animation.empty()&&!p.terrain.empty()&&p.terrainPropertyA0.size()==p.terrain.size(),"native profile assets are missing");
  if(c.native.route.useRegions)c.environment.updateRegions=true;
  const auto n=c.world.creatures.size();c.originalBehaviorBindings=true;c.maintenance.resize(n);c.movement.resize(n);c.actions.callbacks.resize(n);c.actions.motion.resize(n);c.actions.segments.resize(n);
  c.actions.objects.dimensions=c.world.dimensions;if(c.actions.objects.heads.empty())c.actions.objects.heads.assign(std::size_t(c.world.dimensions.x)*c.world.dimensions.y*c.world.dimensions.z,0xffff);
  c.native.present=true;c.native.profileStamp=stamp(p);c.native.animation.resize(n);c.native.search.resize(n);c.native.displacementOwners.resize(n);c.native.initialization.resize(n);
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
