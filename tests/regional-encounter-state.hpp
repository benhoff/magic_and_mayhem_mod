#pragma once
#include "../apps/command-session/native_regional.hpp"
#include "paired-encounter-projection.hpp"
#include <algorithm>
#include <cstring>
namespace mnm::encounter::regional {
// Declared fixture graph, shared as input by both implementations. Its
// production is a test policy, not a claim to recover the original graph builder.
inline Profile profile(game::Bytes inputs,unsigned scenario){
  auto p=decodeInputs(std::move(inputs));auto &c=p.initial;auto &a=c.world.creatures[0];
  for(auto &v:c.native.cells){v.flags&=std::uint16_t(~3u);v.occupant=0xffff;}
  for(auto &actor:c.world.creatures){actor.ai.owner=0;actor.destination=actor.ai.position;actor.ai.enemyRadius.fill(0);actor.ai.allyRadius.fill(0);}
  // A long sealed wall with a passable opening exercises local replanning.
  if(scenario==1||scenario==2)for(int z=0;z<c.world.dimensions.z;++z)for(int y=1;y<31;++y)if(scenario==2||y<4||y>10)c.native.cells[(z*32+y)*32+16].flags|=0x4000;
  cs::NativeCreatureGeometry g(c,p.native);
  for(int x=2;x<30;x+=4)for(int y=2;y<30;y+=4)for(int z=1;z<10;++z){cs::model::Cell point{x,y,z};if(cs::nativeRegionalLegal(g,a,point,0)){
    cs::model::NavigationRegion v;v.position=point;c.native.regions.push_back(v);break;
  }}
  if(c.native.regions.size()<12)throw std::runtime_error("regional installed fixture graph");
  // Fill connections only when a bounded native tile search reaches the other
  // authored region. This fixture preparation supplies data to both engines.
  auto snapshot=g.snapshot(a,a.ai.position);auto helpers=reconstruction::snapshot_neighbors(snapshot);
  for(unsigned i=0;i<c.native.regions.size();++i){auto &v=c.native.regions[i];unsigned edge=0;v.connections={};
    for(unsigned j=0;j<c.native.regions.size()&&edge<18;++j)if(i!=j){const auto q=c.native.regions[j].position;
      if(std::abs(q.x-v.position.x)>4||std::abs(q.y-v.position.y)>4)continue;
      auto world=reconstruction::neighbor_search_world(helpers,{q.x,q.y,q.z},1);auto object=snapshot->object;
      std::memcpy(reinterpret_cast<unsigned char *>(&object)+8,&v.position.x,4);std::memcpy(reinterpret_cast<unsigned char *>(&object)+12,&v.position.y,4);std::memcpy(reinterpret_cast<unsigned char *>(&object)+16,&v.position.z,4);
      reconstruction::RouteContextPrefix context{};context.unknown_route_flag=1;reconstruction::SearchState search;int budget=300;
      const auto route=reconstruction::route_search(context,object,0,{q.x,q.y,q.z},budget,search,world);
      if(route.path.back()!=world.node_at({q.x,q.y,q.z}))continue;
      auto &e=v.connections[edge++];e.destination=int(j);e.cost=cs::model::regionHeuristic(v.position,q,c.world.dimensions)*2;e.clearance[0]=e.clearance[1]=e.clearance[2]=8;
    }
  }
  auto &regions=c.native.regions;
  const auto choose=[&](int x,int y){unsigned best=0;int cost=INT32_MAX;for(unsigned i=0;i<regions.size();++i){auto q=regions[i].position;int v=std::abs(q.x-x)+std::abs(q.y-y);if(v<cost){cost=v;best=i;}}return best;};
  unsigned source=choose(6,6),target=choose(26,26);const auto ally=choose(2,26);
  if(scenario!=2){int best=-1;
    for(unsigned start=0;start<regions.size();++start){std::vector<unsigned> open{start};std::vector<int> depth(regions.size(),-1);depth[start]=0;
      for(unsigned at=0;at<open.size();++at){auto node=open[at];for(const auto &edge:regions[node].connections)if(edge.cost&&depth.at(unsigned(edge.destination))==-1){depth[unsigned(edge.destination)]=depth[node]+1;open.push_back(unsigned(edge.destination));}}
      for(unsigned end=0;end<regions.size();++end)if(depth[end]>=3){const auto a=regions[start].position,b=regions[end].position;int cost=std::abs(a.x-b.x)+std::abs(a.y-b.y);if(scenario==1&&((a.x<16)==(b.x<16)))continue;if(cost>best&&start!=ally&&end!=ally){best=cost;source=start;target=end;}}
    }
    if(best<0){std::string info;for(unsigned j=0;j<regions.size();++j){auto v=regions[j].position;unsigned edges=0;for(auto &e:regions[j].connections)edges+=e.cost!=0;info+=" ["+std::to_string(v.x)+","+std::to_string(v.y)+":"+std::to_string(edges)+"]";}throw std::runtime_error("regional connected fixture journey missing:"+info);}
  }
  a.ai.position=regions[source].position;a.ai.home=a.ai.position;c.execution[0].roamHome=a.ai.position;
  c.world.creatures[1].ai.position=regions[ally].position;c.world.creatures[1].ai.home=regions[ally].position;c.execution[1].roamHome=regions[ally].position;c.world.creatures[1].destination=regions[ally].position;
  a.destination=regions[target].position;a.ai.behavior=2;a.ai.action=15;a.routePhase=1;c.bindings[0]=cs::model::CommandBehaviorBinding::move;
  if(scenario==3){ // Allied blocker on a retained regional leg.
    auto neighbor=a.ai.position;neighbor.x+=2;c.world.creatures[1].ai.position=neighbor;c.world.creatures[1].destination=neighbor;c.execution[1].roamHome=neighbor;
    auto &route=c.actions.segments[0];route.count=1;route.route[0].position={neighbor.x-1,neighbor.y,neighbor.z};route.route[0].remainder={2,0,0,0};a.routePhase=2;a.ai.marker=0;
    route.regionPending=true;route.regionPath.fill(-1);route.regionPath[0]=int(source);route.regionPath[1]=int(target);route.regionCursor=1;c.native.routeMetadata.resize(2);c.native.routeMetadata[0].regionDestination=a.destination;
  }
  c.native.route.useRegions=true;cs::prepareNativeContinuation(c,p.native);
  return p;
}
inline Profile profile(const std::filesystem::path &install,unsigned scenario){return profile(loadInputs(install,{0,0,32,32},0),scenario);}
inline paired::Projection projection(const cs::Continuation &c,const Profile &p){auto out=paired::project(c,p,false);
  const auto add=[&](std::string name,unsigned value){out.push_back({std::move(name),value});};
  for(unsigned i=0;i<c.actions.segments.size();++i){auto &v=c.actions.segments[i];const auto prefix="regional.actor["+std::to_string(i)+"].";add(prefix+"pending",v.regionPending);add(prefix+"cursor",v.regionCursor);for(unsigned j=0;j<50;++j)add(prefix+"path["+std::to_string(j)+"]",unsigned(v.regionPath[j]));const auto m=c.native.routeMetadata.empty()?cs::model::ActorRouteMetadata{}:c.native.routeMetadata[i];add(prefix+"destination.x",m.regionDestination.x);add(prefix+"destination.y",m.regionDestination.y);add(prefix+"destination.z",m.regionDestination.z);}
  for(unsigned i=0;i<c.native.regions.size();++i)add("regional.traffic["+std::to_string(i)+"]",c.native.regions[i].traffic);
  std::uint64_t grid=1469598103934665603ull;for(const auto &v:c.native.cells)for(unsigned word:{unsigned(v.kind),unsigned(v.occupant),unsigned(v.flags),unsigned(v.links)})grid=(grid^word)*1099511628211ull;
  add("regional.grid.low",unsigned(grid));add("regional.grid.high",unsigned(grid>>32));return out;
}
} // namespace mnm::encounter::regional
