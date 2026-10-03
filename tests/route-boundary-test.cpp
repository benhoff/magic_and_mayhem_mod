#include "route_boundary.hpp"
#include "route_cell_support.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <tuple>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "boundary-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
using Call=std::tuple<unsigned,int,int,int>;
std::size_t direct_cases=0,map_cases=0,movement_cases=0;
void compare(int mode,int boundary,int extent,int z,int capability,unsigned support,bool valid) {
    CreatureMovementParameters p; p.type_44=mode; p.type_0c=extent; p.object_104=capability;
    p.type_08=2; p.type_index=22; p.unknown_10=0xab;
    const auto original=p; std::vector<Call> calls;
    BoundaryHelpers h{[&]{ return boundary; },[&](Coordinates q,const CreatureMovementParameters& parameters) {
        assert(&parameters==&p); calls.emplace_back(0x4f4330,q.x,q.y,q.z); return support;
    },[&](Coordinates q,const CreatureMovementParameters& parameters) {
        assert(&parameters==&p); calls.emplace_back(0x4f46b0,q.x,q.y,q.z); return valid;
    }};
    const int lower=boundary-extent;
    bool expected=false; std::vector<Call> order;
    if(mode==0) expected=z<boundary;
    if(mode==1) {
        expected=z<=lower || (z<boundary && !support && !capability);
        if(z>lower && z<boundary) order.emplace_back(0x4f4330,-3,99,z);
    }
    if(mode==2) {
        expected=z<lower || (z==lower && !valid) || (z>lower && z<boundary && !support);
        if(z==lower) order.emplace_back(0x4f46b0,-3,99,z+1);
        else if(z>lower && z<boundary) order.emplace_back(0x4f4330,-3,99,z);
    }
    if(mode==3) {
        expected=z<boundary && !support && !capability;
        if(z<boundary) order.emplace_back(0x4f4330,-3,99,z);
    }
    const auto result=test_boundary({-3,99,z},p,h);
    assert(result==expected && calls==order);
#ifdef MNM_NATIVE_REFERENCE
    calls.clear(); assert(native_reference::run_boundary({-3,99,z},p,h)==result && calls==order);
#endif
    assert(std::memcmp(&p,&original,sizeof(p))==0); ++direct_cases;
}
void extremes_and_rereads() {
    const int minimum=std::numeric_limits<int>::min(),maximum=std::numeric_limits<int>::max();
    struct Case { int boundary,extent,z,mode; bool expected; int valid_z; };
    for(auto c:{Case{minimum,0,minimum,1,true,0},Case{minimum,1,minimum,1,false,0},
        Case{minimum,1,minimum,2,true,0},Case{minimum,1,maximum,2,true,minimum},
        Case{maximum,0,maximum-1,1,false,0},Case{maximum,1,maximum-1,2,true,maximum}}) {
        CreatureMovementParameters p; p.type_44=c.mode; p.type_0c=c.extent; int count=0;
        BoundaryHelpers h{[&]{ return c.boundary; },[](Coordinates,const CreatureMovementParameters&)->unsigned {
            assert(false); return 0;
        },[&](Coordinates q,const CreatureMovementParameters&) { assert(q.z==c.valid_z); ++count; return false; }};
        assert(test_boundary({0,0,c.z},p,h)==c.expected);
#ifdef MNM_NATIVE_REFERENCE
        const int expected_count=count; count=0;
        assert(native_reference::run_boundary({0,0,c.z},p,h)==c.expected && count==expected_count);
#endif
        ++direct_cases;
    }
    // Mode 1 reads capability after support returns zero.
    CreatureMovementParameters p; p.type_44=1; p.type_0c=4;
    BoundaryHelpers h{[]{ return 6; },[&](Coordinates,const CreatureMovementParameters&) {
        p.object_104=1; return 0U;
    },[](Coordinates,const CreatureMovementParameters&){ assert(false); return true; }};
    assert(!test_boundary({0,0,3},p,h));
#ifdef MNM_NATIVE_REFERENCE
    p.object_104=0; assert(!native_reference::run_boundary({0,0,3},p,h));
#endif
    ++direct_cases;
    // Mode 2 recomputes its exact lower boundary after a successful support query.
    p={}; p.type_44=2; p.type_0c=4;
    h.support=[&](Coordinates,const CreatureMovementParameters&){ p.type_0c=3; return 1U; };
    h.valid=[](Coordinates q,const CreatureMovementParameters&){ assert(q.z==4); return false; };
    assert(test_boundary({0,0,3},p,h));
#ifdef MNM_NATIVE_REFERENCE
    p.type_0c=4; assert(native_reference::run_boundary({0,0,3},p,h));
#endif
    ++direct_cases;
}
void direct() {
    for(int mode:{-1,0,1,2,3,4,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()})
        for(int boundary:{-2,0,1,4,8}) for(int extent:{-2,0,1,2,4}) for(int z=-3;z<=10;++z)
            for(int capability:{0,1,-1}) for(unsigned support:{0U,1U,0x80000000U}) for(bool valid:{false,true})
                compare(mode,boundary,extent,z,capability,support,valid);
    extremes_and_rereads();
}
void full_chain() {
    std::vector<OccupancyCell> data(4*3*16);
    std::vector<int> rows{0,4,8},layers; for(int z=0;z<16;++z) layers.push_back(z*12);
    std::vector<TerrainValidityRecord> terrain(2); terrain[1].flags_b0=8;
    CellValidityMapView v{{{4,3,8},data.data(),data.size(),rows.data(),rows.size(),layers.data(),layers.size(),12,8},
        terrain.data(),terrain.size()};
    int boundary=4;
    auto q=with_cell_support(RecordQueryHelpers{v.map.dimensions,[&]{ return boundary; },{}},v);
    const auto valid=with_cell_validity_test(ValidityHelpers{v.map.dimensions,{}},v);
    MovementHelpers m; m.dimensions=v.map.dimensions; m.boundary=q.boundary; m.layer_count=[]{ return 8; };
    m.check=[](MovementCheck kind,Coordinates,const CreatureMovementParameters&) {
        assert(kind==MovementCheck::check_44e0 || kind==MovementCheck::check_45b0); return false;
    };
    m=with_boundary_test(with_validity_test(with_record_query(m,q),valid));
    for(unsigned pattern=0;pattern<256;++pattern) {
        for(auto& c:data) c=OccupancyCell{};
        for(unsigned bit=0;bit<8;++bit) if(pattern&(1U<<bit))
            data[(bit%4)+4*(bit/4)+12].unknown_0b=std::byte{0x40}; // below support
        const auto before=data; const auto records=terrain;
        for(int mode:{0,1,2,3}) for(int extent:{1,2}) for(int z:{1,2,3,4,5})
            for(int capability:{0,1}) for(int type:{1,2}) {
                CreatureMovementParameters p; p.type_44=mode; p.type_0c=extent; p.object_104=capability; p.type_08=type;
                const auto original=p;
                const BoundaryHelpers h{m.boundary,m.record,[&](Coordinates pos,const CreatureMovementParameters& parameters) {
                    return m.check(MovementCheck::check_46b0,pos,parameters);
                }};
                const auto result=test_boundary({3,1,z},p,h);
#ifdef MNM_NATIVE_REFERENCE
                assert(native_reference::run_boundary_map({3,1,z},p,v,boundary)==result);
#else
                (void)result;
#endif
                assert(std::memcmp(&p,&original,sizeof(p))==0); ++map_cases;
            }
        for(int mode:{0,1,2,3}) for(int capability:{0,1}) for(int type:{1,2})
            for(int source:{-1,0,1}) for(int dz:{-1,0,1}) {
                CreatureMovementParameters p; p.type_44=mode; p.type_0c=1; p.object_104=capability; p.type_08=type;
                const auto original=p; int category=9;
                const auto result=test_movement({1,0,2},{3,1,2+dz},p,category,source,m);
                assert(result?(category>=0 && category<=4):category==5);
#ifdef MNM_NATIVE_REFERENCE
                int actual_category=9;
                assert(native_reference::run_cell_rules_movement({1,0,2},{3,1,2+dz},p,actual_category,source,m,v)==result);
                assert(actual_category==category);
#endif
                assert(std::memcmp(&p,&original,sizeof(p))==0); ++movement_cases;
            }
        assert(std::memcmp(before.data(),data.data(),data.size()*sizeof(OccupancyCell))==0);
        assert(std::memcmp(records.data(),terrain.data(),terrain.size()*sizeof(TerrainValidityRecord))==0);
    }
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    const bool full=argc==3 && std::string(argv[2])=="--full-chain";
    assert(argc==2 || full); native_reference::initialize_boundary(argv[1],full);
    if(full) full_chain(); else direct();
#else
    (void)argc; (void)argv; direct(); full_chain();
#endif
    std::cout << "Boundary checks passed: " << direct_cases << " direct, " << map_cases
        << " map, " << movement_cases << " movement cases.\n";
}
