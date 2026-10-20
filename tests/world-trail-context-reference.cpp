#include <cstdlib>
#define main unused_sprite_reference_main
#define u32 sprite_read_u32
#include "sprite-binary-reference.cpp"
#undef main
#undef u32
#include "../reconstruction/rendering/world_trail.hpp"
using u32=std::uint32_t;using u8=std::uint8_t;
#include "../runtime/scene/world_raster_callers.h"
using namespace mnm::reconstruction;
static unsigned phase,calls,seen[5],parents[2];
static u32 stack_begin,stack_length=48;
// Test memory-query facade: actual mapped code and this one bounded frame.
static int readable(u32 address,u32 length){
    if(!address||address+length<address)return 0;
    return (address>=0x400000&&address+length<=0x700000)||
        (address>=stack_begin&&address-stack_begin<=stack_length&&length<=stack_length-(address-stack_begin));
}
#include "trail-caller-wrapper.inc"

static std::vector<WorldTrailStamp> observed;
static u32 __attribute__((fastcall,noinline)) raster_probe(void* source,int x,int y,int shade,int ordinal){
    const auto* words=static_cast<const volatile u32*>(__builtin_frame_address(0));
    const u32 caller=words[1],outer=words[12];
    const auto* stack=const_cast<const u32*>(words+1);
    stack_begin=u32(reinterpret_cast<std::uintptr_t>(stack));
    const bool admitted=raster_caller(caller,57,stack);
    if(mnm_world_trail_parent(caller,57,reinterpret_cast<const u8*>(0x4df840),stack,48)!=outer)std::exit(5);
    if(admitted!=(phase!=0)){std::cerr<<"Wrong helper ancestry "<<std::hex<<caller<<' '<<outer<<' '<<phase<<'\n';std::exit(3);}
    if(phase){bool found=false;for(unsigned i=0;i<5;++i)if(caller==trail_raster_sites[i].address){++seen[i];found=true;}if(!found)std::exit(4);++parents[phase-1];}
    observed.push_back({u32(reinterpret_cast<std::uintptr_t>(source)),x,y,shade,ordinal});++calls;return 0x87650000u+u32(observed.size());
}
int main(int argc,char** argv)try{
    if(argc!=2)throw std::invalid_argument("Expected pinned original PE");
    map_image(read(argv[1]));
    auto* entry=reinterpret_cast<u8*>(0x580a90);
    const std::array<u8,5> signature{0x83,0xec,0x30,0x53,0x55};
    if(!std::equal(signature.begin(),signature.end(),entry))throw std::runtime_error("Original leaf prologue changed");
    entry[0]=0xe9;const auto relative=u32(reinterpret_cast<std::uintptr_t>(&raster_probe)-0x580a95);std::memcpy(entry+1,&relative,4);
    const auto* helper=reinterpret_cast<const u8*>(0x4df840);const Bytes helper_before(helper,helper+0x160);
    const auto* queue_entry=reinterpret_cast<const u8*>(0x5002a0);const Bytes queue_before(queue_entry,queue_entry+0xee1);
    unsigned cases=0;std::vector<WorldTrailStamp> expected;
    for(int x1=-3;x1<=3;++x1)for(int y1=-3;y1<=3;++y1)for(int x2=-8;x2<=8;++x2)for(int y2=-8;y2<=8;++y2)for(int shade:{-127,0,3})for(phase=0;phase<3;++phase){
        observed.clear();u32 result;
        if(!phase){using Method=u32 (__attribute__((fastcall)) *)(int,int,int,int,void*,int,int);result=reinterpret_cast<Method>(0x4df840)(x1,y1,x2,y2,reinterpret_cast<void*>(42),shade,-32768);}
        else{u32 points[5]={u32(x1),u32(y1),u32(x2),u32(y2),0};u32 row[9]={0,u32(reinterpret_cast<std::uintptr_t>(points)),42,0x12345678,std::uint16_t(shade)|(u32(0x8000)<<16),0,9,0x8ad08ad0,0};u32 queue[6]={u32(reinterpret_cast<std::uintptr_t>(row)),0,1,1,0,0};
            std::array<u32,9> row_before;std::copy(row,row+9,row_before.begin());const std::array<u32,6> queue_before{queue[0],0,1,1,0,0};global(0x5e1404,phase==1);global(0x6e1f24,0);using Consumer=void (__attribute__((thiscall)) *)(void*);reinterpret_cast<Consumer>(0x5002a0)(queue);result=row[7]>>16;
            row_before[7]=0x8ad0;
            if(!std::equal(row_before.begin(),row_before.end(),row)||!std::equal(queue_before.begin(),queue_before.end(),queue))throw std::runtime_error("Queue metadata/container changed");}
        expected.clear();const auto native=drawWorldTrail({x1,y1,x2,y2,42,phase==2?0:shade,-32768},32,[&](WorldTrailStamp s){expected.push_back(s);return 0x87650000u+u32(expected.size());});
        if(result!=(phase?0:native)||observed.size()!=expected.size())throw std::runtime_error("Trail call/return count differs");
        for(unsigned i=0;i<expected.size();++i){const auto&a=observed[i];const auto&b=expected[i];if(a.source!=b.source||a.x!=b.x||a.y!=b.y||a.shade!=b.shade||a.ordinal!=b.ordinal)throw std::runtime_error("Trail arguments differ");}++cases;
    }
    if(Bytes(helper,helper+0x160)!=helper_before||Bytes(queue_entry,queue_entry+0xee1)!=queue_before)throw std::runtime_error("Original helper/queue changed");
    for(auto count:seen)if(!count)throw std::runtime_error("Unreached helper leaf");
    for(auto count:parents)if(!count)throw std::runtime_error("Unreached outer path");
    std::cout<<"{\"cases\":"<<cases<<",\"calls\":"<<calls<<",\"sites\":5,\"parents\":2,\"native_placement_matches\":true,\"unrelated_direct_helper_refused\":true}\n";
    return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
