// Private, unchanged PE mapping: selected placement helpers only; no Win32 calls.
#include "sprite_visibility.hpp"
#include <array>
#include <algorithm>
#include <random>
#include <limits>
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
    if(p[0x1013c0]!=0x55 || p[0x1015f0]!=0x83)throw std::runtime_error("Controller entry bytes differ");
}


static void require(bool ok){if(!ok)throw std::runtime_error("Original/native visibility mismatch");}
static void put(Bytes& b,std::size_t at,std::uint32_t v){std::memcpy(b.data()+at,&v,4);}
static void global(std::uintptr_t at,std::uint32_t v){std::memcpy(reinterpret_cast<void*>(at),&v,4);}
using namespace mnm::reconstruction;
struct Queue {std::uint32_t base,next,count,capacity,mask,view;};
static Bytes frame(std::mt19937& rng,unsigned rows,unsigned style=0){
    Bytes b(48+rows*8);put(b,0,b.size());put(b,32,40);put(b,36,44+rows*4);
    b[40]=32;b[41]=rows;b[42]=rng()%16;b[43]=rng()%16;
    for(unsigned i=0;i<rows;++i){const auto v=style==1?0xffffffffu:style==2?0u:rng();put(b,44+i*4,v);put(b,44+rows*4+i*4,style?v:rng());}return b;
}
static std::optional<SpriteVisibilityShape> shape(const Bytes& b){
    const auto cover=u32(b,32),test=u32(b,36);
    if(!test || test>=b.size())return {};
    if(cover>test || cover<40)throw std::runtime_error("Visibility frame plane bounds");
    Bytes c(b.begin()+cover,b.begin()+test),t(b.begin()+test,b.end());
    return decodeSpriteVisibility(c,t,std::int32_t(u32(b,12)),std::int32_t(u32(b,16)));
}
int main(int argc,char** argv)try{
    if(argc!=2 && argc!=3)throw std::runtime_error("Expected pinned PE and optional queue fixture");
    mapImage(read(argv[1]));
    global(0x6e817c,43);global(0x6e8178,14577);
    std::mt19937 rng(5015);std::uint64_t helpers=0,passes=0;
    const std::int32_t kinds[]={-2,0,1,2,8,9,10,33,34,35,0x7fffffff};
    using Test=bool(__attribute__((thiscall)) *)(Queue*,void*,std::int32_t,std::int32_t,std::int32_t);
    using Pass=void(__attribute__((thiscall)) *)(Queue*);
    if(argc==3){
        const auto bytes=read(argv[2]);const auto count=u32(bytes,0),expanded=u32(bytes,4);if(count>64 || expanded>1)throw std::runtime_error("Fixture count/view");
        *reinterpret_cast<std::uint8_t*>(0x6de6d5)=expanded;
        std::vector<Bytes> frames;std::vector<std::array<std::uint32_t,9>> records(count);std::vector<SpriteVisibilityEntry> entries;
        std::size_t at=8;
        for(unsigned i=0;i<count;++i){const auto x=u32(bytes,at),y=u32(bytes,at+4),kind=u32(bytes,at+8),length=u32(bytes,at+12);at+=16;
            if(length<40 || length>1024*1024 || at>bytes.size() || length>bytes.size()-at)throw std::runtime_error("Fixture frame extent");
            frames.emplace_back(bytes.begin()+at,bytes.begin()+at+length);at+=length;
            entries.push_back({shape(frames.back()),std::int32_t(x),std::int32_t(y),std::int32_t(kind),0,{}});
            auto& r=records[i];r[1]=reinterpret_cast<std::uintptr_t>(frames.back().data());r[2]=x;r[3]=y;r[6]=kind;
            SpriteVisibilityGrid probe(expanded);probe.testAndCover(entries.back());
        }
        if(at!=bytes.size())throw std::runtime_error("Trailing fixture bytes");
        Bytes grid(SpriteVisibilityGrid::bytes);SpriteVisibilityGrid native(expanded);std::vector<VisibilityOwner> owners;
        Queue q{reinterpret_cast<std::uintptr_t>(records.data()),0,count,count,reinterpret_cast<std::uintptr_t>(grid.data()),0};
        reinterpret_cast<Pass>(0x5015f0)(&q);applySpriteVisibility(entries,native,owners);require(grid==native.data());
        std::cout<<"{\"kinds\":[";for(unsigned i=0;i<count;++i){if(i)std::cout<<',';require(records[i][6]==std::uint32_t(entries[i].kind));std::cout<<entries[i].kind;}
        std::cout<<"],\"all_match\":true}\n";return 0;
    }
    for(unsigned expanded=0;expanded<2;++expanded){*reinterpret_cast<std::uint8_t*>(0x6de6d5)=expanded;
        for(unsigned n=0;n<4096;++n){auto f=frame(rng,rng()%33,n%3);put(f,12,rng());put(f,16,rng());
            const std::int32_t x=int(rng()%850)-40,y=int(rng()%650)-40,kind=kinds[n%11];
            // Origins cancel to exercise bounded and clipping coordinate branches.
            const auto drawX=std::uint32_t(x)+u32(f,12)+f[42],drawY=std::uint32_t(y)+u32(f,16)+f[43];
            Bytes grid(SpriteVisibilityGrid::bytes);for(auto& byte:grid)byte=(n/3)%3==0?0:(n/3)%3==1?255:rng();
            SpriteVisibilityGrid native(expanded);native.seed(grid);Queue q{0,0,0,0,reinterpret_cast<std::uintptr_t>(grid.data()),0};
            SpriteVisibilityEntry entry{shape(f),std::int32_t(drawX),std::int32_t(drawY),kind,0,{}};
            const auto actual=reinterpret_cast<Test>(0x5013c0)(&q,f.data(),entry.x,entry.y,kind);
            require(actual==native.testAndCover(entry) && grid==native.data());++helpers;
        }
        for(unsigned n=0;n<512;++n){
            std::array<Bytes,3> f{frame(rng,4,n%3),frame(rng,4,n%3),frame(rng,4,n%3)};
            // All roles share the same mask origin so overlap probes all flags.
            for(auto& b:f){b[42]=b[43]=0;}
            Bytes descriptor(16),header(16),offsets(20),definitions(356),owner(12);
            put(header,12,3);put(descriptor,0,reinterpret_cast<std::uintptr_t>(header.data()));
            put(descriptor,8,reinterpret_cast<std::uintptr_t>(offsets.data()));put(descriptor,12,reinterpret_cast<std::uintptr_t>(f[0].data()));
            for(unsigned role=0;role<3;++role)put(offsets,(role+1)*4,reinterpret_cast<std::uintptr_t>(f[role].data())-reinterpret_cast<std::uintptr_t>(f[0].data()));
            put(definitions,0x84,1);put(definitions,0xd0,2);put(definitions,0xe0,3);
            global(0x68995d,reinterpret_cast<std::uintptr_t>(descriptor.data()));global(0x689961,0);global(0x65660c,reinterpret_cast<std::uintptr_t>(definitions.data()));
            const auto flags8=std::uint16_t(rng()),flags10=std::uint16_t(rng());std::memcpy(owner.data()+8,&flags8,2);std::memcpy(owner.data()+10,&flags10,2);
            std::vector<VisibilityOwner> owners{{1,2,3,flags8,flags10}};
            std::vector<std::array<std::uint32_t,9>> records(n%17);std::vector<SpriteVisibilityEntry> entries;
            Bytes grid(SpriteVisibilityGrid::bytes,n%2?255:0);SpriteVisibilityGrid native(expanded);native.seed(grid);
            for(unsigned i=0;i<records.size();++i){const auto role=i%3;auto& r=records[i];r[1]=reinterpret_cast<std::uintptr_t>(f[role].data());r[2]=100+(n%4==0?i:0);r[3]=100;r[5]=i%4?reinterpret_cast<std::uintptr_t>(owner.data()):0;r[6]=std::uint32_t(kinds[n%11]);
                entries.push_back({shape(f[role]),std::int32_t(r[2]),std::int32_t(r[3]),std::int32_t(r[6]),role+1,i%4?std::optional<std::size_t>(0):std::nullopt});}
            const auto before=records;Queue q{reinterpret_cast<std::uintptr_t>(records.data()),0,std::uint32_t(records.size()),0,reinterpret_cast<std::uintptr_t>(grid.data()),0};
            reinterpret_cast<Pass>(0x5015f0)(&q);applySpriteVisibility(entries,native,owners);
            require(grid==native.data());
            for(unsigned i=0;i<records.size();++i){auto expected=before[i];expected[6]=std::uint32_t(entries[i].kind);require(records[i]==expected);}
            std::uint16_t a,b;std::memcpy(&a,owner.data()+8,2);std::memcpy(&b,owner.data()+10,2);require(a==owners[0].flags8 && b==owners[0].flags10);++passes;
        }
    }
    std::cout<<"{\"helpers\":"<<helpers<<",\"passes\":"<<passes<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
