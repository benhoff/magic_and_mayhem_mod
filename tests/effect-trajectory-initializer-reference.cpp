#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_trajectory_initializer.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Trajectory initializer/reference mismatch");}
static std::uint32_t randomWord(){static std::uint32_t n=0x4df3a0;n^=n<<13;n^=n>>17;n^=n<<5;return n;}
struct Guarded {
    std::array<std::uint32_t,4> before;
    EffectTrajectory state;
    std::array<std::uint32_t,4> after;
};
struct Counter {std::uint32_t before,value,after;};
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    require(argc==3);map_image(read(argv[1]));const int outArg=2;
    const unsigned char entry[]={0x83,0xec,0x08,0x8b,0x44,0x24,0x28};
    const unsigned char xyEntry[]={0x8b,0x54,0x24,0x0c,0x33,0xc0};
    require(!std::memcmp(reinterpret_cast<void*>(0x4df3a0),entry,sizeof(entry)));
    require(!std::memcmp(reinterpret_cast<void*>(0x4df260),xyEntry,sizeof(xyEntry)));
    using Init=void(__attribute__((thiscall)) *)(void*,std::uint32_t,std::uint32_t,std::uint32_t,
        std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t*,std::uint32_t);
    using Step=void(__attribute__((thiscall)) *)(void*,std::uint32_t*,std::uint32_t*,std::uint32_t*,std::uint32_t*);
#else
    require(argc==2);const int outArg=1;
#endif
    std::ofstream out(argv[outArg],std::ios::binary);
    std::uint64_t fixtures=0,steps=0,xyEnabled=0,zPrimary=0;
    const std::array<std::uint32_t,12> edge{0,1,2,15,16,4095,4096,0x7fffffffu,0x80000000u,0x80000001u,0xfffffffeu,0xffffffffu};
    auto check=[&](std::array<std::uint32_t,3> from,std::array<std::uint32_t,3> to,
                   std::array<std::uint32_t,2> periods,std::uint32_t multiplier){
        Guarded value{};for(auto& n:value.before)n=randomWord();for(auto& n:value.after)n=randomWord();
        for(auto& n:value.state.words)n=randomWord();const auto preserved=value.state.words[3];
        Counter count{randomWord(),randomWord(),randomWord()};
#ifdef ORIGINAL_REFERENCE
        auto original=value;auto originalCount=count;
        *reinterpret_cast<std::uint32_t*>(0x6c5494)=periods[0];
        *reinterpret_cast<std::uint32_t*>(0x6c5498)=periods[1];
        reinterpret_cast<Init>(0x4df3a0)(&original.state,from[0],from[1],from[2],to[0],to[1],to[2],&originalCount.value,multiplier);
#endif
        initializeEffectTrajectory(value.state,from,to,periods,multiplier,count.value);
        require(value.state.words[3]==preserved && count.value==0);
#ifdef ORIGINAL_REFERENCE
        require(!std::memcmp(&value,&original,sizeof(value)) && !std::memcmp(&count,&originalCount,sizeof(count)));
#endif
        auto emit=[&](){out.write(reinterpret_cast<const char*>(&value),sizeof(value));
            out.write(reinterpret_cast<const char*>(&count),sizeof(count));
            out.write(reinterpret_cast<const char*>(from.data()),sizeof(from));};
        emit();++fixtures;xyEnabled+=value.state.words[2]!=0;
#ifdef ORIGINAL_REFERENCE
        auto originalUnits=from;
#endif
        for(unsigned i=0;i<32;++i){
            value.state.step(from,count.value);++steps;zPrimary+=value.state.words[3]!=0;
#ifdef ORIGINAL_REFERENCE
            reinterpret_cast<Step>(0x4df500)(&original.state,&originalUnits[0],&originalUnits[1],&originalUnits[2],&originalCount.value);
            require(!std::memcmp(&value,&original,sizeof(value)) && !std::memcmp(&count,&originalCount,sizeof(count)) && from==originalUnits);
#endif
            emit();
        }
    };
    // Coordinate differences, equality, ties, signed/absolute/doubling overflow,
    // asymmetric/zero/wrapped periods; branch inputs retain their raw words.
    for(auto a:edge)for(auto b:edge)for(unsigned axis=0;axis<3;++axis)
        for(auto multiplier:std::array<std::uint32_t,4>{0,1,32,0xffffffffu}){
            std::array<std::uint32_t,3> from{16,16,16},to{16,16,16};from[axis]=a;to[axis]=b;
            check(from,to,{128,64},multiplier);
        }
    for(unsigned i=0;i<16384;++i){
        std::array<std::uint32_t,3> from{},to{};for(auto& n:from)n=randomWord();for(auto& n:to)n=randomWord();
        check(from,to,{randomWord(),randomWord()},randomWord());
    }
    require(bool(out));std::cout<<"{\"fixtures\":"<<fixtures<<",\"steps\":"<<steps
        <<",\"xy_enabled\":"<<xyEnabled<<",\"z_primary\":"<<zPrimary<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
