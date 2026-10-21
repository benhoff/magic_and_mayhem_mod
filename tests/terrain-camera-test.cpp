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
 c.column=45;c.row=-5;c.f45=17;c.f49=23;clampTerrainCamera(c,40,28);
 check(c.column==45 && c.f45==0 && c.row==-5 && c.f49==23);
 c.column=44;c.row=-6;c.f45=17;c.f49=23;clampTerrainCamera(c,40,28);
 check(c.column==44 && c.f45==17 && c.row==-5 && c.f49==0);
 c.column=std::numeric_limits<int>::max();c.row=0;c.f45=19;c.f49=7;
 moveTerrainCameraCells(c,40,28,1,-1);check(c.column==-5 && c.f45==0 && c.row==-1 && c.f49==7);
 before=c;rejected=false;try{moveTerrainCameraCells(c,0,28,1,1);}catch(const std::invalid_argument&){rejected=true;}
 check(rejected && c.column==before.column && c.row==before.row && c.f45==before.f45);
 c=TerrainCamera{};setTerrainCameraCellPosition(c,40,28,9,-1,29,-1);
 check(c.column==std::uint32_t(0x7ffffff)%40 && c.row==1 && c.f41==8 && c.f4d==0);
 followTerrainCamera(c,40,28,9,TerrainCameraFocus{-1,-897,-1});
 check(c.column==39 && c.row==27 && c.f45==31 && c.f49==31 && c.f41==8 && c.f4d==15);
 before=c;followTerrainCamera(c,0,0,0,{});check(c.column==before.column && c.f45==before.f45);
 rejected=false;try{followTerrainCamera(c,0,28,9,TerrainCameraFocus{});}catch(const std::invalid_argument&){rejected=true;}
 check(rejected && c.column==before.column && c.row==before.row && c.f45==before.f45);
 std::cout<<"Camera setter boundaries passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
