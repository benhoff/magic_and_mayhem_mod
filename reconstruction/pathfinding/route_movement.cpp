#include "route_movement.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
namespace {
std::int32_t bits(std::uint32_t n) {
    return n <= 0x7fffffffU ? static_cast<std::int32_t>(n) : -1 - static_cast<std::int32_t>(~n);
}
std::int32_t add(std::int32_t a, std::int32_t b) { return bits(std::uint32_t(a) + std::uint32_t(b)); }
std::int32_t sub(std::int32_t a, std::int32_t b) { return bits(std::uint32_t(a) - std::uint32_t(b)); }
bool dimensions_match(MapDimensions a, MapDimensions b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
bool clearance_rejects(Coordinates from, Coordinates to, const CreatureMovementParameters& p,
    const MovementHelpers& h) {
    const auto check = [&](MovementCheck kind, Coordinates pos) { return h.check(kind, pos, p); };
    const auto solid = [&](Coordinates pos) { return check(MovementCheck::check_44e0, pos); };
    const auto tall = [&](Coordinates pos) { return check(MovementCheck::check_45b0, pos); };
    // 0x4f3d09: parameter +0 == 1 selects the extra vertical clearance checks.
    if (p.type_0c != 1) return false;
    if (from.z == to.z) {
        if (from.z >= sub(h.layer_count(), 1)) return false;
        if (tall(from) && solid({to.x, to.y, add(to.z, 1)})) return true;
        return solid({from.x, from.y, add(from.z, 1)}) && tall(to);
    }
    const auto low = from.z < to.z ? from : to;
    const auto high = from.z < to.z ? to : from;
    if (from.x == to.x && from.y == to.y) return solid(high);
    if (solid({from.x, from.y, high.z}) && solid({to.x, to.y, high.z})) return true;
    if (solid({low.x, low.y, add(low.z, 1)}) && tall({high.x, high.y, low.z})) return true;
    return high.z < sub(h.layer_count(), 1)
        && solid({low.x, low.y, add(high.z, 1)}) && tall(high);
}
} // namespace

bool test_movement(Coordinates from, Coordinates to, const CreatureMovementParameters& p,
    std::int32_t& category, std::int32_t override_record, const MovementHelpers& h) {
    category = 5; // 0x4f399e: set even for an early rejection.
    if (p.type_index == 22 && !p.special_state && !p.object_104) return false;
    if (!h.check || !h.record || !h.boundary || !h.layer_count)
        throw std::invalid_argument("movement record, checks, and map-state providers required");
    const auto check = [&](MovementCheck kind, Coordinates pos) { return h.check(kind, pos, p); };
    if (!check(MovementCheck::check_46b0, to)) return false;
    const auto dest_record = h.record(to, p);
    if (!dest_record && to.z >= h.boundary() && p.type_44 != 2 && !p.object_104 && !p.special_state)
        return false;
    const auto dz = sub(to.z, from.z);
    auto source_record = static_cast<std::uint32_t>(override_record);
    if (override_record == -1) {
        if (!check(MovementCheck::check_46b0, from)) return false;
        source_record = h.record(from, p);
    }
    // 0x4f3ae4..0x4f3b64: derive the special transition flag in the boundary band.
    bool transition = false;
    if (p.type_44 == 2) {
        const auto boundary = h.boundary(), lower = sub(boundary, p.type_0c);
        const auto in_band = [&](std::int32_t z) { return z > lower && z <= boundary; };
        if (in_band(from.z) || in_band(to.z)) {
            if (dz > 0 && !source_record) transition = true;
            else if (dz < 0 && !dest_record)
                transition = to.z != lower || check(MovementCheck::check_46b0, {to.x,to.y,add(to.z,1)});
        }
    }
    if (!transition && !check(MovementCheck::check_41a0, from)
        && check(MovementCheck::check_41a0, to)) return false;
    if (p.type_44 == 0) {
        const auto boundary = h.boundary(), lower = sub(boundary, p.type_0c);
        if (from.z > lower && from.z < boundary && dz < 0) return false;
    }
    if (p.type_44 == 2 && from.z < sub(h.boundary(), p.type_0c) && dz <= 0) return false;
    // 0x4f3c18..0x4f3d03: category assignment before late clearance rejection.
    if (source_record) {
        if (dest_record) category = 0;
        else if (to.z >= h.boundary() && !transition) {
            if ((!p.object_104 && !p.special_state) || (dz != 1 && !p.special_state)) return false;
            category = 2;
        } else if (p.type_44 == 2) category = 0;
        else {
            if (!p.object_104) return false;
            category = 1;
        }
    } else {
        if (from.z >= h.boundary() && !transition) {
            if (!p.object_104) return false;
            if (dest_record) {
                if (dz != -1) return false;
                category = 3;
            } else category = 1;
        } else if (dest_record) category = 0;
        else if (p.type_44 == 2) category = 4;
        else {
            if (!p.object_104) return false;
            category = 1;
        }
    }
    const auto reject_late = [&category] { category = 5; return false; };
    if (clearance_rejects(from, to, p, h)) return reject_late();
    // 0x4f3fff: +4 == 1 enables the final mixed-coordinate group checks.
    if (p.type_08 != 1) return true;
    const auto dx = std::uint32_t(wrapped_difference(to.x, from.x, h.dimensions.x));
    const auto dy = std::uint32_t(wrapped_difference(to.y, from.y, h.dimensions.y));
    const auto zbits = std::uint32_t(dz);
    if (bits(dx*dx + dy*dy + zbits*zbits) <= 1) return true;
    const auto valid = [&](Coordinates pos) { return check(MovementCheck::check_46b0, pos); };
    if (valid({to.x,from.y,from.z}) && valid({from.x,to.y,from.z})
        && valid({to.x,to.y,from.z})) return true;
    if (valid({from.x,from.y,to.z}) && valid({to.x,from.y,to.z})
        && valid({from.x,to.y,to.z})) return true;
    return reject_late();
}

CreatureAcceptanceHelpers with_movement_test(CreatureAcceptanceHelpers acceptance, MovementHelpers movement) {
    if (!dimensions_match(acceptance.dimensions, movement.dimensions))
        throw std::invalid_argument("acceptance and movement dimensions must match");
    acceptance.movement_test = [movement = std::move(movement)](Coordinates from, Coordinates to,
        const CreatureMovementParameters& p, std::int32_t& category, std::int32_t record) {
        return test_movement(from, to, p, category, record, movement);
    };
    return acceptance;
}
NeighborHelpers with_standard_movement_test(NeighborHelpers neighbors, MovementHelpers movement) {
    if (!dimensions_match(neighbors.dimensions, movement.dimensions))
        throw std::invalid_argument("generator and movement dimensions must match");
    const auto creature_path = neighbors.accept;
    neighbors.accept = [creature_path, movement = std::move(movement)](const NeighborDescriptor& desc,
        Coordinates from, Coordinates to, const NeighborRecord& record, bool special, std::int32_t& category) {
        if (!desc.mode) {
            if (!creature_path) throw std::invalid_argument("creature acceptance provider required");
            return creature_path(desc, from, to, record, special, category);
        }
        CreatureMovementParameters p;
        std::memcpy(static_cast<void*>(&p), desc.unknown_18.data(), sizeof(p));
        return test_movement(from, to, p, category, bits(record.engine_token), movement);
    };
    return neighbors;
}
} // namespace mnm::reconstruction
