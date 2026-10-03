#include "route_occupancy.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t signed_bits(std::uint32_t n) {
    return n <= 0x7fffffffU ? static_cast<std::int32_t>(n) : -1 - static_cast<std::int32_t>(~n);
}
}
bool test_occupancy(Coordinates pos, std::int32_t object, const CreatureMovementParameters& p,
    const OccupancyMapView& map) {
    // The original forms the starting cell address even when the scan is empty.
    if (!map.row_offsets || !map.layer_offsets || pos.y < 0 || pos.z < 0
        || std::size_t(pos.y) >= map.row_count || std::size_t(pos.z) >= map.layer_offset_count)
        throw std::invalid_argument("occupancy offset tables do not cover the starting coordinates");
    auto index = std::uint32_t(map.row_offsets[pos.y]) + std::uint32_t(map.layer_offsets[pos.z])
        + std::uint32_t(pos.x);
    const auto end = signed_bits(std::uint32_t(pos.z) + std::uint32_t(p.type_0c)); // 0x4f357f
    for (auto z = pos.z; z < end && z < map.layer_count; ++z) {
        if (!map.cells || index >= map.cell_count)
            throw std::out_of_range("occupancy cell storage does not cover the scan");
        const auto& cell = map.cells[index];
        if ((cell.flags_0a & 3) && cell.occupant_04 != 0xffff
            && std::int32_t(cell.occupant_04) != object) return false;
        index += std::uint32_t(map.plane_stride); // 0x4f35b0..0x4f35b6
    }
    return true;
}
CellHelpers with_occupancy_test(CellHelpers cells, OccupancyMapView map) {
    const auto a = cells.dimensions, b = map.dimensions;
    if (a.x != b.x || a.y != b.y || a.z != b.z)
        throw std::invalid_argument("footprint and occupancy dimensions must match");
    cells.check = [map](Coordinates pos, std::int32_t object, const CreatureMovementParameters& p) {
        return test_occupancy(pos, object, p, map);
    };
    return cells;
}
} // namespace mnm::reconstruction
