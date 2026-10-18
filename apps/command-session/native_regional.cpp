#include "native_regional.hpp"
#include <algorithm>
#include <cstring>
namespace mnm::command_session {
namespace r=reconstruction;
bool nativeRegionalLegal(NativeCreatureGeometry &geometry,const model::AutonomousCreature &a,model::Cell p,int boundary){
  auto h=geometry.movement();auto parameters=r::creature_movement_parameters(geometry.actor(a.ai.slot));const r::Coordinates q{p.x,p.y,p.z};
  return h.check(r::MovementCheck::check_46b0,q,parameters)&&!h.check(r::MovementCheck::check_41a0,q,parameters)&&
    (h.record(q,parameters)||(parameters.type_44==2&&p.z<boundary)||parameters.object_104);
}
void nativeRegionalAdjust(Continuation &c,NativeCreatureGeometry &g,const model::AutonomousCreature &a,model::Cell &p,int boundary){
  const auto d=c.world.dimensions;
  const auto kind=[&](model::Cell p){return c.native.cells.at((p.z*d.y+p.y)*d.x+p.x).kind;};
  bool lowered=false;
  if(!a.flag104&&!kind(p)){while(p.z>1){--p.z;lowered=true;if(kind(p))break;}}
  auto parameters=r::creature_movement_parameters(g.actor(a.ai.slot));
  if(parameters.type_44==2&&p.z<boundary)p.z=boundary-parameters.type_0c;
  if(nativeRegionalLegal(g,a,p,boundary))return;
  auto h=g.movement();auto point=r::Coordinates{p.x,p.y,p.z};auto probe=parameters;probe.type_0c=probe.type_08=probe.object_104=1;probe.type_44=3;probe.type_index=0;probe.special_state=0;
  // The one-cell probe has flag104=1, so4f4900 bypasses support.
  bool free=!h.check(r::MovementCheck::check_46b0,point,probe)||h.check(r::MovementCheck::check_41a0,point,probe);
  for(auto q:r::adjacent_cells(point,{d.x,d.y,d.z})){
    model::Cell candidate{q.x,q.y,q.z};int category=5;
    if(!free&&!r::test_movement(point,q,probe,category,-1,h))continue;
    if(nativeRegionalLegal(g,a,candidate,boundary)){p=candidate;return;}
  }
  (void)lowered; // Original return flag is not consumed by5128a0.
}
void nativeRegionalCandidate(Continuation &c,const NativeCreatureProfile &p,NativeCreatureGeometry &g,const model::AutonomousCreature &a,model::Cell &candidate,unsigned parameter,model::Cell desired){
  const auto &t=p.types.at(a.ai.type);auto h=g.movement();const auto parameters=r::creature_movement_parameters(g.actor(a.ai.slot));
  model::regionalCandidate(a,candidate,parameter,desired,t.scalar.type_3c!=0,t.regionalFlightClearance,t.movementKind,p.boundary,c.world.dimensions.z,
    [h,parameters](auto q){return h.check(r::MovementCheck::check_46b0,{q.x,q.y,q.z},parameters);});
}
model::RegionalQuery nativeRegionalQuery(const NativeCreatureGeometry &g,const model::AutonomousCreature &a,const model::ActorRegionQuery &q){
  return {r::creature_movement_parameters(g.actor(a.ai.slot)),true,q.command!=0,q.marker69c!=0};
}
model::RegionalServices nativeRegionalServices(Continuation &c,const NativeCreatureProfile &p,NativeCreatureGeometry &g,const model::AutonomousCreature &a){
  model::RegionalServices s;
  s.legal=[&g,&a,&p](auto q){return nativeRegionalLegal(g,a,q,p.boundary);};
  s.line=[&g,&a](auto from,auto to){return g.line(a,from,to);};
  s.danger=[&c,&a](int id){return model::regionDanger(a,c.native.regions.at(unsigned(id)),c.native.regionalRiskPowers);};
  s.boundedRoute=[&g,&a](auto from,auto to){
    auto snapshot=g.snapshot(a,to);std::memcpy(reinterpret_cast<unsigned char *>(&snapshot->object)+8,&from.x,4);std::memcpy(reinterpret_cast<unsigned char *>(&snapshot->object)+12,&from.y,4);std::memcpy(reinterpret_cast<unsigned char *>(&snapshot->object)+16,&from.z,4);
    auto neighbors=r::snapshot_neighbors(snapshot);auto world=r::neighbor_search_world(neighbors,{to.x,to.y,to.z},a.ai.slot+1);
    world.expand=[neighbors,from,to,token=a.ai.slot+1](auto current,const auto &prior,const auto &object,unsigned){auto desc=r::creature_descriptor(object,token,0,{to.x,to.y,to.z});desc.start={from.x,from.y,from.z};return r::generate_neighbors(current,prior,desc,neighbors);};
    r::RouteContextPrefix context{};context.unknown_route_flag=1;r::SearchState state;int budget=12;
    const auto result=r::route_search(context,snapshot->object,0,{to.x,to.y,to.z},budget,state,world);
    const auto endpoint=world.node_at({to.x,to.y,to.z});return std::find(result.path.begin(),result.path.end(),endpoint)!=result.path.end();
  };
  return s;
}
model::SegmentCompletionEnvironment nativeRegionalEnvironment(const Continuation &c){
  model::SegmentCompletionEnvironment env;env.regionContinuation=c.native.route.useRegions;for(const auto &r:c.native.regions)env.regions.push_back({r.position,r.parameter});return env;
}
} // namespace mnm::command_session
