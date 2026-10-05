#include "creature_footprint.hpp"
namespace mnm::reconstruction {
CreatureOccupancy initializeCreatureFootprint(std::uint32_t height,
                                             std::uint32_t selector) noexcept {
    CreatureOccupancy result;
    result.height = height;
    if (selector) {
        for (unsigned row = 2; row < 14; ++row) result.rows[row] = 0x3ffc;
    } else {
        result.rows = {0x1800, 0x3c00, 0x7e00, 0xff00, 0xff00, 0x7e00,
                       0x3c00, 0x1800, 0, 0, 0, 0, 0, 0, 0, 0};
    }
    return result;
}
}
