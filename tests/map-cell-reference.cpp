// Private, unchanged PE mapping: selected placement helpers only; no Win32 calls.
#include "map.hpp"
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



static void put(void* p,unsigned off,std::uint32_t v){std::memcpy(static_cast<unsigned char*>(p)+off,&v,4);}
static void require(bool ok){if(!ok)throw std::runtime_error("Original MAP indexing/selected cell check mismatch");}
int main(int argc,char** argv)try{
 if(argc<3)throw std::runtime_error("Expected pinned PE and decoded MAP fixtures");
 mapImage(read(argv[1]));require(*reinterpret_cast<unsigned char*>(0x4f13f0)==0x83);
 using Check=unsigned(__attribute__((thiscall)) *)(void*,int,int,int,int,int,unsigned);
 std::uint64_t cells=0,checks=0;
 for(int arg=2;arg<argc;++arg){const auto raw=read(argv[arg]);auto decoded=mnm::assets::decodeMapPayload(raw);require(std::holds_alternative<mnm::assets::MapAsset>(decoded));const auto map=std::get<mnm::assets::MapAsset>(std::move(decoded));
  Bytes world(0x6700);put(world.data(),4,map.width);put(world.data(),8,map.height);put(world.data(),12,map.layers);put(world.data(),16,map.width*map.height);put(world.data(),0x4c,reinterpret_cast<std::uintptr_t>(raw.data()+76));
  for(unsigned y=0;y<map.height;++y)put(world.data(),0x64b2+4*y,y*map.width);
  for(unsigned z=0;z<map.layers;++z)put(world.data(),0x6432+4*z,z*map.width*map.height);
  for(unsigned z=0;z<map.layers;++z)for(unsigned y=0;y<map.height;++y)for(unsigned x=0;x<map.width;++x){const auto& cell=map.cell(x,y,z);
   for(unsigned token:{0xfffeU,unsigned(cell.references[1])}){const auto expected=!(cell.flags10&0x80) && (cell.flags10&3) && cell.references[1]!=token;
    require(reinterpret_cast<Check>(0x4f13f0)(world.data(),x,y,z,1,1,token)==expected);++checks;}
   ++cells;
  }
 }
 std::cout<<"{\"cells\":"<<cells<<",\"checks\":"<<checks<<"}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
