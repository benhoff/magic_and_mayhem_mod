#include "route_record_query.hpp"
#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>
#ifdef MNM_NATIVE_REFERENCE
#include "record-query-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
namespace {
using Call=std::tuple<unsigned,int,int,int>;
std::size_t cases=0,movement_cases=0;
struct Step { Coordinates pos; std::uint32_t value; };
int wrap(int a,int d) { const auto n=a%d; return n<0?n+d:n; }
void scripted(unsigned pattern,int type,int branch,int boundary,int z,MapDimensions dims) {
    CreatureMovementParameters p; p.type_08=type; p.type_44=branch;
    p.object_104=-7; p.unknown_10=0xab; p.type_index=22; p.special_state=1;
    const auto original=p;
    const Coordinates pos{dims.x-1,dims.y-1,z};
    std::vector<Step> script; unsigned support_mask=0,top_mask=0;
    // Independent oracle: supported corners are 3/4 corners or a diagonal pair.
    for(unsigned corner=0;corner<4;++corner) {
        const int x=corner%2,y=corner/2; const unsigned state=(pattern>>(2*corner))&3;
        const auto in_band=[&](int level) { return branch==2 && level>=boundary && level<boundary+2; };
        const bool top=state&1,below=state&2;
        script.push_back({{wrap(pos.x+x,dims.x),wrap(pos.y+y,dims.y),z},top?0x80000001U:0U});
        if(top) { support_mask|=1U<<corner; top_mask|=1U<<corner; }
        else if(in_band(z)) support_mask|=1U<<corner;
        else if(z>0) {
            script.push_back({{wrap(pos.x+x,dims.x),wrap(pos.y+y,dims.y),z-1},below?7U:0U});
            if(below || in_band(z-1)) support_mask|=1U<<corner;
        }
    }
    unsigned count=0; for(unsigned bit=0;bit<4;++bit) count+=(support_mask>>bit)&1;
    const unsigned expected=(top_mask && (count>=3 || support_mask==6 || support_mask==9))?1:0;
    std::size_t cursor=0; std::vector<Call> calls;
    RecordQueryHelpers h{dims,[&] { return boundary; },[&](Coordinates q,const CreatureMovementParameters& parameters) {
        assert(&parameters==&p && std::memcmp(&parameters,&original,sizeof(p))==0);
        assert(cursor<script.size()); const auto step=script[cursor++];
        assert(q.x==step.pos.x && q.y==step.pos.y && q.z==step.pos.z);
        calls.emplace_back(0x4f3320,q.x,q.y,q.z); return step.value;
    }};
    const auto result=query_record(pos,p,h);
    assert(result==expected && cursor==script.size());
#ifdef MNM_NATIVE_REFERENCE
    const auto model_calls=calls; calls.clear(); cursor=0;
    assert(native_reference::run_record_query(pos,p,h)==result && cursor==script.size() && calls==model_calls);
#endif
    ++cases;
}
void single_and_extremes() {
    CreatureMovementParameters p; p.type_08=1;
    for(std::uint32_t value:{0U,1U,7U,0x80000000U,0xffffffffU}) {
        int count=0;
        RecordQueryHelpers h{{0,0,0},{},[&](Coordinates pos,const CreatureMovementParameters& parameters) {
            assert(pos.x==-3 && pos.y==99 && pos.z==-1 && &parameters==&p); ++count; return value;
        }};
        assert(query_record({-3,99,-1},p,h)==value && count==1);
#ifdef MNM_NATIVE_REFERENCE
        count=0; assert(native_reference::run_record_query({-3,99,-1},p,h)==value && count==1);
#endif
        ++cases;
    }
    p.type_08=0;
    RecordQueryHelpers h{{8,7,8},[]{ return 0; },[](Coordinates,const CreatureMovementParameters&)->std::uint32_t {
        assert(false); return 0;
    }};
    for(int z:{-1,-2,std::numeric_limits<int>::min()}) {
        assert(query_record({0,0,z},p,h)==0);
#ifdef MNM_NATIVE_REFERENCE
        assert(native_reference::run_record_query({0,0,z},p,h)==0);
#endif
        ++cases;
    }
    const int maximum=std::numeric_limits<int>::max();
    h.dimensions={maximum,maximum,8}; h.query=[&](Coordinates q,const CreatureMovementParameters&) {
        assert((q.x==0 || q.x==maximum-1) && (q.y==0 || q.y==maximum-1)); return 1U;
    };
    assert(query_record({maximum,maximum,maximum},p,h)==1);
#ifdef MNM_NATIVE_REFERENCE
    assert(native_reference::run_record_query({maximum,maximum,maximum},p,h)==1);
#endif
    ++cases;
    // Wrapped upper boundary makes this band empty, so zeros stay unsupported.
    p.type_44=2; h.dimensions={8,7,8}; h.boundary=[&]{ return maximum-1; };
    h.query=[](Coordinates,const CreatureMovementParameters&){ return 0U; };
    assert(query_record({0,0,maximum},p,h)==0);
#ifdef MNM_NATIVE_REFERENCE
    assert(native_reference::run_record_query({0,0,maximum},p,h)==0);
#endif
    ++cases;
}
void parameter_rereads() {
    CreatureMovementParameters p; unsigned count=0;
    RecordQueryHelpers h{{1,1,8},[]{ return 0; },[&](Coordinates,const CreatureMovementParameters&) {
        ++count; p.type_08=1; p.type_44=2; return 0U;
    }};
    assert(query_record({0,0,1},p,h)==0 && count==4);
#ifdef MNM_NATIVE_REFERENCE
    count=0; p.type_08=0; p.type_44=0;
    assert(native_reference::run_record_query({0,0,1},p,h)==0 && count==4);
#endif
}
void integrated() {
    std::vector<Call> calls;
    for(unsigned pattern=0;pattern<256;++pattern) for(int type:{0,1,2}) for(int branch:{0,2})
        for(int capability:{0,1}) for(int source:{-1,0,7}) {
            CreatureMovementParameters p; p.type_08=type; p.type_44=branch; p.object_104=capability;
            RecordQueryHelpers q{{4,4,8},[]{ return 3; },[&](Coordinates pos,const CreatureMovementParameters& parameters) {
                assert(&parameters==&p);
                calls.emplace_back(0x4f3320,pos.x,pos.y,pos.z);
                return (pattern >> ((pos.x+4*pos.y+3*pos.z)&7))&1U;
            }};
            MovementHelpers m; m.dimensions=q.dimensions; m.boundary=q.boundary; m.layer_count=[]{ return 8; };
            m.check=[&](MovementCheck kind,Coordinates pos,const CreatureMovementParameters&) {
                calls.emplace_back(unsigned(kind),pos.x,pos.y,pos.z);
                return kind==MovementCheck::check_46b0;
            };
            m=with_record_query(m,q); calls.clear(); int category=9;
            const auto result=test_movement({1,1,2},{2,2,2},p,category,source,m);
            assert(result?(category>=0 && category<=4):category==5);
#ifdef MNM_NATIVE_REFERENCE
            const auto model_calls=calls; calls.clear(); int actual_category=9;
            const auto actual=native_reference::run_support_movement({1,1,2},{2,2,2},p,actual_category,source,m,q);
            if(actual!=result || actual_category!=category || calls!=model_calls) {
                std::cerr << "Mismatch pattern=" << pattern << " type=" << type << " branch=" << branch
                    << " capability=" << capability << " source=" << source << " model=" << result << ":" << category
                    << " native=" << actual << ":" << actual_category << "\n";
                for(const auto& c:model_calls) std::cerr << "model " << std::hex << std::get<0>(c) << std::dec << " "
                    << std::get<1>(c) << "," << std::get<2>(c) << "," << std::get<3>(c) << "\n";
                for(const auto& c:calls) std::cerr << "native " << std::hex << std::get<0>(c) << std::dec << " "
                    << std::get<1>(c) << "," << std::get<2>(c) << "," << std::get<3>(c) << "\n";
                assert(false);
            }
#endif
            ++movement_cases;
        }
}
}
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2); native_reference::initialize_record_query(argv[1]);
#else
    (void)argc; (void)argv;
#endif
    for(unsigned pattern=0;pattern<256;++pattern) for(int type:{-1,0,2}) for(int branch:{0,2})
        for(int boundary:{0,1,2,3}) for(int z:{0,1,2})
            for(MapDimensions dims:{MapDimensions{1,1,8},MapDimensions{2,2,8},MapDimensions{8,7,8}})
                scripted(pattern,type,branch,boundary,z,dims);
    single_and_extremes(); parameter_rereads(); integrated();
    std::cout << "Support query checks passed: " << cases << " direct, " << movement_cases << " movement cases.\n";
}
