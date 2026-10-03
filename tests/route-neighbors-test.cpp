#include "route_neighbors.hpp"
#include <cassert>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>

using namespace mnm::reconstruction;
namespace {
NodeId node(Coordinates p) { return 1 + p.x + 8 * p.y + 64 * p.z; }
Coordinates position(NodeId n) { --n; return {static_cast<int>(n % 8), static_cast<int>((n / 8) % 8), static_cast<int>(n / 64)}; }
bool same(Coordinates a, Coordinates b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
struct Fixture {
    NeighborHelpers h;
    NeighborDescriptor desc;
    std::map<NodeId, NeighborCell> cells;
    std::int32_t category = 1, d03 = 0;
    std::uint32_t input_scalar = 0;
    bool special = false;
    int accepts = 0, scalar_calls = 0;
    Coordinates accepted{3,2,1};
    bool all = false;
    Fixture() {
        h.dimensions = {8,8,4}; h.node_at = node; h.coordinates = position;
        h.cell = [&](NodeId n) { return cells[n]; };
        h.record = [](const NeighborDescriptor&, Coordinates) { return NeighborRecord{100, false}; };
        h.object_d03 = [&](std::uint32_t token) { assert(token == 123); return d03; };
        h.accept = [&](const NeighborDescriptor&, Coordinates, Coordinates next,
            const NeighborRecord&, bool flag, std::int32_t& out) {
            ++accepts; special = flag; out = category; return all || same(next, accepted);
        };
        h.scalar = [&](const NeighborDescriptor&, Coordinates, Coordinates,
            std::int32_t cat, std::uint32_t& value, std::int32_t argument) {
            assert(cat == category && argument == 4); ++scalar_calls; input_scalar = value; value = 40;
        };
        desc.object = 123; desc.start = {2,2,1}; desc.target = accepted;
    }
    std::vector<Candidate> run(NeighborMovement prior = {}) {
        return generate_neighbors(node({2,2,1}), movement_payload(prior), desc, h);
    }
};
void standard_order_and_budget() {
    Fixture f; f.desc.mode = 1; f.all = true;
    auto out = f.run(); const auto ordered = adjacent_cells({2,2,1}, f.h.dimensions);
    assert(out.size() == 26 && f.accepts == 26 && f.scalar_calls == 0);
    for (std::size_t i = 0; i < out.size(); ++i) {
        assert(out[i].node == node(ordered[i]));
        const auto p = ordered[i];
        const auto expected = 8 * distance_metric(2 * (p.x - 2), 2 * (p.y - 2), p.z - 1);
        assert(out[i].edge_cost == expected);
        const NeighborMovement expected_movement{1,0,0,0,0,0,0};
        const auto bytes = movement_payload(expected_movement);
        assert(std::memcmp(&out[i].movement, &bytes, 28) == 0);
    }
    std::vector<Candidate> appended{{999,888,{}}}; int budget = 0;
    expand_neighbors(node({2,2,1}), {}, f.desc, f.h, budget, appended);
    assert(appended.size() == 1 && budget == 0);
    budget = 1;
    expand_neighbors(node({2,2,1}), {}, f.desc, f.h, budget, appended);
    assert(appended.size() == 27 && appended.front().edge_cost == 999 && budget == 0);
    f.all = false; f.accepted = {7,7,3}; budget = 2; appended.clear();
    expand_neighbors(node({2,2,1}), {}, f.desc, f.h, budget, appended);
    assert(appended.empty() && budget == 1);
    // Boundary: standard branch never offers invalid Z.
    auto boundary = generate_neighbors(node({0,0,0}), {}, f.desc, f.h);
    assert(boundary.empty());
    f.all = true;
    f.desc.start = f.desc.target = {0,0,0};
    boundary = generate_neighbors(node({0,0,0}), {}, f.desc, f.h);
    assert(boundary.size() == 17 && boundary[0].node == node({0,7,0}));
}
void creature_cost_and_payload() {
    Fixture f; NeighborMovement prior; prior.direction = 2; prior.scalar = 99; prior.accumulated = 7;
    auto out = f.run(prior); assert(out.size() == 1 && out[0].edge_cost == 40);
    auto m = neighbor_movement(out[0].movement);
    assert(m.category == 1 && m.direction == 2 && m.vertical_delta == 0 && m.scalar == 40);
    assert(m.accumulated == 64.6F && m.accumulated_cost == 0 && m.unknown_18 == 0);
    assert(f.input_scalar == 99);
    for (const auto category : {2,3,4,5}) {
        f.category = category;
        out = f.run(prior); assert(out[0].edge_cost == (category <= 3 ? 120 : 80));
    }
    f.category = 1; prior.direction = 0; f.run(prior); assert(f.input_scalar == 0);
    prior.direction = 1; f.run(prior); assert(f.input_scalar == 99);
    prior.direction = 2; prior.vertical_delta = 1; f.run(prior); assert(f.input_scalar == 0);
    // Straight vertical movement retains direction and can reuse scalar.
    f.accepted = {2,2,2}; out = f.run(prior); m = neighbor_movement(out[0].movement);
    assert(m.direction == 2 && m.vertical_delta == 1 && f.input_scalar == 99);
    assert(out[0].edge_cost == 20);
    f.h.record = [](const NeighborDescriptor&, Coordinates) { return NeighborRecord{100,true}; };
    f.cells[node({2,2,1})].flags = 0x10;
    out = f.run(prior); assert(out[0].edge_cost == 60); // lower cell penalty
    f.cells.clear(); f.cells[node({2,2,2})].flags = 0x10;
    out = f.run(prior); assert(out[0].edge_cost == 60); // destination penalty
    f.h.record = [](const NeighborDescriptor&, Coordinates) { return NeighborRecord{-1,false}; };
    f.accepted = {3,2,1}; out = f.run(prior);
    assert(out[0].edge_cost == static_cast<int>((0xfffffff0U) / 40)); // unsigned division
}
void special_and_restrictions() {
    Fixture f;
    f.h.accept = [&](const NeighborDescriptor&, Coordinates, Coordinates next,
        const NeighborRecord&, bool flag, std::int32_t& category) {
        if (same(next, f.accepted)) f.special = flag;
        category = 1; return same(next, f.accepted);
    };
    f.run(); assert(f.special);
    f.d03 = 1; f.run(); assert(!f.special);
    f.desc.unknown_08 = 1; f.run(); assert(f.special);
    f.desc.unknown_08 = 0; f.d03 = 0;
    f.accepted = f.desc.target = {3,3,1}; f.run(); assert(!f.special); // diagonal target
    f.desc.mode = 1; f.desc.start = {0,0,0}; f.desc.target = {0,0,0};
    f.all = true; f.accepts = 0;
    // Current (2,2,1) is outside zero-span X/Y, so only (1,1,*) passes.
    f.h.accept = [&](const NeighborDescriptor&, Coordinates, Coordinates next,
        const NeighborRecord&, bool, std::int32_t& category) {
        ++f.accepts; assert(next.x == 1 && next.y == 1 && next.z <= 1); category = 3; return true;
    };
    auto out = f.run(); assert(out.size() == 2 && f.accepts == 2);
}
void six_links() {
    Fixture f; f.desc.linked = 1;
    f.cells[node({2,2,1})].links = 0x3f00;
    const std::array<Coordinates, 6> positions{{{1,2,1},{3,2,1},{2,1,1},{2,3,1},{2,2,0},{2,2,2}}};
    const std::array<int, 6> costs{{2,2,2,2,1,3}};
    for (auto p : positions) f.cells[node(p)].first_word = 1;
    NeighborMovement prior; prior.accumulated = 4.5F; prior.accumulated_cost = 10;
    prior.direction = 7; prior.scalar = 99;
    auto out = f.run(prior); assert(out.size() == 6 && f.accepts == 0 && f.scalar_calls == 0);
    for (std::size_t i = 0; i < out.size(); ++i) {
        assert(out[i].node == node(positions[i]) && out[i].edge_cost == costs[i]);
        const auto expected = movement_payload({5,0,0,0,5.5F,10 + costs[i],0});
        assert(std::memcmp(&expected, &out[i].movement, 28) == 0);
    }
    f.desc.float_limit = 4; assert(f.run(prior).empty());
    f.desc.float_limit = 5; assert(f.run(prior).size() == 6);
    prior.accumulated = 5; assert(f.run(prior).empty());
    prior.accumulated = std::numeric_limits<float>::quiet_NaN(); assert(f.run(prior).empty());
    f.desc.float_limit = -1; assert(f.run(prior).size() == 6);
    prior.accumulated = 4.5F; f.desc.radius_limit = 1;
    out = f.run(prior); assert(out.size() == 2); // strict radius, vertical XY distance zero
    f.desc.radius_limit = 0; assert(f.run(prior).empty());
    f.desc.radius_limit = -1; f.cells[node({2,2,1})].links = 0x800;
    assert(f.run(prior).size() == 1);
    f.cells[node({3,2,1})].first_word = 0; assert(f.run(prior).empty());
    // No invented Z guard: invalid linked Z reaches provider, which rejects it.
    f.h.coordinates = [](NodeId) { return Coordinates{0,0,0}; };
    f.h.cell = [](NodeId n) { return NeighborCell{static_cast<std::uint16_t>(n == 7),512,0}; };
    bool invalid_seen = false;
    f.h.node_at = [&](Coordinates p) { invalid_seen = p.z == -1; return NodeId{0}; };
    assert(f.run().empty() && invalid_seen);
}
void search_adapter() {
    Fixture f; ObjectPrefix object{};
    const int x = 2, z = 1;
    std::memcpy(reinterpret_cast<std::byte*>(&object) + 8, &x, 4);
    std::memcpy(reinterpret_cast<std::byte*>(&object) + 12, &x, 4);
    std::memcpy(reinterpret_cast<std::byte*>(&object) + 16, &z, 4);
    const auto desc = creature_descriptor(object, 123, 42, {3,2,1});
    assert(desc.object == 123 && desc.unknown_08 == 42 && same(desc.start, {0,0,0}));
    assert(desc.linked == 0 && desc.radius_limit == 0 && desc.float_limit == 0);
    auto world = neighbor_search_world(f.h, {3,2,1}, 123);
    RouteContextPrefix context{}; context.unknown_route_flag = 1;
    SearchState state; int budget = 5;
    auto result = route_search(context, object, 42, {3,2,1}, budget, state, world);
    assert(result.path.size() == 2 && result.path.back() == node({3,2,1}));
    assert(result.expansions == 1 && budget == 4 && context.route.waypoint_count == 1);
}
} // namespace
int main() {
    standard_order_and_budget(); creature_cost_and_payload(); special_and_restrictions();
    six_links(); search_adapter();
}
