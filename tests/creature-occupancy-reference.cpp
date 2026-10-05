#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "creature_occupancy.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Creature occupancy mismatch");}
static std::int32_t signedWord(std::uint32_t n){return n<0x80000000u?std::int32_t(n):std::int32_t(std::int64_t(n)-0x100000000ll);}
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    require(argc==3);map_image(read(argv[1]));const int outArg=2;
    const unsigned char entry[]={0x56,0x8b,0xf1,0x8b,0x4c,0x24,0x08};
    require(!std::memcmp(reinterpret_cast<void*>(0x4e1200),entry,sizeof(entry)));
    using Query=unsigned(__attribute__((thiscall)) *)(void*,std::int32_t,std::int32_t,std::int32_t);
#else
    require(argc==2);const int outArg=1;
#endif
    std::ofstream out(argv[outArg],std::ios::binary);std::uint64_t calls=0,hits=0,guardBytes=0;
    const std::array<std::uint32_t,13> heights{0,1,2,3,8,32,127,0x08000000u,0x0fffffffu,0x10000000u,0x10000001u,0xffffffffu,0x80000001u};
    for(unsigned fixture=0;fixture<52;++fixture){
        CreatureOccupancy value;value.height=heights[fixture%heights.size()];
        std::uint32_t seed=0x9e3779b9u+fixture*12345;
        for(unsigned y=0;y<16;++y){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;
            value.rows[y]=fixture/13==0?0:fixture/13==1?0xffff:fixture/13==2?std::uint16_t(0x8000u>>y):std::uint16_t(seed);}
        const auto rows=value.rows;const auto height=value.height;
        const auto limit=value.height<<4;
        const std::array<std::int32_t,9> zs{0,1,15,16,signedWord(limit-1),signedWord(limit),0x7fffffff,-1,signedWord(0x80000000u)};
#ifdef ORIGINAL_REFERENCE
        std::array<unsigned char,68> original{};original.fill(0xa5);auto* footprint=original.data()+16;
        std::memcpy(footprint,rows.data(),32);std::memcpy(footprint+32,&height,4);const auto before=original;
#endif
        auto check=[&](std::int32_t x,std::int32_t y,std::int32_t z){
            const auto result=value.occupied(x,y,z);
#ifdef ORIGINAL_REFERENCE
            require(reinterpret_cast<Query>(0x4e1200)(footprint,x,y,z)==unsigned(result));require(original==before);
#endif
            require(value.rows==rows && value.height==height);out.put(result);++calls;hits+=result;guardBytes+=68;
        };
        for(auto z:zs)for(int y=0;y<64;++y)for(int x=0;x<64;++x)check(x,y,z);
        const std::array<std::int32_t,12> edges{signedWord(0x80000000u),-65,-1,0,1,3,4,60,63,64,65,0x7fffffff};
        for(auto z:zs)for(auto y:edges)for(auto x:edges)check(x,y,z);
    }
    require(bool(out));std::cout<<"{\"fixtures\":52,\"calls\":"<<calls<<",\"hits\":"<<hits<<",\"guard_bytes\":"<<guardBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
