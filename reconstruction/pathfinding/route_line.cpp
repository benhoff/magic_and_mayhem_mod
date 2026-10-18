#include "route_line.hpp"
#include <optional>
#include <stdexcept>
namespace mnm::reconstruction {
bool test_creature_line(Coordinates from, Coordinates target,
    const CreatureMovementParameters &parameters, const CreatureLineServices &helpers) {
  const auto d=helpers.dimensions;
  auto distance=[&](Coordinates p) {
    auto abs=[](int n){return n<0?-n:n;};
    const int x=abs(p.x-target.x),y=abs(p.y-target.y);
    return distance_metric(x>d.x/2?d.x-x:x,y>d.y/2?d.y-y:y,abs(p.z-target.z));
  };
  auto admitted=[&](Coordinates p){return p.x>=0&&p.x<d.x&&p.y>=0&&p.y<d.y&&p.z>=0&&p.z<d.z;};
  if(d.x<=0||d.x>4096||d.y<=0||d.y>4096||d.z<=0||d.z>256||!admitted(from)||!admitted(target)||!helpers.movement)
    throw std::invalid_argument("bounded creature line coordinates");
  Coordinates previous{-1,-1,-1};int category=-1,best=distance(from);
  auto same=[](Coordinates a,Coordinates b){return a.x==b.x&&a.y==b.y&&a.z==b.z;};
  while(!same(from,target)) {
    std::optional<Coordinates> chosen;
    for(auto candidate:adjacent_cells(from,d)) {
      if(same(candidate,previous))continue;
      const auto next=distance(candidate);if(next>=best)continue;
      bool accepted=helpers.movement(from,candidate,parameters,category,-1);
      if(!accepted && parameters.object_104 && candidate.z==from.z &&
         candidate.z+1<d.z && same(candidate,target)) {
        auto above=candidate;++above.z;
        accepted=helpers.movement(from,above,parameters,category,-1) &&
                 helpers.movement(above,candidate,parameters,category,-1);
      }
      if(accepted){best=next;chosen=candidate;}
    }
    if(!chosen)return false;
    previous=from;from=*chosen;
  }
  return true;
}
} // namespace mnm::reconstruction
