#include "route_creature_acceptance.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <vector>

using namespace mnm::reconstruction;
namespace {
bool same(Coordinates a, Coordinates b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
struct Fixture {
    CreatureAcceptanceState state{77,19,23,0,0,0,31,43};
    CreatureAcceptanceHelpers h;
    std::vector<Coordinates> calls;
    bool base = true;
    int base_calls = 0, reads = 0;
    std::int32_t base_category = 3;
    Fixture() {
        h.dimensions = {8,8,8};
        h.object_state = [&](std::uint32_t token) { assert(token == 123); ++reads; return state; };
        h.movement_test = [&](Coordinates from, Coordinates to, const CreatureMovementParameters& p,
            std::int32_t& category, std::int32_t override_value) {
            assert(from.x >= 0 && to.x >= 0 && override_value == -1);
            const auto expected = creature_movement_parameters(state);
            assert(std::memcmp(&p, &expected, 22) == 0);
            ++base_calls; category = base_category; return base;
        };
        h.cell_test = [&](Coordinates p, std::int32_t id, const CreatureMovementParameters& parameters) {
            assert(id == state.object_00);
            const auto expected = creature_movement_parameters(state);
            assert(std::memcmp(&parameters, &expected, 22) == 0);
            calls.push_back(p); return true;
        };
    }
};
void parameters_and_failure() {
    Fixture f;
    for (auto flag : {0,1,255}) for (auto code : {0,13,14,15}) {
        f.state.object_723 = static_cast<std::uint8_t>(flag); f.state.object_5ec = code;
        const auto p = creature_movement_parameters(f.state);
        assert(p.type_0c == 31 && p.type_08 == 0 && p.object_104 == 23 && p.type_44 == 43);
        assert(p.type_index == 19 && p.unknown_10 == 0 && p.special_state == (flag || code == 14));
    }
    f.base = false; int category = 99;
    assert(!accept_creature_move(123, {1,2,3}, {2,3,4}, true, category, f.h));
    assert(category == 5 && f.base_calls == 1 && f.calls.empty() && f.reads == 1);
    f.base = true; category = 99;
    assert(accept_creature_move(123, {1,2,3}, {2,3,4}, true, category, f.h));
    assert(category == 3 && f.calls.empty());
    f.h.cell_test = [](Coordinates, std::int32_t, const CreatureMovementParameters&) { return false; };
    f.state.type_08 = 2; category = 99;
    assert(!accept_creature_move(123, {1,2,3}, {2,3,4}, false, category, f.h));
    assert(category == 5); // type exemption does not bypass destination failure
}
void all_intermediate_combinations() {
    // Independent Boolean oracle over destination + six distinct intermediate cells.
    const Coordinates from{1,2,3}, to{2,3,4};
    const std::array<Coordinates, 7> points{{to,{2,2,3},{1,3,3},{2,3,3},
        {1,2,4},{2,2,4},{1,3,4}}};
    for (unsigned mask = 0; mask < 128; ++mask) {
        Fixture f;
        f.h.cell_test = [&](Coordinates p, std::int32_t id, const CreatureMovementParameters& parameters) {
            assert(id == 77 && parameters.type_index == 19);
            f.calls.push_back(p);
            const auto found = std::find_if(points.begin(), points.end(), [&](Coordinates point) { return same(p, point); });
            assert(found != points.end());
            return (mask & (1U << static_cast<unsigned>(found - points.begin()))) != 0;
        };
        const bool expected = (mask & 1) && ((mask & 0x0e) == 0x0e || (mask & 0x70) == 0x70);
        int category = 88;
        assert(accept_creature_move(123, from, to, false, category, f.h) == expected);
        assert(category == (expected ? 3 : 5));
        std::vector<Coordinates> expected_calls{to};
        if (mask & 1) {
            bool group_passed = true;
            for (unsigned i = 1; i <= 3; ++i) {
                expected_calls.push_back(points[i]);
                if (!(mask & (1U << i))) { group_passed = false; break; }
            }
            if (!group_passed) for (unsigned i = 4; i <= 6; ++i) {
                expected_calls.push_back(points[i]);
                if (!(mask & (1U << i))) break;
            }
        }
        assert(f.calls.size() == expected_calls.size());
        for (std::size_t i = 0; i < f.calls.size(); ++i) assert(same(f.calls[i], expected_calls[i]));
    }
}
void exemptions_wrapping_and_rereads() {
    Fixture f; int category;
    for (auto to : {Coordinates{1,2,3}, Coordinates{2,2,3}, Coordinates{1,3,3}, Coordinates{1,2,4}}) {
        f.calls.clear(); assert(accept_creature_move(123,{1,2,3},to,false,category,f.h));
        assert(f.calls.size() == 1 && same(f.calls[0], to));
    }
    f.calls.clear();
    assert(accept_creature_move(123,{7,2,3},{0,2,3},false,category,f.h));
    assert(f.calls.size() == 1); // seam is one cell apart
    f.calls.clear(); f.state.type_08 = 2;
    assert(accept_creature_move(123,{1,2,3},{3,4,5},false,category,f.h));
    assert(f.calls.size() == 1);
    f.state.type_08 = 0;
    // Signed comparison after DWORD squared-distance overflow.
    for (int z : {46341,65536}) {
        f.calls.clear(); assert(accept_creature_move(123,{1,2,0},{1,2,z},false,category,f.h));
        assert(f.calls.size() == 1);
    }
    // Destination helper can update state; type +8 is reread afterward.
    f.calls.clear();
    f.h.cell_test = [&](Coordinates p, std::int32_t, const CreatureMovementParameters&) {
        f.calls.push_back(p); f.state.type_08 = 2; return true;
    };
    assert(accept_creature_move(123,{1,2,3},{2,3,4},false,category,f.h));
    assert(f.calls.size() == 1);
    // Every cell wrapper constructs fresh parameters, not a cached member.
    f.state.object_723 = 255;
    f.h.cell_test = [&](Coordinates, std::int32_t id, const CreatureMovementParameters& p) {
        assert(id == 77 && p.type_08 == 2 && p.special_state == 1); return true;
    };
    assert(test_creature_cell(123,{1,2,3},f.h));
}
void generator_integration() {
    Fixture f; NeighborHelpers neighbors;
    neighbors.dimensions = f.h.dimensions;
    neighbors.coordinates = [](NodeId) { return Coordinates{2,2,2}; };
    neighbors.node_at = [](Coordinates p) { return static_cast<NodeId>(1 + p.x + 8*p.y + 64*p.z); };
    neighbors.record = [](const NeighborDescriptor&, Coordinates) { return NeighborRecord{100,false}; };
    neighbors.scalar = [](const NeighborDescriptor&, Coordinates, Coordinates,
        std::int32_t category, std::uint32_t& scalar, std::int32_t argument) {
        assert(category == 3 && argument == 4); scalar = 40;
    };
    int other_calls = 0;
    neighbors.accept = [&](const NeighborDescriptor&, Coordinates, Coordinates,
        const NeighborRecord&, bool, std::int32_t&) { ++other_calls; return false; };
    auto bound = with_creature_acceptance(neighbors, f.h);
    NeighborDescriptor desc; desc.object = 123; desc.unknown_08 = 1;
    auto out = generate_neighbors(1, {}, desc, bound);
    assert(out.size() == 26 && f.base_calls == 26 && f.calls.empty() && other_calls == 0);
    for (const auto& candidate : out) assert(candidate.movement.category == 3);
    desc.mode = 1; desc.start = desc.target = {2,2,2};
    assert(generate_neighbors(1,{},desc,bound).empty());
    assert(other_calls == 26 && f.base_calls == 26);
}
} // namespace
int main() {
    parameters_and_failure(); all_intermediate_combinations();
    exemptions_wrapping_and_rereads(); generator_integration();
}
