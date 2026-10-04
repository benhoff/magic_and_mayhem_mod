// Private, unchanged PE mapping: selected placement helpers only; no Win32 calls.
#include "placement.hpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sys/mman.h>
using Bytes=std::vector<unsigned char>;
static Bytes read(const char* path){std::ifstream f(path,std::ios::binary);if(!f)throw std::runtime_error("Cannot open input");return Bytes(std::istreambuf_iterator<char>(f),{});}
static std::uint32_t u32(const Bytes& b,std::size_t at){if(at>b.size() || b.size()-at<4)throw std::runtime_error("Read extent");std::uint32_t v;std::memcpy(&v,b.data()+at,4);return v;}
static void mapImage(const Bytes& b){
    const auto pe=u32(b,60),opt=pe+24,length=u32(b,opt+56);
    if(u32(b,opt+28)!=0x400000 || length>32*1024*1024)throw std::runtime_error("Unexpected PE image");
    auto* p=static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000),length,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0));
    if(p==MAP_FAILED)throw std::runtime_error("Private PE mapping failed");
    const unsigned count=std::uint16_t(b.at(pe+6))|(std::uint16_t(b.at(pe+7))<<8);
    const auto size=std::uint16_t(b.at(pe+20))|(std::uint16_t(b.at(pe+21))<<8);
    for(unsigned i=0;i<count;++i){const auto at=opt+size+i*40,rva=u32(b,at+12),n=u32(b,at+16),raw=u32(b,at+20);
        if(std::uint64_t(raw)+n>b.size() || std::uint64_t(rva)+n>length)throw std::runtime_error("PE extent");
        std::memcpy(p+rva,b.data()+raw,n);}
    if(p[0x64cb0]!=0x8b || p[0x64ec0]!=0x51 || p[0x65000]!=0x51)throw std::runtime_error("Controller entry bytes differ");
}
static void pointer(Bytes& bytes,std::size_t at,void* p){const auto value=reinterpret_cast<std::uintptr_t>(p);std::memcpy(bytes.data()+at,&value,4);}
static void require(bool ok){if(!ok)throw std::runtime_error("Original/native placement mismatch");}
int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected PE ANI");
    mapImage(read(argv[1]));auto b=read(argv[2]);
    if(u32(b,0)!=0x00494e41 || u32(b,12)!=5 || u32(b,4)!=b.size())throw std::runtime_error("ANI version/header");
    const auto count=u32(b,8),offsets=u32(b,20);const auto base=44+std::uint64_t(offsets)*4;
    if(count>65536 || offsets<2 || offsets>4097 || base+std::uint64_t(count)*44!=b.size())throw std::runtime_error("ANI extent");
    Bytes creature(0x900),type(12),asset(0x114);
    std::uint32_t header[4]={0,0,0,65535},marker[10]={};
    std::vector<std::uint32_t> table(65536,reinterpret_cast<std::uintptr_t>(marker));
    std::uint32_t spr[4]={reinterpret_cast<std::uintptr_t>(header),0,reinterpret_cast<std::uintptr_t>(table.data()),0};
    pointer(creature,0xac,type.data());pointer(creature,0xcc,asset.data());pointer(creature,0x841,asset.data());pointer(asset,0x110,spr);
    for(auto va:{0x507190u,0x507250u,0x5072c0u,0x507330u}){
        const auto byte=*reinterpret_cast<unsigned char*>(va);require(byte==(va==0x507190u?0x56:0x8b));}
    std::uint64_t checked=0,sprites=0;
    static_assert(sizeof(mnm::assets::AnimationRecord)==44);
    for(unsigned i=0;i<count;++i){mnm::assets::AnimationRecord r;std::memcpy(&r,b.data()+base+i*44,44);if(r.opcode!=0)continue;
        if(r.argument<0 || r.argument>65535)throw std::runtime_error("Unsafe SPR reference");
        ++sprites;
        pointer(creature,0xb4,&r);pointer(creature,0x829,&r);
        for(auto kind:{0u,1u,2u,3u})for(auto view:{0u,1u,2u,3u,4u,0xffffffffu}){
            std::memcpy(type.data()+8,&kind,4);*reinterpret_cast<std::uint32_t*>(0x6e9850)=view;
            using Body=void(__attribute__((thiscall)) *)(void*,std::uint32_t*);
            for(auto va:{0x507190u,0x507330u}){
                std::uint32_t result[3]={};reinterpret_cast<Body>(va)(creature.data(),result);
                const auto native=mnm::reconstruction::spriteOffset(r,kind,view);
                require(result[0]==reinterpret_cast<std::uintptr_t>(marker) && result[1]==std::uint32_t(native.x) && result[2]==std::uint32_t(native.y));++checked;
            }
            using Attach=void(__attribute__((thiscall)) *)(void*,std::int32_t*,std::int32_t*);
            for(unsigned slot=0;slot<2;++slot){std::int32_t x=0,y=0;
                reinterpret_cast<Attach>(slot?0x5072c0:0x507250)(creature.data(),&x,&y);
                const auto native=mnm::reconstruction::attachmentOffset(r,slot?mnm::reconstruction::AttachmentPoint::second:mnm::reconstruction::AttachmentPoint::first,kind,view);
                require(x==native.x && y==native.y);++checked;
            }
        }
    }
    std::cout<<"{\"sprite_records\":"<<sprites<<",\"helper_matches\":"<<checked<<"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
