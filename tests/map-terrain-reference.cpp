// Private, unchanged PE mapping: selected placement helpers only; no Win32 calls.
#include "terrain_submission.hpp"
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
static void require(bool ok){if(!ok)throw std::runtime_error("Terrain producer mismatch");}

int main(int argc,char** argv)try{
 if(argc!=13)throw std::runtime_error("Expected PE TTD SPR decoded_MAP x y z width height view overlap visibility");
 mapImage(read(argv[1]));const auto catalog=read(argv[2]),sprite=read(argv[3]),raw=read(argv[4]);
 const auto width=u32(raw,4),height=u32(raw,8),layers=u32(raw,12);unsigned args[8];for(unsigned i=0;i<8;++i)args[i]=std::stoul(argv[5+i]);
 const auto x=args[0],y=args[1],z=args[2],w=args[3],h=args[4],view=args[5],overlap=args[6],visibility=args[7];require(view<4 && w>0 && h>0 && w<=3 && h<=3 && x<width && y<height && z<layers && w<=width-x && h<=height-y);
 require(raw.size()==76+std::uint64_t(width)*height*layers*12);
 const auto count=u32(sprite,12),palettes=u32(sprite,16),table=24+768*palettes,base=table+4*count;
 std::array<std::uint32_t,4> collection{{reinterpret_cast<std::uintptr_t>(sprite.data()),0,reinterpret_cast<std::uintptr_t>(sprite.data()+table),reinterpret_cast<std::uintptr_t>(sprite.data()+base)}};
 put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(catalog.data()+16));put(reinterpret_cast<void*>(0x68995d),0,reinterpret_cast<std::uintptr_t>(collection.data()));put(reinterpret_cast<void*>(0x689961),0,view);put(reinterpret_cast<void*>(0x6de6d5),0,0);put(reinterpret_cast<void*>(0x6e817c),0,43);put(reinterpret_cast<void*>(0x5e41ac),0,1);put(reinterpret_cast<void*>(0x5e41b0),0,0);put(reinterpret_cast<void*>(0x6a49cc),0,0);
 for(unsigned i=0;i<500;++i)put(reinterpret_cast<void*>(0x6e818c),4*i,82*i/100);
 Bytes light(std::size_t(width)*height);put(reinterpret_cast<void*>(0x6c5c5c),0,reinterpret_cast<std::uintptr_t>(light.data()));for(unsigned i=0;i<height;++i)put(reinterpret_cast<void*>(0x6cb942),4*i,i*width);for(unsigned i=0;i<layers;++i)put(reinterpret_cast<void*>(0x6cb8c2),4*i,0);
 std::array<std::array<unsigned char,12>,9> owners{};std::array<std::array<std::uint32_t,9>,27> queue{};Bytes grid(14577);std::array<unsigned char,0x260> scene{};
 put(scene.data(),0x2d,reinterpret_cast<std::uintptr_t>(collection.data()));put(scene.data(),0x31,view);put(scene.data(),0x61,1);put(scene.data(),0x65,1);put(scene.data(),0x85,z);put(scene.data(),0x91,0xffff0000);put(scene.data(),0x95,0xffff0000);
 put(scene.data(),0x229,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x22d,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x235,27);put(scene.data(),0x239,reinterpret_cast<std::uintptr_t>(grid.data()));put(scene.data(),0x23d,view);
 using Op=void(__attribute__((thiscall)) *)(void*);
 for(unsigned row=0;row<h;++row)for(unsigned col=0;col<w;++col){const auto tile=row*w+col;auto& owner=owners[tile];const auto at=76+12*((z*height+y+row)*width+x+col);std::copy_n(raw.data()+at,12,owner.data());
  const unsigned id=owner[0]|(unsigned(owner[1])<<8);require(id<u32(catalog,12) && !(owner[11]&0x20));
  // Selected ordinary terrain only: fixture references do not create objects.
  for(unsigned i=2;i<8;++i)owner[i]=255;
  put(scene.data(),0x79,reinterpret_cast<std::uintptr_t>(owner.data()));put(scene.data(),0x7d,x+col);put(scene.data(),0x81,y+row);put(scene.data(),0x89,overlap?256:256+32*(int(col)-int(row)));put(scene.data(),0x8d,overlap?160:96+16*(col+row));reinterpret_cast<Op>(0x4f8960)(scene.data());
 }
 reinterpret_cast<Op>(0x4fff60)(scene.data()+0x229);if(visibility)reinterpret_cast<Op>(0x5015f0)(scene.data()+0x229);
 std::uint32_t queued;std::memcpy(&queued,scene.data()+0x231,4);std::cout<<"{\"queue\":[";
 for(unsigned i=0;i<queued;++i){if(i)std::cout<<',';const auto& r=queue[i];unsigned frame=0;for(;frame<count;++frame)if(r[1]==reinterpret_cast<std::uintptr_t>(sprite.data()+base+u32(sprite,table+4*frame)))break;require(frame<count);const auto tile=(r[5]-reinterpret_cast<std::uintptr_t>(owners.data()))/12;
  std::cout<<"{\"tile\":"<<tile<<",\"frame\":"<<frame<<",\"key\":"<<std::int32_t(r[0])<<",\"kind\":"<<std::int32_t(r[6])<<",\"x\":"<<std::int32_t(r[2])<<",\"y\":"<<std::int32_t(r[3])<<",\"shade\":"<<std::int16_t(r[4]&0xffff)<<"}";
 }
 std::cout<<"],\"owners\":[";for(unsigned i=0;i<w*h;++i){if(i)std::cout<<',';std::uint16_t a,b;std::memcpy(&a,owners[i].data()+8,2);std::memcpy(&b,owners[i].data()+10,2);std::cout<<'['<<a<<','<<b<<']';}std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
