#include "regional_navigation.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int bits(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
int add(int a, int b) { return bits(unsigned(a) + unsigned(b)); }
int mul(int a, unsigned b) { return bits(unsigned(a) * b); }
int sub(int a, int b) { return bits(unsigned(a) - unsigned(b)); }
const NavigationRegion &region(const std::vector<NavigationRegion> &g, int n) {
  if (n < 0) throw std::invalid_argument("negative navigation region");
  return g.at(unsigned(n));
}
int norm(int n, int d) { if(d<=0)throw std::invalid_argument("regional dimension");n%=d;return n<0?n+d:n; }
int separation(int a,int b,int d) { int v=std::abs(a-b);return v>d/2?d-v:v; }
}
int regionEdgeDistance(const std::vector<NavigationRegion> &g,int from,int to) {
  for(const auto &v:region(g,from).connections)if(v.cost&&v.destination==to)return v.cost;
  return 0;
}
int regionHeuristic(Cell a,Cell b,Extent d) {
  return mul(distance_metric(2*wrapped_difference(a.x,b.x,d.x),
                             2*wrapped_difference(a.y,b.y,d.y),a.z-b.z),4);
}
int regionLowerBound(const std::vector<NavigationRegion> &g,int x) {
  if(g.empty())throw std::invalid_argument("empty regional index");
  int i=int(g.size())/2, step=(i+1)/2;
  while(step>1){i=std::clamp(i+(g.at(unsigned(i)).position.x<x?step:-step),0,int(g.size())-1);step=(step+1)/2;}
  if(g.at(unsigned(i)).position.x<x){while(i<int(g.size())-1&&g.at(unsigned(i)).position.x<x)++i;}
  else {while(i>0&&g.at(unsigned(i-1)).position.x>=x)--i;}
  return std::clamp(i,0,int(g.size())-1);
}
bool regionConnectionAdmitted(const RegionConnection &c,const CreatureMovementParameters &p) {
  if(!c.cost)return false;
  if(p.type_08<0||p.type_08>2)throw std::invalid_argument("regional footprint width");
  const auto cap=[&](unsigned base){return c.clearance.at(base)!=0&&int(c.clearance.at(base+unsigned(p.type_08)))>=p.type_0c;};
  if(cap(0)||(p.object_104&&cap(3)))return true;
  if(p.type_44==1)return cap(12)&&p.type_0c>=int(c.clearance.at(14+unsigned(p.type_08)));
  if(p.type_44==2)return cap(6);
  return p.type_44==3&&cap(9);
}
int regionDanger(const AutonomousCreature &a,const NavigationRegion &r,std::array<unsigned,2> powers,bool subtract) {
  if(a.ai.owner<0||a.ai.owner>=8)throw std::invalid_argument("regional danger owner");
  const auto &v=r.danger.at(unsigned(a.ai.owner));const auto &old=r.deductedDanger.at(unsigned(a.ai.owner));
  unsigned i=a.flag104&&r.parameter?1:0;int result=v[i]-(subtract?old[i]:0);
  if(!a.ai.physical)result+=v[2]-(subtract?old[2]:0);
  for(unsigned j:{3u,4u})result+=v[j]-(subtract?old[j]:0);
  for(unsigned j=5;j<7;++j){
    if(v[j]){const unsigned denominator=unsigned(a.ai.bravery)+powers[j-5];if(!denominator)throw std::invalid_argument("regional danger division");const auto ratio=powers[j-5]*100u/denominator;result=add(result,bits(ratio*unsigned(v[j]))/100);}
    if(subtract)result-=old[j];
  }
  return add(result,r.dangerBase);
}
int lookupNavigationRegion(const std::vector<NavigationRegion> &g,Cell p,const RegionalQuery &q,Extent d,int boundary,int hx,int hy,const RegionalServices &s) {
  if(g.empty())return -1;
  if(!s.legal||!s.line||!s.boundedRoute)throw std::invalid_argument("regional lookup services");
  const int lo=regionLowerBound(g,norm(p.x-4,d.x)), hi=regionLowerBound(g,norm(p.x+5,d.x));
  std::vector<std::pair<int,int>> candidates;
  auto scan=[&](int a,int b,bool wrapped){for(int i=a;i<=b&&candidates.size()<15;++i){const auto v=g.at(unsigned(i)).position;
    const auto metric=[&](Cell from){int x=separation(v.x,from.x,d.x),y=separation(v.y,from.y,d.y);return x>y?2*x+y:x+2*y+(wrapped?0:std::abs(v.z-p.z)/2);};
    if(metric(p)>10)continue;
    const int score=hx||hy?metric({norm(p.x+hx,d.x),norm(p.y+hy,d.y),p.z}):metric(p);
    if(s.legal(v))candidates.emplace_back(i,score);
  }};
  scan(lo,hi>lo?hi:int(g.size())-1,false);
  if(hi<lo)scan(0,hi,true);
  // Original qsort tie ordering is reproduced below by its maximum-selection
  // shortsort (lookup admits at most15; equal-score cases are oracle checked).
  const auto shortsort=[&](int lo,int hi){while(hi>lo){int max=lo;for(int j=lo+1;j<=hi;++j)if(candidates[j].second>candidates[max].second)max=j;std::swap(candidates[max],candidates[hi--]);}};
  // MSVC's qsort uses shortsort for <=8 records and a middle-pivot partition
  // above that threshold. Smaller partitions are processed first.
  std::function<void(int,int)> sort=[&](int lo,int hi){if(hi-lo+1<=8){shortsort(lo,hi);return;}int mid=lo+(hi-lo+1)/2;std::swap(candidates[mid],candidates[lo]);int a=lo,b=hi+1;
    for(;;){do{++a;}while(a<=hi&&candidates[a].second<=candidates[lo].second);do{--b;}while(b>lo&&candidates[b].second>=candidates[lo].second);if(b<a)break;std::swap(candidates[a],candidates[b]);}
    std::swap(candidates[lo],candidates[b]);if(lo<b-1)sort(lo,b-1);if(a<hi)sort(a,hi);
  };
  if(candidates.size()>1)sort(0,int(candidates.size())-1);
  const auto endpoint=[&](int id){auto v=g.at(unsigned(id)).position;if(q.movement.type_44==2&&v.z<boundary)v.z=boundary-q.movement.type_0c;return v;};
  for(auto [id,score]:candidates){(void)score;if(s.line(p,endpoint(id)))return id;}
  for(auto [id,score]:candidates){(void)score;if(s.boundedRoute(p,endpoint(id)))return id;}
  return -1;
}
bool searchNavigationRegions(std::vector<NavigationRegion> &g,int start,int end,const RegionalQuery &q,Extent d,std::array<int,50> &path,int &budget,bool reset,RegionalSearchState &s,const RegionalServices &services) {
  region(g,start);region(g,end);
  if(reset)s.dirty=true;
  if(s.dirty){s={};s.dirty=false;s.initialized=true;s.start=start;s.destination=end;s.best=start;s.bestHeuristic=regionHeuristic(g[start].position,g[end].position,d);s.open.push_back({0,start});s.records.push_back({start,0,-1,{}});}
  if(!s.initialized||s.start!=start||s.destination!=end)throw std::invalid_argument("regional search identity");
  auto record=[&](int node)->RegionalSearchRecord &{for(auto &v:s.records)if(v.node==node)return v;s.records.push_back({node,INT32_MAX,-1,{}});return s.records.back();};
  auto heuristic=[&](int node){return regionHeuristic(region(g,node).position,region(g,end).position,d);};
  while(!s.open.empty()){
    const auto front=*std::min_element(s.open.begin(),s.open.end(),[](auto a,auto b){return a.priority<b.priority;});
    if(front.priority>=record(end).priority){s.dirty=true;break;}
    std::vector<Candidate> admitted;
    const auto prior=neighbor_movement(record(front.node).movement);
    if(budget>0){for(const auto &edge:region(g,front.node).connections)if(regionConnectionAdmitted(edge,q.movement)){
      region(g,edge.destination);int cost=edge.cost;NeighborMovement payload;payload.accumulated_cost=add(prior.accumulated_cost,edge.cost);
      if(q.creature){if(!services.danger)throw std::invalid_argument("regional danger service");const int danger=services.danger(edge.destination);payload.unknown_18=add(prior.unknown_18,danger);if(q.dangerPenalty)cost=mul(cost,unsigned(add(mul(danger,10),1)));}
      if(q.trafficPenalty)cost=mul(mul(cost,g.at(unsigned(front.node)).traffic+1),2);
      admitted.push_back({cost,unsigned(edge.destination),movement_payload(payload)});
    }--budget;}
    if(budget<=0)break; // Last expansion is discarded and remains queued.
    auto at=std::find_if(s.open.begin(),s.open.end(),[&](auto v){return v.node==front.node&&v.priority==front.priority;});s.open.erase(at);
    const auto h=heuristic(front.node);
    for(const auto &candidate:admitted){const int destination=int(candidate.node);const auto nh=heuristic(destination),priority=add(add(sub(candidate.edge_cost,h),nh),front.priority);auto &v=record(destination);
      if(priority<v.priority){v.priority=priority;v.predecessor=front.node;v.movement=candidate.movement;s.open.push_back({priority,destination});if(nh<s.bestHeuristic){s.bestHeuristic=nh;s.best=destination;}}
    }
  }
  if(s.open.empty())s.dirty=true;
  std::vector<int> chain;std::set<int> seen;
  for(int node=s.best;;node=record(node).predecessor){if(!seen.insert(node).second)throw std::invalid_argument("regional predecessor cycle");chain.push_back(node);if(node==start)break;}
  std::reverse(chain.begin(),chain.end());
  unsigned n=0;for(int node:chain)if(n<50)path[n++]=node;
  if(n<50)path[n]=-1;
  const bool reached=chain.back()==end;
  if(reached&&q.trafficPenalty)for(unsigned i=0;i<n;++i)++g.at(unsigned(path[i])).traffic;
  return reached;
}
void regionalCandidate(const AutonomousCreature &a,Cell &p,unsigned parameter,Cell desired,bool flight,unsigned clearance,int kind,int boundary,int layers,const std::function<bool(Cell)> &cell) {
  if(flight&&parameter){
    bool raise=false;
    if(unsigned(desired.z)>clearance){unsigned z=unsigned(desired.z),bottom=z-clearance;for(;z>bottom;--z)if(!cell({desired.x,desired.y,bits(z)})||bits(z)<=0)break;raise=z<=bottom;}
    if(raise){const int initial=p.z;while(unsigned(p.z+1)<clearance+unsigned(initial)&&p.z+1<layers){if(!cell({p.x,p.y,p.z+1}))break;++p.z;}}
    else if(p.z<a.ai.position.z&&cell({p.x,p.y,a.ai.position.z}))p.z=a.ai.position.z;
  }
  if(kind==2&&p.z<boundary)p.z=boundary-int(a.height);
}
} // namespace mnm::reconstruction::combat
