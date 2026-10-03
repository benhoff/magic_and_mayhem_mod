#include "route_clearance.hpp"
#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t next(std::int32_t n,std::int32_t dimension) {
    const auto u=std::uint32_t(n)+1U;
    const auto value=u<=0x7fffffffU?static_cast<std::int32_t>(u):-1-static_cast<std::int32_t>(~u);
    return normalize_coordinate(value,dimension);
}
bool scan(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v,bool tall) {
    const auto& m=v.map;
    if(m.dimensions.x<=0 || m.dimensions.y<=0)
        throw std::invalid_argument("positive global clearance XY dimensions required");
    auto y=pos.y;
    std::int32_t y_steps=0;
    while(wrapped_difference(y,pos.y,m.dimensions.y)<p.type_08) {
        auto x=pos.x;
        std::int32_t x_steps=0;
        while(wrapped_difference(x,pos.x,m.dimensions.x)<p.type_08) {
            if(!m.row_offsets || !m.layer_offsets || y<0 || pos.z<0
                || std::size_t(y)>=m.row_count || std::size_t(pos.z)>=m.layer_offset_count)
                throw std::invalid_argument("global clearance tables do not cover the coordinates");
            const auto index=std::uint32_t(m.row_offsets[y])+std::uint32_t(m.layer_offsets[pos.z])+std::uint32_t(x);
            if(!m.cells || index>=m.cell_count) throw std::out_of_range("global clearance cell unavailable");
            std::uint16_t id; std::memcpy(&id,m.cells[index].unknown_00.data(),2);
            if(id) {
                if(!tall) return true;
                if(!v.terrain || id>=v.terrain_count) throw std::out_of_range("clearance terrain record unavailable");
                if(std::uint32_t(v.terrain[id].classification_94)>=8) return true;
            }
            x=next(x,m.dimensions.x);
            // Original code can repeat indefinitely when no wrapped distance
            // reaches the requested extent. This is a host guard, not a game fix.
            if(++x_steps>=m.dimensions.x && wrapped_difference(x,pos.x,m.dimensions.x)<p.type_08)
                throw std::runtime_error("clearance X scan repeats without reaching its bound");
        }
        y=next(y,m.dimensions.y);
        if(++y_steps>=m.dimensions.y && wrapped_difference(y,pos.y,m.dimensions.y)<p.type_08)
            throw std::runtime_error("clearance Y scan repeats without reaching its bound");
    }
    return false;
}
}
bool test_any_terrain(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v) {
    return scan(pos,p,v,false);
}
bool test_tall_terrain(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v) {
    return scan(pos,p,v,true);
}
MovementHelpers with_clearance_tests(MovementHelpers movement,CellValidityMapView global_map) {
    const auto a=movement.dimensions,b=global_map.map.dimensions;
    if(a.x!=b.x || a.y!=b.y || a.z!=b.z)
        throw std::invalid_argument("movement and global clearance dimensions must match");
    const auto previous=movement.check;
    movement.check=[previous,global_map](MovementCheck kind,Coordinates pos,const CreatureMovementParameters& p) {
        if(kind==MovementCheck::check_44e0) return test_any_terrain(pos,p,global_map);
        if(kind==MovementCheck::check_45b0) return test_tall_terrain(pos,p,global_map);
        if(!previous) throw std::invalid_argument("other movement providers required");
        return previous(kind,pos,p);
    };
    return movement;
}
} // namespace mnm::reconstruction
