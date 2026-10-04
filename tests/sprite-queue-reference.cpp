// Private, unchanged PE mapping: selected placement helpers only; no Win32 calls.
#include "sprite_queue.hpp"
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
    if(p[0xffd30]!=0x56 || p[0xfff60]!=0x55)throw std::runtime_error("Controller entry bytes differ");
}

static void require(bool ok){if(!ok)throw std::runtime_error("Original/native queue mismatch");}
struct Queue {std::uint32_t base,next,count,capacity,mask,view;};
using Record=std::array<std::uint32_t,9>;
int main(int argc,char** argv)try{
    if(argc!=2)throw std::runtime_error("Expected pinned PE");
    mapImage(read(argv[1]));
    *reinterpret_cast<std::uint32_t*>(0x5e41ac)=1;
    for(unsigned i=0;i<500;++i)reinterpret_cast<std::uint32_t*>(0x6e818c)[i]=82*i/100;
    using Add=std::uint32_t(__attribute__((thiscall)) *)(Queue*,std::uint32_t,std::int32_t,std::int32_t,std::int32_t,std::int32_t,std::int32_t,std::int32_t,std::int32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t,std::uint32_t);
    using Sort=void(__attribute__((thiscall)) *)(Queue*);
    std::mt19937 rng(79186);std::uint64_t keys=0,sorts=0,records=0;
    const std::int32_t heights[]={0,1,99,499,500,501,100000,0x7fffffff};
    for(auto h:heights)for(unsigned view=0;view<4;++view)for(unsigned n=0;n<128;++n){
        mnm::reconstruction::SpriteDepth p{std::int32_t(rng()),std::int32_t(rng()),h,std::int32_t(rng())};
        Record r{};Queue q{reinterpret_cast<std::uintptr_t>(&r),reinterpret_cast<std::uintptr_t>(&r),0,1,0,view};
        const auto result=reinterpret_cast<Add>(0x4ffd30)(&q,0,p.x,p.y,p.height,12,13,p.priority,0,0,0,0,0,0,0);
        require(result==0 && q.count==1 && r[0]==std::uint32_t(mnm::reconstruction::spriteDepthKey(p,view)));++keys;
    }
    for(unsigned kind=0;kind<5;++kind)for(unsigned n=0;n<=256;++n){
        std::vector<Record> original(n);std::vector<mnm::reconstruction::SpriteQueueEntry> native;
        for(unsigned i=0;i<n;++i){std::int32_t key=kind==0?0:kind==1?std::int32_t(i):kind==2?-std::int32_t(i):kind==3?std::int32_t(rng()%7)-3:std::int32_t(rng());
            for(unsigned j=0;j<9;++j)original[i][j]=i*19+j;
            original[i][0]=std::uint32_t(key);original[i][1]=i;native.push_back({key,i});}
        const auto before=original;Queue q{reinterpret_cast<std::uintptr_t>(original.data()),0,n,n,0,0};
        reinterpret_cast<Sort>(0x4fff60)(&q);mnm::reconstruction::sortSpriteQueue(native);
        for(unsigned i=0;i<n;++i){require(original[i]==before[native[i].payload]);if(i)require(native[i-1].key<=native[i].key);}
        ++sorts;records+=n;
    }
    std::cout<<"{\"keys\":"<<keys<<",\"sorts\":"<<sorts<<",\"records\":"<<records<<",\"scenes\":[";
    bool comma=false;
    for(unsigned view=0;view<4;++view)for(unsigned kind=0;kind<6;++kind){
        std::array<mnm::reconstruction::SpriteDepth,4> positions{};
        for(unsigned i=0;i<4;++i){auto& p=positions[i];
            if(kind==0)p.x=i*32;
            if(kind==1)p.x=-std::int32_t(i*32);
            if(kind==3)p.priority=-std::int32_t(i*2);
            if(kind==4){p.x=i*7;p.y=-std::int32_t(i*11);p.height=499+i;}
            if(kind==5)p.priority=0x7fffffff-std::int32_t(i);
        }
        std::array<Record,12> original{};Queue q{reinterpret_cast<std::uintptr_t>(original.data()),reinterpret_cast<std::uintptr_t>(original.data()),0,12,0,view};
        for(unsigned i=0;i<4;++i)for(unsigned asset=0;asset<3;++asset){const auto& p=positions[i];const unsigned id=i*3+asset;
            const auto bits=std::uint32_t(p.priority)+(asset==0?6:asset==1?8:9);std::int32_t priority;std::memcpy(&priority,&bits,4);
            reinterpret_cast<Add>(0x4ffd30)(&q,id,p.x,p.y,p.height,0,0,priority,0,0,0,0,0,0,0);
        }
        reinterpret_cast<Sort>(0x4fff60)(&q);
        if(comma)std::cout<<',';
        comma=true;
        std::cout<<"{\"view\":"<<view<<",\"kind\":"<<kind<<",\"positions\":[";
        for(unsigned i=0;i<4;++i){if(i)std::cout<<',';const auto& p=positions[i];std::cout<<'['<<p.x<<','<<p.y<<','<<p.height<<','<<p.priority<<']';}
        std::cout<<"],\"order\":[";for(unsigned i=0;i<12;++i){if(i)std::cout<<',';std::cout<<original[i][1];}
        std::cout<<"],\"depth_keys\":[";for(unsigned i=0;i<12;++i){if(i)std::cout<<',';std::int32_t key;std::memcpy(&key,&original[i][0],4);std::cout<<key;}
        std::cout<<"]}";
    }
    std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
