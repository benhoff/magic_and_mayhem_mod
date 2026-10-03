#include "route_cell.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
namespace {
std::int32_t next(std::int32_t value, std::int32_t dimension) {
    // INC is a DWORD operation; the constructors then normalize signed XY.
    const auto n = std::uint32_t(value) + 1U;
    const auto signed_n = n <= 0x7fffffffU ? static_cast<std::int32_t>(n)
        : -1 - static_cast<std::int32_t>(~n);
    const auto remainder = signed_n % dimension;
    return remainder < 0 ? remainder + dimension : remainder;
}
}
bool test_cell(Coordinates pos, std::int32_t object, const CreatureMovementParameters& p,
    const CellHelpers& h) {
    if (!h.check || h.dimensions.x <= 0 || h.dimensions.y <= 0)
        throw std::invalid_argument("cell provider and positive XY dimensions required");
    if (!h.check(pos, object, p)) return false; // 0x4f4808
    if (p.type_08 != 2) return true;           // 0x4f4816, after first call
    const auto y = next(pos.y, h.dimensions.y);
    if (!h.check({pos.x,y,pos.z}, object, p)) return false; // 0x4f4854
    const auto x = next(pos.x, h.dimensions.x);
    if (!h.check({x,pos.y,pos.z}, object, p)) return false; // 0x4f4896
    return h.check({x,y,pos.z}, object, p);                // 0x4f48de
}
CreatureAcceptanceHelpers with_cell_test(CreatureAcceptanceHelpers acceptance, CellHelpers cells) {
    const auto a = acceptance.dimensions, b = cells.dimensions;
    if (a.x != b.x || a.y != b.y || a.z != b.z)
        throw std::invalid_argument("acceptance and cell dimensions must match");
    acceptance.cell_test = [cells = std::move(cells)](Coordinates pos, std::int32_t object,
        const CreatureMovementParameters& p) { return test_cell(pos, object, p, cells); };
    return acceptance;
}
} // namespace mnm::reconstruction
