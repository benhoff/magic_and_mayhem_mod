#include "route_clearance.hpp"
#include "route_cell_support.hpp"
#include "route_boundary.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "clearance-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
std::size_t any_cases=0,tall_cases=0,movement_cases=0;
void set_cell(OccupancyCell& c,std::uint16_t id,unsigned flags=0) {
    std::memcpy(c.unknown_00.data(),&id,2); c.flags_0a=flags&255; c.unknown_0b=std::byte(flags>>8);
}
struct Fixture {
    std::vector<OccupancyCell> data;
    std::vector<int> rows,layers;
    std::vector<TerrainValidityRecord> terrain=std::vector<TerrainValidityRecord>(2);
    CellValidityMapView v;
    Fixture(int width=8,int height=7):data(width*height*8) {
        for(int y=0;y<height;++y) rows.push_back(width*y);
        for(int z=0;z<8;++z) layers.push_back(width*height*z);
        v={{{width,height,8},data.data(),data.size(),rows.data(),rows.size(),layers.data(),layers.size(),width*height,8},
            terrain.data(),terrain.size()};
    }
    OccupancyCell& cell(Coordinates q) { return data.at(q.x+rows.at(q.y)+layers.at(q.z)); }
    void reset() { for(auto& c:data) c=OccupancyCell{}; }
    void check(Coordinates pos,const CreatureMovementParameters& p,bool expected,bool tall) {
        const auto before=data; const auto records=terrain;
        const auto parameters=p;
        const auto result=tall?test_tall_terrain(pos,p,v):test_any_terrain(pos,p,v);
        assert(result==expected);
#ifdef MNM_NATIVE_REFERENCE
        assert(native_reference::run_clearance(pos,p,v,tall)==result);
#endif
        assert(std::memcmp(before.data(),data.data(),data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(records.data(),terrain.data(),terrain.size()*sizeof(TerrainValidityRecord))==0);
        assert(std::memcmp(&parameters,&p,sizeof(p))==0);
        if(tall) ++tall_cases; else ++any_cases;
    }
};
void predicates() {
    for(int shape:{0,1}) {
        Fixture f(shape?8:6,shape?7:6); const auto d=f.v.map.dimensions;
        for(unsigned mask=0;mask<512;++mask) for(int seam:{0,1}) {
            f.reset(); const int x=seam?d.x-1:0,y=seam?d.y-1:0;
            for(unsigned bit=0;bit<9;++bit) if(mask&(1U<<bit))
                set_cell(f.cell({(x+int(bit%3))%d.x,(y+int(bit/3))%d.y,2}),1,0xffff);
            for(int extent:{-1,0,1,2,3}) for(int classification:{0,7,8,16,-1}) {
                CreatureMovementParameters p; p.type_08=extent; p.type_0c=99; p.object_104=-7;
                p.type_index=22; p.unknown_10=255; p.special_state=1; p.type_44=3;
                f.terrain[1].classification_94=classification;
                bool hit=false;
                if(extent>0) for(unsigned bit=0;bit<9;++bit)
                    if(int(bit%3)<extent && int(bit/3)<extent && (mask&(1U<<bit))) hit=true;
                f.check({x,y,2},p,hit,false);
                f.check({x,y,2},p,hit && (classification<0 || classification>=8),true);
            }
        }
    }
}
void layout_and_cycles() {
    Fixture f; CreatureMovementParameters p; p.type_08=1;
    // Flags and terrain eligibility do not affect either check; 44e0 does not read a record.
    set_cell(f.cell({1,1,2}),1,0x80); f.v.terrain=nullptr; f.v.terrain_count=0;
    f.check({1,1,2},p,true,false);
    f.v.terrain=f.terrain.data(); f.v.terrain_count=f.terrain.size();
    f.terrain[1].classification_94=8; f.terrain[1].flags_b0=0;
    f.check({1,1,2},p,true,true);
    // Global row/layer tables select storage even when their offsets are unusual.
    f.reset(); f.rows[0]=8; f.layers[2]=2*56+8; set_cell(f.data[8+2*56+8+1],1);
    f.check({1,0,2},p,true,false); f.check({1,0,2},p,true,true);
    // Empty extent performs no cell/table reads.
    f.v.map.cells=nullptr; f.v.map.row_offsets=nullptr; f.v.map.layer_offsets=nullptr;
    p.type_08=0; f.check({0,0,-1},p,false,false); f.check({0,0,-1},p,false,true);
    Fixture cycle(2,2); p.type_08=2;
    for(bool tall:{false,true}) {
        bool threw=false;
        try { if(tall) (void)test_tall_terrain({0,0,2},p,cycle.v); else (void)test_any_terrain({0,0,2},p,cycle.v); }
        catch(const std::runtime_error&) { threw=true; }
        assert(threw);
        set_cell(cycle.cell({0,0,2}),1); cycle.terrain[1].classification_94=8;
        cycle.check({0,0,2},p,true,tall); // preserve early hits despite oversized extent
        cycle.reset();
    }
    Fixture ycycle(4,2); p.type_08=2;
    bool threw=false;
    try { (void)test_any_terrain({0,0,2},p,ycycle.v); } catch(const std::runtime_error&) { threw=true; }
    assert(threw);
    // Unsigned terrain indices and negative classifications.
    Fixture large; large.terrain.resize(65536); large.v.terrain=large.terrain.data(); large.v.terrain_count=large.terrain.size();
    p.type_08=1;
    for(unsigned id:{32768U,65535U}) {
        set_cell(large.cell({1,1,2}),id); large.terrain[id].classification_94=-1;
        large.check({1,1,2},p,true,true);
    }
}
void complete_movement() {
    Fixture receiver(6,6),global(6,6);
    global.v.terrain=receiver.terrain.data(); global.v.terrain_count=receiver.terrain.size();
    receiver.terrain[1].classification_94=8; receiver.terrain[1].flags_b0=8;
    auto support=with_cell_support(RecordQueryHelpers{receiver.v.map.dimensions,[]{ return 4; },{}},receiver.v);
    const auto valid=with_cell_validity_test(ValidityHelpers{receiver.v.map.dimensions,{}},receiver.v);
    MovementHelpers m; m.dimensions=receiver.v.map.dimensions; m.boundary=support.boundary; m.layer_count=[]{ return 8; };
    m=with_clearance_tests(m,global.v);
    m=with_boundary_test(with_validity_test(with_record_query(m,support),valid));
    for(unsigned pattern=0;pattern<64;++pattern) {
        receiver.reset(); global.reset();
        for(unsigned bit=0;bit<6;++bit) {
            if(pattern&(1U<<bit)) set_cell(receiver.cell({int(bit%3),int(bit/3),1}),0,0x4000);
            if(pattern&(1U<<bit)) set_cell(global.cell({int(bit%3),int(bit/3),3}),1,0x80);
        }
        const auto receiver_before=receiver.data,global_before=global.data;
        const auto records=receiver.terrain;
        for(int mode:{0,1,2,3}) for(int capability:{0,1}) for(int type:{1,2})
            for(int source:{-1,0,1}) for(int dz:{-1,0,1}) for(int index:{0,22}) {
                CreatureMovementParameters p; p.type_44=mode; p.type_0c=1; p.object_104=capability;
                p.type_08=type; p.type_index=index; const auto original=p; int category=9;
                const auto result=test_movement({1,0,2},{2,1,2+dz},p,category,source,m);
                assert(result?(category>=0 && category<=4):category==5);
#ifdef MNM_NATIVE_REFERENCE
                int actual_category=9;
                assert(native_reference::run_complete_movement({1,0,2},{2,1,2+dz},p,actual_category,source,
                    receiver.v,global.v,4)==result);
                assert(actual_category==category);
#endif
                assert(std::memcmp(&p,&original,sizeof(p))==0); ++movement_cases;
            }
        assert(std::memcmp(receiver_before.data(),receiver.data.data(),receiver.data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(global_before.data(),global.data.data(),global.data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(records.data(),receiver.terrain.data(),records.size()*sizeof(TerrainValidityRecord))==0);
    }
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2 || argc==3); native_reference::initialize(argv[1],true,true,true,true); // no instruction redirects
    if(argc==3) {
        Fixture f(2,2); CreatureMovementParameters p; p.type_08=2;
        (void)native_reference::run_clearance({0,0,2},p,f.v,std::string(argv[2])=="--cycle-tall");
        return 3; // empty oversized scan is expected to be killed by the driver's timeout
    }
#else
    (void)argc; (void)argv;
#endif
    predicates(); layout_and_cycles(); complete_movement();
    std::cout << "Clearance checks passed: " << any_cases << " any-terrain, " << tall_cases
        << " tall-terrain, " << movement_cases << " complete movement cases.\n";
}
