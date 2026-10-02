#include "route_request.hpp"

#include <cassert>
#include <cstring>
#include <limits>
#include <stdexcept>

using namespace mnm::reconstruction;

struct Observation {
    ObjectPrefix* object;
    EngineState* engine;
    std::uint32_t count;
    int calls = 0;
    std::int32_t expected_budget = 300;
};

void fake_search(RouteContextPrefix& scratch, ObjectPrefix& object,
    std::uint32_t unknown_argument, std::int32_t x, std::int32_t y,
    std::int32_t z, std::int32_t& remaining_budget, void* user_data) {
    auto& observation = *static_cast<Observation*>(user_data);
    assert(&object == observation.object);
    assert(&scratch == &observation.engine->scratch);
    assert(scratch.unknown_route_flag == 1);
    assert(object.route.waypoint_count == 99); // copy occurs AFTER search
    assert(unknown_argument == 0xfedcba98U);
    assert(x == 9 && y == 2 && z == -7);
    assert(remaining_budget == observation.expected_budget);
    remaining_budget = 0; // must not change configured budget
    scratch.route.target_x = x;
    scratch.route.target_y = y;
    scratch.route.target_z = z;
    scratch.route.unknown_0c = 0x12345678;
    scratch.route.waypoint_count = observation.count;
    scratch.route.unknown_14.fill(std::byte{0x5a});
    scratch.unknown_route_flag = 0x72; // search can change the aliased flag
    scratch.unknown_20d.fill(std::byte{0x72});
    ++observation.calls;
}

int main() {
    for (std::int32_t dimension = 1; dimension <= 31; ++dimension) {
        for (std::int32_t value = -1024; value <= 1024; ++value) {
            auto expected = value % dimension;
            if (expected < 0) expected += dimension;
            assert(normalize_coordinate(value, dimension) == expected);
        }
    }
    const auto minimum = std::numeric_limits<std::int32_t>::min();
    const auto maximum = std::numeric_limits<std::int32_t>::max();
    assert(normalize_coordinate(minimum, maximum) == maximum - 1);
    assert(normalize_coordinate(maximum, maximum) == 0);
    for (auto value : {minimum, -7, 0, 7, maximum}) assert(copy_z(value) == value);
    for (auto bad_dimension : {0, -1}) {
        bool rejected = false;
        try { normalize_coordinate(1, bad_dimension); }
        catch (const std::invalid_argument&) { rejected = true; }
        assert(rejected);
    }

    for (auto count : {0U, 1U, 16U, 0xffffffffU}) {
        ObjectPrefix object{};
        object.unknown_000.fill(std::byte{0x31});
        object.unknown_b77.fill(std::byte{0x41});
        object.unknown_b8f.fill(std::byte{0x51});
        object.route.waypoint_count = 99;
        object.route_present = 0xdeadbeef;
        object.unknown_d03 = 42;
        EngineState engine{10, 8, 300};
        Observation observation{&object, &engine, count};
        assert(route_request(object, engine, -21, 18, -7, 0xfedcba98U,
                             {fake_search, &observation}) == (count != 0));
        assert(observation.calls == 1);
        assert(std::memcmp(&object.route, &engine.scratch.route, 0x20c) == 0);
        assert(object.route_present == (count == 0 ? 0U : 1U));
        assert(object.unknown_d03 == 0);
        assert(engine.node_budget == 300 && engine.scratch.unknown_route_flag == 0x72);
        for (auto byte : object.unknown_000) assert(byte == std::byte{0x31});
        for (auto byte : object.unknown_b77) assert(byte == std::byte{0x41});
        for (auto byte : object.unknown_b8f) assert(byte == std::byte{0x51});
    }
    // The wrapper passes the configured budget through, even if zero/negative.
    // What search does with such a budget is outside this reconstruction.
    for (auto budget : {0, -1, 300}) {
        ObjectPrefix object{};
        object.route.waypoint_count = 99;
        EngineState engine{10, 8, budget};
        Observation observation{&object, &engine, 0};
        observation.expected_budget = budget;
        assert(!route_request(object, engine, -21, 18, -7, 0xfedcba98U,
                              {fake_search, &observation}));
        assert(engine.node_budget == budget);
    }
    ObjectPrefix object{};
    EngineState engine{10, 8, 300};
    bool rejected = false;
    try { route_request(object, engine, 0, 0, 0, 0, {nullptr}); }
    catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected && engine.scratch.unknown_route_flag == 0);
}
