#include "route_boundary.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t bits(std::uint32_t n) { return n<=0x7fffffffU?static_cast<std::int32_t>(n):-1-static_cast<std::int32_t>(~n); }
std::int32_t add(std::int32_t a,int b) { return bits(std::uint32_t(a)+std::uint32_t(b)); }
std::int32_t sub(std::int32_t a,std::int32_t b) { return bits(std::uint32_t(a)-std::uint32_t(b)); }
}
bool test_boundary(Coordinates pos,const CreatureMovementParameters& p,const BoundaryHelpers& h) {
    const auto mode=std::uint32_t(p.type_44); // 0x4f41a9, before boundary load
    if(!h.boundary) throw std::invalid_argument("boundary provider required");
    const auto boundary=h.boundary();
    const auto support=[&] {
        if(!h.support) throw std::invalid_argument("boundary support provider required");
        return h.support(pos,p);
    };
    switch(mode) {
    case 0: return pos.z<boundary;
    case 1: {
        const auto lower=sub(boundary,p.type_0c);
        if(pos.z<add(lower,1)) return true;
        if(pos.z<=lower || pos.z>=boundary) return false;
        if(support()) return false;
        return p.object_104==0; // read after support
    }
    case 2: {
        const auto lower=sub(boundary,p.type_0c);
        if(pos.z<lower) return true;
        if(pos.z>lower && pos.z<boundary && !support()) return true;
        // 0x4f4285: reread extent even if the earlier support callback changed it.
        if(pos.z!=sub(boundary,p.type_0c)) return false;
        if(!h.valid) throw std::invalid_argument("boundary validity provider required");
        return !h.valid({pos.x,pos.y,add(pos.z,1)},p); // EAX is tested at 4f4222
    }
    case 3:
        if(pos.z>=boundary) return false;
        if(support()) return false;
        return p.object_104==0;
    default: return false; // unsigned mode >3, including negative values
    }
}
MovementHelpers with_boundary_test(MovementHelpers movement) {
    const auto previous=movement.check;
    BoundaryHelpers h{movement.boundary,movement.record,
        [previous](Coordinates pos,const CreatureMovementParameters& p) {
            if(!previous) throw std::invalid_argument("movement validity provider required");
            return previous(MovementCheck::check_46b0,pos,p);
        }};
    movement.check=[previous,h](MovementCheck kind,Coordinates pos,const CreatureMovementParameters& p) {
        if(kind==MovementCheck::check_41a0) return test_boundary(pos,p,h);
        if(!previous) throw std::invalid_argument("other movement check providers required");
        return previous(kind,pos,p);
    };
    return movement;
}
} // namespace mnm::reconstruction
