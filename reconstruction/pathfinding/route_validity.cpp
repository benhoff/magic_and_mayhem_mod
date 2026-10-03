#include "route_validity.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
bool test_validity(Coordinates pos, const CreatureMovementParameters& p, const ValidityHelpers& h) {
    if (!h.check) throw std::invalid_argument("lower validity provider required");
    // 46b0 and 47d0 have identical traversal, wrapping, and parameter reread.
    // The synthetic object argument stays inside the host adapter; 3440 has none.
    return test_cell(pos, 0, p, CellHelpers{h.dimensions,
        [&h](Coordinates q, std::int32_t, const CreatureMovementParameters& parameters) {
            return h.check(q, parameters);
        }});
}
MovementHelpers with_validity_test(MovementHelpers movement, ValidityHelpers validity) {
    const auto a = movement.dimensions, b = validity.dimensions;
    if (a.x != b.x || a.y != b.y || a.z != b.z)
        throw std::invalid_argument("movement and validity dimensions must match");
    const auto previous = movement.check;
    movement.check = [previous, validity = std::move(validity)](MovementCheck kind, Coordinates pos,
        const CreatureMovementParameters& p) {
        if (kind == MovementCheck::check_46b0) return test_validity(pos,p,validity);
        if (!previous) throw std::invalid_argument("other movement check providers required");
        return previous(kind,pos,p);
    };
    return movement;
}
} // namespace mnm::reconstruction
