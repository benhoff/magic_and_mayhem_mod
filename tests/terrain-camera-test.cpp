#include "terrain_camera.hpp"
#include <iostream>
#include <stdexcept>
#include <limits>
using namespace mnm::reconstruction;
void check(bool b){if(!b)throw std::runtime_error("Camera test mismatch");}
int main()try{
 TerrainCamera c;c.column=7;c.f45=19;c.mode=1;bindTerrainCamera(c,40,28,9);
 check(c.span==28 && c.diagonal==14 && c.cutLevel==9 && c.column==7 && c.f45==19 && c.mode==1);
 setTerrainCameraViewport(c,{-1,-3,512,256});check(c.f51==513 && c.f55==259);
 setTerrainCameraPosition(c,40,28,9,40*32+31,28*32+15,999);check(c.column==0 && c.row==0 && c.f45==31 && c.f49==15 && c.f41==8 && c.f4d==7);
 for(unsigned view=0;view<4;++view){c.view=view;setTerrainCameraPosition(c,40,28,9,0,0,0);scrollTerrainCamera(c,40,28,-32,0);
  const int expectedX[]={39,39,1,1},expectedY[]={1,27,27,1};check(c.column==expectedX[view] && c.row==expectedY[view] && c.f45==0 && c.f49==0);
 }
 bool rejected=false;try{bindTerrainCamera(c,0,40,9);}catch(const std::invalid_argument&){rejected=true;}check(rejected);
 c.view=0;auto before=c;rejected=false;try{scrollTerrainCamera(c,40,28,std::numeric_limits<int>::max(),std::numeric_limits<int>::max());}catch(const std::overflow_error&){rejected=true;}check(rejected && c.column==before.column && c.f45==before.f45);
 std::cout<<"Camera setter boundaries passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
