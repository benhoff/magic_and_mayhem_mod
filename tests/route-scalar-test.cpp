#include "route_scalar.hpp"
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#ifdef MNM_NATIVE_REFERENCE
#include "movement-native-reference.hpp"
#endif
using namespace mnm::reconstruction;
int main(int argc,char** argv) {
#ifdef MNM_NATIVE_REFERENCE
    assert(argc==2); native_reference::initialize(argv[1],true,true,true,true);
    using Fn=void (__attribute__((thiscall)) *)(void*,int,int,int,int,int,std::uint32_t*,std::uint32_t*);
    auto fn=reinterpret_cast<Fn>(0x5205b0);
#else
    (void)argc; (void)argv;
#endif
    std::size_t count=0;
    const auto check=[&](const CreatureScalarState& s,int argument,int category,Coordinates d,std::uint32_t prior) {
        auto result=creature_movement_scalar(argument,category,d,prior,s);
#ifdef MNM_NATIVE_REFERENCE
        std::array<unsigned char,0x780> object{};
        std::array<unsigned char,0x198> record{};
        auto write=[](auto& bytes,std::size_t at,const auto& value) { std::memcpy(bytes.data()+at,&value,sizeof(value)); };
        auto pointer=reinterpret_cast<std::uintptr_t>(record.data());
        write(object,0xac,pointer); write(object,0x778,s.object_778); write(object,0x77c,s.object_77c);
        write(record,0x3c,s.type_3c); write(record,0x10,s.type_10);
        std::memcpy(record.data()+0xd8,s.type_d8.data(),sizeof(s.type_d8));
        std::memcpy(reinterpret_cast<void*>(0x6c007d),&s.default_scalar,4);
        std::memcpy(reinterpret_cast<void*>(0x5e15f0),&s.slope_global,4);
        const auto original_object=object; const auto original_record=record;
        auto native=prior; std::uint32_t base=0;
        fn(object.data(),argument,category,d.x,d.y,d.z,&native,&base);
        if(native!=result.scalar || base!=result.base) {
            std::cerr<<"Mismatch case "<<count<<" cat "<<category<<" delta "<<d.x<<' '<<d.y<<' '<<d.z
                <<" prior "<<prior<<": "<<native<<'/'<<base<<" vs "<<result.scalar<<'/'<<result.base<<'\n';
            std::abort();
        }
        assert(object==original_object && record==original_record);
#else
        assert(result.base==base_movement_scalar(category,d,s));
#endif
        ++count;
    };
    CreatureScalarState s; s.default_scalar=720;
    for(unsigned i=0;i<s.type_d8.size();++i) s.type_d8[i]=24+i*3;
    for(int type:{0,1}) for(int acceleration:{-100,0,1,64,500})
    for(int boost:{-1,0,1}) for(int slow:{-1,0,1})
    for(int category:{-1,0,1,2,3,4,5})
    for(int dx=-1;dx<=1;++dx) for(int dy=-1;dy<=1;++dy) for(int dz=-1;dz<=1;++dz)
    for(std::uint32_t prior:{0U,1U,200U,720U,2000U,0x7fffffffU,0xffffffffU}) {
        s.type_3c=type; s.type_10=acceleration; s.object_778=boost; s.object_77c=slow;
        s.slope_global=0.968999981880188F;
        check(s,4,category,{dx,dy,dz},prior);
    }
    std::mt19937 random(0x5205b0);
    for(unsigned i=0;i<30000;++i) {
        for(auto& word:s.type_d8) word=random();
        s.default_scalar=random(); s.type_3c=i%2;
        const auto signed_bits=[](std::uint32_t n) { return n<=0x7fffffffU?static_cast<std::int32_t>(n):-1-static_cast<std::int32_t>(~n); };
        s.type_10=signed_bits(random()); s.object_778=i%3==0; s.object_77c=i%5==0;
        constexpr float slopes[]={0,0.969F,-0.969F,2.5F,1000000};
        s.slope_global=slopes[i%5];
        check(s,int(i%4)+1,int(i%7)-1,{int(random()%3)-1,int(random()%3)-1,int(random()%3)-1},random());
    }
    s.type_3c=1; s.type_10=0; s.object_778=0; s.object_77c=0;
    s.type_d8.fill(60);
    assert(creature_movement_scalar(4,0,{1,0,0},123,s).scalar==123);
    bool threw=false; try { creature_movement_scalar(0,0,{1,0,0},0,s); }
    catch(const std::domain_error&) { threw=true; } assert(threw);
    std::cout<<"Scalar cases: "<<count<<'\n';
}
