#include "route_cell.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "cell-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
using Call = std::tuple<int,int,int,int>;
std::size_t cases = 0;
void compare(MapDimensions dims, Coordinates pos, int type, unsigned mask) {
    CreatureMovementParameters p;
    p.type_08 = type; p.type_0c = 17; p.object_104 = -7;
    p.type_44 = 3; p.unknown_10 = 0xab; p.type_index = 22; p.special_state = 1;
    const auto original = p;
    std::vector<Call> calls;
    CellHelpers h{dims, [&](Coordinates q, int object, const CreatureMovementParameters& parameters) {
        assert(&parameters == &p);
        assert(std::memcmp(&parameters, &original, sizeof(p)) == 0);
        const auto index = calls.size();
        calls.emplace_back(q.x,q.y,q.z,object);
        return (mask & (1U << index)) != 0;
    }};
    const auto result = test_cell(pos, -1234567, p, h);
    const auto wrap = [](int a, int d) { const auto n = (std::int64_t(a)+1)%d; return int(n<0?n+d:n); };
    const int x = wrap(pos.x,dims.x), y = wrap(pos.y,dims.y);
    const std::vector<Call> order{{pos.x,pos.y,pos.z,-1234567}, {pos.x,y,pos.z,-1234567},
        {x,pos.y,pos.z,-1234567}, {x,y,pos.z,-1234567}};
    std::size_t count = 1;
    bool expected = (mask & 1) != 0;
    if (expected && type == 2) {
        while (count < 4 && expected) { expected = (mask & (1U << count)) != 0; ++count; }
    }
    assert(result == expected && calls.size() == count);
    assert(calls == std::vector<Call>(order.begin(), order.begin()+count));
#ifdef MNM_NATIVE_REFERENCE
    const auto reconstructed = calls;
    calls.clear();
    assert(native_reference::run_cell(pos,-1234567,p,h) == result);
    assert(calls == reconstructed);
#endif
    ++cases;
}
void adapters() {
    std::vector<Call> calls;
    CellHelpers cells{{4,4,3}, [&](Coordinates q, int object, const CreatureMovementParameters& p) {
        assert(p.type_08 == 2 && object == 99);
        calls.emplace_back(q.x,q.y,q.z,object); return true;
    }};
    CreatureAcceptanceHelpers acceptance;
    acceptance.dimensions = cells.dimensions;
    acceptance.object_state = [](std::uint32_t token) {
        assert(token == 123); CreatureAcceptanceState s; s.object_00 = 99; s.type_08 = 2; return s;
    };
    acceptance.movement_test = [](Coordinates, Coordinates, const CreatureMovementParameters&,
        int& category, int override_record) { assert(override_record == -1); category=3; return true; };
    acceptance = with_cell_test(acceptance,cells);
    int category = 5;
    assert(accept_creature_move(123,{1,1,1},{3,3,1},false,category,acceptance));
    assert(category == 3);
    assert((calls == std::vector<Call>{{3,3,1,99},{3,0,1,99},{0,3,1,99},{0,0,1,99}}));
    calls.clear();
    assert(accept_creature_move(123,{1,1,1},{3,3,1},true,category,acceptance));
    assert(calls.empty());
    cells.dimensions.x = 5;
    bool threw = false;
    try { (void)with_cell_test(acceptance,cells); } catch (const std::invalid_argument&) { threw = true; }
    assert(threw);
}
void reread_parameter() {
    CreatureMovementParameters p;
    unsigned calls = 0;
    CellHelpers h{{2,2,1}, [&](Coordinates, int, const CreatureMovementParameters&) {
        ++calls; p.type_08 = 2; return true;
    }};
    assert(test_cell({0,0,0},0,p,h) && calls == 4);
#ifdef MNM_NATIVE_REFERENCE
    calls = 0; p.type_08 = 0;
    assert(native_reference::run_cell({0,0,0},0,p,h) && calls == 4);
#endif
}
}
int main(int argc, char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc == 2); native_reference::initialize_cell(argv[1]);
#else
    (void)argc; (void)argv;
#endif
    for (int width : {1,2,8}) for (int height : {1,2,7})
        for (int x=0; x<width; ++x) for (int y=0; y<height; ++y)
            for (int z : {-1,0,2,9}) for (int type : {-1,0,1,2,3})
                for (unsigned mask=0; mask<16; ++mask) compare({width,height,3},{x,y,z},type,mask);
    compare({8,7,3},{-2,-1,2},2,15);
    adapters(); reread_parameter();
    std::cout << "Cell reconstruction checks passed: " << cases << " cases.\n";
}
