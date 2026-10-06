#ifdef ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#endif
#include "effect_animation_selection.hpp"
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Animation selection/reference mismatch");}
#ifdef ORIGINAL_REFERENCE
static void put(unsigned char* p,unsigned at,std::uint32_t n){std::memcpy(p+at,&n,4);}
#endif
int main(int argc,char** argv)try{
#ifdef ORIGINAL_REFERENCE
    require(argc==3);map_image(read(argv[1]));const int outArg=2;
    const unsigned char entry[]={0x56,0x57,0x8b,0xf9,0x83,0xc8,0xff};
    require(!std::memcmp(reinterpret_cast<void*>(0x489200),entry,sizeof(entry)));
    const std::array<std::uint32_t,30> targets{0x4892b3u,0x4892edu,0x48932fu,0x489374u,0x4893c5u,0x4893adu,0x4893f5u,0x4893edu,0x4893b5u,0x4893e5u,0x489327u,0x489337u,0x4893fdu,0x4892abu,0x48938fu,0x48940du,0x489286u,0x48936cu,0x4893cdu,0x4893bdu,0x4893ddu,0x4893d5u,0x48939eu,0x48928bu,0x489293u,0x48929bu,0x4892a3u,0x48927eu,0x489405u,0x489412u};
    const std::array<unsigned char,103> mapping{0,0,1,0,1,0,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,29,29,29,29,29,29,2,3,4,29,29,5,29,29,6,7,8,9,29,29,29,29,29,10,29,11,29,12,29,29,29,13,29,29,29,14,29,29,29,29,29,15,29,29,16,29,17,18,29,29,19,29,20,29,29,21,29,29,29,29,29,29,22,29,29,29,23,24,25,26,29,23,27,29,29,28};
    require(!std::memcmp(reinterpret_cast<void*>(0x489418),targets.data(),sizeof(targets)));
    require(!std::memcmp(reinterpret_cast<void*>(0x489490),mapping.data(),sizeof(mapping)));
    using Select=std::uint32_t(__attribute__((thiscall)) *)(void*);
#else
    require(argc==2);const int outArg=1;
#endif
    std::ofstream out(argv[outArg],std::ios::binary);
    const std::array<std::uint32_t,7> classes{0,1,2,3,0x7fffffffu,0x80000000u,0xffffffffu};
    std::vector<std::uint32_t> types,kinds;
    for(unsigned n=0;n<89;++n)types.push_back(n);
    for(auto n:std::array<std::uint32_t,7>{89,102,103,255,0x7fffffffu,0x80000000u,0xffffffffu})types.push_back(n);
    for(unsigned n=0;n<103;++n)kinds.push_back(n);
    for(auto n:std::array<std::uint32_t,5>{103,255,0x7fffffffu,0x80000000u,0xffffffffu})kinds.push_back(n);
    std::uint64_t calls=0,none=0,overrides=0,creatureCalls=0;
    std::array<std::uint64_t,88> results{};
    for(auto category:classes)for(auto descriptorClass:classes){
        const std::vector<std::uint32_t> metadata(103,category),descriptors(3,descriptorClass);
#ifdef ORIGINAL_REFERENCE
        std::array<unsigned char,0x22e + 32> receiver{};receiver.fill(0xa5);auto* record=receiver.data()+16;
        std::vector<unsigned char> creatures(3*0xe4b+32,0x5a);auto* creatureBase=creatures.data()+16;
        std::array<unsigned char,3*12+32> descriptorBytes{};descriptorBytes.fill(0x3c);auto* descriptorBase=descriptorBytes.data()+16;
        for(unsigned n=0;n<3;++n){put(creatureBase+n*0xe4b,0xac,reinterpret_cast<std::uintptr_t>(descriptorBase+n*12));put(descriptorBase+n*12,8,descriptorClass);}
        const auto initialCreatures=creatures;const auto initialDescriptors=descriptorBytes;
        put(reinterpret_cast<unsigned char*>(0x6def58),0,reinterpret_cast<std::uintptr_t>(creatureBase));
        put(reinterpret_cast<unsigned char*>(0x6def5c),0,3);
        for(unsigned n=0;n<103;++n)put(reinterpret_cast<unsigned char*>(0x6b13cd+n*721),0,category);
#endif
        for(auto type:types)for(auto kind:kinds)for(unsigned ordinal=0;ordinal<3;++ordinal){
            // The original special override reads unchecked metadata even for
            // out-of-table kinds. Test its invalid owned-input refusal separately.
            if((type==6 || type==7) && kind>=103)continue;
            const auto result=selectEffectAnimation(type,kind,metadata,ordinal,descriptors);
#ifdef ORIGINAL_REFERENCE
            put(record,0x28,type);put(record,0x4c,kind);put(record,0x48,ordinal);const auto before=receiver;
            const auto original=reinterpret_cast<Select>(0x489200)(record);
            if(original!=result)std::cerr<<"type "<<type<<" kind "<<kind<<" category "<<category<<" descriptor "<<descriptorClass<<" original "<<original<<" native "<<result<<'\n';
            require(original==result && receiver==before);
#endif
            const std::array<std::uint32_t,6> row{type,kind,category,descriptorClass,ordinal,result};
            out.write(reinterpret_cast<const char*>(row.data()),sizeof(row));++calls;
            none+=result==NoEffectAnimation;
            if(result!=NoEffectAnimation){require(result<results.size());++results[result];}
            overrides+=(type==6 || type==7) && kind!=68;
            creatureCalls+=kind==52 && type!=6 && type!=7;
        }
#ifdef ORIGINAL_REFERENCE
        require(creatures==initialCreatures && descriptorBytes==initialDescriptors);
        for(unsigned n=0;n<103;++n){std::uint32_t value;std::memcpy(&value,reinterpret_cast<void*>(0x6b13cd+n*721),4);require(value==category);}
#endif
    }
    unsigned distinct=0;for(auto count:results)distinct+=count!=0;
    require(bool(out));std::cout<<"{\"calls\":"<<calls<<",\"no_animation\":"<<none
        <<",\"override_calls\":"<<overrides<<",\"creature_calls\":"<<creatureCalls
        <<",\"distinct_animations\":"<<distinct<<",\"tables_verified\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
