#include "route_movement.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <tuple>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "movement-native-reference.hpp"
#endif

using namespace mnm::reconstruction;
namespace {
using Call = std::tuple<std::uint32_t,int,int,int>;
std::size_t case_count = 0;
struct Fixture {
    MovementHelpers h;
    CreatureMovementParameters p;
    std::vector<Call> calls;
    std::uint32_t source_record = 1, dest_record = 1;
    bool passable = true, band = false, solid = false, tall = false;
    int boundary = 4;
    Coordinates from{1,1,2}, to{2,1,2};
    Fixture() {
        h.dimensions = {8,8,8};
        h.boundary = [&] { return boundary; }; h.layer_count = [] { return 8; };
        h.record = [&](Coordinates pos, const CreatureMovementParameters&) {
            calls.emplace_back(0x4f4330,pos.x,pos.y,pos.z);
            return pos.x == to.x && pos.y == to.y && pos.z == to.z ? dest_record : source_record;
        };
        h.check = [&](MovementCheck kind, Coordinates pos, const CreatureMovementParameters&) {
            calls.emplace_back(static_cast<std::uint32_t>(kind),pos.x,pos.y,pos.z);
            switch (kind) {
                case MovementCheck::check_46b0: return passable;
                case MovementCheck::check_41a0: return band;
                case MovementCheck::check_44e0: return solid;
                case MovementCheck::check_45b0: return tall;
            }
            return false;
        };
    }
    bool run(int& category, int override_record = -1) {
        ++case_count;
        calls.clear();
        const auto result = test_movement(from,to,p,category,override_record,h);
#ifdef MNM_NATIVE_REFERENCE
        const auto model_calls = calls; calls.clear(); int reference_category = 99;
        const auto reference = native_reference::run(from,to,p,reference_category,override_record,h);
        if (reference != result || reference_category != category || calls != model_calls) {
            std::cerr << "Native mismatch: from " << from.x << ',' << from.y << ',' << from.z
                << " to " << to.x << ',' << to.y << ',' << to.z << " mode " << p.type_44
                << " result/category " << result << '/' << category << " reference "
                << reference << '/' << reference_category << '\n';
            std::cerr << "extent/width " << p.type_0c << '/' << p.type_08 << " override " << override_record << '\n';
            for (const auto& call : model_calls) std::cerr << "model " << std::hex << std::get<0>(call)
                << std::dec << ' ' << std::get<1>(call) << ',' << std::get<2>(call) << ',' << std::get<3>(call) << '\n';
            for (const auto& call : calls) std::cerr << "native " << std::hex << std::get<0>(call)
                << std::dec << ' ' << std::get<1>(call) << ',' << std::get<2>(call) << ',' << std::get<3>(call) << '\n';
            assert(false);
        }
#endif
        return result;
    }
};
void categories_and_early_exits() {
    Fixture f; int category = 99;
    assert(f.run(category) && category == 0);
    f.p.type_index = 22; assert(!f.run(category) && category == 5 && f.calls.empty());
    f.p.special_state = 1; assert(f.run(category)); f.p.type_index = 0; f.p.special_state = 0;
    f.passable = false; assert(!f.run(category) && category == 5 && f.calls.size() == 1);
    f.passable = true; f.dest_record = 0; f.p.object_104 = 1;
    assert(f.run(category) && category == 1);
    f.to.z = 4; f.from.z = 3; assert(f.run(category) && category == 2);
    f.to.z = 5; assert(!f.run(category) && category == 5);
    f.p.special_state = 1; assert(f.run(category) && category == 2);
    f.p.special_state = 0; f.source_record = 0; f.dest_record = 1;
    f.from.z = 4; f.to.z = 3; assert(f.run(category) && category == 3);
    f.to.z = 2; assert(!f.run(category) && category == 5);
    f.p.type_44 = 2; f.p.type_0c = 2; f.from.z = 2; f.to.z = 2;
    f.dest_record = 0; assert(f.run(category) && category == 4);
    f.from.z = f.to.z = 1; assert(!f.run(category) && category == 5);
    f.from.z = 2; f.to.z = 2; f.source_record = 1;
    assert(f.run(category, 1) && category == 0); // skip source valid/record callbacks
    for (const auto& call : f.calls) assert(std::get<0>(call) != 0x4f4330 || std::get<1>(call) == f.to.x);
}
void late_rejection_and_groups() {
    Fixture f; int category;
    f.p.type_0c = 1; f.tall = f.solid = true;
    assert(!f.run(category) && category == 5);
    f.from.z = f.to.z = 7; assert(f.run(category) && category == 0); // top-layer exemption
    f.from.z = 2; f.to.z = 3; f.to.x = f.from.x;
    assert(!f.run(category) && category == 5);
    f.p.type_0c = 0; f.p.type_08 = 1; f.from = {1,2,3}; f.to = {2,3,4};
    // Initial source/destination remain valid; six intermediate probes vary independently.
    const std::array<Coordinates,6> points{{{2,2,3},{1,3,3},{2,3,3},{1,2,4},{2,2,4},{1,3,4}}};
    for (unsigned mask = 0; mask < 64; ++mask) {
        f.h.check = [&](MovementCheck kind, Coordinates pos, const CreatureMovementParameters&) {
            f.calls.emplace_back(static_cast<std::uint32_t>(kind),pos.x,pos.y,pos.z);
            if (kind != MovementCheck::check_46b0) return false;
            for (unsigned i = 0; i < points.size(); ++i)
                if (pos.x == points[i].x && pos.y == points[i].y && pos.z == points[i].z) return (mask & (1U<<i)) != 0;
            return true;
        };
        const bool expected = (mask & 7) == 7 || (mask & 56) == 56;
        assert(f.run(category) == expected && category == (expected ? 0 : 5));
    }
}
void broad_cases() {
    // Deterministic matrix covers all categories, boundary endpoints, vertical
    // directions, record overrides and parameter modes with exact native comparison.
    Fixture f; int category;
    for (int mode : {0,1,2,3}) for (int extent : {0,1,2})
    for (int z : {1,2,3,4,5}) for (int dz : {-1,0,1})
    for (unsigned flags = 0; flags < 16; ++flags) {
        f.p = {}; f.p.type_44 = mode; f.p.type_0c = extent;
        f.p.object_104 = flags & 1; f.p.special_state = (flags >> 1) & 1;
        f.source_record = (flags >> 2) & 1; f.dest_record = (flags >> 3) & 1;
        f.from = {1,1,z}; f.to = {2,2,z+dz};
        f.p.type_08 = flags % 3; f.solid = (flags == 12); f.tall = (flags == 12);
        f.run(category);
        f.run(category, static_cast<int>(f.source_record));
    }
}
void varied_lower_helpers() {
    Fixture f; int category;
    std::uint32_t state = 0x572981ab;
    const auto random = [&state] { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; };
    for (unsigned scenario = 0; scenario < 20000; ++scenario) {
        const auto salt = random();
        f.p = {};
        f.p.type_0c = random() % 4; f.p.type_08 = random() % 3;
        f.p.type_44 = random() % 4; f.p.object_104 = random() % 2;
        f.p.type_index = random() % 24; f.p.special_state = random() % 2;
        f.boundary = 2 + random() % 5;
        f.from = {int(random()%8),int(random()%8),int(random()%8)};
        f.to = {int(random()%8),int(random()%8),int(random()%8)};
        // Pure lower providers: results vary by coordinate/kind without hidden
        // mutation, so original and model see exactly the same world snapshot.
        const auto hash = [salt](Coordinates pos, std::uint32_t kind) {
            auto n = salt ^ kind ^ std::uint32_t(pos.x)*0x13579U
                ^ std::uint32_t(pos.y)*0x24681U ^ std::uint32_t(pos.z)*0x87654U;
            n ^= n >> 11; n *= 0x9e3779b1U; n ^= n >> 16; return n;
        };
        f.h.record = [&](Coordinates pos, const CreatureMovementParameters&) {
            f.calls.emplace_back(0x4f4330,pos.x,pos.y,pos.z);
            return hash(pos,0x4f4330) % 2 ? 0x600010U : 0U;
        };
        f.h.check = [&](MovementCheck kind, Coordinates pos, const CreatureMovementParameters&) {
            f.calls.emplace_back(static_cast<std::uint32_t>(kind),pos.x,pos.y,pos.z);
            return hash(pos,static_cast<std::uint32_t>(kind)) % 4 != 0;
        };
        f.run(category, scenario % 3 == 0 ? -1 : (scenario % 3 == 1 ? 0 : static_cast<int>(0x600010)));
    }
}
void mixed_z_clearance_regression() {
    Fixture f; int category;
    f.from = {1,6,3}; f.to = {3,7,5}; f.p.type_0c = 1; f.p.type_08 = 1;
    f.h.check = [&](MovementCheck kind, Coordinates pos, const CreatureMovementParameters&) {
        f.calls.emplace_back(static_cast<std::uint32_t>(kind),pos.x,pos.y,pos.z);
        if (kind == MovementCheck::check_46b0 || kind == MovementCheck::check_41a0) return true;
        if (kind == MovementCheck::check_44e0) return pos.x == 1 && pos.y == 6 && pos.z == 4;
        return pos.x == 3 && pos.y == 7 && pos.z == 3; // high XY at LOW Z, not (3,7,5)
    };
    assert(!f.run(category,1) && category == 5);
}
void adapters() {
    Fixture f; CreatureAcceptanceHelpers acceptance;
    acceptance.dimensions = f.h.dimensions;
    acceptance.object_state = [](std::uint32_t token) { assert(token == 123); return CreatureAcceptanceState{}; };
    acceptance.cell_test = [](Coordinates, std::int32_t, const CreatureMovementParameters&) { return true; };
    acceptance = with_movement_test(acceptance, f.h);
    int category = 99;
    assert(accept_creature_move(123,f.from,f.to,true,category,acceptance) && category == 0);
    NeighborHelpers neighbors;
    neighbors.dimensions = f.h.dimensions;
    neighbors.coordinates = [](NodeId) { return Coordinates{2,2,2}; };
    neighbors.node_at = [](Coordinates pos) { return static_cast<NodeId>(1+pos.x+8*pos.y+64*pos.z); };
    neighbors.record = [](const NeighborDescriptor&, Coordinates) { return NeighborRecord{100,false,1}; };
    neighbors.scalar = [](const NeighborDescriptor&, Coordinates, Coordinates, std::int32_t category,
        std::uint32_t& scalar, std::int32_t) { assert(category == 0); scalar = 40; };
    neighbors = with_standard_movement_test(neighbors, f.h);
    neighbors = with_creature_acceptance(neighbors, acceptance);
    NeighborDescriptor desc; desc.object = 123; desc.unknown_08 = 1;
    // All records are present; the movement core emits category zero on both paths.
    auto out = generate_neighbors(1,{},desc,neighbors);
    assert(out.size() == 26);
    for (const auto& candidate : out) assert(candidate.movement.category == 0);
    desc.mode = 1; desc.start = desc.target = {2,2,2};
    out = generate_neighbors(1,{},desc,neighbors);
    assert(out.size() == 26);
    for (const auto& candidate : out) assert(candidate.movement.category == 0);
}
} // namespace
int main(int argc, char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc == 2); native_reference::initialize(argv[1]);
#else
    (void)argc; (void)argv;
#endif
    categories_and_early_exits(); late_rejection_and_groups(); broad_cases();
    varied_lower_helpers(); mixed_z_clearance_regression(); adapters();
    std::cout << "Movement reconstruction checks passed: " << case_count << " cases.\n";
}
