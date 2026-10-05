#include "occupancy.hpp"
#include <stdexcept>

namespace mnm::game {
std::size_t Occupancy::index(Point p) const {
    if(p.x<0 || p.y<0 || p.z<0 || p.x>=dimensions_.x || p.y>=dimensions_.y || p.z>=dimensions_.z)
        throw std::invalid_argument("occupancy coordinate outside map");
    return (std::size_t(p.z)*dimensions_.y+p.y)*dimensions_.x+p.x;
}
Occupancy::Occupancy(const State& state,Point d,std::uint32_t type,Point footprint):dimensions_(d) {
    if(d.x<1 || d.x>1024 || d.y<1 || d.y>1024 || d.z<1 || d.z>32 ||
       std::uint64_t(d.x)*d.y*d.z>4096 || footprint.x<1 || footprint.x>2 ||
       footprint.y!=footprint.x || footprint.z<1 || footprint.z>32)
        throw std::invalid_argument("occupancy dimensions/profile outside bounded policy");
    cells_.resize(std::size_t(d.x)*d.y*d.z);
    for(std::uint32_t i=0;i<state.slots.size();++i) {
        const auto& slot=state.slots[i];
        if(!slot.entity || slot.entity->family!=Family::creature || slot.entity->cleaned) continue;
        const auto& e=*slot.entity;
        if(e.type!=type || i>=65535 || slot.generation==0 || e.x<0 || e.y<0 || e.z<0 ||
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
