// Unchanged PE32 traversal + ordinary producer, no Win32 calls.
#include "terrain_submission.hpp"
#include "terrain_traversal.hpp"
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
 if(argc!=2)throw std::runtime_error("Expected pinned PE");
 mapImage(read(argv[1]));
 std::array<unsigned char,356> definition{};put(definition.data(),0x84,1);
 put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(definition.data()));
 std::array<unsigned char,16> header{};put(header.data(),12,3);std::array<unsigned char,256> frames{};std::array<std::uint32_t,4> offsets{{0,64,128,192}};
 for(unsigned i=0;i<4;++i){put(frames.data(),64*i+4,1);put(frames.data(),64*i+8,1);}
 std::array<std::uint32_t,4> collection{{reinterpret_cast<std::uintptr_t>(header.data()),0,reinterpret_cast<std::uintptr_t>(offsets.data()),reinterpret_cast<std::uintptr_t>(frames.data())}};
 put(reinterpret_cast<void*>(0x5e41ac),0,1);put(reinterpret_cast<void*>(0x5e41b0),0,0);put(reinterpret_cast<void*>(0x6a49cc),0,0);
 for(unsigned i=0;i<500;++i)put(reinterpret_cast<void*>(0x6e818c),4*i,82*i/100);
 std::mt19937 rng(0x4f83f0);unsigned records=0,wrapped=0,clipped=0;
 for(unsigned n=0;n<4096;++n){
  const unsigned width=28+rng()%13,height=28+rng()%13,layers=1+rng()%9;Bytes cells(width*height*layers*12);
  for(unsigned i=0;i<width*height*layers;++i){put(cells.data(),i*12+2,0xffffffff);cells[i*12+6]=cells[i*12+7]=255;cells[i*12+8]=(rng()%5==0)?0x80:0;cells[i*12+11]=(rng()%7==0)?0x40:0;}
  auto nativeCells=cells;Bytes map(0x66c0);put(map.data(),4,width);put(map.data(),8,height);put(map.data(),12,layers);put(map.data(),16,width*height);put(map.data(),0x4c,reinterpret_cast<std::uintptr_t>(cells.data()));
  mnm::reconstruction::TerrainCamera c;c.span=1+rng()%20;c.diagonal=rng()%10;c.column=rng()%width;c.row=rng()%height;c.cutLevel=1+rng()%layers;c.mode=rng()%2;c.specialLayer=rng()%layers;
  c.f11=int(rng()%641)-64;c.f15=int(rng()%193)-64;c.f41=2*c.diagonal+int(rng()%3);c.f45=rng()%32;c.f49=rng()%32;c.f4d=int(rng()%17)-8;c.f51=int(rng()%17)-8;c.f55=int(rng()%17)-8;c.wrapColumnPriority=int(rng()%1024)-512;c.wrapRowPriority=int(rng()%1024)-512;
  put(map.data(),0x7f0,c.specialLayer);for(unsigned z=0;z<layers;++z)put(map.data(),0x6432+4*z,z*width*height);for(unsigned y=0;y<height;++y)put(map.data(),0x64b2+4*y,y*width);
  Bytes light(width*height);put(reinterpret_cast<void*>(0x6c5c5c),0,reinterpret_cast<std::uintptr_t>(light.data()));for(unsigned y=0;y<height;++y)put(reinterpret_cast<void*>(0x6cb942),4*y,y*width);for(unsigned z=0;z<layers;++z)put(reinterpret_cast<void*>(0x6cb8c2),4*z,0);
  std::array<unsigned char,0x260> scene{};
  put(scene.data(),0x21,reinterpret_cast<std::uintptr_t>(map.data()));put(scene.data(),0x25,reinterpret_cast<std::uintptr_t>(map.data()));put(scene.data(),0x2d,reinterpret_cast<std::uintptr_t>(collection.data()));
  for(const auto field:std::vector<std::pair<unsigned,std::int32_t>>{{0x11,c.f11},{0x15,c.f15},{0x39,c.column},{0x3d,c.row},{0x41,c.f41},{0x45,c.f45},{0x49,c.f49},{0x4d,c.f4d},{0x51,c.f51},{0x55,c.f55},{0x59,int(c.span)},{0x5d,int(c.diagonal)},{0x61,int(c.cutLevel)},{0x65,1},{0x6d,int(c.mode)},{0x91,int(0xffff0000)},{0x95,int(0xffff0000)},{0xa1,c.wrapColumnPriority},{0xa5,c.wrapRowPriority}})put(scene.data(),field.first,field.second);
  std::vector<std::array<std::uint32_t,9>> queue(width*height*layers);put(scene.data(),0x229,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x22d,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x235,queue.size());
  std::array<int,4> viewport{{int(rng()%33),int(rng()%17),384+int(rng()%129),192+int(rng()%65)}};
  using Run=void(__attribute__((thiscall)) *)(void*,void*);reinterpret_cast<Run>(0x4f83f0)(scene.data(),viewport.data());
  const auto visits=mnm::reconstruction::traverseTerrain(width,height,layers,c,{viewport[0],viewport[1],viewport[2],viewport[3]},[&](unsigned x,unsigned y,unsigned z){const auto at=12*((z*height+y)*width+x);return std::array<std::uint16_t,2>{std::uint16_t(nativeCells[at+8]),std::uint16_t(unsigned(nativeCells[at+11])<<8)};});
  std::vector<std::pair<unsigned,mnm::reconstruction::TerrainDraw>> draws;mnm::reconstruction::TerrainDefinition d;d.body[0]=1;
  for(const auto& v:visits){const unsigned cell=(v.layer*height+v.row)*width+v.column,at=12*cell;std::uint16_t f8,f10;std::memcpy(&f8,nativeCells.data()+at+8,2);std::memcpy(&f10,nativeCells.data()+at+10,2);
   mnm::reconstruction::TerrainTile t{int(v.row),int(v.column),int(v.layer),v.anchorX,v.anchorY,v.priority,f8,f10,0};
   const auto produced=mnm::reconstruction::submitTerrain(d,t,{0,c.cutLevel,1,c.mode},3);for(const auto& draw:produced)draws.push_back({cell,draw});std::memcpy(nativeCells.data()+at+8,&t.flags8,2);std::memcpy(nativeCells.data()+at+10,&t.flags10,2);wrapped+=v.priority!=0;clipped+=v.anchorX<viewport[0] || v.anchorY<viewport[1];
  }
  const unsigned count=u32(Bytes(scene.begin(),scene.end()),0x231);
  if(count!=draws.size()){std::cerr<<"case "<<n<<" count "<<count<<" expected "<<draws.size()<<'\n';require(false);}
  for(unsigned i=0;i<count;++i){const auto& r=queue[i];const auto& [cell,q]=draws[i];if(r[0]!=std::uint32_t(q.key) || r[2]!=std::uint32_t(q.anchorX) || r[3]!=std::uint32_t(q.anchorY) || r[5]!=reinterpret_cast<std::uintptr_t>(cells.data()+12*cell)){std::cerr<<"case "<<n<<" record "<<i<<" expected cell "<<cell<<" got "<<(r[5]-reinterpret_cast<std::uintptr_t>(cells.data()))/12<<" key "<<std::int32_t(r[0])<<" expected "<<q.key<<'\n';require(false);}require(r[1]==reinterpret_cast<std::uintptr_t>(frames.data()+64) && (r[4]&0xffff)==0 && r[6]==unsigned(q.kind));++records;}
  require(nativeCells==cells);
 }
 std::cout<<"{\"cases\":4096,\"records\":"<<records<<",\"wrapped_visits\":"<<wrapped<<",\"margin_visits\":"<<clipped<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
