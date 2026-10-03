#include "route_cell_validity.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "cell-validity-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
std::size_t cases=0,footprint_cases=0,movement_cases=0;
void set_cell(OccupancyCell& c,std::uint16_t id,std::uint16_t flags) {
    std::memcpy(c.unknown_00.data(),&id,2); c.flags_0a=flags&255; c.unknown_0b=std::byte(flags>>8);
}
struct Fixture {
    std::vector<OccupancyCell> data=std::vector<OccupancyCell>(4*3*16);
    std::vector<std::int32_t> rows{0,4,8},layers;
    std::vector<TerrainValidityRecord> terrain=std::vector<TerrainValidityRecord>(3);
    CellValidityMapView v;
    Fixture() {
        for(int z=0;z<16;++z) layers.push_back(12*z);
        v={{{4,3,8},data.data(),data.size(),rows.data(),rows.size(),layers.data(),layers.size(),12,8},
            terrain.data(),terrain.size()};
    }
    OccupancyCell& cell(Coordinates q) { return data.at(q.x+rows.at(q.y)+layers.at(q.z)); }
    void reset() { for(auto& c:data) c=OccupancyCell{}; }
    void check(Coordinates pos,const CreatureMovementParameters& p,bool expected,bool footprint=false) {
        const auto before=data; const auto records=terrain; const auto parameters=p;
        const auto h=with_cell_validity_test(ValidityHelpers{v.map.dimensions,{}},v);
        const auto result=footprint?test_validity(pos,p,h):test_cell_validity(pos,p,v);
        assert(result==expected);
#ifdef MNM_NATIVE_REFERENCE
        assert(native_reference::run_cell_rules(pos,p,v,footprint)==result);
#endif
        assert(std::memcmp(before.data(),data.data(),data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(records.data(),terrain.data(),terrain.size()*sizeof(TerrainValidityRecord))==0);
        assert(std::memcmp(&parameters,&p,sizeof(p))==0);
        if(footprint) ++footprint_cases; else ++cases;
    }
};
void base_rules() {
    Fixture f; CreatureMovementParameters p;
    for(unsigned low=0;low<256;++low) for(unsigned high:{0U,0x40U,0x80U,0xc0U})
        for(unsigned id:{0U,1U}) for(int classification:{0,7,8,16,-1})
            for(unsigned special:{0U,1U,255U}) for(int extent:{0,1,2}) {
                const auto flags=std::uint16_t(low|(high<<8));
                set_cell(f.cell({2,1,1}),id,flags); f.terrain[1].classification_94=classification;
                p.type_0c=extent; p.unknown_10=special;
                p.type_08=-7; p.object_104=99; p.type_44=2; p.type_index=22; p.special_state=1;
                const bool class_block=id && !(low&0x80) && classification==16;
                const bool flag_block=!special && (low&0x20) && !(low&0x10);
                f.check({2,1,1},p,!(high&0x40) && !class_block && !flag_block);
            }
}
void above_rules() {
    Fixture f; CreatureMovementParameters p; p.type_0c=3;
    f.terrain[1].classification_94=16; // overhead flags bypass classification entirely
    for(unsigned low=0;low<256;++low) for(unsigned high:{0U,0x40U,0x80U,0xc0U})
        for(unsigned id:{0U,1U}) for(unsigned special:{0U,1U,255U}) {
            set_cell(f.cell({1,2,2}),id,std::uint16_t(low|(high<<8))); p.unknown_10=special;
            const bool blocked=(high&0x40) || (id && !(low&0x80))
                || (!special && (low&0x20) && !(low&0x10));
            f.check({1,2,1},p,!blocked);
        }
}
void height_and_boundaries() {
    Fixture f; CreatureMovementParameters p;
    for(int start:{0,1,4,7,8,10}) for(int extent:{-2,0,1,2,4})
        for(int limit:{0,1,4,8,12}) for(int classification:{0,7,8,16,-1}) {
            f.reset(); f.terrain[1].classification_94=classification;
            set_cell(f.cell({1,1,start}),1,0); p.type_0c=extent;
            f.v.map.layer_count=limit;
            const auto actual_extent=classification!=16 && (classification<0 || classification>=8) && extent==1?2:extent;
            f.check({1,1,start},p,classification!=16 && start+actual_extent<=limit);
        }
    f.reset(); f.v.map.layer_count=8; p.type_0c=std::numeric_limits<int>::max();
    f.check({1,1,1},p,true); // signed end overflow skips the overhead scan
    p.type_0c=std::numeric_limits<int>::min(); f.check({1,1,1},p,true);
    // Promotion and overhead rejection are independent of occupant flags.
    f.reset(); set_cell(f.cell({1,1,1}),1,0); set_cell(f.cell({1,1,2}),0,0x4000);
    p.type_0c=1; f.terrain[1].classification_94=7; f.check({1,1,1},p,true);
    f.terrain[1].classification_94=8; f.check({1,1,1},p,false);
    f.terrain[1].classification_94=-1; f.check({1,1,1},p,false);
    set_cell(f.cell({1,1,1}),1,0x80); f.check({1,1,1},p,true); // classification lookup bypassed
    // Arbitrary offsets and plane stride select the overhead cells.
    f.reset(); f.rows[0]=4; f.layers[1]=24; f.v.map.plane_stride=24;
    set_cell(f.data[4+24+1+48],1,0); p.type_0c=3;
    f.check({1,0,1},p,false);
}
void lookup_shortcuts_and_unsigned_id() {
    Fixture f; CreatureMovementParameters p; p.type_0c=1;
    f.v.terrain=nullptr; f.v.terrain_count=0;
    set_cell(f.cell({1,1,1}),1,0x4000); f.check({1,1,1},p,false);
    set_cell(f.cell({1,1,1}),1,0x80); f.check({1,1,1},p,true);
    set_cell(f.cell({1,1,1}),0,0); f.check({1,1,1},p,true);
    f.terrain.resize(65536); f.v.terrain=f.terrain.data(); f.v.terrain_count=f.terrain.size();
    for(unsigned id:{32768U,65535U}) {
        set_cell(f.cell({1,1,1}),id,0);
        f.terrain[id].classification_94=16; f.check({1,1,1},p,false);
        f.terrain[id].classification_94=7; f.check({1,1,1},p,true);
    }
}
void integrated_chains() {
    Fixture f; CreatureMovementParameters p; p.type_0c=3;
    const auto validity=with_cell_validity_test(ValidityHelpers{f.v.map.dimensions,{}},f.v);
    for(int type:{1,2}) for(int x:{0,3}) for(int y:{0,2})
        for(int bx=0;bx<4;++bx) for(int by=0;by<3;++by) for(int bz:{0,1,2,3,4,7}) {
            f.reset(); set_cell(f.cell({bx,by,bz}),0,0x4000); p.type_08=type;
            const bool in_xy=(bx==x || (type==2 && bx==(x+1)%4)) && (by==y || (type==2 && by==(y+1)%3));
            f.check({x,y,1},p,!(in_xy && bz>=1 && bz<4),true);
        }
    MovementHelpers m; m.dimensions=f.v.map.dimensions;
    m.boundary=[] { return 6; }; m.layer_count=[] { return 8; };
    m.record=[](Coordinates,const CreatureMovementParameters&) { return 1U; };
    m.check=[](MovementCheck kind,Coordinates,const CreatureMovementParameters&) {
        assert(kind!=MovementCheck::check_46b0); return false;
    };
    m=with_validity_test(m,validity);
    for(unsigned pattern=0;pattern<256;++pattern) for(int type:{0,1,2}) for(int record:{-1,0}) {
        f.reset();
        for(unsigned bit=0;bit<8;++bit) if(pattern&(1U<<bit))
            set_cell(f.cell({int(bit%4),int(bit/4),2}),0,0x4000);
        p.type_0c=0; p.type_08=type; int category=9;
        const auto result=test_movement({1,0,2},{3,1,2},p,category,record,m);
        assert(category==(result?0:5));
#ifdef MNM_NATIVE_REFERENCE
        const auto before=f.data; const auto parameters=p; int actual_category=9;
        assert(native_reference::run_cell_rules_movement({1,0,2},{3,1,2},p,actual_category,record,m,f.v)==result);
        assert(actual_category==category);
        assert(std::memcmp(before.data(),f.data.data(),f.data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(&parameters,&p,sizeof(p))==0);
#endif
        ++movement_cases;
    }
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2); native_reference::initialize(argv[1],true); // preserve 46b0 and 3440
#else
    (void)argc; (void)argv;
#endif
    base_rules(); above_rules(); height_and_boundaries(); lookup_shortcuts_and_unsigned_id(); integrated_chains();
    std::cout << "Cell validity checks passed: " << cases << " direct, " << footprint_cases
        << " footprint, " << movement_cases << " movement cases.\n";
}
