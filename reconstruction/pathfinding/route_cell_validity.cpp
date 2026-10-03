#include "route_cell_validity.hpp"
#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t signed_bits(std::uint32_t n) {
    return n<=0x7fffffffU ? static_cast<std::int32_t>(n) : -1-static_cast<std::int32_t>(~n);
}
std::uint16_t terrain_id(const OccupancyCell& c) {
    std::uint16_t value; std::memcpy(&value,c.unknown_00.data(),sizeof(value)); return value;
}
std::uint16_t flags(const OccupancyCell& c) {
    return std::uint16_t(c.flags_0a) | (std::uint16_t(std::to_integer<unsigned>(c.unknown_0b))<<8);
}
}
bool test_cell_validity(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v) {
    const auto& m=v.map;
    if (!m.row_offsets || !m.layer_offsets || pos.y<0 || pos.z<0
        || std::size_t(pos.y)>=m.row_count || std::size_t(pos.z)>=m.layer_offset_count)
        throw std::invalid_argument("validity offset tables do not cover the starting coordinates");
    auto index=std::uint32_t(m.row_offsets[pos.y])+std::uint32_t(m.layer_offsets[pos.z])+std::uint32_t(pos.x);
    const auto cell=[&](std::uint32_t i)->const OccupancyCell& {
        if (!m.cells || i>=m.cell_count) throw std::out_of_range("validity cell storage does not cover the scan");
        return m.cells[i];
    };
    const auto& base=cell(index);
    const auto base_flags=flags(base);
    if (base_flags&0x4000) return false; // 0x4f3472
    const auto id=terrain_id(base);
    std::int32_t classification=0;
    if (id && !(base_flags&0x80)) {
        if (!v.terrain || id>=v.terrain_count) throw std::out_of_range("validity terrain record unavailable");
        classification=v.terrain[id].classification_94;
        if (classification==16) return false; // 0x4f34aa
    }
    const auto special=p.unknown_10; // cached once at 0x4f34bc
    const auto restricted=[special](std::uint16_t f) {
        return !special && (f&0x30) && !(f&0x10);
    };
    if (restricted(base_flags)) return false;
    const auto extent=std::uint32_t(classification)>=8 && p.type_0c==1 ? 2 : p.type_0c;
    const auto end=signed_bits(std::uint32_t(pos.z)+std::uint32_t(extent));
    if (end>m.layer_count) return false; // no clipping in this routine
    index+=std::uint32_t(m.plane_stride);
    for (auto z=signed_bits(std::uint32_t(pos.z)+1); z<end; ++z) {
        const auto& above=cell(index);
        const auto f=flags(above);
        if ((f&0x4000) || (terrain_id(above) && !(f&0x80)) || restricted(f)) return false;
        index+=std::uint32_t(m.plane_stride);
    }
    return true;
}
ValidityHelpers with_cell_validity_test(ValidityHelpers validity,CellValidityMapView view) {
    const auto a=validity.dimensions,b=view.map.dimensions;
    if (a.x!=b.x || a.y!=b.y || a.z!=b.z)
        throw std::invalid_argument("validity and cell-map dimensions must match");
    validity.check=[view](Coordinates pos,const CreatureMovementParameters& p) {
        return test_cell_validity(pos,p,view);
    };
    return validity;
}
} // namespace mnm::reconstruction
