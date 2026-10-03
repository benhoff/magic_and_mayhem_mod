#include "route_cell_support.hpp"
#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::uint16_t terrain_id(const OccupancyCell& c) {
    std::uint16_t id; std::memcpy(&id,c.unknown_00.data(),2); return id;
}
std::uint16_t flags(const OccupancyCell& c) {
    return std::uint16_t(c.flags_0a)|(std::uint16_t(std::to_integer<unsigned>(c.unknown_0b))<<8);
}
}
std::uint32_t test_cell_support(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v) {
    const auto& m=v.map;
    if(!m.row_offsets || !m.layer_offsets || pos.y<0 || pos.z<0
        || std::size_t(pos.y)>=m.row_count || std::size_t(pos.z)>=m.layer_offset_count)
        throw std::invalid_argument("support offset tables do not cover the starting coordinates");
    auto index=std::uint32_t(m.row_offsets[pos.y])+std::uint32_t(m.layer_offsets[pos.z])+std::uint32_t(pos.x);
    const auto cell=[&](std::uint32_t i)->const OccupancyCell& {
        if(!m.cells || i>=m.cell_count) throw std::out_of_range("support cell storage unavailable");
        return m.cells[i];
    };
    const auto terrain=[&](std::uint16_t id)->const TerrainValidityRecord& {
        if(!v.terrain || id>=v.terrain_count) throw std::out_of_range("support terrain record unavailable");
        return v.terrain[id];
    };
    const auto special=p.unknown_10; // cached before either cell is tested
    const auto restricted=[special](std::uint16_t f) { return !special && (f&0x20) && !(f&0x10); };
    const auto eligible=[&](const TerrainValidityRecord& record) {
        return (record.flags_b0&8) || p.type_index==22;
    };
    const auto& current=cell(index);
    const auto f=flags(current);
    if(restricted(f) || (f&0x4000)) return 0;
    const auto id=terrain_id(current);
    if(id && !(f&0x80)) {
        const auto& record=terrain(id);
        return std::uint32_t(record.classification_94)<16 && eligible(record) ? 1U:0U;
    }
    // 0x4f33c1: fallback only for zero index or flag 80, never for failed terrain.
    if(pos.z<=0) return 0;
    index-=std::uint32_t(m.plane_stride);
    const auto& below=cell(index);
    const auto bf=flags(below);
    if(restricted(bf)) return 0;
    if(bf&0x4000) return 1;
    const auto below_id=terrain_id(below);
    if(!below_id || (bf&0x80)) return 0;
    const auto& record=terrain(below_id);
    return record.classification_94==16 && eligible(record) ? 1U:0U;
}
RecordQueryHelpers with_cell_support(RecordQueryHelpers query,CellValidityMapView view) {
    const auto a=query.dimensions,b=view.map.dimensions;
    if(a.x!=b.x || a.y!=b.y || a.z!=b.z)
        throw std::invalid_argument("support query and map dimensions must match");
    query.query=[view](Coordinates pos,const CreatureMovementParameters& p) {
        return test_cell_support(pos,p,view);
    };
    return query;
}
} // namespace mnm::reconstruction
