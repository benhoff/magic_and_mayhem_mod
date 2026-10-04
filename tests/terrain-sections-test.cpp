#include "terrain_sections.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool v){if(!v)throw std::runtime_error("Section contract");}
template<class F> void rejects(F f){try{f();}catch(const std::exception&){return;}throw std::runtime_error("Expected section rejection");}
int main()try{
 assets::MapAsset src;src.width=3;src.height=4;src.layers=2;src.cells.resize(24);
 assets::TerrainCatalog t;t.records.resize(32);
 for(unsigned i=0;i<32;++i)for(unsigned r=1;r<4;++r)t.records[i][0x94-4*r]=i+r;
 for(unsigned i=0;i<24;++i)src.cells[i]={std::uint16_t(i),{1,2,3},0x4401,0x8000};
 const auto original=src.cells;
 auto grid=reconstruction::assembleTerrainRegion(2,2,2,2,{{&src,1,1,0,0,0},{&src,1,1,1,0,1},{&src,1,1,0,1,2},{&src,1,1,1,1,3}},t);
 require(grid.width==4 && grid.height==4 && grid.layers==2);
 for(unsigned r=0;r<4;++r)for(unsigned z=0;z<2;++z)for(unsigned y=0;y<2;++y)for(unsigned x=0;x<2;++x){
  unsigned tx=x,ty=y;if(r==1){tx=1-y;ty=x;}if(r==2){tx=1-x;ty=1-y;}if(r==3){tx=y;ty=1-x;}
  const auto& c=grid.cells[z*16+(2*(r/2)+ty)*4+2*(r%2)+tx];
  require(c.definition==src.cells[z*12+(y+1)*3+x+1].definition+r && c.references==std::array<std::uint16_t,3>{1,2,3} && c.flags10==0x8000);
  require(c.flags8==std::uint16_t(0x4001|(0x400<<r)));
 }
 require(src.cells[4].definition==original[4].definition);
 const auto before=grid.cells;
 src.cells[19].flags10|=8;
 rejects([&]{reconstruction::copyTerrainSection(grid,src,t,2,1,1,0,0,1);});
 require(grid.cells[0].definition==before[0].definition && grid.cells[31].definition==before[31].definition);
 rejects([&]{reconstruction::copyTerrainSection(grid,src,t,2,2,1,0,0,0);});
 rejects([&]{reconstruction::copyTerrainSection(grid,src,t,2,1,1,0,0,4);});
 rejects([&]{reconstruction::assembleTerrainRegion(2,1,2,2,{{&src,0,0,0,0,0},{&src,0,0,0,0,0}},t);});
 rejects([&]{reconstruction::assembleTerrainRegion(1,1,0,2,{{&src,0,0,0,0,0}},t);});
 // Rotation zero never performs definition lookup, even for object-linked cells.
 src.cells[0].definition=65535;src.cells[0].flags10=8;
 reconstruction::copyTerrainSection(grid,src,t,1,0,0,0,0,0);require(grid.cells[0].definition==65535 && grid.cells[0].flags10==8);
 std::cout<<"Section coverage, rotations, crops, ownership and atomic rejection pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
