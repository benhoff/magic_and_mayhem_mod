#include "terrain_traversal.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
int main(){
 TerrainCamera c;c.span=4;c.diagonal=1;c.column=0;c.row=4;c.f41=2;c.f15=64;c.cutLevel=3;c.wrapColumnPriority=100;c.wrapRowPriority=200;
 const auto flags=[](unsigned,unsigned,unsigned){return std::array<std::uint16_t,2>{0,0};};
 auto visits=traverseTerrain(4,5,3,c,{},flags);assert(visits.size()==48);
 assert(visits[0].column==3 && visits[0].row==3 && visits[0].layer==0 && visits[0].priority==0);
 assert(visits[1].column==0 && visits[1].priority==100 && visits[1].anchorX==288 && visits[1].anchorY==80);
 assert(visits[8].row==0 && visits[8].priority==200 && visits[8].anchorX==192);
 assert(visits[16].layer==1 && visits[16].anchorY==48);
 c.cutLevel=1;assert(traverseTerrain(4,5,3,c,{},flags).size()==32);
 c.mode=1;assert(traverseTerrain(4,5,3,c,{},flags).size()==48);
 c.specialLayer=1;const auto gated=[](unsigned,unsigned,unsigned){return std::array<std::uint16_t,2>{0x80,0};};
 assert(traverseTerrain(4,5,3,c,{},gated).size()==16);
 assert(traverseTerrain(4,5,3,c,{},[](unsigned,unsigned,unsigned){return std::array<std::uint16_t,2>{0,0x4000};}).empty());
 unsigned rejected=0;const auto fails=[&](auto run){try{run();}catch(const std::exception&){++rejected;return;}assert(false);};
 auto bad=c;bad.view=1;fails([&]{traverseTerrain(4,5,3,bad,{},flags);});
 bad=c;bad.span=5;fails([&]{traverseTerrain(4,5,3,bad,{},flags);});
 bad=c;bad.cutLevel=0;fails([&]{traverseTerrain(4,5,3,bad,{},flags);});
 bad=c;bad.f11=0x7fffffff;bad.f51=100;fails([&]{traverseTerrain(4,5,3,bad,{},flags);});
 fails([&]{traverseTerrain(0,5,3,c,{},flags);});fails([&]{traverseTerrain(4,5,3,c,{0,0,0,256},flags);});fails([&]{traverseTerrain(4,5,3,c,{},{});});
 std::cout<<"Terrain traversal wraps, layer/flag gates and "<<rejected<<" rejected domains\n";
}
