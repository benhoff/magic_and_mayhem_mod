#include "route_search.hpp"

#include <algorithm>
#include <cstring>
#include <set>
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t signed_bits(std::uint32_t bits) {
    return bits <= 0x7fffffffU ? static_cast<std::int32_t>(bits)
        : -1 - static_cast<std::int32_t>(~bits);
}
std::int32_t add(std::int32_t a, std::int32_t b) {
    return signed_bits(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}
std::int32_t subtract(std::int32_t a, std::int32_t b) {
    return signed_bits(static_cast<std::uint32_t>(a) - static_cast<std::uint32_t>(b));
}
std::int32_t multiply(std::int32_t a, std::uint32_t b) {
    return signed_bits(static_cast<std::uint32_t>(a) * b);
}
std::int32_t absolute(std::int32_t value) {
    return value < 0 ? subtract(0, value) : value;
}
void validate_dimensions(MapDimensions d) {
    if (d.x <= 0 || d.y <= 0 || d.z <= 0)
        throw std::invalid_argument("positive map dimensions required");
}
void validate_coordinates(Coordinates p, MapDimensions d) {
    if (p.x < 0 || p.x >= d.x || p.y < 0 || p.y >= d.y || p.z < 0 || p.z >= d.z)
        throw std::invalid_argument("canonical in-map coordinates required");
}
template<class T> T load(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(base) + offset, sizeof(value));
    return value;
}
template<class T> void store(void* base, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(base) + offset, &value, sizeof(value));
}
Coordinates object_position(const ObjectPrefix& object) {
    return {load<std::int32_t>(&object, 8), load<std::int32_t>(&object, 12),
            load<std::int32_t>(&object, 16)};
}
void bridge(RouteContextPrefix& context, ObjectPrefix& object, std::uint32_t unknown,
    std::int32_t x, std::int32_t y, std::int32_t z, std::int32_t& budget, void* user) {
    auto& search = *static_cast<ReconstructedSearch*>(user);
    route_search(context, object, unknown, {x, y, z}, budget, search.state, search.world);
}
} // namespace

std::int32_t distance_metric(std::int32_t dx, std::int32_t dy, std::int32_t dz) {
    std::array<std::int32_t, 3> magnitudes{absolute(dx), absolute(dy), absolute(dz)};
    std::sort(magnitudes.begin(), magnitudes.end());
    return add(add(multiply(magnitudes[2], 2), magnitudes[1]), magnitudes[0]);
}

std::int32_t wrapped_difference(std::int32_t a, std::int32_t b, std::int32_t dimension) {
    if (dimension <= 0 || a < 0 || a >= dimension || b < 0 || b >= dimension)
        throw std::invalid_argument("canonical coordinates and positive dimension required");
    auto difference = a - b; // canonical operands keep this subtraction in range
    if (difference < 0) difference += dimension;
    if (difference > dimension / 2) difference -= dimension;
    return difference; // exactly half a dimension remains positive
}

std::int32_t route_heuristic(Coordinates a, Coordinates b, MapDimensions dimensions) {
    validate_dimensions(dimensions);
    validate_coordinates(a, dimensions);
    validate_coordinates(b, dimensions);
    return multiply(distance_metric(
        multiply(wrapped_difference(a.x, b.x, dimensions.x), 2),
        multiply(wrapped_difference(a.y, b.y, dimensions.y), 2), a.z - b.z), 20);
}

std::vector<Coordinates> adjacent_cells(Coordinates from, MapDimensions dimensions) {
    validate_dimensions(dimensions);
    validate_coordinates(from, dimensions);
    // File-backed table at VA 0x005e178c; verified by export-search-support.py.
    static constexpr std::array<Coordinates, 26> offsets{{
        {0,-1,0}, {1,-1,0}, {1,0,0}, {1,1,0}, {0,1,0}, {-1,1,0}, {-1,0,0}, {-1,-1,0},
        {0,0,1}, {0,-1,1}, {1,-1,1}, {1,0,1}, {1,1,1}, {0,1,1}, {-1,1,1}, {-1,0,1}, {-1,-1,1},
        {0,0,-1}, {0,-1,-1}, {1,-1,-1}, {1,0,-1}, {1,1,-1}, {0,1,-1}, {-1,1,-1}, {-1,0,-1}, {-1,-1,-1}
    }};
    std::vector<Coordinates> result;
    for (const auto offset : offsets) {
        const auto z = static_cast<std::int64_t>(from.z) + offset.z;
        if (z < 0 || z >= dimensions.z) continue;
        // Add using 64 bits for the harness; canonical bounds avoid engine overflow.
        const auto wrap = [](std::int32_t value, std::int32_t offset_value, std::int32_t dimension) {
            auto sum = static_cast<std::int64_t>(value) + offset_value;
            if (sum < 0) sum += dimension;
            if (sum >= dimension) sum -= dimension;
            return static_cast<std::int32_t>(sum);
        };
        result.push_back({wrap(from.x, offset.x, dimensions.x),
                          wrap(from.y, offset.y, dimensions.y), static_cast<std::int32_t>(z)});
    }
    return result;
}

SearchResult route_search(RouteContextPrefix& context, const ObjectPrefix& object,
    std::uint32_t unknown_argument, Coordinates target, std::int32_t& remaining_budget,
    SearchState& state, const SearchWorld& world) {
    // These checks protect the host harness; they are not discovered engine branches.
    validate_dimensions(world.dimensions);
    const auto start_position = object_position(object);
    validate_coordinates(start_position, world.dimensions);
    validate_coordinates(target, world.dimensions);
    if (!world.node_at || !world.coordinates || !world.expand)
        throw std::invalid_argument("map and movement callbacks required");
    if (!context.unknown_route_flag && !state.initialized)
        throw std::invalid_argument("continuation requires initialized search state");
    const auto start = world.node_at(start_position);
    const auto destination = world.node_at(target);
    if (!start || !destination) throw std::invalid_argument("node tokens must be nonzero");
    const auto heuristic = [&](NodeId node) {
        return route_heuristic(world.coordinates(node), world.coordinates(destination), world.dimensions);
    };
    std::memcpy(&context.route, &object.route, sizeof(RouteSnapshot));
    if (context.unknown_route_flag) {
        context.unknown_route_flag = 0;
        state.open.clear();
        state.records.clear();
        state.open.emplace(0, start); // observed seed is zero, NOT h(start)
        state.records[start].priority = 0;
        state.best_node = start;
        state.best_heuristic = heuristic(start);
        state.initialized = true;
    }

    SearchResult result{SearchStop::queue_empty, 0, {}};
    bool budget_exit = false;
    while (!state.open.empty()) {
        const auto front = state.open.begin();
        const auto priority = front->first;
        const auto current = front->second;
        if (priority >= state.records[destination].priority) {
            result.stop = SearchStop::target_bound;
            break;
        }
        std::vector<Candidate> candidates;
        if (remaining_budget > 0) {
            candidates = world.expand(current, state.records[current].movement, object, unknown_argument);
            --remaining_budget; // 0x004ec76a: one decrement per expansion
            ++result.expansions;
        }
        if (remaining_budget <= 0) {
            // Last expansion's candidates are deliberately not consumed and its
            // queue entry stays in place. Preserve continuation flag zero.
            result.stop = SearchStop::budget_exhausted;
            budget_exit = true;
            break;
        }
        state.open.erase(front);
        const auto current_h = heuristic(current);
        for (const auto& candidate : candidates) {
            if (!candidate.node) throw std::invalid_argument("candidate node token must be nonzero");
            const auto neighbor_h = heuristic(candidate.node);
            const auto tentative = add(add(subtract(candidate.edge_cost, current_h), neighbor_h), priority);
            auto& record = state.records[candidate.node];
            if (tentative < record.priority) {
                record.priority = tentative;
                record.predecessor = current;
                record.movement = candidate.movement;
                state.open.emplace(tentative, candidate.node); // do not remove older entries
                if (neighbor_h < state.best_heuristic) {
                    state.best_heuristic = neighbor_h;
                    state.best_node = candidate.node;
                }
            }
        }
    }
    if (!budget_exit) context.unknown_route_flag = 1;

    // The engine appends best-to-start records, then walks the vector backwards.
    // Include start for initialization, but never emit it as a waypoint.
    std::set<NodeId> seen;
    for (auto node = state.best_node;; node = state.records.at(node).predecessor) {
        if (!node || !seen.insert(node).second)
            throw std::runtime_error("invalid/cyclic predecessor chain in host model");
        result.path.push_back(node);
        if (node == start) break;
    }
    std::reverse(result.path.begin(), result.path.end());
    context.route.target_x = target.x;
    context.route.target_y = target.y;
    context.route.target_z = target.z;
    context.route.waypoint_count = 0;
    auto previous = world.coordinates(start);
    auto direction = load<std::int32_t>(&object, 0x608);
    static constexpr std::array<std::int32_t, 9> directions{7,0,1,6,0,2,5,4,3};
    for (std::size_t index = 1; index < result.path.size() && index <= 16; ++index) {
        const auto position = world.coordinates(result.path[index]);
        std::int32_t vertical_delta;
        if (position.x == previous.x && position.y == previous.y) {
            vertical_delta = position.z - previous.z;
        } else {
            const auto dx = wrapped_difference(position.x, previous.x, world.dimensions.x);
            const auto dy = wrapped_difference(position.y, previous.y, world.dimensions.y);
            if (dx < -1 || dx > 1 || dy < -1 || dy > 1)
                throw std::invalid_argument("nonadjacent waypoint requires unresolved engine direction lookup");
            direction = directions[static_cast<std::size_t>(dx + 3 * dy + 4)];
            vertical_delta = 0;
        }
        const auto category = state.records.at(result.path[index]).movement.category;
        const Waypoint waypoint{position.x, position.y, position.z, direction, vertical_delta,
            static_cast<std::uint32_t>(category == 1 || category == 2 || category == 3), category};
        std::memcpy(context.route.unknown_14.data() + (index - 1) * sizeof(Waypoint), &waypoint, sizeof(waypoint));
        ++context.route.waypoint_count;
        previous = position;
    }
    context.route.unknown_0c = 0;
    store(&context, 0x211, state.best_heuristic);
    store(&context, 0x215, state.best_node);
    store(&context, 0x265, state.records.at(state.best_node).priority);
    return result;
}

SearchBackend search_backend(ReconstructedSearch& search) {
    return {bridge, &search};
}
} // namespace mnm::reconstruction
