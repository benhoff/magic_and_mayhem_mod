#include "route_validity.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "validity-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
using Call = std::tuple<unsigned,int,int,int>;
std::size_t cases=0, movement_cases=0;
void compare(MapDimensions dims, Coordinates pos, int type, unsigned mask) {
    CreatureMovementParameters p;
    p.type_0c=3; p.type_08=type; p.object_104=-7; p.type_44=2;
    p.unknown_10=0xab; p.type_index=22; p.special_state=1;
    const auto original=p;
    std::vector<Call> calls;
    ValidityHelpers h{dims,[&](Coordinates q,const CreatureMovementParameters& parameters) {
        assert(&parameters==&p && std::memcmp(&parameters,&original,sizeof(p))==0);
        const auto i=calls.size(); calls.emplace_back(0x4f3440,q.x,q.y,q.z);
        return (mask & (1U<<i))!=0;
    }};
    const auto result=test_validity(pos,p,h);
    const auto wrap=[](int a,int d) { const auto n=(std::int64_t(a)+1)%d; return int(n<0?n+d:n); };
    const int x=wrap(pos.x,dims.x), y=wrap(pos.y,dims.y);
    const std::vector<Call> order{{0x4f3440,pos.x,pos.y,pos.z},{0x4f3440,pos.x,y,pos.z},
        {0x4f3440,x,pos.y,pos.z},{0x4f3440,x,y,pos.z}};
    bool expected=(mask&1)!=0; std::size_t count=1;
    if (expected && type==2) while(count<4 && expected) {
        expected=(mask & (1U<<count))!=0; ++count;
    }
    assert(result==expected && calls==std::vector<Call>(order.begin(),order.begin()+count));
#ifdef MNM_NATIVE_REFERENCE
    const auto model_calls=calls; calls.clear();
    assert(native_reference::run_validity(pos,p,h)==result && calls==model_calls);
#endif
    assert(std::memcmp(&p,&original,sizeof(p))==0);
    ++cases;
}
void reread() {
    CreatureMovementParameters p;
    unsigned count=0;
    ValidityHelpers v{{1,1,1},[&](Coordinates,const CreatureMovementParameters&) {
        ++count; p.type_08=2; return true;
    }};
    assert(test_validity({0,0,0},p,v) && count==4);
#ifdef MNM_NATIVE_REFERENCE
    count=0; p.type_08=0;
    assert(native_reference::run_validity({0,0,0},p,v) && count==4);
#endif
}
void integrated_movement() {
    std::vector<Call> calls;
    for (unsigned blocked=0; blocked<512; ++blocked) for (int type : {0,1,2})
        for (int record : {-1,0}) {
            CreatureMovementParameters p; p.type_08=type;
            ValidityHelpers v{{3,3,4},[&](Coordinates pos,const CreatureMovementParameters& parameters) {
                assert(&parameters==&p);
                calls.emplace_back(0x4f3440,pos.x,pos.y,pos.z);
                return (blocked & (1U << (pos.x+3*pos.y)))==0;
            }};
            MovementHelpers h; h.dimensions=v.dimensions;
            h.boundary=[] { return 3; }; h.layer_count=[] { return 4; };
            h.record=[&](Coordinates q,const CreatureMovementParameters&) {
                calls.emplace_back(0x4f4330,q.x,q.y,q.z); return 1U;
            };
            h.check=[&](MovementCheck kind,Coordinates q,const CreatureMovementParameters&) {
                assert(kind!=MovementCheck::check_46b0);
                calls.emplace_back(unsigned(kind),q.x,q.y,q.z); return false;
            };
            h=with_validity_test(h,v);
            calls.clear(); int category=9;
            const auto result=test_movement({1,1,1},{2,2,1},p,category,record,h);
            assert(category==(result?0:5));
#ifdef MNM_NATIVE_REFERENCE
            const auto model_calls=calls; calls.clear(); int actual_category=9;
            const auto actual=native_reference::run_valid_movement({1,1,1},{2,2,1},p,actual_category,record,h,v);
            assert(actual==result && actual_category==category && calls==model_calls);
#endif
            ++movement_cases;
        }
    MovementHelpers movement; movement.dimensions={3,3,4};
    ValidityHelpers mismatched{{4,3,4},[](Coordinates,const CreatureMovementParameters&) { return true; }};
    bool threw=false;
    try { (void)with_validity_test(movement,mismatched); } catch(const std::invalid_argument&) { threw=true; }
    assert(threw);
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2); native_reference::initialize_validity(argv[1]);
#else
    (void)argc; (void)argv;
#endif
    for(int width : {1,2,8}) for(int height : {1,2,7})
        for(int x=0;x<width;++x) for(int y=0;y<height;++y)
            for(int z : {-1,0,2,9}) for(int type : {-1,0,1,2,3})
                for(unsigned mask=0;mask<16;++mask) compare({width,height,3},{x,y,z},type,mask);
    compare({8,7,3},{-2,-1,2},2,15);
    reread(); integrated_movement();
    std::cout << "Validity reconstruction checks passed: " << cases << " footprint cases, "
        << movement_cases << " integrated movement cases.\n";
}
