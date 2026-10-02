#include "route_request.hpp"

#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {

std::int32_t normalize_coordinate(std::int32_t value, std::int32_t dimension) {
    // Harness precondition, not a newly discovered engine validation branch.
    if (dimension <= 0) {
        throw std::invalid_argument("map dimension must be positive");
    }
    if (value < 0) {
        do {
            value += dimension;
        } while (value < 0);
    } else {
        while (value >= dimension) {
            value -= dimension;
        }
    }
    return value;
}

std::int32_t copy_z(std::int32_t value) {
    return value;
}

bool route_request(ObjectPrefix& object, EngineState& engine,
    std::int32_t x, std::int32_t y, std::int32_t z,
    std::uint32_t unknown_argument, SearchBackend search) {
    // Reject an invalid test setup before touching modeled engine state.
    if (search.run == nullptr || engine.x_dimension <= 0 || engine.y_dimension <= 0) {
        throw std::invalid_argument("valid dimensions and a search backend are required");
    }

    std::int32_t remaining_budget = engine.node_budget; // stack-local budget
    engine.scratch.unknown_route_flag = 1; // 0x00690354 = 0x00690148 + 0x20c
    const auto z_value = copy_z(z);
    const auto wrapped_y = normalize_coordinate(y, engine.y_dimension);
    const auto wrapped_x = normalize_coordinate(x, engine.x_dimension);

    search.run(engine.scratch, object, unknown_argument,
        wrapped_x, wrapped_y, z_value, remaining_budget, search.user_data);

    // Keep all 0x20c bytes, including opaque fields: original uses rep movsd.
    std::memcpy(&object.route, &engine.scratch.route, sizeof(RouteSnapshot));
    const auto count = object.route.waypoint_count;
    object.route_present = 1;
    object.unknown_d03 = 0;
    if (count == 0) {
        object.route_present = 0;
    }
    return count != 0;
}

} // namespace mnm::reconstruction
