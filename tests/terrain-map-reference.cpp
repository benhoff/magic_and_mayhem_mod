// Unchanged PE32 traversal + ordinary producer, no Win32 calls.
#include "terrain_submission.hpp"
#include "terrain_map.hpp"
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
static Bytes read(const char* path){
 std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)throw std::runtime_error("Cannot open input");
 const auto n=f.tellg();if(n<0 || n>32*1024*1024)throw std::runtime_error("Input extent");
 Bytes b(static_cast<std::size_t>(n));f.seekg(0);if(n && !f.read(reinterpret_cast<char*>(b.data()),n))throw std::runtime_error("Input read");return b;
}
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


static mnm::assets::TerrainCatalog catalog(const Bytes& b){
 if(b.size()<16 || (b.size()-16)%356)throw std::runtime_error("TTD extent");
 mnm::assets::TerrainCatalog t;t.records.resize((b.size()-16)/356);
 for(unsigned i=0;i<t.records.size();++i)std::memcpy(t.records[i].data(),b.data()+16+356*i,356);return t;
}
static mnm::assets::MapAsset decoded(const Bytes& b){
 if(b.size()<76)throw std::runtime_error("MAP extent");mnm::assets::MapAsset m;m.width=u32(b,4);m.height=u32(b,8);m.layers=u32(b,12);
 if(!m.width || !m.height || !m.layers || m.width>128 || m.height>128 || m.layers>32 || b.size()!=76+12*std::uint64_t(m.width)*m.height*m.layers)throw std::runtime_error("MAP bounds");
 m.cells.resize(m.width*m.height*m.layers);for(unsigned i=0;i<m.cells.size();++i){std::array<std::uint16_t,6> c;std::memcpy(c.data(),b.data()+76+12*i,12);m.cells[i]={c[0],{c[1],c[2],c[3]},c[4],c[5]};}return m;
}
static Bytes serialized(const mnm::assets::MapAsset& m){Bytes b(m.cells.size()*12);for(unsigned i=0;i<m.cells.size();++i){const auto& c=m.cells[i];const std::array<std::uint16_t,6> fields{{c.definition,c.references[0],c.references[1],c.references[2],c.flags8,c.flags10}};std::memcpy(b.data()+12*i,fields.data(),12);}return b;}
static unsigned compare(mnm::assets::MapAsset input,const mnm::assets::TerrainCatalog& t,const Bytes* expected=nullptr){
#ifndef MNM_NATIVE_ONLY
 auto bytes=serialized(input);Bytes map(0x66c0);put(map.data(),4,input.width);put(map.data(),8,input.height);put(map.data(),12,input.layers);put(map.data(),16,input.width*input.height);put(map.data(),0x4c,reinterpret_cast<std::uintptr_t>(bytes.data()));
 for(unsigned y=0;y<input.height;++y)put(map.data(),0x64b2+4*y,y*input.width);
 for(unsigned z=0;z<input.layers;++z)put(map.data(),0x6432+4*z,z*input.width*input.height);
 put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(t.records.data()));
 using Op=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Op>(0x4ecf20)(map.data());
 #endif
 mnm::reconstruction::deriveTerrainSurfaces(input,t);const auto native=serialized(input);
#ifdef MNM_NATIVE_ONLY
 const auto& bytes=native;
#endif
 if(expected && native!=*expected)throw std::runtime_error("Independent geometry bytes differ");
 if(native!=bytes){for(unsigned i=0;i<native.size();++i)if(native[i]!=bytes[i]){std::cerr<<"cell "<<i/12<<" byte "<<i%12<<" native "<<unsigned(native[i])<<" original "<<unsigned(bytes[i])<<'\n';break;}throw std::runtime_error("Surface pass mismatch");}
 return input.cells.size();
}
int main(int argc,char** argv)try{
 if(argc<2 || (argc-2)%4)throw std::runtime_error("Expected PE then TTD/raw/base/final fixtures");
#ifndef MNM_NATIVE_ONLY
 mapImage(read(argv[1]));
#endif
 unsigned syntheticCells=0,installedCells=0;
#ifndef MNM_NATIVE_ONLY
 std::mt19937 rng(0x4ecf20);
 for(unsigned n=0;n<2048;++n){mnm::assets::TerrainCatalog t;t.records.resize(16);for(auto& r:t.records){r[0xa8]=rng()%256;r[0x94]=rng()%32;}
  mnm::assets::MapAsset m;m.width=1+rng()%12;m.height=1+rng()%12;m.layers=1+rng()%8;m.cells.resize(m.width*m.height*m.layers);
  for(auto& c:m.cells){c.definition=rng()%16;for(auto& ref:c.references)ref=(rng()%5)?0xffff:rng()%100;c.flags8=rng()%65536;c.flags10=rng()%65536;}
  syntheticCells+=compare(m,t);
 }
#endif
 for(int i=2;i<argc;i+=4){auto t=catalog(read(argv[i]));auto m=decoded(read(argv[i+1]));const auto base=decoded(read(argv[i+2]));const auto expected=serialized(decoded(read(argv[i+3])));
  const auto geometry=mnm::reconstruction::prepareTerrainGeometry(m,t);if(serialized(geometry.map)!=expected)throw std::runtime_error("Prepared geometry differs from independent bytes");
  installedCells+=compare(base,t,&expected);
 }
 std::cout<<"{\"synthetic_cases\":"<<(syntheticCells?2048:0)<<",\"synthetic_cells\":"<<syntheticCells<<",\"installed_maps\":"<<(argc-2)/4<<",\"installed_cells\":"<<installedCells<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
