#include "route_creature_acceptance.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
namespace {
std::int32_t signed_bits(std::uint32_t n) {
    return n <= 0x7fffffffU ? static_cast<std::int32_t>(n)
        : -1 - static_cast<std::int32_t>(~n);
}
std::int32_t move_squared(Coordinates from, Coordinates to, MapDimensions dimensions) {
    const auto dx = static_cast<std::uint32_t>(wrapped_difference(to.x, from.x, dimensions.x));
    const auto dy = static_cast<std::uint32_t>(wrapped_difference(to.y, from.y, dimensions.y));
    const auto dz = static_cast<std::uint32_t>(to.z) - static_cast<std::uint32_t>(from.z);
    return signed_bits(dx * dx + dy * dy + dz * dz);
}
CreatureAcceptanceState object_state(std::uint32_t object, const CreatureAcceptanceHelpers& h) {
    if (!h.object_state) throw std::invalid_argument("creature object-state provider required");
    return h.object_state(object);
}
} // namespace

CreatureMovementParameters creature_movement_parameters(const CreatureAcceptanceState& s) {
    return {s.type_0c, s.type_08, s.object_104, s.type_44, 0, s.type_index,
        static_cast<std::uint8_t>(s.object_723 != 0 || s.object_5ec == 14)};
}

bool test_creature_cell(std::uint32_t object, Coordinates position,
    const CreatureAcceptanceHelpers& h) {
    // 0x4f5a40..0x4f5ae0: reread the object for every occupancy invocation.
    const auto state = object_state(object, h);
    if (!h.cell_test) throw std::invalid_argument("0x4f47d0 cell-test provider required");
    return h.cell_test(position, state.object_00, creature_movement_parameters(state));
}

bool accept_creature_move(std::uint32_t object, Coordinates from, Coordinates to,
    bool special, std::int32_t& category, const CreatureAcceptanceHelpers& h) {
    // 0x514369..0x5143b0: DWORD-wrapped squared distance, signed comparison later.
    const auto squared = move_squared(from, to, h.dimensions);
    // 0x5143b2..0x514477: parameter construction and base movement test.
    const auto state = object_state(object, h);
    if (!h.movement_test) throw std::invalid_argument("0x4f3990 movement-test provider required");
    const auto reject = [&category] { category = 5; return false; };
    if (!h.movement_test(from, to, creature_movement_parameters(state), category, -1))
        return reject();
    // 0x514490: only the low-byte flag matters to the original method.
    if (special) return true;
    // 0x51449c..0x5144d7: destination check precedes type/distance exemptions.
    if (!test_creature_cell(object, to, h)) return reject();
    // 0x5144da: the record is reread after the destination callback.
    if (object_state(object, h).type_08 == 2 || squared <= 1) return true;
    // 0x5144f3..0x514579: first AND group at the source Z plane.
    if (test_creature_cell(object, {to.x, from.y, from.z}, h)
        && test_creature_cell(object, {from.x, to.y, from.z}, h)
        && test_creature_cell(object, {to.x, to.y, from.z}, h)) return true;
    // 0x514581..0x514609: alternate AND group at the destination Z plane.
    if (test_creature_cell(object, {from.x, from.y, to.z}, h)
        && test_creature_cell(object, {to.x, from.y, to.z}, h)
        && test_creature_cell(object, {from.x, to.y, to.z}, h)) return true;
    return reject(); // 0x51460b: neither group passed, overwrite category with 5.
}

NeighborHelpers with_creature_acceptance(NeighborHelpers neighbors,
    CreatureAcceptanceHelpers acceptance) {
    if (neighbors.dimensions.x != acceptance.dimensions.x
        || neighbors.dimensions.y != acceptance.dimensions.y
        || neighbors.dimensions.z != acceptance.dimensions.z)
        throw std::invalid_argument("generator and acceptance dimensions must match");
    const auto other_path = neighbors.accept;
    neighbors.accept = [other_path, acceptance = std::move(acceptance)](
        const NeighborDescriptor& desc, Coordinates from, Coordinates to,
        const NeighborRecord& record, bool special, std::int32_t& category) {
        if (!desc.mode) return accept_creature_move(desc.object, from, to, special, category, acceptance);
        if (!other_path) throw std::invalid_argument("other standard acceptance provider required");
        return other_path(desc, from, to, record, special, category);
    };
    return neighbors;
}
} // namespace mnm::reconstruction
