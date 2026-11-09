// Independent original/native processes; no game, Win32 imports or PE file patch.
#include "ground_picking.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>
#include <stdexcept>
#include <sys/mman.h>
using namespace mnm::reconstruction;
using Bytes=std::vector<std::uint8_t>;
using U=std::uint32_t;using I=std::int32_t;
static void require(bool b,const char* s){if(!b)throw std::runtime_error(s);}
static Bytes read(const char* p){std::ifstream f(p,std::ios::binary);require(bool(f),"Input open");return Bytes(std::istreambuf_iterator<char>(f),{});}
static U get(const void* p){U v;std::memcpy(&v,p,4);return v;}
static U word(const Bytes& b,std::size_t p){require(p<=b.size()&&b.size()-p>=4,"Input extent");return get(b.data()+p);}
static void put(void* p,std::size_t n,U v){std::memcpy(static_cast<std::uint8_t*>(p)+n,&v,4);}
static void half(void* p,std::size_t n,std::uint16_t v){std::memcpy(static_cast<std::uint8_t*>(p)+n,&v,2);}
static void global(U p,U v){put(reinterpret_cast<void*>(p),0,v);}
static U at(U p){return get(reinterpret_cast<void*>(p));}
static U address(const void* p){return U(reinterpret_cast<std::uintptr_t>(p));}
static I bits(U v){I n;std::memcpy(&n,&v,4);return n;}
static std::size_t mapImage(const Bytes& b,bool original){
    const auto pe=word(b,60),opt=pe+24,length=word(b,opt+56);
    require(word(b,opt+28)==0x400000&&length<32*1024*1024,"PE image");
    void* p=mmap(reinterpret_cast<void*>(0x400000),length,PROT_READ|PROT_WRITE,
                 MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0);
    require(p!=MAP_FAILED,"Private PE mapping");
    const auto sections=std::uint16_t(b.at(pe+6))|(std::uint16_t(b.at(pe+7))<<8);
    const auto size=std::uint16_t(b.at(pe+20))|(std::uint16_t(b.at(pe+21))<<8);
    for(unsigned k=0;k<unsigned(sections);++k){const auto s=opt+size+k*40,rva=word(b,s+12),n=word(b,s+16),raw=word(b,s+20);
        require(std::uint64_t(raw)+n<=b.size()&&std::uint64_t(rva)+n<=length,"PE section");
        std::memcpy(static_cast<std::uint8_t*>(p)+rva,b.data()+raw,n);
    }
    const std::uint8_t leaf[]={0x8b,0x41,0x10,0x53,0x56};
    const std::uint8_t cell[]={0x55,0x8b,0xec,0x83,0xe4,0xf8};
    require(!std::memcmp(reinterpret_cast<void*>(0x57dcf0),leaf,sizeof(leaf)),"Point leaf guard");
    require(!std::memcmp(reinterpret_cast<void*>(0x4f8960),cell,sizeof(cell)),"Terrain leaf guard");
    if(original){
        for(unsigned k=0;k<unsigned(sections);++k){const auto s=opt+size+k*40;
            if(word(b,s+36)&0x20000000u){const U rva=word(b,s+12),n=std::max(word(b,s+8),word(b,s+16));
                require(!mprotect(static_cast<std::uint8_t*>(p)+(rva&~4095u),((rva&4095u)+n+4095u)&~4095u,PROT_READ|PROT_EXEC),"Original code RX");
            }
        }
    }else require(!mprotect(p,length,PROT_NONE),"Native PE poison");
    return length;
}
// Only the four previously unacquired maximum-layer scenarios execute here.
int main(int argc,char** argv)try{
    require(argc==4,"Expected original/native, PE and output");
    const bool original=std::string(argv[1])=="original";
    require(original||std::string(argv[1])=="native","Mode");
    mapImage(read(argv[2]),original);
    std::ofstream out(argv[3],std::ios::binary);require(bool(out),"Output");
    out.write("MNMRZ031",8);
    const auto emit=[&](U value){out.write(reinterpret_cast<const char*>(&value),4);require(bool(out),"Write");};
    std::vector<PickingMask> masks;
    masks.emplace_back(0,0,0,0,std::vector<std::uint8_t>{});
    masks.emplace_back(1,1,0,0,std::vector<std::uint8_t>{1});
    std::vector<TerrainDefinition> definitions(2);definitions[1].body.fill(1);
    std::array<std::uint8_t,712> def{};
    for(U v=0;v<4;++v)put(def.data(),356+0x84+4*v,1);
    std::array<std::uint8_t,128> rawFrames{};
    put(rawFrames.data(),0,40);put(rawFrames.data(),64,52);
    put(rawFrames.data(),68,1);put(rawFrames.data(),72,1);
    put(rawFrames.data(),104,48);put(rawFrames.data(),108,50);
    rawFrames[112]=0;rawFrames[113]=1; // opaque word at114 is zero.
    std::array<U,4> header{{0,0,0,1}};std::array<U,2> offsets{{0,64}};
    const std::array<U,4> bag{{address(header.data()),0,address(offsets.data()),address(rawFrames.data())}};
    for(U view=0;view<4;++view){
        TerrainCamera c;c.view=view;c.column=c.row=64;c.span=1;c.diagonal=0;
        c.cutLevel=32;c.f11=400;c.f15=560;c.f41=0;c.specialLayer=~0u;
        const U index=(31*128+64)*128+64;
        std::vector<GroundTile> cells(128*128*32);cells[index]={1,0x8004,0};
        U x,y,z,row,count;I depth;
        if(!original){
            const auto picked=pickWorldGround(128,128,32,c,{0,0,800,600},1,false,cells,definitions,masks,400,64);
            require(picked.ground.has_value(),"Native upper layer missed");
            x=picked.ground->column;y=picked.ground->row;z=picked.ground->layer;
            row=picked.depthRow;count=picked.submittedRows;depth=picked.terrainDepth;
            require(picked.flags[index][0]==0x8004,"Native flag mutation");
        }else{
            Bytes map(0x6800),rawCells(cells.size()*12),light(128*128+1);
            for(U n=0;n<cells.size();++n){put(rawCells.data(),12*n+2,~0u);half(rawCells.data(),12*n+6,0xffff);}
            half(rawCells.data(),12*index,1);half(rawCells.data(),12*index+8,0x8004);
            put(map.data(),4,128);put(map.data(),8,128);put(map.data(),12,32);put(map.data(),16,128*128);
            put(map.data(),0x4c,address(rawCells.data()));put(map.data(),0x7f0,~0u);
            for(U n=0;n<32;++n)put(map.data(),0x6432+4*n,n*128*128);
            for(U n=0;n<128;++n)put(map.data(),0x64b2+4*n,n*128);
            std::array<std::uint8_t,0x260> scene{};
            put(scene.data(),0x21,address(map.data()));put(scene.data(),0x25,address(map.data()));put(scene.data(),0x2d,address(bag.data()));
            for(const auto& v:std::vector<std::pair<U,U>>{{0x31,view},{0x23d,view},{0x39,64},{0x3d,64},{0x59,1},{0x61,32},{0x65,1},{0x11,400},{0x15,560},{0x91,400},{0x95,64},{0x1ed,U(-8192)}})put(scene.data(),v.first,v.second);
            std::array<std::array<U,9>,128> queue{};
            put(scene.data(),0x229,address(queue.data()));put(scene.data(),0x22d,address(queue.data()));put(scene.data(),0x235,queue.size());
            global(0x65660c,address(def.data()));global(0x6c5c5c,address(light.data()));
            global(0x5e41ac,1);global(0x5e41b0,0);global(0x6a49cc,0);global(0x6c3750,0);
            for(U n=0;n<128;++n)global(0x6cb942+4*n,n*128);
            for(U n=0;n<32;++n)global(0x6cb8c2+4*n,0);
            for(U n=0;n<500;++n)global(0x6e818c+4*n,82*n/100);
            for(U p:{0x6c5488u,0x6c5484u,0x6cbb64u})global(p,~0u);
            const std::array<I,4> viewport{{0,0,800,600}};
            const std::array<U,4> entries{{0x4f83f0,0x4fbe00,0x4fc3f0,0x4fc930}};
            using Run=void(__attribute__((thiscall)) *)(void*,const void*);
            reinterpret_cast<Run>(entries[view])(scene.data(),viewport.data());
            x=at(0x6c5488);y=at(0x6c5484);z=at(0x6cbb64);row=at(0x6c3750);
            count=get(scene.data()+0x231);depth=bits(get(scene.data()+0x1ed));
            require(rawCells[12*index+8]==4&&rawCells[12*index+9]==0x80,"Original flag mutation");
        }
        require(x==64&&y==64&&z==31&&row==0&&count==1,"Upper-layer publication");
        for(U value:{view,x,y,z,U(depth),row,count})emit(value);
    }
    std::cout<<"{\"success\":true,\"world_cases\":4,\"grid\":[128,128,32],\"ground_z\":31,\"views\":[0,1,2,3],\"native_pe_unreadable\":"<<(original?"false":"true")<<"}\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
