// Unchanged PE32 traversal + ordinary producer, no Win32 calls.
#include "terrain_submission.hpp"
#include "terrain_camera.hpp"
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

static unsigned get(const void* p,unsigned off){unsigned n;std::memcpy(&n,static_cast<const unsigned char*>(p)+off,4);return n;}
static void compare(const void* p,const mnm::reconstruction::TerrainCamera& c){
 for(const auto f:std::vector<std::pair<unsigned,int>>{{0x31,int(c.view)},{0x35,0},{0x6d,int(c.mode)},{0x39,c.column},{0x3d,c.row},{0x41,c.f41},{0x45,c.f45},{0x49,c.f49},{0x4d,c.f4d},{0x11,c.f11},{0x15,c.f15},{0x51,c.f51},{0x55,c.f55},{0x59,int(c.span)},{0x5d,int(c.diagonal)},{0x61,int(c.cutLevel)}})
  if(get(p,f.first)!=unsigned(f.second))throw std::runtime_error("Camera field mismatch at "+std::to_string(f.first));
}
int main(int argc,char** argv)try{
 if(argc!=2)throw std::runtime_error("Expected pinned PE");mapImage(read(argv[1]));
 using One=void(__attribute__((thiscall)) *)(void*,void*);
 using Three=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned);
 using Two=void(__attribute__((thiscall)) *)(void*,int,int);
 std::mt19937 rng(0x4f7930);unsigned checks=0;
 for(unsigned n=0;n<8192;++n){
  const unsigned w=1+rng()%128,h=1+rng()%128,l=1+rng()%32;
  std::array<unsigned char,0x260> receiver{};std::array<unsigned,5> map{{0,w,h,l,w*h}};
  mnm::reconstruction::TerrainCamera c;c.column=0;c.row=0;c.f41=0;c.f11=0;c.f15=0;c.mode=1;put(receiver.data(),0x6d,c.mode);
  reinterpret_cast<One>(0x4f7930)(receiver.data(),map.data());mnm::reconstruction::bindTerrainCamera(c,w,h,l);
  std::array<int,4> viewport{{-100+int(rng()%200),-100+int(rng()%200),512,256}};
  reinterpret_cast<One>(0x4f7970)(receiver.data(),viewport.data());mnm::reconstruction::setTerrainCameraViewport(c,{viewport[0],viewport[1],viewport[2],viewport[3]});
  const unsigned x=rng()%16384,y=rng()%16384,z=rng()%1024;
  reinterpret_cast<Three>(0x4f7a20)(receiver.data(),x,y,z);mnm::reconstruction::setTerrainCameraPosition(c,w,h,l,x,y,z);compare(receiver.data(),c);++checks;
  c.view=n%4;put(receiver.data(),0x31,c.view);
  for(unsigned step=0;step<16;++step){
   const int dx=step==0?0:step<5?-32*int(step):int(rng()%1025)-512;
   const int dy=step==0?0:int(rng()%1025)-512;
   reinterpret_cast<Two>(0x4f7c60)(receiver.data(),dx,dy);mnm::reconstruction::scrollTerrainCamera(c,w,h,dx,dy);compare(receiver.data(),c);++checks;
  }
 }
 std::cout<<"{\"fixtures\":8192,\"state_comparisons\":"<<checks<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
