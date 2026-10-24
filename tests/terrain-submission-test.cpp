#include "terrain_submission.hpp"
#include <cstring>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Terrain submission boundary");}
int main()try{
 TerrainDefinition d;d.body.fill(1);d.first.fill(2);d.second.fill(3);
 TerrainAdmission a;
 for(unsigned view=0;view<4;++view)for(const auto level:{0,1,31,32,499,0x07ffffff}){
  a.view=view;TerrainTile t;t.column=std::numeric_limits<int>::max();t.row=std::numeric_limits<int>::min();t.level=level;
  const auto draws=submitTerrain(d,t,a,3);
  require(draws.size()==3 && t.flags8==0);
  for(unsigned i=0;i<3;++i){
   const auto& q=draws[i];
   require(q.frame==i+1 && q.role==i && q.kind==33 && std::uint32_t(q.depth.x)==0xfffffff0u &&
           std::uint32_t(q.depth.y)==16 && std::uint32_t(q.depth.height)==std::uint32_t(level)*16);
  }
 }
 for(const auto level:{-1,0x08000000,std::numeric_limits<int>::min(),std::numeric_limits<int>::max()}){
  TerrainTile invalid;invalid.level=level;invalid.flags8=0x1000;
  bool refused=false;try{submitTerrain(d,invalid,a,3);}catch(const std::invalid_argument&){refused=true;}
  require(refused && invalid.flags8==0x1000);
 }
 for(const auto pair:{std::pair<int,int>{-32767,32767},{-32768,-32768},{-65535,-1},{-65536,0},{std::numeric_limits<int>::min(),-127}}){
  a.lightBias=pair.first;TerrainTile tile;
  for(const auto& draw:submitTerrain(d,tile,a,3))require(draw.shade==pair.second);
 }
 a={};TerrainTile t;auto draws=submitTerrain(d,t,a,1);
 require(draws.size()==3 && draws[0].frame==1 && draws[1].frame==0 && draws[2].frame==0);
 d.first.fill(0);draws=submitTerrain(d,t,a,3);require(draws.size()==1 && t.flags8==0x8004);
 t={};t.flags10=8;d.first.fill(2);d.second.fill(0);draws=submitTerrain(d,t,a,3);
 require(draws.size()==2 && draws[0].kind==31 && draws[1].kind==33 && t.flags8==0x8000);
 d.body.fill(0);t={};draws=submitTerrain(d,t,a,3);require(draws.empty() && t.flags8==0);
 for(unsigned invalid=0;invalid<2;++invalid){
  a={};a.view=invalid?0:4;a.player=invalid?4:1;t={};t.flags8=0x1234;t.flags10=0x5678;
  bool refused=false;try{submitTerrain(d,t,a,3);}catch(const std::invalid_argument&){refused=true;}
  require(refused && t.flags8==0x1234 && t.flags10==0x5678);
 }
 std::cout<<"Terrain coordinate wrap, roles, frame admission and refusals passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
