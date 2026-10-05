#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_trajectory.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static std::uint32_t randomWord(){static std::uint32_t seed=0x4df500;seed=seed*1664525u+1013904223u;return seed;}
static void require(bool ok){if(!ok)throw std::runtime_error("Original trajectory mismatch");}
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    if(argc!=3)throw std::runtime_error("Expected executable and output");
    map_image(read(argv[1]));
    const unsigned char entry[]={0x8b,0x41,0x30,0x8b,0x51,0x2c,0x2b,0xd0};
    require(!std::memcmp(reinterpret_cast<void*>(0x4df500),entry,sizeof(entry)));
    using Step=void(__attribute__((thiscall)) *)(void*,std::uint32_t*,std::uint32_t*,std::uint32_t*,std::uint32_t*);
    struct GuardedState {std::array<std::uint32_t,4> before;EffectTrajectory trajectory;std::array<std::uint32_t,4> after;};
    struct GuardedOutput {std::uint32_t before;std::array<std::uint32_t,3> units;std::uint32_t changes,after;};
    const int outArg=2;
#else
    if(argc!=2)throw std::runtime_error("Expected output");const int outArg=1;
#endif
    std::ofstream output(argv[outArg],std::ios::binary);
    unsigned steps=0,primary=0,alternate=0,skipped=0,xyPrimary=0,xyAlternate=0;
    const std::array<std::uint32_t,8> edges{0,1,2,0x7fffffffu,0x80000000u,0x80000001u,0xfffffffeu,0xffffffffu};
    for(unsigned fixture=0;fixture<8192;++fixture){
        EffectTrajectory t;for(auto& n:t.words)n=randomWord();
        std::array<std::uint32_t,3> units{randomWord(),randomWord(),randomWord()};std::uint32_t changes=randomWord();
        // Exhaust signed/error boundaries and zero deltas, then retain raw random state.
        if(fixture<4096){
            t.words[11]=edges[fixture%8];t.words[12]=edges[(fixture/8)%8];
            t.words[10]=edges[(fixture/64)%8];t.words[9]=edges[(fixture/512)%8];
        }
        t.words[2]=fixture%2;
        if(fixture%5==0)t.words[0]=0;
        if(fixture%7==0)t.words[1]=0;
        if(fixture%11==0)t.words[4]=t.words[5]=t.words[6]=t.words[7]=0;
#ifdef ORIGINAL_REFERENCE
        GuardedState original{{0xa5a5a5a5u,0xa5a5a5a5u,0xa5a5a5a5u,0xa5a5a5a5u},t,{0xa5a5a5a5u,0xa5a5a5a5u,0xa5a5a5a5u,0xa5a5a5a5u}};
        GuardedOutput actual{0xababababu,units,changes,0xcdcdcdcdu};
#endif
        for(unsigned tick=0;tick<32;++tick){
            const auto xyError=t.words[10]-t.words[9];
            t.step(units,changes);
            if(t.words[3])++primary;else ++alternate;
            if(t.words[3] && t.words[2]==0)++skipped;
            else if(xyError!=0 && xyError<0x80000000u)++xyPrimary;
            else ++xyAlternate;
#ifdef ORIGINAL_REFERENCE
            reinterpret_cast<Step>(0x4df500)(&original.trajectory,actual.units.data(),actual.units.data()+1,actual.units.data()+2,&actual.changes);
            require(original.trajectory.words==t.words && actual.units==units && actual.changes==changes);
            for(auto n:original.before)require(n==0xa5a5a5a5u);
            for(auto n:original.after)require(n==0xa5a5a5a5u);
            require(actual.before==0xababababu && actual.after==0xcdcdcdcdu);
#endif
            output.write(reinterpret_cast<const char*>(t.words.data()),56);
            output.write(reinterpret_cast<const char*>(units.data()),12);
            output.write(reinterpret_cast<const char*>(&changes),4);++steps;
        }
    }
    if(!output)throw std::runtime_error("Output failed");
    std::cout<<"{\"fixtures\":8192,\"steps\":"<<steps<<",\"primary_z\":"<<primary<<",\"alternate_z\":"<<alternate<<",\"skipped_xy\":"<<skipped<<",\"primary_xy\":"<<xyPrimary<<",\"alternate_xy\":"<<xyAlternate<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
