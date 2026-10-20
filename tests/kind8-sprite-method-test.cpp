#ifdef MNM_ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#define u32 sprite_original_word
#include "sprite-binary-reference.cpp"
#undef main
#undef u32
#else
#include <cstring>
#include <iostream>
#include <stdexcept>
#endif
#include "../reconstruction/rendering/kind8_sprite_method.hpp"
#include <cstdio>
using namespace mnm::reconstruction;
using u32=std::uint32_t;using u8=unsigned char;
#if defined(MNM_ORIGINAL_REFERENCE) && defined(MNM_SPRITE_PRIMARY_FRAGMENT)
#include "../runtime/scene/world_raster_callers.h"
#endif
namespace {
std::uint32_t draws[3][8],calls,randoms,randomValues[2],backendReturn;
unsigned char* frameBase;
#if defined(MNM_ORIGINAL_REFERENCE) && defined(MNM_SPRITE_PRIMARY_FRAGMENT)
struct ContextSpan {u32 start,bytes;};
ContextSpan context_ranges[6];u32 context_sp,context_stack_bytes,image_bytes,context_tag_variant,context_hits;
static u32 get(const void* p){u32 v;std::memcpy(&v,p,4);return v;}
static int readable(u32 p,u32 n){
    if(!p||!n||p+n<p)return 0;
    if(p>=0x400000 && n<=image_bytes && p-0x400000<=image_bytes-n)return 1;
    if(p>=context_sp && n<=context_stack_bytes && p-context_sp<=context_stack_bytes-n)return 1;
    for(const auto& span:context_ranges)if(p>=span.start && n<=span.bytes && p-span.start<=span.bytes-n)return 1;
    return 0;
}
#include "../runtime/scene/kind8_sprite_admission.h"
void checkPrimaryCaller(u32 tag,const u32* stack){
    context_sp=u32(reinterpret_cast<std::uintptr_t>(stack));context_stack_bytes=(tag==22||tag==36)?32:40;
    if(!kind8_sprite_caller(stack[0],tag,stack))throw std::runtime_error("Actual primary sprite caller refused");
    ++context_hits;
}
__attribute__((naked,noinline)) unsigned invokePrimary(const u32*){
    __asm__ volatile("push %ebp\nmov %esp,%ebp\npush %ebx\npush %esi\npush %edi\nmov 8(%ebp),%esi\npush $1f\nmov $0x5006a2,%eax\njmp *%eax\n1:\npop %edi\npop %esi\npop %ebx\nleave\nret");
}
#endif

std::uint32_t nextRandom(){if(randoms>=2)throw std::runtime_error("Excess random draws");return randomValues[randoms++];}
unsigned capture(unsigned target,unsigned token,int x,int y,unsigned a,unsigned b,unsigned n){
    if(calls>=3)throw std::runtime_error("Excess raster calls");
    const std::uint32_t row[8]={target,token,std::uint32_t(x),std::uint32_t(y),a,b,n,0};
    std::memcpy(draws[calls++],row,sizeof(row));return backendReturn;
}
#ifdef MNM_ORIGINAL_REFERENCE
unsigned __attribute__((thiscall)) originalRandom(void* object){
    if(object!=reinterpret_cast<void*>(0x6574c8))throw std::runtime_error("Wrong RNG receiver");
    return nextRandom();
}
unsigned token(void* frame){const auto offset=static_cast<unsigned char*>(frame)-frameBase;if(offset<0||offset%32||offset>=256)throw std::runtime_error("Wrong frame pointer");return 100+unsigned(offset/32);}
unsigned word(void* frame,int x,int y){
#ifdef MNM_SPRITE_PRIMARY_FRAGMENT
    checkPrimaryCaller(context_tag_variant?36:22,static_cast<const u32*>(__builtin_frame_address(0))+1);
#endif
    return capture(0,token(frame),x,y,0,0,3);
}
unsigned generic(void* frame,int x,int y,unsigned a,unsigned b){
#ifdef MNM_SPRITE_PRIMARY_FRAGMENT
    checkPrimaryCaller(context_tag_variant?14:13,static_cast<const u32*>(__builtin_frame_address(0))+1);
#endif
    return capture(1,token(frame),x,y,a,b,5);
}
unsigned clipped(void* frame,int x,int y,unsigned a,unsigned b){
#ifdef MNM_SPRITE_PRIMARY_FRAGMENT
    checkPrimaryCaller(17,static_cast<const u32*>(__builtin_frame_address(0))+1);
#endif
    return capture(2,token(frame),x,y,a,b,5);
}
void bind(unsigned address,std::uintptr_t target){auto* p=reinterpret_cast<unsigned char*>(address);p[0]=0xe9;const auto delta=std::uint32_t(target-address-5);std::memcpy(p+1,&delta,4);}
#endif
void output(unsigned result,const std::uint32_t* object){
    const std::uint32_t head[3]={result,calls,randoms};
    if(std::fwrite(head,sizeof(head),1,stdout)!=1 || std::fwrite(draws,sizeof(draws),1,stdout)!=1 || std::fwrite(object,32,1,stdout)!=1)throw std::runtime_error("Fixture output failed");
}
}
int main(int argc,char** argv)try{
#ifdef MNM_ORIGINAL_REFERENCE
    if(argc!=2)throw std::invalid_argument("Expected pinned PE32 image");
    const auto image=read(argv[1]);map_image(image);
#ifdef MNM_SPRITE_PRIMARY_FRAGMENT
    image_bytes=sprite_original_word(image,sprite_original_word(image,60)+24+56);
    if(get(reinterpret_cast<void*>(0x500848))!=0x1e468966 || *reinterpret_cast<const u8*>(0x50084c)!=0x8b)throw std::runtime_error("Primary metadata/exit bytes changed");
    *reinterpret_cast<u8*>(0x50084c)=0xc3; // Private exit after unchanged AX store.
    global(0x5e1404,1);global(0x6e1f88,0);
#endif
    bind(0x54e200,reinterpret_cast<std::uintptr_t>(&originalRandom));bind(0x57de00,reinterpret_cast<std::uintptr_t>(&clipped));
    global(0x6903dc,std::uint32_t(reinterpret_cast<std::uintptr_t>(&word)));global(0x6c4e3c,std::uint32_t(reinterpret_cast<std::uintptr_t>(&generic)));
#else
    (void)argv;if(argc!=1)throw std::invalid_argument("Expected owned native inputs only");
#endif
    const unsigned tables[]={0x5c74e4,0x5c74fc},methods[]={0x5492b0,0x549730};
    const std::uint32_t randomPairs[][2]={{0,0},{1,1},{49,7},{50,8},{51,9},{99,0xffffffffu},{100,0x80000000u},{0xffffffffu,0x80000033u},{0x80000032u,50},{0x80000033u,51}};
#ifdef MNM_SPRITE_PRIMARY_FRAGMENT
    constexpr unsigned variants=2;
#else
    constexpr unsigned variants=1;
#endif
    for(unsigned family=0;family<2;++family)for(bool present:{false,true})for(int player=-1;player<=8;++player)
    for(unsigned enabled:{0u,0x80000000u})for(unsigned count:{0u,1u,2u,5u,8u})for(const auto& pair:randomPairs)
    for(unsigned wordMask:{0u,0x55u,0xffu})for(unsigned returned:{0u,1u,2u,0x1234u,0x10001u,0xffffffffu})for(unsigned variant=0;variant<variants;++variant){
        (void)variant;
        std::uint32_t object[8],owner[17]{},bank[8]{},offsets[8]{},frames[8][8]{};
        for(unsigned i=0;i<8;++i){object[i]=0x12340000+i;owner[9+i]=enabled;offsets[i]=i*32;frames[i][7]=(wordMask&(1u<<i))?0xffffffffu:0x80000000u;}
        object[0]=tables[family];bank[3]=count;frameBase=reinterpret_cast<unsigned char*>(frames);calls=randoms=0;std::memset(draws,0,sizeof(draws));randomValues[0]=pair[0];randomValues[1]=pair[1];backendReturn=returned;
        const int x=-123,y=456;unsigned result;
#ifdef MNM_ORIGINAL_REFERENCE
        if(*reinterpret_cast<const unsigned*>(tables[family]+12)!=methods[family])throw std::runtime_error("Original slot changed");
        object[family?4:7]=std::uint32_t(reinterpret_cast<std::uintptr_t>(owner));global(0x644520,std::uint32_t(player));
        const unsigned base=family?0x6f2268:0x6f2150;global(base,present?std::uint32_t(reinterpret_cast<std::uintptr_t>(bank)):0);global(base+8,std::uint32_t(reinterpret_cast<std::uintptr_t>(offsets)));global(base+12,std::uint32_t(reinterpret_cast<std::uintptr_t>(frames)));
        auto before=std::vector<std::uint32_t>(object,object+8);
#ifdef MNM_SPRITE_PRIMARY_FRAGMENT
        u32 row[9]={0,u32(reinterpret_cast<std::uintptr_t>(object)),u32(x),u32(y),0,0,8,0x8ad08ad0,0x12345678};
        context_tag_variant=variant;
        const ContextSpan spans[]={{u32(reinterpret_cast<std::uintptr_t>(row)),sizeof(row)},{u32(reinterpret_cast<std::uintptr_t>(object)),sizeof(object)},{u32(reinterpret_cast<std::uintptr_t>(owner)),sizeof(owner)},{u32(reinterpret_cast<std::uintptr_t>(bank)),sizeof(bank)},{u32(reinterpret_cast<std::uintptr_t>(offsets)),sizeof(offsets)},{u32(reinterpret_cast<std::uintptr_t>(frames)),sizeof(frames)}};
        std::copy(std::begin(spans),std::end(spans),context_ranges);
        result=std::uint16_t(invokePrimary(row));
        if(row[7]!=0x8ad0 || row[8]!=0x12345678)throw std::runtime_error("Primary AX metadata differs");
#else
        using Method=unsigned(__attribute__((thiscall)) *)(void*,int,int);result=std::uint16_t(reinterpret_cast<Method>(methods[family])(object,x,y));
#endif
        if(!std::equal(before.begin(),before.end(),object))throw std::runtime_error("Original method changed object");
#else
        (void)owner;(void)bank;(void)offsets;(void)frameBase;Kind8SpriteBank input;input.present=present;input.players.fill(enabled);
        for(unsigned i=0;i<count;++i)input.frames.push_back({100+i,(wordMask&(1u<<i))!=0});
        result=drawKind8Sprite(kind8SpriteClass(tables[family],methods[family]),input,player,x,y,nextRandom,[](const QueueCall& c){
            const unsigned target=c.backend==QueueBackend::wordCopy?0:c.backend==QueueBackend::copy?1:c.backend==QueueBackend::clippedCopy?2:99;
            return capture(target,c.values[0],int(c.values[1]),int(c.values[2]),c.values[3],c.values[4],c.count);
        });
#endif
        object[family?4:7]=777;output(result,object);
    }
#ifndef MNM_ORIGINAL_REFERENCE
    unsigned controls=0;
    auto refuse=[&](auto action){try{action();}catch(const std::invalid_argument&){++controls;return;}throw std::runtime_error("Malformed native sprite input admitted");};
    for(unsigned family=0;family<2;++family){refuse([&]{kind8SpriteClass(tables[family],methods[family]^1);});refuse([&]{kind8SpriteClass(tables[family]^1,methods[family]);});}
    Kind8SpriteBank invalid;invalid.present=true;invalid.players.fill(1);invalid.frames.resize(4097);
    auto forbidden=[]()->unsigned{throw std::runtime_error("Invalid input consumed random word");};auto unused=[](const QueueCall&)->unsigned{throw std::runtime_error("Invalid input reached raster");};
    refuse([&]{drawKind8Sprite(Kind8Sprite::flies,invalid,0,0,0,forbidden,unused);});
    refuse([&]{drawKind8Sprite(Kind8Sprite::cure,invalid,0,0,0,forbidden,unused);});
    refuse([&]{drawKind8Sprite(static_cast<Kind8Sprite>(99),invalid,0,0,0,forbidden,unused);});
    invalid.present=false;if(drawKind8Sprite(Kind8Sprite::flies,invalid,0,0,0,forbidden,unused))throw std::runtime_error("Absent bank did not retain no-op");++controls;
    if(controls!=8)throw std::runtime_error("Incomplete native controls");
    std::fprintf(stderr,"native_controls=8\n");
#endif
#if defined(MNM_ORIGINAL_REFERENCE) && defined(MNM_SPRITE_PRIMARY_FRAGMENT)
    std::fprintf(stderr,"primary_context_hits=%u\n",context_hits);
#endif
    return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
