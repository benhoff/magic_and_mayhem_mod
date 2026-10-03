#pragma once
#include "route_cell.hpp"

namespace mnm::reconstruction {
#pragma pack(push, 1)
struct OccupancyCell {
    std::array<std::byte, 4> unknown_00{};
    std::uint16_t occupant_04 = 0xffff;
    std::array<std::byte, 4> unknown_06{};
    std::uint8_t flags_0a = 0;
    std::byte unknown_0b{};
};
#pragma pack(pop)
static_assert(sizeof(OccupancyCell) == 12);
static_assert(offsetof(OccupancyCell, occupant_04) == 4);
static_assert(offsetof(OccupancyCell, flags_0a) == 10);
// Borrowed, stable storage. Tables contain cell offsets, not byte offsets.
// These model receiver +4c, +64b2, +6432, +10 and global 5e1780.
struct OccupancyMapView {
    MapDimensions dimensions;
    const OccupancyCell* cells = nullptr;
    std::size_t cell_count = 0;
    const std::int32_t* row_offsets = nullptr;
    std::size_t row_count = 0;
    const std::int32_t* layer_offsets = nullptr;
    std::size_t layer_offset_count = 0;
    std::int32_t plane_stride = 0;
    std::int32_t layer_count = 0;
};
// 0x4f3550: vertical occupant exclusion, using parameter +0 as scan extent.
bool test_occupancy(Coordinates position, std::int32_t object_00,
    const CreatureMovementParameters& parameters, const OccupancyMapView& map);
CellHelpers with_occupancy_test(CellHelpers cells, OccupancyMapView map);
} // namespace mnm::reconstruction
