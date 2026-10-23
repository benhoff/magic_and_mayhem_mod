#include "../renderer/canvas_sequence.hpp"
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
int main()try{
 constexpr int width=800,height=600,stride=803;
 const Image base{width,height,std::vector<std::uint32_t>(width*height,0x2468)};
 double genericMs=0,tracedMs=0;unsigned cohorts=0;std::size_t calls=0;
 for(unsigned orientation=0;orientation<4;++orientation)for(unsigned kind=0;kind<4;++kind){
  CanvasSequence generic,traced;generic.create(1,width,height);traced.create(1,width,height);
  generic.update(1,0,0,base);traced.update(1,0,0,base);
  MinimapOverlayView view{80,60,40,30,80,60,400,150,orientation,false};
  MinimapTerrain terrain{80,60,40,30,400,150,orientation,{},{}};
  for(unsigned i=0;i<80*60;++i){terrain.colours.push_back(i&1?0x1357:0xeca8);terrain.hidden.push_back(0);}
  const auto draw=[&](MinimapPlane& p)->std::size_t{
   if(kind==0){drawMinimapTerrain(p,terrain);return 0;}
   if(kind==1){std::array<std::uint16_t,9> palette{};palette[0]=0x1357;return drawMinimapCellMarker(p,view,{3,2,0,true},palette,true);}
   if(kind==2)return drawMinimapCreatureMarkers(p,view,{{3,2,false},{0,0,true},{3,2,false}},true).drawn;
   MinimapCameraOutline outline{800,600,{{{-1,-1},{1,-1},{-1,1},{1,1}}},true};
   const auto r=drawMinimapCameraOutline(p,view,outline);return r.originX+r.originY*stride;
  };
  generic.minimap(1,stride,draw);traced.minimap(1,stride,draw,true);
  for(unsigned turn=0;turn<4;++turn)for(unsigned order=0;order<2;++order){
   const bool receipt=(turn+order)%2;auto& canvas=receipt?traced:generic;
   const auto start=std::chrono::steady_clock::now();
   for(unsigned i=0;i<8;++i)canvas.minimap(1,stride,draw,receipt);
   const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
   if(receipt)tracedMs+=elapsed;else genericMs+=elapsed;calls+=8;
  }
  if(generic.read(1).pixels!=traced.read(1).pixels)throw std::runtime_error("Measured minimap receipt path differs from complete independent scans");
  ++cohorts;
 }
 if(!std::isfinite(genericMs)||!std::isfinite(tracedMs)||genericMs<=0||tracedMs<=0)throw std::runtime_error("Invalid measured CPU duration");
 std::cout<<"{\"success\":true,\"cohorts\":"<<cohorts<<",\"calls\":"<<calls<<",\"compared_words\":"<<cohorts*std::size_t(width)*height<<",\"generic_ms\":"<<genericMs<<",\"traced_ms\":"<<tracedMs<<",\"alternating_measurement_order\":true,\"performance_threshold_claimed\":false,\"original_pixels_used\":false}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
