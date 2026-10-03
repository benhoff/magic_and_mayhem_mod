#pragma once
#include "route_occupancy.hpp"
#include "route_validity.hpp"

namespace mnm::reconstruction {
#pragma pack(push,1)
struct TerrainValidityRecord {
    std::array<std::byte,0x94> unknown_00{};
    std::int32_t classification_94=0;
    std::array<std::byte,0x18> unknown_98{};
    std::uint8_t flags_b0=0;
    std::array<std::byte,0xb3> unknown_b1{};
};
#pragma pack(pop)
static_assert(sizeof(TerrainValidityRecord)==0x164);
static_assert(offsetof(TerrainValidityRecord,classification_94)==0x94);
static_assert(offsetof(TerrainValidityRecord,flags_b0)==0xb0);
struct CellValidityMapView {
    OccupancyMapView map;
    // Global 65660c: array indexed by the cell's unsigned first WORD.
    const TerrainValidityRecord* terrain=nullptr;
    std::size_t terrain_count=0;
};
// 0x4f3440: base-cell flags/classification followed by overhead clearance.
bool test_cell_validity(Coordinates position,const CreatureMovementParameters& parameters,
    const CellValidityMapView& view);
ValidityHelpers with_cell_validity_test(ValidityHelpers validity,CellValidityMapView view);
} // namespace mnm::reconstruction
