#include "route_occupancy.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "occupancy-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
std::size_t cases = 0, footprint_cases = 0;
struct Fixture {
    std::vector<OccupancyCell> data = std::vector<OccupancyCell>(4*3*16);
    std::vector<std::int32_t> rows{0,4,8}, layers;
    OccupancyMapView map;
    Fixture() {
        for (int z=0;z<16;++z) layers.push_back(z*12);
        map = {{4,3,8},data.data(),data.size(),rows.data(),rows.size(),layers.data(),layers.size(),12,8};
    }
    OccupancyCell& cell(Coordinates q) { return data.at(q.x+rows.at(q.y)+layers.at(q.z)); }
    bool run(Coordinates pos, int object, const CreatureMovementParameters& p, bool expected) {
        const auto before = data;
        const auto parameters = p;
        const auto result = test_occupancy(pos,object,p,map);
        assert(result == expected);
#ifdef MNM_NATIVE_REFERENCE
        assert(native_reference::run_occupancy(pos,object,p,map) == result);
#endif
        assert(std::memcmp(before.data(),data.data(),data.size()*sizeof(OccupancyCell)) == 0);
        assert(std::memcmp(&parameters,&p,sizeof(p)) == 0);
        ++cases; return result;
    }
};
void flags_and_identity() {
    Fixture f;
    CreatureMovementParameters p; p.type_0c = 1;
    for (unsigned flags=0;flags<256;++flags)
        for (unsigned owner : {0U,7U,65534U,65535U})
            for (int object : {0,7,-1,65534,65535,65543}) {
                p.type_08 = int(flags); p.object_104 = -7; p.type_44 = 2;
                p.unknown_10 = 0xaa; p.type_index = -17; p.special_state = 1;
                auto& c = f.cell({2,1,3});
                c.flags_0a = flags; c.occupant_04 = owner;
                c.unknown_00.fill(std::byte{0xcd}); c.unknown_06.fill(std::byte{0xef}); c.unknown_0b=std::byte{0xff};
                f.run({2,1,3},object,p,!(flags&3) || owner==65535 || int(owner)==object);
            }
}
void extent_and_limits() {
    Fixture f; CreatureMovementParameters p;
    for (int start : {0,1,3,7,8,10}) for (int extent : {-3,-1,0,1,2,5,12})
        for (int limit : {-1,0,1,4,8}) for (int blocked=0;blocked<16;++blocked) {
            f.data.assign(f.data.size(),OccupancyCell{}); // capacity and address retained
            f.map.cells = f.data.data();
            f.cell({1,2,blocked}).flags_0a = 1;
            f.cell({1,2,blocked}).occupant_04 = 99;
            f.map.layer_count = limit; p.type_0c = extent;
            const bool blocked_in_scan = blocked>=start && blocked<start+extent && blocked<limit;
            f.run({1,2,start},7,p,!blocked_in_scan);
        }
    f.map.layer_count = 8;
    p.type_0c = std::numeric_limits<int>::max();
    f.run({1,2,1},7,p,true); // signed DWORD sum wraps negative, so no scan
    p.type_0c = std::numeric_limits<int>::min();
    f.run({1,2,1},7,p,true);
}
void arbitrary_layout() {
    Fixture f; CreatureMovementParameters p; p.type_0c=3;
    // Row/layer offsets and plane stride, rather than dimensions, select storage.
    f.rows = {4,0,8}; f.layers[1] = 24; f.map.plane_stride = 24;
    f.map.row_offsets=f.rows.data(); f.data[4+24+1+48].flags_0a=2;
    f.data[4+24+1+48].occupant_04=5;
    f.run({1,0,1},7,p,false);
    f.data[4+24+1+48].occupant_04=7;
    f.run({1,0,1},7,p,true);
    // First rejection must happen before later cells can be accessed.
    f.map.cell_count = 1; f.rows[0]=0; f.layers[0]=0;
    f.data[0].flags_0a=3; f.data[0].occupant_04=9;
    f.run({0,0,0},7,p,false);
}
void footprint_integration() {
    Fixture f; CreatureMovementParameters p; p.type_0c=3;
    auto cells = with_occupancy_test(CellHelpers{f.map.dimensions,{}},f.map);
    for (int type : {1,2}) for (int x : {0,3}) for (int y : {0,2})
        for (int bx=0;bx<4;++bx) for (int by=0;by<3;++by)
            for (int bz : {0,1,2,3,4,7}) {
                for (auto& c : f.data) c=OccupancyCell{};
                f.cell({bx,by,bz}).flags_0a=2; f.cell({bx,by,bz}).occupant_04=13;
                p.type_08=type;
                const bool in_xy = (bx==x || (type==2 && bx==(x+1)%4))
                    && (by==y || (type==2 && by==(y+1)%3));
                const bool expected = !(in_xy && bz>=1 && bz<4);
                const auto result = test_cell({x,y,1},7,p,cells);
                assert(result==expected);
#ifdef MNM_NATIVE_REFERENCE
                const auto before=f.data;
                assert(native_reference::run_occupancy({x,y,1},7,p,f.map,true)==result);
                assert(std::memcmp(before.data(),f.data.data(),f.data.size()*sizeof(OccupancyCell))==0);
#endif
                ++footprint_cases;
            }
    // The real occupancy provider participates in creature acceptance too.
    for (auto& c : f.data) c=OccupancyCell{};
    CreatureAcceptanceHelpers acceptance;
    acceptance.dimensions=f.map.dimensions;
    acceptance.object_state=[](std::uint32_t) {
        CreatureAcceptanceState s; s.object_00=7; s.type_0c=3; s.type_08=2; return s;
    };
    acceptance.movement_test=[](Coordinates,Coordinates,const CreatureMovementParameters&,int& category,int) {
        category=3; return true;
    };
    acceptance=with_cell_test(acceptance,cells);
    int category=5;
    assert(accept_creature_move(1,{1,1,1},{3,2,1},false,category,acceptance) && category==3);
    f.cell({0,0,2}).flags_0a=1; f.cell({0,0,2}).occupant_04=13;
    assert(!accept_creature_move(1,{1,1,1},{3,2,1},false,category,acceptance) && category==5);
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2); native_reference::initialize(argv[1]);
#else
    (void)argc; (void)argv;
#endif
    flags_and_identity(); extent_and_limits(); arbitrary_layout(); footprint_integration();
    std::cout << "Occupancy reconstruction checks passed: " << cases << " cell cases, "
        << footprint_cases << " integrated footprint cases.\n";
}
