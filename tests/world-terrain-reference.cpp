// Selected unchanged four-orientation world traversal, no Win32 calls.
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
 if(argc!=13)throw std::runtime_error("Expected PE TTD SPR decoded_MAP column row span cut panX panY visibility view");
 mapImage(read(argv[1]));const auto catalog=read(argv[2]),sprite=read(argv[3]),raw=read(argv[4]);
 const auto width=u32(raw,4),height=u32(raw,8),layers=u32(raw,12);const int column=std::stoi(argv[5]),row=std::stoi(argv[6]),span=std::stoi(argv[7]),cut=std::stoi(argv[8]),panX=std::stoi(argv[9]),panY=std::stoi(argv[10]),visibility=std::stoi(argv[11]),view=std::stoi(argv[12]);
 require(view>=0 && view<4 && width<=128 && height<=128 && layers<=32 && raw.size()==76+12*std::uint64_t(width)*height*layers && span>0 && unsigned(span)<=std::min(width,height) && column>=0 && unsigned(column)<width && row>=0 && unsigned(row)<height && cut>0 && unsigned(cut)<=layers);
 Bytes cells(raw.begin()+76,raw.end()),map(0x66c0);
 for(unsigned i=0;i<width*height*layers;++i){const auto at=12*i;for(unsigned n=2;n<8;++n)cells[at+n]=255;cells[at+11]&=0xdf;} // Isolate ordinary terrain, no creature path.
 put(map.data(),4,width);put(map.data(),8,height);put(map.data(),12,layers);put(map.data(),16,width*height);put(map.data(),0x4c,reinterpret_cast<std::uintptr_t>(cells.data()));put(map.data(),0x7f0,0xffffffff);
 for(unsigned y=0;y<height;++y)put(map.data(),0x64b2+4*y,y*width);
 for(unsigned z=0;z<layers;++z)put(map.data(),0x6432+4*z,z*width*height);
 const auto count=u32(sprite,12),palettes=u32(sprite,16),table=24+768*palettes,base=table+4*count;
 std::array<std::uint32_t,4> collection{{reinterpret_cast<std::uintptr_t>(sprite.data()),0,reinterpret_cast<std::uintptr_t>(sprite.data()+table),reinterpret_cast<std::uintptr_t>(sprite.data()+base)}};
 put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(catalog.data()+16));put(reinterpret_cast<void*>(0x68995d),0,reinterpret_cast<std::uintptr_t>(collection.data()));put(reinterpret_cast<void*>(0x689961),0,view);put(reinterpret_cast<void*>(0x6de6d5),0,0);put(reinterpret_cast<void*>(0x6e817c),0,43);put(reinterpret_cast<void*>(0x5e41ac),0,1);put(reinterpret_cast<void*>(0x5e41b0),0,0);put(reinterpret_cast<void*>(0x6a49cc),0,0);
 for(unsigned i=0;i<500;++i)put(reinterpret_cast<void*>(0x6e818c),4*i,82*i/100);
 Bytes light(width*height+1);put(reinterpret_cast<void*>(0x6c5c5c),0,reinterpret_cast<std::uintptr_t>(light.data()));for(unsigned y=0;y<height;++y)put(reinterpret_cast<void*>(0x6cb942),4*y,y*width);for(unsigned z=0;z<layers;++z)put(reinterpret_cast<void*>(0x6cb8c2),4*z,0);
 std::array<unsigned char,0x260> scene{};Bytes grid(14577);std::vector<std::array<std::uint32_t,9>> queue(width*height*layers*3);
 put(scene.data(),0x21,reinterpret_cast<std::uintptr_t>(map.data()));put(scene.data(),0x25,reinterpret_cast<std::uintptr_t>(map.data()));put(scene.data(),0x2d,reinterpret_cast<std::uintptr_t>(collection.data()));
 for(const auto field:std::vector<std::pair<unsigned,int>>{{0x31,view},{0x23d,view},{0x11,panX},{0x15,panY},{0x39,column},{0x3d,row},{0x41,2*(span/2)},{0x59,span},{0x5d,span/2},{0x61,cut},{0x65,1},{0x91,int(0xffff0000)},{0x95,int(0xffff0000)}})put(scene.data(),field.first,field.second);
 put(scene.data(),0x229,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x22d,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x235,queue.size());put(scene.data(),0x239,reinterpret_cast<std::uintptr_t>(grid.data()));
 std::array<int,4> viewport{{0,0,512,256}};using Traversal=void(__attribute__((thiscall)) *)(void*,void*);using Op=void(__attribute__((thiscall)) *)(void*);
 const std::array<unsigned,4> routines{{0x4f83f0,0x4fbe00,0x4fc3f0,0x4fc930}};reinterpret_cast<Traversal>(routines[view])(scene.data(),viewport.data());reinterpret_cast<Op>(0x4fff60)(scene.data()+0x229);if(visibility)reinterpret_cast<Op>(0x5015f0)(scene.data()+0x229);
 const unsigned queued=u32(Bytes(scene.begin(),scene.end()),0x231);std::cout<<"{\"queue\":[";
 for(unsigned i=0;i<queued;++i){if(i)std::cout<<',';const auto& r=queue[i];unsigned frame=0;for(;frame<count;++frame)if(r[1]==reinterpret_cast<std::uintptr_t>(sprite.data()+base+u32(sprite,table+4*frame)))break;require(frame<count);const auto cell=(r[5]-reinterpret_cast<std::uintptr_t>(cells.data()))/12;require(cell<width*height*layers);
  std::cout<<"{\"cell\":"<<cell<<",\"frame\":"<<frame<<",\"key\":"<<std::int32_t(r[0])<<",\"kind\":"<<std::int32_t(r[6])<<",\"x\":"<<std::int32_t(r[2])<<",\"y\":"<<std::int32_t(r[3])<<",\"shade\":"<<std::int16_t(r[4]&0xffff)<<"}";
 }
 std::cout<<"],\"owners\":[";for(unsigned i=0;i<width*height*layers;++i){if(i)std::cout<<',';std::uint16_t a,b;std::memcpy(&a,cells.data()+12*i+8,2);std::memcpy(&b,cells.data()+12*i+10,2);b|=std::uint16_t(raw[76+12*i+11]&0x20)<<8;std::cout<<'['<<a<<','<<b<<']';}std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
