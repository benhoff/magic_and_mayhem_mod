#include "route_cell_support.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "cell-support-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
std::size_t direct_cases=0,aggregate_cases=0,movement_cases=0;
void set_cell(OccupancyCell& c,std::uint16_t id,std::uint16_t f) {
    std::memcpy(c.unknown_00.data(),&id,2); c.flags_0a=f&255; c.unknown_0b=std::byte(f>>8);
}
struct Fixture {
    std::vector<OccupancyCell> data=std::vector<OccupancyCell>(4*3*16);
    std::vector<std::int32_t> rows{0,4,8},layers;
    std::vector<TerrainValidityRecord> terrain=std::vector<TerrainValidityRecord>(3);
    CellValidityMapView v;
    int boundary=4;
    Fixture() {
        for(int z=0;z<16;++z) layers.push_back(z*12);
        v={{{4,3,8},data.data(),data.size(),rows.data(),rows.size(),layers.data(),layers.size(),12,8},
            terrain.data(),terrain.size()};
    }
    OccupancyCell& cell(Coordinates pos) { return data.at(pos.x+rows.at(pos.y)+layers.at(pos.z)); }
    void reset() { for(auto& c:data) c=OccupancyCell{}; }
    void check(Coordinates pos,const CreatureMovementParameters& p,unsigned expected,bool aggregate=false) {
        const auto before=data; const auto parameters=p;
        const auto records=terrain;
        const auto h=with_cell_support(RecordQueryHelpers{v.map.dimensions,[&]{ return boundary; },{}},v);
        const auto result=aggregate?query_record(pos,p,h):test_cell_support(pos,p,v);
        assert(result==expected);
#ifdef MNM_NATIVE_REFERENCE
        assert(native_reference::run_cell_support(pos,p,v,boundary,aggregate)==result);
#endif
        assert(std::memcmp(before.data(),data.data(),data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(records.data(),terrain.data(),terrain.size()*sizeof(TerrainValidityRecord))==0);
        assert(std::memcmp(&parameters,&p,sizeof(p))==0);
        if(aggregate) ++aggregate_cases; else ++direct_cases;
    }
};
void current_rules() {
    Fixture f; CreatureMovementParameters p;
    for(unsigned low=0;low<256;++low) for(unsigned high:{0U,0x40U,0x80U,0xc0U})
        for(unsigned special:{0U,255U}) for(unsigned id:{0U,1U})
            for(int classification:{0,15,16,17,-1}) for(unsigned flag:{0U,8U,255U}) for(int type:{0,22}) {
                set_cell(f.cell({1,1,2}),id,std::uint16_t(low|(high<<8)));
                f.terrain[1].classification_94=classification; f.terrain[1].flags_b0=flag;
                p.unknown_10=special; p.type_index=type;
                const bool allowed=!(!special && (low&0x20) && !(low&0x10)) && !(high&0x40)
                    && id && !(low&0x80) && classification>=0 && classification<16 && ((flag&8) || type==22);
                f.check({1,1,2},p,allowed?1:0);
            }
}
void below_rules() {
    Fixture f; CreatureMovementParameters p;
    for(unsigned low=0;low<256;++low) for(unsigned high:{0U,0x40U,0x80U,0xc0U})
        for(unsigned special:{0U,255U}) for(unsigned id:{0U,1U})
            for(int classification:{0,15,16,17,-1}) for(unsigned flag:{0U,8U,255U}) for(int type:{0,22}) {
                set_cell(f.cell({1,1,1}),id,std::uint16_t(low|(high<<8)));
                f.terrain[1].classification_94=classification; f.terrain[1].flags_b0=flag;
                p.unknown_10=special; p.type_index=type;
                const bool restricted=!special && (low&0x20) && !(low&0x10);
                const bool allowed=!restricted && ((high&0x40)
                    || (id && !(low&0x80) && classification==16 && ((flag&8) || type==22)));
                f.check({1,1,2},p,allowed?1:0);
            }
}
void regressions() {
    Fixture f; CreatureMovementParameters p;
    f.terrain[1].classification_94=0;
    for(unsigned flags=0;flags<256;++flags) for(int type:{0,22}) {
        f.terrain[1].flags_b0=flags; p.type_index=type;
        p.type_0c=-17; p.type_08=999; p.type_44=2; p.object_104=-1; p.special_state=1;
        set_cell(f.cell({1,1,0}),1,0); f.check({1,1,0},p,((flags&8) || type==22)?1:0);
    }
    f.reset(); p={}; p.type_index=22;
    // Invalid current terrain never falls back to the supporting cell below.
    set_cell(f.cell({1,1,1}),0,0x4000); set_cell(f.cell({1,1,2}),1,0);
    f.terrain[1].flags_b0=8;
    for(int classification:{16,17,-1}) { f.terrain[1].classification_94=classification; f.check({1,1,2},p,0); }
    set_cell(f.cell({1,1,2}),1,0x80); f.check({1,1,2},p,1);
    // Restriction has priority even over below-cell 4000 support.
    set_cell(f.cell({1,1,1}),0,0x4020); f.check({1,1,2},p,0);
    p.unknown_10=1; f.check({1,1,2},p,1);
    p.unknown_10=0; set_cell(f.cell({1,1,1}),0,0x4030); f.check({1,1,2},p,1);
    f.v.terrain=nullptr; f.v.terrain_count=0;
    set_cell(f.cell({1,1,2}),1,0x4000); f.check({1,1,2},p,0);
    set_cell(f.cell({1,1,2}),0,0); f.check({1,1,2},p,1);
    set_cell(f.cell({1,1,2}),1,0x80); f.check({1,1,2},p,1);
    set_cell(f.cell({1,1,0}),0,0); f.check({1,1,0},p,0); // no below-cell query at Z=0
    // The below pointer subtracts plane stride rather than using the Z-1 table.
    f.reset(); f.rows[0]=4; f.layers[2]=48; f.v.map.plane_stride=24;
    set_cell(f.data[4+48+1-24],0,0x4000); f.check({1,0,2},p,1);
    // Terrain WORD indices are unsigned, including their high bit.
    f.terrain.resize(65536); f.v.terrain=f.terrain.data(); f.v.terrain_count=f.terrain.size();
    for(unsigned id:{32768U,65535U}) {
        f.reset(); set_cell(f.cell({1,1,2}),id,0);
        f.terrain[id].classification_94=0; f.terrain[id].flags_b0=8; f.check({1,1,2},p,1);
        f.terrain[id].classification_94=-1; f.check({1,1,2},p,0);
    }
}
void aggregate_and_movement() {
    Fixture f; CreatureMovementParameters p;
    f.terrain[1].classification_94=0; f.terrain[1].flags_b0=8;
    const auto q=with_cell_support(RecordQueryHelpers{f.v.map.dimensions,[&]{ return f.boundary; },{}},f.v);
    for(unsigned top=0;top<16;++top) for(unsigned below=0;below<16;++below)
        for(int type:{1,2}) for(int boundary:{2,4}) for(int x:{0,3}) for(int y:{0,2}) {
            f.reset(); f.boundary=boundary; p.type_08=type; p.type_44=2;
            for(unsigned bit=0;bit<4;++bit) {
                const int cx=(x+int(bit%2))%4,cy=(y+int(bit/2))%3;
                if(top&(1U<<bit)) set_cell(f.cell({cx,cy,2}),1,0);
                if(below&(1U<<bit)) set_cell(f.cell({cx,cy,1}),0,0x4000);
            }
            // A below-cell 4000 makes a lower-query hit at the requested Z.
            const auto supported=top|below;
            unsigned count=0; for(unsigned bit=0;bit<4;++bit) count+=(supported>>bit)&1;
            const unsigned expected=type==1 ? ((supported&1)?1:0)
                : boundary==2 ? (supported?1:0)
                : ((count>=3 || supported==6 || supported==9)?1:0);
            f.check({x,y,2},p,expected,true);
        }
    f.boundary=6; p.type_44=0;
    const auto validity=with_cell_validity_test(ValidityHelpers{f.v.map.dimensions,{}},f.v);
    MovementHelpers m; m.dimensions=f.v.map.dimensions;
    m.boundary=[&]{ return f.boundary; }; m.layer_count=[]{ return 8; };
    m.check=[](MovementCheck kind,Coordinates,const CreatureMovementParameters&) {
        assert(kind!=MovementCheck::check_46b0); return false;
    };
    m=with_validity_test(with_record_query(m,q),validity);
    for(unsigned pattern=0;pattern<256;++pattern) for(int type:{1,2}) for(int capability:{0,1})
        for(int override_record:{-1,0,1}) {
            f.reset();
            for(unsigned bit=0;bit<8;++bit) if(pattern&(1U<<bit))
                set_cell(f.cell({int(bit%4),int(bit/4),1}),0,0x4000);
            p={}; p.type_0c=1; p.type_08=type; p.object_104=capability;
            const auto before=f.data; const auto parameters=p; int category=9;
            const auto result=test_movement({1,0,2},{3,1,2},p,category,override_record,m);
            assert(result?(category>=0 && category<=4):category==5);
#ifdef MNM_NATIVE_REFERENCE
            int actual_category=9;
            assert(native_reference::run_cell_rules_movement({1,0,2},{3,1,2},p,actual_category,override_record,m,f.v)==result);
            assert(actual_category==category);
#endif
            assert(std::memcmp(before.data(),f.data.data(),f.data.size()*sizeof(OccupancyCell))==0);
            assert(std::memcmp(&parameters,&p,sizeof(p))==0);
            ++movement_cases;
        }
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2); native_reference::initialize(argv[1],true,true); // preserve all five target routines
#else
    (void)argc; (void)argv;
#endif
    current_rules(); below_rules(); regressions(); aggregate_and_movement();
    std::cout << "Cell support checks passed: " << direct_cases << " direct, " << aggregate_cases
        << " aggregate, " << movement_cases << " movement cases.\n";
}
