#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "creature_footprint.hpp"
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
    const unsigned char entry[]={0x8b,0x44,0x24,0x04,0x33,0xd2};
    require(!std::memcmp(reinterpret_cast<void*>(0x4e1260),entry,sizeof(entry)));
    using Init=void(__attribute__((thiscall)) *)(void*,std::uint32_t,std::uint32_t);
    using Query=unsigned(__attribute__((thiscall)) *)(void*,std::int32_t,std::int32_t,std::int32_t);
#else
    require(argc==2);const int outArg=1;
#endif
    std::ofstream out(argv[outArg],std::ios::binary);
    std::uint64_t fixtures=0,calls=0,hits=0,guardBytes=0;
    const std::array<std::uint32_t,13> heights{0,1,2,3,8,32,127,0x08000000u,0x0fffffffu,0x10000000u,0x10000001u,0xffffffffu,0x80000001u};
    const std::array<std::uint32_t,7> selectors{0,1,2,255,0x7fffffffu,0x80000000u,0xffffffffu};
    for(auto height:heights)for(auto selector:selectors)for(unsigned fill:{0u,0xa5u,0xffu}){
        ++fixtures;const auto value=initializeCreatureFootprint(height,selector);
        std::array<unsigned char,68> expected{};expected.fill(static_cast<unsigned char>(fill));
        std::memcpy(expected.data()+16,value.rows.data(),32);std::memcpy(expected.data()+48,&height,4);
#ifdef ORIGINAL_REFERENCE
        std::array<unsigned char,68> original{};original.fill(static_cast<unsigned char>(fill));
        reinterpret_cast<Init>(0x4e1260)(original.data()+16,height,selector);
        require(original==expected);
#endif
        out.write(reinterpret_cast<const char*>(expected.data()),expected.size());guardBytes+=68;
        const auto limit=height<<4;
        const std::array<std::int32_t,6> zs{0,15,signedWord(limit-1),signedWord(limit),-1,0x7fffffff};
        for(auto z:zs)for(int y=-1;y<=64;++y)for(int x=-1;x<=64;++x){
            auto result=value.occupied(x,y,z);
#ifdef ORIGINAL_REFERENCE
            require(reinterpret_cast<Query>(0x4e1200)(original.data()+16,x,y,z)==unsigned(result));require(original==expected);
#endif
            out.put(result);++calls;hits+=result;
        }
    }
    require(bool(out));std::cout<<"{\"fixtures\":"<<fixtures<<",\"calls\":"<<calls<<",\"hits\":"<<hits<<",\"guard_bytes\":"<<guardBytes<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
