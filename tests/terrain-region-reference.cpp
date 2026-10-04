// Unchanged PE32 section-copy ordinary branch; no Win32 calls.
#include "terrain_submission.hpp"
#include "terrain_region.hpp"
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


static unsigned compare(unsigned columns,unsigned rows,unsigned width,unsigned height,unsigned rotation,unsigned x,unsigned y){
 std::vector<mnm::assets::MapAsset> maps;maps.reserve(25);mnm::assets::MapAsset first;first.width=width;first.height=height;first.layers=1;first.metadata[0]=width;first.metadata[1]=height;first.cells.resize(width*height);maps.push_back(first);
 mnm::assets::RegionRecipe recipe;recipe.columns=columns;recipe.rows=rows;recipe.specific.push_back({37,int(rotation),int(x),int(y)});
 std::vector<bool> occupied(columns*rows);for(unsigned sy=0;sy<height;++sy)for(unsigned sx=0;sx<width;++sx){int dx=sx,dy=sy;if(rotation==1){dx=-int(sy);dy=sx;}if(rotation==2){dx=-int(sx);dy=-int(sy);}if(rotation==3){dx=sy;dy=-int(sx);}occupied[((int(y)+dy+int(rows))%int(rows))*columns+(int(x)+dx+int(columns))%int(columns)]=true;}
 for(unsigned row=0;row<rows;++row)for(unsigned col=0;col<columns;++col)if(!occupied[row*columns+col]){auto m=first;m.width=m.height=1;m.metadata[0]=m.metadata[1]=1;m.cells.resize(1);maps.push_back(m);recipe.specific.push_back({unsigned(50+maps.size()),0,int(col),int(row)});}
 std::vector<const mnm::assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);const auto plan=mnm::reconstruction::planFixedTerrainRegion(recipe,sources);
#ifndef MNM_NATIVE_ONLY
 Bytes host(0x4000,0xcc);put(host.data(),4,columns);put(host.data(),8,rows);put(host.data(),12,10);
 using Init=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Init>(0x52d320)(host.data());reinterpret_cast<Init>(0x52d440)(host.data());
 unsigned entry=0;std::vector<unsigned> bases;
 for(unsigned i=0;i<maps.size();++i){bases.push_back(entry);const auto& m=maps[i];for(unsigned sy=0;sy<m.height;++sy)for(unsigned sx=0;sx<m.width;++sx){const unsigned at=0x2c+63*entry++;
  put(host.data(),at,recipe.specific[i].section);host[at+5]=m.width*m.height>1;put(host.data(),at+10,0);put(host.data(),at+22,sx);put(host.data(),at+26,sy);put(host.data(),at+30,m.width);put(host.data(),at+34,m.height);
  // Unique internal connector pairs; outer edges are below threshold.
  const std::array<unsigned,4> edges{{sy?12+sx:0,sx+1<m.width?10+sy:0,sy+1<m.height?12+sx:0,sx?10+sy:0}};
  for(unsigned d=0;d<4;++d)put(host.data(),at+42+4*d,edges[d]);
 }}
 using Place=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);
 for(unsigned i=0;i<maps.size();++i){const auto& s=recipe.specific[i];reinterpret_cast<Place>(0x531b20)(host.data(),bases[i],s.rotation,s.column,s.row);}
 for(const auto& b:plan.blocks){const auto* slot=host.data()+0x39b4+37*(b.column*5+b.row);const unsigned blockX=b.sourceX,blockY=b.sourceY;
  require(slot[0]==1);const std::array<unsigned,5> expected{{recipe.specific[b.source].section,bases[b.source]+blockY*maps[b.source].width+blockX,b.rotation,blockX,blockY}};
  for(unsigned i=0;i<expected.size();++i)require(u32(host,std::size_t(slot-host.data())+1+4*i)==expected[i]);
 }
 for(unsigned i=0;i<entry;++i)require(u32(host,0x36+63*i)==1);
#endif
 return plan.blocks.size();
}
static unsigned installed(const Bytes& fixture){
 const unsigned columns=u32(fixture,0),rows=u32(fixture,4),count=u32(fixture,8);require(fixture.size()==12+92*count);
 std::vector<mnm::assets::MapAsset> maps;maps.reserve(count);mnm::assets::RegionRecipe recipe;recipe.columns=columns;recipe.rows=rows;
 for(unsigned i=0;i<count;++i){const unsigned at=12+92*i;recipe.specific.push_back({u32(fixture,at),int(u32(fixture,at+4)),int(u32(fixture,at+8)),int(u32(fixture,at+12))});
  mnm::assets::MapAsset m;m.width=u32(fixture,at+20);m.height=u32(fixture,at+24);m.layers=u32(fixture,at+28);m.cells.resize(m.width*m.height*m.layers);for(unsigned n=0;n<13;++n)m.metadata[n]=u32(fixture,at+40+4*n);maps.push_back(std::move(m));
 }
 std::vector<const mnm::assets::MapAsset*> sources;for(const auto& m:maps)sources.push_back(&m);const auto plan=mnm::reconstruction::planFixedTerrainRegion(recipe,sources);
#ifndef MNM_NATIVE_ONLY
 Bytes host(0x4000,0);put(host.data(),4,columns);put(host.data(),8,rows);put(host.data(),12,1000);for(unsigned d=0;d<4;++d)put(host.data(),16+4*d,1000+d);
 using Init=void(__attribute__((thiscall)) *)(void*);reinterpret_cast<Init>(0x52d320)(host.data());reinterpret_cast<Init>(0x52d440)(host.data());
 using Describe=void(__attribute__((thiscall)) *)(void*,unsigned*,const void*,const void*,unsigned,unsigned,unsigned);
 std::vector<unsigned> bases;unsigned entries=0;
 for(unsigned i=0;i<count;++i){bases.push_back(entries);const unsigned at=12+92*i;for(unsigned y=0;y<maps[i].metadata[1];++y)for(unsigned x=0;x<maps[i].metadata[0];++x)reinterpret_cast<Describe>(0x52edf0)(host.data(),&entries,fixture.data()+at+16,fixture.data()+at,1,x,y);}
 require(u32(host,32)==plan.layers);
 using Load=void(__attribute__((thiscall)) *)(void*,unsigned);
 using Admit=unsigned(__attribute__((thiscall)) *)(void*,unsigned,unsigned);
 using Place=void(__attribute__((thiscall)) *)(void*,unsigned,unsigned,unsigned,unsigned);
 for(unsigned i=0;i<count;++i){const auto& placement=recipe.specific[i];reinterpret_cast<Init>(0x52d400)(host.data());reinterpret_cast<Load>(0x52f3a0)(host.data(),bases[i]);reinterpret_cast<Load>(0x531880)(host.data(),placement.rotation);
  if(!(reinterpret_cast<Admit>(0x5300a0)(host.data(),placement.column,placement.row)&255))throw std::runtime_error("Authored region requires original admission fallback");
  reinterpret_cast<Place>(0x531b20)(host.data(),bases[i],placement.rotation,placement.column,placement.row);
 }
 for(const auto& b:plan.blocks){const unsigned at=0x39b4+37*(b.column*5+b.row),x=b.sourceX/plan.side,y=b.sourceY/plan.side;require(host[at]==1);
  const std::array<unsigned,5> expected{{recipe.specific[b.source].section,bases[b.source]+y*maps[b.source].metadata[0]+x,b.rotation,x,y}};for(unsigned n=0;n<5;++n)require(u32(host,at+1+4*n)==expected[n]);
 }
#endif
 return plan.blocks.size();
}
int main(int argc,char** argv)try{
 if(argc<2)throw std::runtime_error("Expected pinned PE and optional authored fixtures");
#ifndef MNM_NATIVE_ONLY
 mapImage(read(argv[1]));
#endif
 unsigned cases=0,blocks=0,installedBlocks=0;
 for(unsigned c=2;c<=5;++c)for(unsigned r=2;r<=5;++r)for(unsigned w=1;w<=2;++w)for(unsigned h=1;h<=2;++h)for(unsigned rotation=0;rotation<4;++rotation)for(unsigned x=0;x<c;++x)for(unsigned y=0;y<r;++y){blocks+=compare(c,r,w,h,rotation,x,y);++cases;}
 for(int n=2;n<argc;++n)installedBlocks+=installed(read(argv[n]));
 std::cout<<"{\"cases\":"<<cases<<",\"blocks\":"<<blocks<<",\"installed_regions\":"<<argc-2<<",\"installed_blocks\":"<<installedBlocks<<",\"all_match\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
