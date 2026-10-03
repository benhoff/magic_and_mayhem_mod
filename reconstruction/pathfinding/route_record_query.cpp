#include "route_record_query.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
namespace {
std::int32_t bits(std::uint32_t n) {
    return n<=0x7fffffffU?static_cast<std::int32_t>(n):-1-static_cast<std::int32_t>(~n);
}
std::int32_t add(std::int32_t a,int b) { return bits(std::uint32_t(a)+std::uint32_t(b)); }
}
std::uint32_t query_record(Coordinates pos,const CreatureMovementParameters& p,const RecordQueryHelpers& h) {
    if (!h.query) throw std::invalid_argument("lower support query required");
    if (p.type_08==1) return h.query(pos,p); // 0x4f4375: preserve the whole EAX result
    if (h.dimensions.x<=0 || h.dimensions.y<=0)
        throw std::invalid_argument("positive support query XY dimensions required");
    int count=0,sum_x=0,sum_y=0;
    bool requested_level_hit=false;
    const auto lower=add(pos.z,-1);
    // 0x4f439f: Y outer, X inner. Each column visits Z then (at most) Z-1.
    for(int y=0;y<2;++y) for(int x=0;x<2;++x) {
        for(auto z=pos.z;z>=lower && z>=0;z=add(z,-1)) {
            const Coordinates q{normalize_coordinate(add(pos.x,x),h.dimensions.x),
                normalize_coordinate(add(pos.y,y),h.dimensions.y),z};
            if (h.query(q,p)) {
                ++count; sum_x+=x; sum_y+=y;
                if(z==pos.z) requested_level_hit=true;
                break;
            }
            if(p.type_44==2) {
                if (!h.boundary) throw std::invalid_argument("support boundary provider required");
                const auto boundary=h.boundary();
                if(z>=boundary && z<add(boundary,2)) {
                    ++count; sum_x+=x; sum_y+=y;
                    break; // 0x4f4458: does not set requested-level hit
                }
            }
        }
    }
    const auto abs=[](int n) { return n<0?-n:n; };
    return abs((2*sum_x-count)*2)<count && abs((2*sum_y-count)*2)<count
        && requested_level_hit ? 1U:0U;
}
MovementHelpers with_record_query(MovementHelpers movement,RecordQueryHelpers query) {
    const auto a=movement.dimensions,b=query.dimensions;
    if(a.x!=b.x || a.y!=b.y || a.z!=b.z)
        throw std::invalid_argument("movement and support query dimensions must match");
    movement.record=[query=std::move(query)](Coordinates pos,const CreatureMovementParameters& p) {
        return query_record(pos,p,query);
    };
    return movement;
}
} // namespace mnm::reconstruction
