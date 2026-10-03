#include "route_neighbors.hpp"
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <type_traits>

namespace mnm::reconstruction {
namespace {
std::int32_t bits(std::uint32_t n) {
    return n <= 0x7fffffffU ? static_cast<std::int32_t>(n)
        : -1 - static_cast<std::int32_t>(~n);
}
std::int32_t add(std::int32_t a, std::int32_t b) {
    return bits(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}
std::int32_t sub(std::int32_t a, std::int32_t b) {
    return bits(static_cast<std::uint32_t>(a) - static_cast<std::uint32_t>(b));
}
std::int32_t absolute(std::int32_t n) { return n < 0 ? sub(0, n) : n; }
std::int32_t wrap(std::int32_t p, std::int32_t offset, std::int32_t d) {
    auto n = static_cast<std::int64_t>(p) + offset;
    n %= d;
    if (n < 0) n += d;
    return static_cast<std::int32_t>(n);
}
std::int32_t separation(std::int32_t a, std::int32_t b, std::int32_t d) {
    auto n = absolute(sub(a, b));
    return n > d / 2 ? sub(d, n) : n;
}
bool restricted_axis(std::int32_t current, std::int32_t next,
    std::int32_t a, std::int32_t b, std::int32_t dimension) {
    const auto distance = [&](std::int32_t x, std::int32_t y) {
        return dimension ? separation(x, y, dimension) : absolute(sub(x, y));
    };
    const auto span = distance(a, b);
    if (distance(current, a) <= span && distance(current, b) <= span) return true;
    return distance(next, a) <= add(span, 1) && distance(next, b) <= add(span, 1);
}
bool same_xy(Coordinates a, Coordinates b) { return a.x == b.x && a.y == b.y; }
bool same(Coordinates a, Coordinates b) { return same_xy(a, b) && a.z == b.z; }
// __ftol at 0x59bee0 truncates to signed QWORD and returns its low DWORD.
std::int32_t radius_integer(std::int32_t dx, std::int32_t dy) {
    const auto squared = bits(static_cast<std::uint32_t>(dx) * static_cast<std::uint32_t>(dx)
        + static_cast<std::uint32_t>(dy) * static_cast<std::uint32_t>(dy));
    if (squared < 0) return 0; // masked invalid conversion -> QWORD INT_MIN, low DWORD 0
    return static_cast<std::int32_t>(std::sqrt(static_cast<long double>(squared)));
}
} // namespace

NeighborMovement neighbor_movement(const MovementPayload& payload) {
    static_assert(std::is_trivially_copyable_v<NeighborMovement>);
    static_assert(std::is_trivially_copyable_v<MovementPayload>);
    NeighborMovement result;
    std::memcpy(static_cast<void*>(&result), &payload, sizeof(result));
    return result;
}
MovementPayload movement_payload(const NeighborMovement& movement) {
    MovementPayload result;
    std::memcpy(static_cast<void*>(&result), &movement, sizeof(result));
    return result;
}

namespace {
// 0x4ebae0..0x4ebb8e: host map validation, current XYZ, and prior payload.
// The default NeighborMovement initializer represents the zeroed candidate
// payload with category 5 at 0x4ebb38..0x4ebb4c.
struct ExpansionInput { Coordinates from; NeighborMovement prior; };
ExpansionInput prepare_expansion(NodeId current, const MovementPayload& prior,
    const NeighborHelpers& h) {
    if (h.dimensions.x <= 0 || h.dimensions.y <= 0 || h.dimensions.z <= 0
        || !h.coordinates || !h.node_at)
        throw std::invalid_argument("neighbor map callbacks and positive dimensions required");
    return {h.coordinates(current), neighbor_movement(prior)};
}

// 0x4ec4d4..0x4ec74d (standard) / 0x4ebcf1 (six links): append one
// complete 36-byte entry; std::vector owns host allocation and copying.
void append_candidate(std::vector<Candidate>& output, const Candidate& candidate) {
    output.push_back(candidate);
}

// 0x4ebb9b..0x4ebd02: separate six-link expansion path.
void expand_six_links(NodeId current, const ExpansionInput& input,
    const NeighborDescriptor& desc, const NeighborHelpers& h,
    std::vector<Candidate>& result) {
    const auto& from = input.from;
    const auto& prior = input.prior;
    if (desc.float_limit != -1
        && !(static_cast<long double>(prior.accumulated) < static_cast<long double>(desc.float_limit)))
        return;
    if (!h.cell) throw std::invalid_argument("six-link cell callback required");
    // Hash-checked file-backed tables 0x5c70c0..0x5c7137, float 0x5c5388.
    constexpr std::array<Coordinates, 6> offsets{{{-1,0,0},{1,0,0},{0,-1,0},
        {0,1,0},{0,0,-1},{0,0,1}}};
    constexpr std::array<std::uint16_t, 6> masks{{8192,2048,1024,4096,512,256}};
    constexpr std::array<std::int32_t, 6> costs{{2,2,2,2,1,3}};
    const auto links = h.cell(current).links;
    for (std::size_t i = 0; i < offsets.size(); ++i) {
        if (!(links & masks[i])) continue;
        const Coordinates next{wrap(from.x, offsets[i].x, h.dimensions.x),
            wrap(from.y, offsets[i].y, h.dimensions.y), add(from.z, offsets[i].z)};
        // No recovered Z guard here. The map provider decides how to expose
        // an invalid linked cell; no out-of-bounds host memory is accessed.
        if (desc.radius_limit != -1) {
            const auto dx = wrapped_difference(next.x, desc.start.x, h.dimensions.x);
            const auto dy = wrapped_difference(next.y, desc.start.y, h.dimensions.y);
            if (radius_integer(dx, dy) >= desc.radius_limit) continue;
        }
        const auto node = h.node_at(next);
        if (!h.cell(node).first_word) continue;
        NeighborMovement movement;
        movement.accumulated = static_cast<float>(static_cast<long double>(prior.accumulated) + 1.0L);
        movement.accumulated_cost = add(prior.accumulated_cost, costs[i]);
        append_candidate(result, {costs[i], node, movement_payload(movement)});
    }
}

// 0x4ebd07 onward: resolve standard movement record once per expansion.
NeighborRecord prepare_standard_movement(const NeighborDescriptor& desc,
    Coordinates from, const NeighborHelpers& h) {
    if (!h.record || !h.accept)
        throw std::invalid_argument("standard record/accept helpers required");
    return h.record(desc, from);
}

// 0x4ebf84..0x4ec19f: axis restrictions, helper boolean, acceptance/category.
bool accept_standard_neighbor(Coordinates from, Coordinates next,
    const NeighborDescriptor& desc, const NeighborRecord& record,
    const NeighborHelpers& h, std::int32_t& category) {
    if (desc.mode && (!restricted_axis(from.x, next.x, desc.start.x, desc.target.x, h.dimensions.x)
        || !restricted_axis(from.y, next.y, desc.start.y, desc.target.y, h.dimensions.y)
        || !restricted_axis(from.z, next.z, desc.start.z, desc.target.z, 0))) return false;
    bool special = false;
    if (!desc.mode) {
        if (desc.unknown_08) special = true;
        else if (same(next, desc.target) && (from.x == next.x || from.y == next.y)) {
            if (!h.object_d03) throw std::invalid_argument("object d03 helper required");
            special = h.object_d03(desc.object) == 0;
        }
    }
    return h.accept(desc, from, next, record, special, category);
}

// 0x4ec1a7..0x4ec4d0: accepted neighbor's exact cost and movement payload.
Candidate build_standard_candidate(Coordinates from, Coordinates next,
    const NeighborMovement& prior, const NeighborDescriptor& desc,
    const NeighborRecord& record, std::int32_t category, const NeighborHelpers& h) {
    const auto dx = wrapped_difference(next.x, from.x, h.dimensions.x);
    const auto dy = wrapped_difference(next.y, from.y, h.dimensions.y);
    const auto dz = sub(next.z, from.z);
    const auto metric = distance_metric(bits(static_cast<std::uint32_t>(dx) * 2),
        bits(static_cast<std::uint32_t>(dy) * 2), dz);
    NeighborMovement movement;
    movement.category = category;
    std::uint32_t cost;
    const auto node = h.node_at(next);
    if (desc.mode) cost = static_cast<std::uint32_t>(metric) * 8;
    else {
        if (!h.scalar) throw std::invalid_argument("creature scalar helper required");
        if (same_xy(from, next)) {
            movement.direction = prior.direction;
            movement.vertical_delta = dz;
        } else {
            constexpr std::array<std::int32_t, 9> directions{{7,0,1,6,0,2,5,4,3}};
            if (dx < -1 || dx > 1 || dy < -1 || dy > 1)
                throw std::invalid_argument("adjacent direction index out of range");
            movement.direction = directions[static_cast<std::size_t>(dx + 3 * dy + 4)];
        }
        auto turn = add(sub(prior.direction, movement.direction), 8) % 8;
        if (turn > 4) turn -= 8;
        if (movement.vertical_delta == prior.vertical_delta && absolute(turn) <= 1)
            movement.scalar = prior.scalar;
        h.scalar(desc, from, next, category, movement.scalar, 4);
        if (!movement.scalar) throw std::domain_error("engine unsigned cost division by zero");
        cost = (static_cast<std::uint32_t>(metric) * static_cast<std::uint32_t>(record.cost_factor) * 4)
            / movement.scalar;
        cost *= category == 1 ? 1U : (category == 2 || category == 3 ? 3U : 2U);
        if (record.flag_penalty) {
            if (!h.cell) throw std::invalid_argument("flag penalty cell callback required");
            bool penalty = (h.cell(node).flags & 0x10) != 0;
            if (!penalty && next.z > 1)
                penalty = (h.cell(h.node_at({next.x, next.y, next.z - 1})).flags & 0x10) != 0;
            if (penalty) cost *= 3;
        }
        const auto numerator = static_cast<std::uint32_t>(metric) * 0x900U;
        const auto denominator = bits(movement.scalar * 4);
        movement.accumulated = static_cast<float>(static_cast<long double>(numerator)
            / static_cast<long double>(denominator) + static_cast<long double>(prior.accumulated));
    }
    return {bits(cost), node, movement_payload(movement)};
}

// 0x4ebf1b..0x4ec75d: ordered 26-offset loop (with wrapped XY/bounded Z).
void expand_standard_neighbors(const ExpansionInput& input,
    const NeighborDescriptor& desc, const NeighborHelpers& h,
    std::vector<Candidate>& output) {
    const auto record = prepare_standard_movement(desc, input.from, h);
    for (const auto next : adjacent_cells(input.from, h.dimensions)) {
        std::int32_t category = 0;
        if (!accept_standard_neighbor(input.from, next, desc, record, h, category)) continue;
        const auto candidate = build_standard_candidate(input.from, next, input.prior,
            desc, record, category, h);
        append_candidate(output, candidate);
    }
}
} // namespace

std::vector<Candidate> generate_neighbors(NodeId current, const MovementPayload& prior_payload,
    const NeighborDescriptor& desc, const NeighborHelpers& h) {
    const auto input = prepare_expansion(current, prior_payload, h);
    std::vector<Candidate> result;
    // 0x4ebb92: descriptor +0x3a selects the expansion branch.
    if (desc.linked) expand_six_links(current, input, desc, h, result);
    else expand_standard_neighbors(input, desc, h, result);
    return result;
}

void expand_neighbors(NodeId current, const MovementPayload& prior,
    const NeighborDescriptor& desc, const NeighborHelpers& helpers,
    std::int32_t& remaining_budget, std::vector<Candidate>& output) {
    // 0x4ebaf1..0x4ebafd: guard before map reads or output changes.
    if (remaining_budget <= 0) return;
    auto candidates = generate_neighbors(current, prior, desc, helpers);
    output.insert(output.end(), candidates.begin(), candidates.end());
    --remaining_budget; // 0x4ec76a: one unit, including an empty expansion.
}

NeighborDescriptor creature_descriptor(const ObjectPrefix& object,
    std::uint32_t token, std::uint32_t unknown, Coordinates target) {
    NeighborDescriptor desc;
    (void)object; // This caller's descriptor endpoint triple is zero, not object XYZ.
    desc.object = token;
    desc.unknown_08 = unknown;
    desc.target = target;
    // 0x54b8a0, 0x54b8aa, 0x54b8be initialize these to zero, not -1.
    desc.radius_limit = desc.float_limit = 0;
    return desc;
}
SearchWorld neighbor_search_world(NeighborHelpers helpers, Coordinates target, std::uint32_t token) {
    SearchWorld world{helpers.dimensions, helpers.node_at, helpers.coordinates, {}};
    world.expand = [helpers, target, token](NodeId current, const MovementPayload& prior,
        const ObjectPrefix& object, std::uint32_t unknown) {
        return generate_neighbors(current, prior, creature_descriptor(object, token, unknown, target), helpers);
    };
    return world;
}
} // namespace mnm::reconstruction
