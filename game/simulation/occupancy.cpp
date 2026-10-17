#include "occupancy.hpp"
#include <stdexcept>
#include <algorithm>

namespace mnm::game {
std::size_t Occupancy::index(Point p) const {
    if(p.x<0 || p.y<0 || p.z<0 || p.x>=dimensions_.x || p.y>=dimensions_.y || p.z>=dimensions_.z)
        throw std::invalid_argument("occupancy coordinate outside map");
    return (std::size_t(p.z)*dimensions_.y+p.y)*dimensions_.x+p.x;
}
namespace {
FootprintResolver singleProfile(std::uint32_t type,Point footprint) {
    return [=](const Entity& e) {
        if(e.type!=type) throw std::invalid_argument("occupant profile mismatch");
        return footprint;
    };
}
void validateFootprint(Point footprint) {
    if(footprint.x<1 || footprint.x>2 || footprint.y!=footprint.x || footprint.z<1 || footprint.z>32)
        throw std::invalid_argument("occupancy profile outside bounded policy");
}
}
Occupancy::Occupancy(const State& state,Point d,std::uint32_t type,Point footprint)
    :Occupancy(state,d,singleProfile(type,footprint)) {validateFootprint(footprint);}
Occupancy::Occupancy(const State& state,Point d,const FootprintResolver& resolve):dimensions_(d) {
    if(d.x<1 || d.x>1024 || d.y<1 || d.y>1024 || d.z<1 || d.z>32 ||
       std::uint64_t(d.x)*d.y*d.z>4096 || !resolve)
        throw std::invalid_argument("occupancy dimensions/profile outside bounded policy");
    cells_.resize(std::size_t(d.x)*d.y*d.z);
    for(std::uint32_t i=0;i<state.slots.size();++i) {
        const auto& slot=state.slots[i];
        if(!slot.entity || slot.entity->family!=Family::creature || slot.entity->cleaned) continue;
        const auto& e=*slot.entity;
        const auto footprint=resolve(e);validateFootprint(footprint);
        if(i>=65535 || slot.generation==0 || e.x<0 || e.y<0 || e.z<0 ||
           e.x>d.x-footprint.x || e.y>d.y-footprint.y || e.z>d.z-footprint.z)
            throw std::invalid_argument("occupant profile/identity/extent outside bounded policy");
        for(int z=0;z<footprint.z;++z) for(int y=0;y<footprint.y;++y) for(int x=0;x<footprint.x;++x) {
            auto& cell=cells_.at(index({e.x+x,e.y+y,e.z+z}));
            if(cell) throw std::invalid_argument("overlapping logical creature footprints");
            cell=Handle{i,slot.generation};
        }
    }
}
std::optional<Handle> Occupancy::owner(Point p) const {return cells_.at(index(p));}
bool Occupancy::blocked(Point p,Handle self) const {const auto h=owner(p);return h && !(*h==self);}
}

namespace mnm::game {
std::size_t MovementReservations::index(Point p) const {
    (void)logical_.owner(p); // Shared bounds guard.
    return (std::size_t(p.z)*dimensions_.y+p.y)*dimensions_.x+p.x;
}
MovementReservations::MovementReservations(const State& state,Point d,std::uint32_t type,Point footprint)
    :MovementReservations(state,d,singleProfile(type,footprint)) {validateFootprint(footprint);}
MovementReservations::MovementReservations(const State& state,Point d,const FootprintResolver& resolve)
    :logical_(state,d,resolve),dimensions_(d),claims_(std::size_t(d.x)*d.y*d.z) {
    for(std::uint32_t i=0;i<state.slots.size();++i) {
        const auto& slot=state.slots[i];
        if(slot.entity && !slot.entity->cleaned && slot.entity->family==Family::creature) {
            origins_.emplace(i,std::make_pair(Handle{i,slot.generation},Point{slot.entity->x,slot.entity->y,slot.entity->z}));
            footprints_.emplace(i,resolve(*slot.entity));
        }
    }
    for(std::uint32_t i=0;i<state.slots.size();++i) {
        const auto& slot=state.slots[i];
        if(!slot.entity || slot.entity->cleaned || !slot.entity->motion || !slot.entity->motion->fine) continue;
        const auto& e=*slot.entity;const auto& m=*e.motion;
        if(e.family!=Family::creature || m.action!=Action::moving || m.next>=m.route.size() ||
           !tryReserve({i,slot.generation},{e.x,e.y,e.z},m.route.at(m.next).position))
            throw std::invalid_argument("overlapping or malformed saved movement reservations");
    }
}
std::optional<Handle> MovementReservations::owner(Point p) const {
    const auto claim=claims_.at(index(p));return claim?claim:logical_.owner(p);
}
bool MovementReservations::tryReserve(Handle self,Point from,Point to) {
    const auto identity=origins_.find(self.slot);
    const auto source=logical_.owner(from);
    if(identity==origins_.end() || !(identity->second.first==self) || identity->second.second!=from || !source || !(*source==self)) throw std::invalid_argument("reservation actor identity mismatch");
    const auto near=[](int a,int b){const auto d=std::int64_t(a)-b;return d>=-1 && d<=1;};
    if(from==to || !near(from.x,to.x) || !near(from.y,to.y) || !near(from.z,to.z)) return false;
    const Point low{std::min(from.x,to.x),std::min(from.y,to.y),std::min(from.z,to.z)};
    const auto footprint=footprints_.at(self.slot);
    const Point high{std::max(from.x,to.x)+footprint.x,std::max(from.y,to.y)+footprint.y,std::max(from.z,to.z)+footprint.z};
    if(low.x<0 || low.y<0 || low.z<0 || high.x>dimensions_.x || high.y>dimensions_.y || high.z>dimensions_.z) return false;
    for(int z=low.z;z<high.z;++z) for(int y=low.y;y<high.y;++y) for(int x=low.x;x<high.x;++x) {
        const auto h=owner({x,y,z});if(h && !(*h==self)) return false;
    }
    for(int z=low.z;z<high.z;++z) for(int y=low.y;y<high.y;++y) for(int x=low.x;x<high.x;++x)
        claims_.at(index({x,y,z}))=self;
    return true;
}
}
