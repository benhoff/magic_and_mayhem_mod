// Unchanged PE32 section-copy ordinary branch; no Win32 calls.
#include "terrain_submission.hpp"
#include "terrain_entity_sections.hpp"
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
static unsigned compare(const mnm::assets::MapAsset& src,const mnm::assets::TerrainCatalog& t,const mnm::assets::Animation& animation,unsigned rotation,unsigned side,unsigned sx,unsigned sy){
 mnm::assets::MapAsset dst;dst.width=side+3;dst.height=side+5;dst.layers=src.layers+1;dst.cells.resize(dst.width*dst.height*dst.layers);
 for(auto& c:dst.cells)c={0xeeee,{0x1234,0xabcd,0xffff},0xdead,0xbeef};
 auto bytes=serialized(dst);Bytes source(side*side*src.layers*12);unsigned i=0;
 for(unsigned z=0;z<src.layers;++z)for(unsigned y=0;y<side;++y)for(unsigned x=0;x<side;++x){const auto& c=src.cells[(z*src.height+sy+y)*src.width+sx+x];const std::array<std::uint16_t,6> fields{{c.definition,c.references[0],c.references[1],c.references[2],c.flags8,c.flags10}};std::memcpy(source.data()+12*i++,fields.data(),12);}
 const auto initial=source;const auto originalAsset=serialized(src);
#ifndef MNM_NATIVE_ONLY
 Bytes map(0x66c0);put(map.data(),4,dst.width);put(map.data(),8,dst.height);put(map.data(),12,dst.layers);put(map.data(),16,dst.width*dst.height);put(map.data(),0x4c,reinterpret_cast<std::uintptr_t>(bytes.data()));put(map.data(),0x50,side);
 for(unsigned y=0;y<dst.height;++y)put(map.data(),0x64b2+4*y,y*dst.width);
 for(unsigned z=0;z<dst.layers;++z)put(map.data(),0x6432+4*z,z*dst.width*dst.height);
 unsigned ttdHeader[]={0,0,0,unsigned(t.records.size())};unsigned aniHeader[]={0,0,0,0,0,unsigned(animation.starts.size())};put(reinterpret_cast<void*>(0x656608),0,reinterpret_cast<std::uintptr_t>(ttdHeader));put(reinterpret_cast<void*>(0x6c4518),0,reinterpret_cast<std::uintptr_t>(aniHeader));put(reinterpret_cast<void*>(0x6c4518),4,reinterpret_cast<std::uintptr_t>(animation.starts.data()));put(reinterpret_cast<void*>(0x6c4518),8,reinterpret_cast<std::uintptr_t>(animation.records.data()));
 put(reinterpret_cast<void*>(0x65660c),0,reinterpret_cast<std::uintptr_t>(t.records.data()));
 using Op=void(__attribute__((thiscall)) *)(void*,void*,unsigned,unsigned,unsigned,unsigned);
 reinterpret_cast<Op>(0x4ef6b0)(map.data(),source.data(),src.layers,1,2,rotation);
#endif
 mnm::reconstruction::copyTerrainEntitySection(dst,src,t,animation,side,sx,sy,1,2,rotation);
#ifndef MNM_NATIVE_ONLY
 if(serialized(dst)!=bytes)throw std::runtime_error("Original section destination differs");
 for(unsigned n=0;n<side*side*src.layers;++n){std::array<unsigned char,12> expected;std::memcpy(expected.data(),initial.data()+12*n,12); // Only the definition WORD may change in source.
  if(rotation){mnm::assets::MapCell before;std::memcpy(&before,initial.data()+12*n,12);if(before.flags10&8){auto selected=mnm::reconstruction::rotateTerrainLinkedDefinition(before,animation,t,rotation);require(selected.action!=mnm::reconstruction::TerrainLinkedRotationAction::reset);expected[0]=selected.definition&255;expected[1]=selected.definition>>8;}else {const unsigned at=0x94-4*rotation;expected[0]=t.records[before.definition][at];expected[1]=t.records[before.definition][at+1];}}
  if(std::memcmp(source.data()+12*n,expected.data(),12))throw std::runtime_error("Original source side effect differs");
 }
#endif
 if(serialized(src)!=originalAsset)throw std::runtime_error("Source ownership");
 return side*side*src.layers;
}
int main(int argc,char** argv)try{
 if(argc<2 || (argc-2)%2)throw std::runtime_error("Expected PE and catalog/MAP pairs");
#ifndef MNM_NATIVE_ONLY
 mapImage(read(argv[1]));
#endif
 std::mt19937 rng(0x4ef6b0);unsigned cases=0,cells=0;
 for(unsigned n=0;n<2048;++n){mnm::assets::TerrainCatalog t;t.records.resize(32);for(unsigned i=0;i<32;++i)for(unsigned rotation=1;rotation<4;++rotation)t.records[i][0x94-4*rotation]=std::min(i+rotation,31u);
  mnm::assets::Animation animation;for(unsigned i=0;i<32;++i){animation.starts.push_back(animation.records.size());animation.records.push_back({0,int(i),{}});animation.records.push_back({6,0,{}});}animation.starts.push_back(animation.records.size());
  mnm::assets::MapAsset m;m.width=1+rng()%12;m.height=1+rng()%12;m.layers=1+rng()%8;m.cells.resize(m.width*m.height*m.layers);
  for(auto& c:m.cells){c.definition=rng()%32;for(auto& ref:c.references)ref=rng()%65536;c.flags8=rng()%65536;c.flags10=rng()%65536;}
  const unsigned side=1+rng()%std::min(m.width,m.height),sx=rng()%(m.width-side+1),sy=rng()%(m.height-side+1);
  for(unsigned rotation=0;rotation<4;++rotation){cells+=compare(m,t,animation,rotation,side,sx,sy);++cases;}
 }
 std::cout<<"{\"synthetic_cases\":"<<cases<<",\"synthetic_cells\":"<<cells<<",\"success\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
