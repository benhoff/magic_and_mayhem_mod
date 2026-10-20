#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/mman.h>
using u32=std::uint32_t;using u8=std::uint8_t;
#include "../runtime/scene/world_raster_callers.h"
static u32 stack_begin=0x900000,stack_length=48;
static int readable(u32 address,u32 length){
    if(!address||address+length<address)return 0;
    return (address>=0x400000&&address+length<=0x700000)||
        (address>=stack_begin&&address-stack_begin<=stack_length&&length<=stack_length-(address-stack_begin));
}
#include "trail-caller-wrapper.inc"
static unsigned controls,pairs;
static void require(bool okay){if(!okay)throw std::runtime_error("Trail context guard failed");++controls;}
int main()try{
    static_assert(sizeof(void*)==4,"Exercise the injected PE32 guard ABI");
    auto* image=mmap(reinterpret_cast<void*>(0x400000),0x300000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    auto* memory=mmap(reinterpret_cast<void*>(stack_begin),4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    auto* guard=mmap(nullptr,4096,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    require(image!=MAP_FAILED&&memory!=MAP_FAILED&&guard!=MAP_FAILED);
    auto* stack=static_cast<u32*>(memory);const u8 prologue[]={0x83,0xec,0x0c,0x53,0x55,0x56,0x57};
    std::memcpy(reinterpret_cast<void*>(0x4df840),prologue,7);
    for(const auto& row:raster_callers)std::memcpy(reinterpret_cast<void*>(row.address),row.bytes,10);
    for(const auto& row:trail_raster_sites)std::memcpy(reinterpret_cast<void*>(row.address),row.bytes,10);
    for(const auto& row:trail_queue_returns)std::memcpy(reinterpret_cast<void*>(row.address),row.bytes,10);
    for(const auto& leaf:trail_raster_sites)for(const auto& parent:trail_queue_returns){
        auto* leaf_bytes=reinterpret_cast<u8*>(leaf.address);auto* parent_bytes=reinterpret_cast<u8*>(parent.address);
        stack[11]=parent.address;require(raster_caller(leaf.address,57,stack));++pairs;
        require(!mnm_world_raster_caller(leaf.address,57,leaf_bytes));
        for(unsigned byte=0;byte<10;++byte){leaf_bytes[byte]^=1;require(!raster_caller(leaf.address,57,stack));leaf_bytes[byte]^=1;parent_bytes[byte]^=1;require(!raster_caller(leaf.address,57,stack));parent_bytes[byte]^=1;}
        auto* entry=reinterpret_cast<u8*>(0x4df840);
        for(unsigned byte=0;byte<7;++byte){entry[byte]^=1;require(!raster_caller(leaf.address,57,stack));entry[byte]^=1;}
        for(unsigned tag=0;tag<64;++tag)if(tag!=57)require(!raster_caller(leaf.address,tag,stack));
        require(!raster_caller(leaf.address,64,stack));require(!raster_caller(leaf.address,UINT32_MAX,stack));
        require(!raster_caller(leaf.address,57,nullptr));require(!raster_caller(leaf.address,57,static_cast<const u32*>(guard)));
        stack[11]=parent.address+1;require(!raster_caller(leaf.address,57,stack));stack[11]=0;require(!raster_caller(leaf.address,57,stack));stack[11]=parent.address;
        for(stack_length=0;stack_length<48;++stack_length){
            require(!raster_caller(leaf.address,57,stack));
            require(!mnm_world_trail_parent(leaf.address,57,static_cast<const u8*>(guard),static_cast<const u32*>(guard),stack_length));
        }
        stack_length=48;require(raster_caller(leaf.address,57,stack));
    }
    require(!raster_caller(0,57,stack));require(!raster_caller(0x4df996,57,stack));
    require(!mnm_world_trail_parent(0,57,static_cast<const u8*>(guard),static_cast<const u32*>(guard),48));
    require(!mnm_world_trail_parent(0x4df995,56,static_cast<const u8*>(guard),static_cast<const u32*>(guard),48));
    require(mprotect(memory,4096,PROT_NONE)==0);stack_length=47;
    for(const auto& leaf:trail_raster_sites)require(!raster_caller(leaf.address,57,stack));
    // Ordinary callers retain their original context-free contract.
    for(const auto& row:raster_callers)for(unsigned tag=0;tag<64;++tag)if(row.tags&(1ull<<tag))require(raster_caller(row.address,tag,stack));
    require(munmap(guard,4096)==0);require(munmap(memory,4096)==0);require(munmap(image,0x300000)==0);
    std::cout<<"{\"success\":true,\"context_pairs\":"<<pairs<<",\"controls\":"<<controls<<",\"protected_short_stack_refused\":true,\"ordinary_callers_preserved\":true}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
