#include "terrain_region.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool b){if(!b)throw std::runtime_error("Fixed region contract");}
template<class F> static void rejects(F f){try{f();}catch(const std::exception&){return;}throw std::runtime_error("Expected rejection");}
int main()try{
 assets::MapAsset a;a.width=4;a.height=4;a.layers=1;a.metadata[0]=2;a.metadata[1]=2;a.cells.resize(16);for(unsigned i=0;i<16;++i)a.cells[i]={std::uint16_t(i),{2,3,4},0,8};
 assets::MapAsset b=a;b.layers=2;b.cells.resize(32);
 assets::RegionRecipe r;r.columns=4;r.rows=2;r.specific={{1,1,0,0},{2,1,2,0}};
 auto p=reconstruction::planFixedTerrainRegion(r,{&a,&b});require(p.side==2 && p.layers==2 && p.blocks.size()==8);
 require(p.blocks[2].column==3 && p.blocks[2].row==0 && p.blocks[2].sourceX==0 && p.blocks[2].sourceY==2);
 assets::TerrainCatalog t;t.records.resize(32);for(unsigned i=0;i<32;++i)t.records[i][0x90]=i;
 auto result=reconstruction::assembleFixedTerrainRegion(p,{&a,&b},t);require(result.map.width==8 && result.map.height==4 && result.projectedObjects==32);
 require(result.map.cells[32].definition==0 && result.map.cells[32].references==std::array<std::uint16_t,3>{65535,65535,65535});
 require(a.cells[0].flags10==8 && a.cells[0].references[0]==2);
 auto bad=r;bad.random={{3,1}};rejects([&]{reconstruction::planFixedTerrainRegion(bad,{&a,&b});});
 bad=r;bad.specific[0].rotation=-1;rejects([&]{reconstruction::planFixedTerrainRegion(bad,{&a,&b});});
 bad=r;bad.specific[1].column=0;rejects([&]{reconstruction::planFixedTerrainRegion(bad,{&a,&b});});
 bad=r;bad.columns=5;rejects([&]{reconstruction::planFixedTerrainRegion(bad,{&a,&b});});
 a.metadata[0]=3;rejects([&]{reconstruction::planFixedTerrainRegion(r,{&a,&b});});
 std::cout<<"Fixed placement, anchor wrapping, mixed height, projection and rejection pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
