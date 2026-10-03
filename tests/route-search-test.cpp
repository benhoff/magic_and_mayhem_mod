#include "route_search.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace mnm::reconstruction;

template<class T> void put(void* base, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(base) + offset, &value, sizeof(value));
}
Waypoint waypoint(const RouteContextPrefix& context, std::size_t index) {
    Waypoint result;
    std::memcpy(&result, context.route.unknown_14.data() + index * sizeof(result), sizeof(result));
    return result;
}

struct Graph {
    MapDimensions dimensions{80,80,8};
    std::map<NodeId, std::vector<Candidate>> edges;
    std::vector<NodeId> expanded;
    NodeId node(Coordinates p) const {
        return 0x1000U + 12U * static_cast<NodeId>(p.x + dimensions.x * (p.y + dimensions.y * p.z));
    }
    Coordinates position(NodeId node_id) const {
        const auto index = (node_id - 0x1000U) / 12;
        return {static_cast<std::int32_t>(index % dimensions.x),
                static_cast<std::int32_t>((index / dimensions.x) % dimensions.y),
                static_cast<std::int32_t>(index / (dimensions.x * dimensions.y))};
    }
    void edge(Coordinates from, Coordinates to, std::int32_t cost = 80, std::int32_t category = 1) {
        MovementPayload movement;
        movement.category = category;
        movement.unknown_04.fill(std::byte{0x7e});
        edges[node(from)].push_back({cost, node(to), movement});
    }
    SearchWorld world() {
        return {dimensions, [this](Coordinates p) { return node(p); },
            [this](NodeId id) { return position(id); },
            [this](NodeId id, const MovementPayload&, const ObjectPrefix&, std::uint32_t unknown) {
                assert(unknown == 42);
                expanded.push_back(id);
                return edges[id];
            }};
    }
};
struct Fixture {
    Graph graph;
    ObjectPrefix object{};
    RouteContextPrefix context{};
    SearchState state;
    std::int32_t budget = 300;
    Fixture() {
        object.route.unknown_14.fill(std::byte{0xa5});
        put(&object, 0x608, std::int32_t{6});
        context.unknown_route_flag = 1;
    }
    SearchResult run(Coordinates target) {
        return route_search(context, object, 42, target, budget, state, graph.world());
    }
};

void test_metric_and_heuristic() {
    assert(distance_metric(1,0,0) == 2);
    assert(distance_metric(1,1,0) == 3);
    assert(distance_metric(1,1,1) == 4);
    // Independent absolute-value/order enumeration of the six assembly branches.
    for (int x = -9; x <= 9; ++x) for (int y = -9; y <= 9; ++y) for (int z = -9; z <= 9; ++z) {
        const int a = x < 0 ? -x : x, b = y < 0 ? -y : y, c = z < 0 ? -z : z;
        const auto maximum = a >= b && a >= c ? a : (b >= c ? b : c);
        assert(distance_metric(x,y,z) == a + b + c + maximum);
    }
    assert(distance_metric(std::numeric_limits<std::int32_t>::max(),0,0) == -2);
    assert(distance_metric(std::numeric_limits<std::int32_t>::min(),0,0) == std::numeric_limits<std::int32_t>::min());
    assert(wrapped_difference(0,79,80) == 1);
    assert(wrapped_difference(79,0,80) == -1);
    assert(wrapped_difference(0,40,80) == 40);
    assert(wrapped_difference(0,3,7) == -3);
    assert(route_heuristic({0,0,0},{1,0,0},{80,80,8}) == 80);
    assert(route_heuristic({0,0,0},{1,1,0},{80,80,8}) == 120);
    assert(route_heuristic({0,0,0},{0,0,1},{80,80,8}) == 40);
    assert(route_heuristic({0,0,0},{79,79,1},{80,80,8}) == 140);
    const auto cells = adjacent_cells({0,0,1},{80,80,3});
    assert(cells.size() == 26);
    assert(cells[0].x == 0 && cells[0].y == 79 && cells[0].z == 1);
    assert(cells[8].x == 0 && cells[8].y == 0 && cells[8].z == 2);
    assert(cells[17].x == 0 && cells[17].y == 0 && cells[17].z == 0);
    assert(adjacent_cells({0,0,0},{80,80,3}).size() == 17);
    // Do not deduplicate on narrow wrapping maps; the engine iterates offsets.
    assert(adjacent_cells({0,0,0},{1,1,1}).size() == 8);
}

void test_routes_and_output() {
    Fixture f;
    f.graph.edge({0,0,0},{1,0,0});
    f.graph.edge({1,0,0},{1,0,1},40,5);
    const auto result = f.run({1,0,1});
    assert(result.stop == SearchStop::target_bound && result.expansions == 2);
    assert(f.budget == 298 && result.path.size() == 3);
    assert(f.context.unknown_route_flag == 1 && f.context.route.waypoint_count == 2);
    const auto first = waypoint(f.context,0), second = waypoint(f.context,1);
    assert(first.x == 1 && first.direction == 2 && first.vertical_delta == 0);
    assert(first.category == 1 && first.category_is_1_to_3 == 1);
    assert(second.z == 1 && second.direction == 2 && second.vertical_delta == 1);
    assert(second.category == 5 && second.category_is_1_to_3 == 0);
    // Only emitted slots changed. All unused route bytes came from the object.
    for (std::size_t i = 56; i < f.context.route.unknown_14.size(); ++i)
        assert(f.context.route.unknown_14[i] == std::byte{0xa5});
    assert(f.state.records.at(f.graph.node({1,0,1})).movement.unknown_04[23] == std::byte{0x7e});
    assert(f.state.records.at(f.graph.node({1,0,1})).priority == 20); // 120 - h(start=100)

    Fixture seam;
    seam.graph.edge({0,0,0},{79,0,0});
    seam.run({79,0,0});
    assert(waypoint(seam.context,0).direction == 6);

    Fixture truncated;
    for (int x = 0; x < 20; ++x) truncated.graph.edge({x,0,0},{x+1,0,0});
    const auto long_route = truncated.run({20,0,0});
    assert(long_route.path.size() == 21 && truncated.context.route.waypoint_count == 16);
    assert(waypoint(truncated.context,15).x == 16);
    for (std::size_t i = 16 * 28; i < truncated.context.route.unknown_14.size(); ++i)
        assert(truncated.context.route.unknown_14[i] == std::byte{0xa5});
}

void test_budget_and_continuation() {
    Fixture f;
    for (int x = 0; x < 3; ++x) f.graph.edge({x,0,0},{x+1,0,0});
    f.budget = 1;
    auto result = f.run({3,0,0});
    assert(result.stop == SearchStop::budget_exhausted && result.expansions == 1);
    assert(f.context.unknown_route_flag == 0 && f.context.route.waypoint_count == 0);
    assert(f.state.open.size() == 1 && f.state.open.begin()->second == f.graph.node({0,0,0}));
    assert(f.state.records.count(f.graph.node({1,0,0})) == 0); // discarded final candidates
    f.budget = 2;
    result = f.run({3,0,0});
    assert(result.stop == SearchStop::budget_exhausted && f.context.route.waypoint_count == 1);
    assert(f.state.open.begin()->second == f.graph.node({1,0,0}));
    f.budget = 3;
    result = f.run({3,0,0});
    assert(result.stop == SearchStop::target_bound && f.context.route.waypoint_count == 3);
    assert(f.budget == 1 && f.context.unknown_route_flag == 1);

    Fixture zero;
    zero.budget = 0;
    assert(zero.run({1,0,0}).stop == SearchStop::budget_exhausted);
    assert(zero.graph.expanded.empty() && zero.budget == 0);
    Fixture blocked;
    assert(blocked.run({1,0,0}).stop == SearchStop::queue_empty);
    assert(blocked.context.route.waypoint_count == 0 && blocked.context.unknown_route_flag == 1);
    Fixture same;
    assert(same.run({0,0,0}).stop == SearchStop::target_bound);
    assert(same.graph.expanded.empty() && same.budget == 300);
}

void test_duplicates_improvements_and_partial_path() {
    Fixture f;
    f.graph.edge({0,0,0},{1,0,0},200,5);
    f.graph.edge({0,0,0},{0,1,0},50,2);
    f.graph.edge({0,1,0},{1,0,0},20,3);
    const auto result = f.run({4,0,0});
    assert(result.stop == SearchStop::queue_empty && result.expansions == 4);
    assert(f.graph.expanded[2] == f.graph.node({1,0,0}));
    assert(f.graph.expanded[3] == f.graph.node({1,0,0})); // stale entry is expanded too
    assert(result.path.size() == 3 && result.path[1] == f.graph.node({0,1,0}));
    assert(f.context.route.target_x == 4 && f.context.route.waypoint_count == 2);
    assert(waypoint(f.context,1).category == 3);
    assert(f.state.best_node == f.graph.node({1,0,0}));

    Fixture ties;
    ties.graph.edge({0,0,0},{1,0,0},40);
    ties.graph.edge({0,0,0},{0,1,0},40);
    // Both priorities are zero for target (4,4). First candidate remains first.
    ties.run({4,4,0});
    assert(ties.graph.expanded[1] == ties.graph.node({1,0,0}));
    assert(ties.graph.expanded[2] == ties.graph.node({0,1,0}));
}

void test_request_bridge_and_guards() {
    Fixture f;
    f.graph.edge({0,0,0},{1,0,0});
    ReconstructedSearch search{{}, f.graph.world()};
    EngineState engine{80,80,300,{}};
    assert(route_request(f.object,engine,81,0,0,42,search_backend(search)));
    assert(f.object.route.target_x == 1 && f.object.route.waypoint_count == 1);
    assert(f.object.route_present == 1 && f.object.unknown_d03 == 0 && engine.node_budget == 300);
    Fixture invalid;
    invalid.context.unknown_route_flag = 0;
    try { invalid.run({1,0,0}); assert(false); } catch (const std::invalid_argument&) {}
    assert(invalid.graph.expanded.empty());
    invalid.context.unknown_route_flag = 1;
    try { invalid.run({1,0,-1}); assert(false); } catch (const std::invalid_argument&) {}
}

int main() {
    test_metric_and_heuristic();
    test_routes_and_output();
    test_budget_and_continuation();
    test_duplicates_improvements_and_partial_path();
    test_request_bridge_and_guards();
    std::cout << "Search reconstruction tests passed.\n";
}
