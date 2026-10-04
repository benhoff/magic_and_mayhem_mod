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
 if(argc!=4)throw std::runtime_error("Expected pinned PE");
 mapImage(read(argv[1]));
 require(*reinterpret_cast<unsigned char*>(0x4f8960)==0x55);
 *reinterpret_cast<std::uint32_t*>(0x5e41ac)=1;
 *reinterpret_cast<std::uint32_t*>(0x6a49cc)=0;
 for(unsigned i=0;i<500;++i)reinterpret_cast<std::uint32_t*>(0x6e818c)[i]=82*i/100;
 std::array<unsigned char,64*64> light{};
 *reinterpret_cast<std::uintptr_t*>(0x6c5c5c)=reinterpret_cast<std::uintptr_t>(light.data());
 for(unsigned i=0;i<64;++i){put(reinterpret_cast<void*>(0x6cb942),4*i,64*i);put(reinterpret_cast<void*>(0x6cb8c2),4*i,0);}
 std::array<unsigned char,356> definition{};
 *reinterpret_cast<std::uintptr_t*>(0x65660c)=reinterpret_cast<std::uintptr_t>(definition.data());
 std::array<unsigned char,16> header{};put(header.data(),12,3);
 std::array<unsigned char,4*64> frames{};std::array<std::uint32_t,4> offsets{{0,64,128,192}};
 for(unsigned i=0;i<4;++i){put(frames.data(),i*64+4,1);put(frames.data(),i*64+8,1);}
 std::array<std::uint32_t,4> collection{{reinterpret_cast<std::uintptr_t>(header.data()),0,reinterpret_cast<std::uintptr_t>(offsets.data()),reinterpret_cast<std::uintptr_t>(frames.data())}};
 using Run=void(__attribute__((thiscall)) *)(void*);
 const auto catalog=read(argv[2]);require(catalog.size()>=16 && catalog.size()==16+std::uint64_t(u32(catalog,12))*356);
 std::mt19937 rng(79186);unsigned cases=0,records=0;
 for(unsigned n=0;n<8192+4*u32(catalog,12);++n){
  mnm::reconstruction::TerrainDefinition d;
  for(unsigned v=0;v<4;++v){d.body[v]=rng()%6;d.first[v]=rng()%6;d.second[v]=rng()%6;put(definition.data(),0x84+4*v,d.body[v]);put(definition.data(),0xd0+4*v,d.first[v]);put(definition.data(),0xe0+4*v,d.second[v]);}
  if(n>=8192){std::copy_n(catalog.data()+16+((n-8192)/4)*356,356,definition.data());d=mnm::reconstruction::decodeTerrainDefinition(definition);}
  mnm::reconstruction::TerrainTile t{int(rng()%8),int(rng()%8),int(rng()%40),100,110,int(rng()),std::uint16_t(rng()&0x80ff),std::uint16_t(rng()&0xe48),std::int8_t(rng())};
  mnm::reconstruction::TerrainAdmission a{std::uint32_t(rng()%4),std::uint32_t(1+rng()%40),std::uint32_t(1+rng()%3),std::uint32_t(rng()%2),int(rng()%100),bool(rng()%2)};
  if(n>=8192){a.view=(n-8192)%4;t.flags8=t.flags10=0;}
  light.fill(std::uint8_t(t.light));
  *reinterpret_cast<std::uint32_t*>(0x5e41b0)=a.forceVisible;
  std::array<unsigned char,12> owner{};put(owner.data(),2,0xffffffff);owner[6]=owner[7]=255;std::memcpy(owner.data()+8,&t.flags8,2);std::memcpy(owner.data()+10,&t.flags10,2);
  std::array<unsigned char,0x260> scene{};
  put(scene.data(),0x2d,reinterpret_cast<std::uintptr_t>(collection.data()));put(scene.data(),0x31,a.view);put(scene.data(),0x61,a.cutLevel);put(scene.data(),0x65,a.player);put(scene.data(),0x6d,a.mode);put(scene.data(),0x79,reinterpret_cast<std::uintptr_t>(owner.data()));put(scene.data(),0x7d,t.column);put(scene.data(),0x81,t.row);put(scene.data(),0x85,t.level);put(scene.data(),0x89,t.anchorX);put(scene.data(),0x8d,t.anchorY);put(scene.data(),0x91,0xffff0000);put(scene.data(),0x95,0xffff0000);put(scene.data(),0x9d,t.priority);put(scene.data(),0xb1,a.lightBias);
  std::array<std::array<std::uint32_t,9>,3> queue{};
  put(scene.data(),0x229,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x22d,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x235,3);put(scene.data(),0x23d,a.view);
  reinterpret_cast<Run>(0x4f8960)(scene.data());
  const auto native=mnm::reconstruction::submitTerrain(d,t,a,3);
  std::uint32_t count;std::memcpy(&count,scene.data()+0x231,4);require(count==native.size());
  require(std::memcmp(owner.data()+8,&t.flags8,2)==0 && std::memcmp(owner.data()+10,&t.flags10,2)==0);
  for(unsigned i=0;i<count;++i){const auto& r=queue[i];const auto& q=native[i];require(r[0]==std::uint32_t(q.key) && r[1]==reinterpret_cast<std::uintptr_t>(frames.data()+64*q.frame) && r[2]==std::uint32_t(q.anchorX) && r[3]==std::uint32_t(q.anchorY) && (r[4]&0xffff)==std::uint16_t(q.shade) && r[5]==reinterpret_cast<std::uintptr_t>(owner.data()) && r[6]==std::uint32_t(q.kind) && r[7]==0x8ad08ad0);++records;}
  ++cases;
 }

 const auto installed=read(argv[3]);const auto frameCount=u32(installed,12),palettes=u32(installed,16),table=24+768*palettes,base=table+4*frameCount;
 std::array<std::uint32_t,4> realCollection{{reinterpret_cast<std::uintptr_t>(installed.data()),0,reinterpret_cast<std::uintptr_t>(installed.data()+table),reinterpret_cast<std::uintptr_t>(installed.data()+base)}};
 *reinterpret_cast<std::uintptr_t*>(0x65660c)=reinterpret_cast<std::uintptr_t>(catalog.data()+16);
 *reinterpret_cast<std::uintptr_t*>(0x68995d)=reinterpret_cast<std::uintptr_t>(realCollection.data());
 *reinterpret_cast<std::uint32_t*>(0x6de6d5)=0;
 *reinterpret_cast<std::uint32_t*>(0x6e817c)=43;
 light.fill(0);
 using QueueOp=void(__attribute__((thiscall)) *)(void*);
 std::cout<<"{\"cases\":"<<cases<<",\"records\":"<<records<<",\"previews\":[";
 bool comma=false;
 for(unsigned view=0;view<4;++view)for(unsigned overlap=0;overlap<2;++overlap)for(unsigned visibility=0;visibility<2;++visibility)for(unsigned repeated=0;repeated<2;++repeated){
  std::array<std::array<unsigned char,12>,9> owners{};std::array<std::array<std::uint32_t,9>,27> queue{};Bytes grid(14577);
  std::array<unsigned char,0x260> scene{};put(scene.data(),0x2d,reinterpret_cast<std::uintptr_t>(realCollection.data()));put(scene.data(),0x31,view);put(scene.data(),0x61,1);put(scene.data(),0x65,1);put(scene.data(),0x91,0xffff0000);put(scene.data(),0x95,0xffff0000);
  put(scene.data(),0x229,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x22d,reinterpret_cast<std::uintptr_t>(queue.data()));put(scene.data(),0x235,27);put(scene.data(),0x239,reinterpret_cast<std::uintptr_t>(grid.data()));put(scene.data(),0x23d,view);
  *reinterpret_cast<std::uint32_t*>(0x689961)=view;
  for(unsigned tile=0;tile<9;++tile){const unsigned id=repeated?1745:5+4*tile,row=tile/3,col=tile%3;auto& owner=owners[tile];owner[0]=id&255;owner[1]=id>>8;put(owner.data(),2,0xffffffff);owner[6]=owner[7]=255;
   put(scene.data(),0x79,reinterpret_cast<std::uintptr_t>(owner.data()));put(scene.data(),0x7d,col);put(scene.data(),0x81,row);put(scene.data(),0x89,overlap?256:256+32*(int(col)-int(row)));put(scene.data(),0x8d,overlap?160:96+16*(col+row));
   reinterpret_cast<Run>(0x4f8960)(scene.data());
  }
  reinterpret_cast<QueueOp>(0x4fff60)(scene.data()+0x229);if(visibility)reinterpret_cast<QueueOp>(0x5015f0)(scene.data()+0x229);
  if(comma)std::cout<<',';
  comma=true;
  std::cout<<"{\"view\":"<<view<<",\"overlap\":"<<overlap<<",\"visibility\":"<<visibility<<",\"repeated\":"<<repeated<<",\"queue\":[";
  std::uint32_t count;std::memcpy(&count,scene.data()+0x231,4);
  for(unsigned i=0;i<count;++i){if(i)std::cout<<',';const auto& r=queue[i];unsigned frame=0;for(;frame<frameCount;++frame)if(r[1]==reinterpret_cast<std::uintptr_t>(installed.data()+base+u32(installed,table+4*frame)))break;require(frame<frameCount);
   const auto tile=(r[5]-reinterpret_cast<std::uintptr_t>(owners.data()))/12;
   std::cout<<"{\"tile\":"<<tile<<",\"frame\":"<<frame<<",\"role\":0,\"key\":"<<std::int32_t(r[0])<<",\"kind\":"<<std::int32_t(r[6])<<",\"x\":"<<std::int32_t(r[2])<<",\"y\":"<<std::int32_t(r[3])<<",\"shade\":0}";
  }
  std::cout<<"],\"owners\":[";for(unsigned i=0;i<9;++i){if(i)std::cout<<',';std::uint16_t f8,f10;std::memcpy(&f8,owners[i].data()+8,2);std::memcpy(&f10,owners[i].data()+10,2);std::cout<<'['<<f8<<','<<f10<<']';}std::cout<<"]}";
 }
 std::cout<<"]}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
